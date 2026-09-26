#include "stdafx.h"
/*/
#include "DBLoadThread.h"
#include "ZtpDataStruct.h"

//每日初始化表
#define DEF_DAY_TABLE_INFO(name, condition, paraNumber)		\
	{"t_Sync_"#name, &C##name##Field::m_Describe, condition, TID_Init##name, paraNumber, false}

#define DEF_OPER_DAY_TABLE_INFO(name, condition, paraNumber)		\
	{"t_Oper_"#name, &C##name##Field::m_Describe, condition, TID_Init##name, paraNumber, false}

#define DEF_DAY_TABLE_END { NULL, NULL, NULL, 0, 0, true}

//这里顺序不能随意改变，请注意
TInitTableInfo g_DayTableInfo[] = 
{
	DEF_DAY_TABLE_INFO(SeatInfo          , "",	1),
	DEF_DAY_TABLE_INFO(Instrument		 , "",	1),
	DEF_DAY_TABLE_INFO(OperatorInfo      , "",	1),
	DEF_DAY_TABLE_INFO(InvestorInfo		 , "",	1),
	DEF_DAY_TABLE_INFO(OperatorInvestor  , "",	1),
	DEF_DAY_TABLE_INFO(InvestorAccount   , "",	1),
	DEF_DAY_TABLE_INFO(InvestorMargin    , "",	1),	
	DEF_DAY_TABLE_INFO(InvestorFee       , "",	1),
	DEF_DAY_TABLE_INFO(InvestorPosition	 , "",	1),
	{"t_oper_balancechange",&CBalanceChangeField::m_Describe,"",TID_InitInvestorBalanceChange,1, false},
	{"t_oper_positionchange",&CPositionChangeField::m_Describe,"",TID_InitInvestorPositionChange,1, false},
	DEF_OPER_DAY_TABLE_INFO(Order		 , "",	1),
 	DEF_OPER_DAY_TABLE_INFO(Trade		 , "",	1),
	DEF_DAY_TABLE_END			//一定要有这一行，表示结束
};

CInitDataLoader::CInitDataLoader(CEventHandler* pLoadHandler)
:m_pLoderHandler(pLoadHandler),m_pInitLoadThread(new CInitDataLoadThread(this))
,m_tableCount(0),m_errorCode(0),m_pCurrTask(NULL),m_pCurrTableInfo(NULL)
,m_startPos(0)
{
}
CInitDataLoader::~CInitDataLoader()
{
	delete m_pInitLoadThread;
}

TInitTableInfo* CInitDataLoader::GetTinitTableInfo()
{
	TInitTableInfo* pCurrTask = NULL;
	m_lock.Lock();
	if(NULL!=m_pCurrTask && NULL!=m_pCurrTask->pszTable)
	{
		pCurrTask = m_pCurrTask;
		m_pCurrTask ++;
	}
	m_lock.UnLock();
	return pCurrTask;
}
void CInitDataLoader::OnDataLoaded(CInitDataLoadThread* pTinitDataLoadThread, char const* pszTableName, int nErrorCode, char const* pszErrorMsg)
{
	m_lock.Lock();
	SetError(pszTableName, nErrorCode, pszErrorMsg);
	(0==nErrorCode) ? AppendData(pTinitDataLoadThread) : Cancel();

	if (--m_tableCount <= 0)
	{
		m_pLoderHandler->PostEvent(UM_INIT_APPEND_END, 0, NULL);
	}
	m_lock.UnLock();
}

bool CInitDataLoader::ConnectDB(char const* databaseName, char const* userName, char const* password)
{
	return m_pInitLoadThread->ConnectDB(databaseName, userName, password);
}
bool CInitDataLoader::StartLoadData(CDateType& WorkDay)
{
	StartWorkThread(g_DayTableInfo);
	return true;
}

void CInitDataLoader::StartWorkThread(TInitTableInfo* pTableInfo)
{
	m_pCurrTask = pTableInfo;
	m_pCurrTableInfo = pTableInfo;
	m_tableCount = 0;
	for(TInitTableInfo* pInfo=pTableInfo; NULL!=pInfo->pszTable; ++pInfo)
	{
		pInfo->bCanceled = false;
		++m_tableCount;
	}
	m_pInitLoadThread->Create();
}
void CInitDataLoader::SetError(char const* pszTableName, int nErrorCode, char const* pszErrorMsg)
{
	m_errorCode = nErrorCode;
	m_strErrorMsg = pszTableName;
	m_strErrorMsg += " : ";
	m_strErrorMsg += pszErrorMsg;
	if (NULL != m_pLoderHandler)
	{
		m_pLoderHandler->SendEvent(UM_INIT_RESULT, (DWORD)nErrorCode, (void*)m_strErrorMsg.c_str());
	}
}

void CInitDataLoader::AppendData(CInitDataLoadThread* pDataLoadThread)
{
	while (m_startPos < pDataLoadThread->GetDataFlow()->GetCount())
	{
		m_startPos += m_pLoderHandler->SendEvent(UM_INIT_APPEND_DATA, m_startPos, pDataLoadThread->GetDataFlow());
	}
}
void CInitDataLoader::Cancel()
{
	if (NULL == m_pCurrTableInfo)
		return;

	for(TInitTableInfo* pTableInfo=m_pCurrTableInfo; NULL!=pTableInfo->pszTable; ++pTableInfo)
	{
		pTableInfo->bCanceled = true;
	}
}

int const RECORD_MAX_SIZE =	5000;
CInitDataLoadThread::CInitDataLoadThread(IInitDataLoadAssigner* pAssigner)
:m_pAssigner(pAssigner),m_pDbConnection(new CDbConnection)
{
	m_saCommand.setConnection(m_pDbConnection);
	m_saCommand.setOption("PreFetchRows") = "1000";

	m_pLoadPackage = CZTPPackage::CreatePackage(ZTP_HEAD_LEN);
	m_pDataFlow = new CCachedFlow(false, 100000, 10000);
}
CInitDataLoadThread::~CInitDataLoadThread()
{
	delete m_pLoadPackage;
	delete m_pDataFlow;
	delete m_pDbConnection;
}
void CInitDataLoadThread::Run()
{
	for(auto& tableInfo : g_DayTableInfo)
	{
		if (tableInfo.bCanceled)
		{
//			m_pAssigner->OnDataLoaded(this, tableInfo.pszTable, -10, "Canceled");
			continue;
		}
		DataLoading(&tableInfo);
	}
}
void CInitDataLoadThread::DataLoading(TInitTableInfo* pTableInfo)
{
	try 
	{
		char szSQL[256] = "SELECT * FROM ";
		int nTableNameOffset = strlen(szSQL);
		int tableConditionOffset = strlen(pTableInfo->pszTable);
		strcpy(szSQL+nTableNameOffset, pTableInfo->pszTable);	//select * from ***table
		if(0 != pTableInfo->nParaNumber)
		{
			strcpy(szSQL+nTableNameOffset+tableConditionOffset, pTableInfo->pszSQL);	//select * from ***table ...
			m_saCommand.setCommandText(szSQL);
		}
		else
		{
			m_saCommand.setCommandText(szSQL);
		}
	
		//执行SQL命令
		m_saCommand.Execute();

		//将结果放入流
		if(!LoadTable(pTableInfo))
		{
			m_pAssigner->OnDataLoaded(this, pTableInfo->pszTable, -1, "LoadTable failed");
		}
		else
		{
			m_pAssigner->OnDataLoaded(this, pTableInfo->pszTable, 0, "LoadTable success");
		}
	}	
	catch(SAException& x)
	{
		try
		{
			m_pDbConnection->CheckConntectionError(x.ErrNativeCode());
			m_pDbConnection->Rollback();
		}
		catch(SAException &)
		{
		}
		m_pAssigner->OnDataLoaded(this, pTableInfo->pszTable, -2, (const char*)x.ErrText());
	}
}
bool CInitDataLoadThread::LoadTable(TInitTableInfo* pTableInfo)
{
	char* buf = new char[RECORD_MAX_SIZE];
	FlushPackage(pTableInfo->tid);
	while(!pTableInfo->bCanceled)
	{
		if (!m_saCommand.FetchNext())
			break;

		if (CDbConnection::GetRow(&m_saCommand, buf, pTableInfo->pFieldDescribe))
		{
			while(m_pLoadPackage->AddField(pTableInfo->pFieldDescribe,buf) < 0)
			{
				FlushPackage(pTableInfo->tid);
			}
		}
	}
	FlushPackage(pTableInfo->tid);
	delete buf;	

	return true;
}
void CInitDataLoadThread::FlushPackage(DWORD TID)
{
	if (m_pLoadPackage->GetLength() > 0)
	{
		m_pLoadPackage->SetSubjectID(0);
		m_pLoadPackage->MakePackage();
		m_pDataFlow->Append(m_pLoadPackage->GetAddress(), m_pLoadPackage->GetLength());
	}
	m_pLoadPackage->PreparePublish(TID);
}

*/