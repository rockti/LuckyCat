#include "stdafx.h"
#include "DBScheduler.h"
#include "DBWorkThread.h"
#include "CSVParser.h"
#include "ZtpDataStruct.h"
#include <fmt/format.h>

#define TASK_METHOD_DEFINE(WorkID, field, MethodType, ObjectName, KeyName)		\
{ WorkID, &field::m_Describe, MethodType, ObjectName, KeyName, {0}, {0}, {0}, {0}}

#define TASK_METHOD_END	{ -1, NULL, TMT_TABLE, "", "", {0}, {0}, {0}, {0}}

//WorkerID为0表示同步工作线程
TTaskMethod g_TaskMethod[] =
{
// 	TASK_METHOD_DEFINE(0, CSystemStatusField, TMT_TABLE, "t_Sync_SystemStatus", "SystemStatus"),	
// 	TASK_METHOD_DEFINE(5, CSeatInfoField, TMT_TABLE, "t_Sync_SeatInfo", "ExchangeID,ParticipantID,SeatID"),
// 	TASK_METHOD_DEFINE(5, CInstrumentField, TMT_TABLE,"t_Sync_Instrument", "ExchangeID,InstrumentID"),
// 	TASK_METHOD_DEFINE(1, CInvestorAccountField, TMT_TABLE, "t_Sync_InvestorAccount", "BrokerID,InvestorID"),
// 	TASK_METHOD_DEFINE(5, CInvestorMarginField, TMT_TABLE,"t_Sync_InvestorMargin", "BrokerID,ExchangeID,InvestorID,InstrumentID"),
// 	TASK_METHOD_DEFINE(5, CInvestorFeeField, TMT_TABLE,"t_Sync_InvestorFee", "BrokerID,ExchangeID,InvestorID,InstrumentID"),
// 	TASK_METHOD_DEFINE(2, CInvestorPositionField, TMT_TABLE, "t_Sync_InvestorPosition", "BrokerID,InvestorID,TradingDay,InstrumentID,Direction"),
	TASK_METHOD_DEFINE(3, COrderField, TMT_TABLE, "t_Oper_Order", "OrderLocalID"),			//成功委托
	TASK_METHOD_DEFINE(4, CTradeField, TMT_TABLE, "t_Oper_Trade", "ExchangeID,OrderSysID,TradeID"),			

//	TASK_METHOD_DEFINE(5, CUserSessionField, TMT_TABLE,"t_Oper_UserSession", "BrokerID,OrderUserID,SessionID"),
	TASK_METHOD_END
};

const int DBCONNECTION_ALIVE_TIMEOUT = 60;

CDBScheduler::CDBScheduler(CReactorThread* pReactorThread, IFlow* pAccomplishFlow)
:CEventHandler(pReactorThread),m_pAccomplishFlow(pAccomplishFlow)
{
	RefreshCurrentTaskNo();
	SetTimer(1, 1000);
}
CDBScheduler::~CDBScheduler()
{
	CWorkerMap::iterator iter = m_workerMap.begin();
	while (iter != m_workerMap.end())
	{
		delete (*iter).second;
		iter++;
	}
}

