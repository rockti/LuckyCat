#include "stdafx.h"
#include "MarketScheduler.h"
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
// 	TASK_METHOD_DEFINE(1, CMarketDataField, TMT_TABLE, "t_MarketData", "TradingDay,InstrumentID,SequenceNo"),	
	TASK_METHOD_END
};


const int DBCONNECTION_ALIVE_TIMEOUT = 60;

CMarketScheduler::CMarketScheduler(CReactorThread* pReactorThread)
:CEventHandler(pReactorThread)/*只是为了用线程的Timer*/
{
	RefreshCurrentTaskNo();
	SetTimer(1, 1000);
}
bool CMarketScheduler::Init(char const* DatabaseName, char const* UserName, char const* Password)
{
	TTaskMethod* taskMethod = g_TaskMethod;
	PrepareTaskMethod(taskMethod);

	m_pWorkThread = new CMDBWorker(taskMethod->nWorkerID, NULL);
	if (!m_pWorkThread->ConnectDB(DatabaseName, UserName, Password))
	{
		return false;
	}
	m_pWorkThread->EnableDbKeepAlive(DBCONNECTION_ALIVE_TIMEOUT);
	m_pWorkThread->Init(taskMethod);
	m_pWorkThread->Create();
	Log::Debug(fmt::format("CMarketScheduler: DBWorker[{}:{}] ConnectDB success", taskMethod->nWorkerID, taskMethod->pszObjectName));

	return true;
}
void CMarketScheduler::AssignTask(DWORD nTaskNo)
{
	MDBTask task;
	task.nTaskNo = nTaskNo;
	task.nActionRef = 0;
	m_taskQueue.push_back(task);
	RefreshCurrentTaskNo();
}
void CMarketScheduler::SetCommitPoint(DWORD nTaskNo)
{
	m_pWorkThread->AssignTask(NULL, NULL, nTaskNo, "", NULL, NULL, TA_COMMIT);
}
void CMarketScheduler::Dispatch(char const* ObjectName, CFieldDescribe* pDescirbe, void const* pData, TMDBActionType nActionType)
{
	if (m_taskQueue.empty())
		return;

	MDBTask& task = m_taskQueue.back();
	m_mutex.Lock();
	++task.nActionRef;		//操作引用计数记为1
	m_mutex.UnLock();

	m_pWorkThread->AssignTask(&m_mutex, &(task.nActionRef), task.nTaskNo, ObjectName, pDescirbe, pData, nActionType);
}

void CMarketScheduler::OnTimer(int nIDEvent)
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

	RefreshCurrentTaskNo();
}
bool CMarketScheduler::CheckTaskSync(int nTaskNo)
{
	return (m_nCurrTaskNo==nTaskNo) ? true : false;
}
void CMarketScheduler::RefreshCurrentTaskNo()
{
	m_nCurrTaskNo = m_taskQueue.empty() ? -1 : m_taskQueue.front().nTaskNo;
}

void CMarketScheduler::PrepareTaskMethod(TTaskMethod* pTaskMethod)
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
				Log::Fatal(fmt::format("CMarketScheduler: PrepareTaskMethod can not find key name in table[{}]", pTaskMethod->pszObjectName));
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
		sprintf(strSQL+strlen(strSQL), "%s=:%d", pTaskMethod->pDescirbe->GetMemberDesc(*pKeyPosition-1)->szName, i+pTaskMethod->pDescirbe->GetMemberCount()+1);
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