bool CDBScheduler::Init(char const* DatabaseName, char const* UserName, char const* Password)
{
	TTaskMethod* pTaskMethod = g_TaskMethod;
	while (pTaskMethod->nWorkerID >= 0)
	{
		//预先准备好各个表的SQL语句
		PrepareTaskMethod(pTaskMethod);

		//创建各表对应的线程
		CMDBWorker* pWorker = GetWorkerFromWorkerMap(pTaskMethod->nWorkerID);
		if (NULL == pWorker)
		{
			//nWorkerID为0表示同步工作线程
			pWorker = new CMDBWorker(pTaskMethod->nWorkerID, (0==pTaskMethod->nWorkerID)?this:NULL);
			m_workerMap[pTaskMethod->nWorkerID] = pWorker;
			if (!pWorker->ConnectDB(DatabaseName, UserName, Password))
			{
				return false;
			}
			pWorker->EnableDbKeepAlive(DBCONNECTION_ALIVE_TIMEOUT);
		}
		m_dispatchMap[pTaskMethod->pszObjectName] = pWorker;
		++pTaskMethod;
 		Log::Debug(fmt::format("CDBScheduler: MDBWorker[{}:{}] ConnectDB success", pTaskMethod->nWorkerID, pTaskMethod->pszObjectName));
	}

	//启动对应的线程
	CWorkerMap::iterator iter = m_workerMap.begin();
	while (iter != m_workerMap.end()) {
		(*iter).second->Init(g_TaskMethod);
		(*iter).second->Create();
		iter++;
	}
	return true;
}
void CDBScheduler::AssignTask(DWORD nTaskNo)
{
// 	if ((int)nTaskNo <= m_pAccomplishFlow->GetCount())
// 		return;		//数据库已处理过，不再重做

	MDBTask task;
	task.nTaskNo = nTaskNo;
	task.nActionRef = 0;
	m_taskQueue.push_back(task);
	RefreshCurrentTaskNo();
}
void CDBScheduler::Dispatch(char const* ObjectName, CFieldDescribe* pDescirbe, void const* pData, TMDBActionType nActionType)
{
	if (m_taskQueue.empty())
		return;

	CMDBWorker* pWorker = GetWorkerFromDispatchMap(ObjectName);
	if (NULL == pWorker)
	{
		Log::Fatal(fmt::format("CDBScheduler: TMDB can not find worker for[{}]", pDescirbe->GetFieldName()));
		return;
	}

	MDBTask& task = m_taskQueue.back();
	m_accomplishCountMutex.Lock();
	++task.nActionRef;		//操作引用计数记为1
	m_accomplishCountMutex.UnLock();

	pWorker->AssignTask(&m_accomplishCountMutex, &(task.nActionRef), task.nTaskNo, ObjectName, pDescirbe, pData, nActionType);
}
void CDBScheduler::SetCommitPoint(DWORD nTaskNo)
{
	for (CWorkerMap::iterator iter=m_workerMap.begin(); iter != m_workerMap.end(); iter++)
	{
		(*iter).second->AssignTask(NULL, NULL, nTaskNo, "", NULL, NULL, TA_COMMIT);
	}
}
void CDBScheduler::SetAccomplishCommPhaseNo(WORD wCommPhaseNo)
{
	m_pAccomplishFlow->SetCommPhaseNo(wCommPhaseNo);
	CWorkerMap::iterator iter = m_workerMap.begin();
	while (iter != m_workerMap.end())
	{
		(*iter).second->Reset();
		iter++;
	}
}

CMDBWorker* CDBScheduler::GetWorkerFromWorkerMap(int nWorkerID)
{
	CWorkerMap::iterator iter = m_workerMap.find(nWorkerID);
	if (iter == m_workerMap.end()) 	{
		return NULL;
	}
	return (*iter).second;
}
CMDBWorker* CDBScheduler::GetWorkerFromDispatchMap(char const* ObjectName)
{
	CDispatchMap::iterator iter = m_dispatchMap.find(ObjectName);
	if (iter == m_dispatchMap.end())
	{
		return NULL;
	}
	return (*iter).second;
}

void CDBScheduler::OnTimer(int nIDEvent)
{
	int nCompleteTaskNo = 0;
	while (!m_taskQueue.empty())
	{
		//nActionCount不为0说明还未Commit
		if (0 != m_taskQueue.front().nActionRef)
			break;

		//nActionCount为0说明已经操作完，从队列中移除
		nCompleteTaskNo = m_taskQueue.front().nTaskNo;
		m_taskQueue.pop_front();
	}

	if (0 != nCompleteTaskNo)
	{
		m_pAccomplishFlow->Truncate(nCompleteTaskNo);
	}

	RefreshCurrentTaskNo();
}
bool CDBScheduler::CheckTaskSync(int nTaskNo)
{
	//m_nCurrTaskNo会被多个线程同时访问，调度线程只写，其它工作线程只读
	return (m_nCurrTaskNo==nTaskNo) ? true : false;
}
void CDBScheduler::RefreshCurrentTaskNo()
{
	m_nCurrTaskNo = m_taskQueue.empty() ? -1 : m_taskQueue.front().nTaskNo;
}
void CDBScheduler::PrepareTaskMethod(TTaskMethod* pTaskMethod)
{
	if (TMT_SP == pTaskMethod->nMethodType)
		return;

	int i = 0;	
	int* pKeyPosition = pTaskMethod->keyPosition;

	//prepare key position
	CCSVParser csvParser(pTaskMethod->pszKeyName);
	do
	{
		char* pToken = csvParser.GetNextToken();
		if (NULL != pToken)
		{
			for (i=0; i<pTaskMethod->pDescirbe->GetMemberCount(); ++i)
			{
				if(0 == strcmp(pToken, pTaskMethod->pDescirbe->GetMemberDesc(i)->szName))
				{
					*pKeyPosition = i+1;
					++pKeyPosition;
					break;
				}
			}
			if (i == pTaskMethod->pDescirbe->GetMemberCount())
			{
				Log::Fatal(FT("CDBScheduler: PrepareTaskMethod can not find key name in table[{}]", pTaskMethod->pszObjectName));
			}
		}
	} while(CPE_HAS_NEXT==csvParser.GetErrorCode());
	*pKeyPosition = 0;		//结束符

	char strSQL[MAX_TASK_COMMAND_LEN];
	//prepare insert statement
	strcpy(strSQL, "INSERT INTO ");
	strcat(strSQL, pTaskMethod->pszObjectName);
	strcat(strSQL, " (");
	for (i=0; i<pTaskMethod->pDescirbe->GetMemberCount(); ++i)
	{
		if (0 != i)
		{
			strcat(strSQL, ", ");
		}
		strcat(strSQL, pTaskMethod->pDescirbe->GetMemberDesc(i)->szName);
	}
	strcat(strSQL, ") VALUES (");
	for (i=0; i<pTaskMethod->pDescirbe->GetMemberCount(); ++i)
	{
		if (0 != i)
		{
			strcat(strSQL, ", ");
		}
		sprintf(strSQL+strlen(strSQL), ":%d", i+1);
	}
	strcat(strSQL, ")");
	strcpy(pTaskMethod->szInsertCommand, strSQL);

	//prepare update statement
	strcpy(strSQL, "UPDATE ");
	strcat(strSQL, pTaskMethod->pszObjectName);
	strcat(strSQL, " SET ");
	for (i=0; i<pTaskMethod->pDescirbe->GetMemberCount(); ++i)
	{
		if (0 != i)
		{
			strcat(strSQL, ", ");
		}
		strcat(strSQL, pTaskMethod->pDescirbe->GetMemberDesc(i)->szName);
		sprintf(strSQL+strlen(strSQL), " = :%d", i+1);
	}
	strcat(strSQL, " WHERE ");

	i = 0;
	pKeyPosition = pTaskMethod->keyPosition;
	while (*pKeyPosition > 0)
	{
		if (i != 0)
		{
			strcat(strSQL, " AND ");
		}
		int conditionCount = i + pTaskMethod->pDescirbe->GetMemberCount() + 1;
		sprintf(strSQL+strlen(strSQL), "%s=:%d", 
			pTaskMethod->pDescirbe->GetMemberDesc(*pKeyPosition-1)->szName,
			conditionCount);
		i++;
		pKeyPosition++;
	}
	strcpy(pTaskMethod->szUpdateCommand, strSQL);

	//prepare delete statement
	strcpy(strSQL, "DELETE FROM ");
	strcat(strSQL, pTaskMethod->pszObjectName);
	strcat(strSQL, " WHERE ");

	i = 0;
	pKeyPosition = pTaskMethod->keyPosition;
	while (*pKeyPosition > 0)
	{
		if (0 != i)
		{
			strcat(strSQL, " AND ");
		}
		sprintf(strSQL+strlen(strSQL), "%s=:%d", pTaskMethod->pDescirbe->GetMemberDesc(*pKeyPosition-1)->szName, i+1);
		i++;
		pKeyPosition++;
	}
	strcpy(pTaskMethod->szDeleteCommand, strSQL);
}

