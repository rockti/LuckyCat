#include "stdafx.h"
#include "DBWorkThread.h"
#include "CSVParser.h"
#include <fmt/format.h>

//每TMDB_CONNECT_TIMER_COUNT×TMDB_COMMIT_INTERVAL毫秒尝试重连一次数据库
const int TMDB_CONNECT_TIMER_COUNT = 50;
//每隔TMDB_COMMIT_INTERVAL毫秒Commit一次
const int TMDB_COMMIT_INTERVAL = 100;
//每TMDB_COMMIT_VOLUME个数据库操作Commit一次
const int TMDB_COMMIT_VOLUME = 1000;

CMDBWorker::CMDBWorker(int nWorkerID, IDBSyncChecker* pSyncChecker)
:m_nWorkerID(nWorkerID),m_pSyncChecker(pSyncChecker),m_bShouldRun(true)
,m_nActionCount(0),m_nConnectTimerCount(0),m_nDbKeepAliveTimeout(0),m_tLastDbActionTime(time(NULL))
{
	m_pDbConnection = new CDbConnection;
	m_pSACommand = new SACommand(m_pDbConnection);
	Reset();
}
CMDBWorker::~CMDBWorker()
{
	for (CSACommandCache::iterator iter=m_SACommandCache.begin(); iter!= m_SACommandCache.end(); ++iter)
	{
		delete (*iter).second;
	}
	m_SACommandCache.clear();
	delete m_pSACommand;
	delete m_pDbConnection;
}
void CMDBWorker::Reset()
{
	m_taskMutex.Lock();
	m_taskQueue.clear();
	m_taskMutex.UnLock();
}
void CMDBWorker::Stop()
{
	m_bShouldRun = false;
	Join();
}
void CMDBWorker::Run()
{
	while (m_bShouldRun)
	{
		Sleep(TMDB_COMMIT_INTERVAL);
		Work();
		DbKeepAlive();
	}
}

void CMDBWorker::Init(TTaskMethod* pTaskMethod)
{
	while (pTaskMethod->nWorkerID >= 0) {
		m_taskMethodMap[pTaskMethod->pszObjectName] = pTaskMethod;
		pTaskMethod++;
	}
}
void CMDBWorker::AssignTask(CMutex* pAccomplisLock, int* pAccomplishRef, int nTaskNo, char const* pszObjectName, CFieldDescribe* pDescirbe, void const* pData, TMDBActionType nActionType)
{
	TMDBAction tmdbAction;
	tmdbAction.pAccomplishLock = pAccomplisLock;
	tmdbAction.pAccomplishRef = pAccomplishRef;
	tmdbAction.nTaskNo = nTaskNo;
	tmdbAction.nActionType = nActionType;
	tmdbAction.pszObjectName = pszObjectName;
	tmdbAction.pDescirbe = pDescirbe;

	m_taskMutex.Lock();
	if (TA_COMMIT!=nActionType || m_nActionCount>0)
	{
		if (TA_COMMIT == nActionType)
		{
			m_nActionCount = 0;
		}
		else
		{
			//要把内存数据copy保存下来，pData指针会失效
			tmdbAction.pData = m_taskDataList.PushBack((void*)pData, pDescirbe->GetStructSize());
			m_nActionCount++;
		}
		m_taskQueue.push_back(tmdbAction);
	}
	m_taskMutex.UnLock();
}

void CMDBWorker::Work()
{
	if (!m_pDbConnection->isConnected())
	{
		++m_nConnectTimerCount;
		if(m_nConnectTimerCount > TMDB_CONNECT_TIMER_COUNT)
		{
			m_nConnectTimerCount = 0;
			m_pDbConnection->ConnectDB();
		}
		if (!m_pDbConnection->isConnected())
		{
			if (0 == m_nConnectTimerCount)
			{
				Log::Fatal(fmt::format("CMDBWorker: worker[{}] connect to db error", m_nWorkerID));
			}
			return;
		}
	}

	CommitActions();	//先Commit一次

	while (m_taskQueue.size() > 0)
	{
		m_taskMutex.Lock();

		//先将任务从队列中取到本地然后将其从队列中去除，防止任务队列过长时间被锁
		TMDBAction mdbAction = m_taskQueue.front();
		{
			if (NULL != m_pSyncChecker)			//本线程需要同步执行工作
			{
				if (!m_pSyncChecker->CheckTaskSync(mdbAction.nTaskNo))
				{			
					m_taskMutex.UnLock();
					break;
				}
			}

			m_taskQueue.pop_front();
			m_taskMutex.UnLock();
			m_actionUncommitted.push_back(mdbAction);
		}

		if (TA_COMMIT != mdbAction.nActionType)
		{
			m_taskDataList.PopFront(mdbAction.pDescirbe->GetStructSize());
			int rtn = DbOperation(mdbAction.pszObjectName, mdbAction.pData, mdbAction.pDescirbe, mdbAction.nActionType, mdbAction.nTaskNo);
			if (rtn<0 && !m_pDbConnection->isConnected())
				break;
		}

		if (m_actionUncommitted.size()>TMDB_COMMIT_VOLUME)
		{
			CommitActions();	//每TMDB_COMMIT_VOLUME数量Commit一次，不足的等下次Work时候Commit
		}
		m_tLastDbActionTime = time(NULL);
	}
}

int const Duplicate_Entry = 1062;	//重复记录的异常不再处理
int CMDBWorker::DbOperation(char const* objectName, void* pData, CFieldDescribe* pDescirbe, TMDBActionType nActionType, int nTaskNo)
{
	TTaskMethod* pTaskMethod = FindTaskMethod(objectName);
	if (NULL == pTaskMethod)
		return -1;

	int ret = 0;
	for (int nRetryTimes=2; nRetryTimes>0; --nRetryTimes)		//重做2次
	{
		try 
		{
			switch(pTaskMethod->nMethodType) 
			{
			case TMT_TABLE:
				switch(nActionType)
				{
				case TA_INSERT:
					ret = DbTableOperation(pDescirbe, pData, pTaskMethod->szInsertCommand, pTaskMethod->keyPosition, nActionType);
					break;
				case TA_UPDATE:
					ret = DbTableOperation(pDescirbe, pData, pTaskMethod->szUpdateCommand, pTaskMethod->keyPosition, nActionType);
					break;
				case TA_DELETE:
					ret = DbTableOperation(pDescirbe, pData, pTaskMethod->szDeleteCommand, pTaskMethod->keyPosition, nActionType);
					break;
				default:
					Log::Fatal(fmt::format("CMDBWorker: Invalid MDB ActionType[{}], MDBWorker[{}:{}]", nActionType, pTaskMethod->nWorkerID, objectName));
				}
				nRetryTimes = 0;					//成功了不需要重做
				break;
			case TMT_SP:
				ret = DbSpOperation(pDescirbe, pData, pTaskMethod->pszKeyName, nActionType);
				nRetryTimes = 0;					//成功了不需要重做
				break;
			default:
				Log::Fatal(fmt::format("CMDBWorker: Invalid MDB methodType[{}], MDBWorker[{}:{}]", pTaskMethod->nMethodType, pTaskMethod->nWorkerID, objectName));
			}
		}	
		catch(SAException &x)
		{
			if (Duplicate_Entry == x.ErrNativeCode())
			{
				ret = 0;
				nRetryTimes = 0;
				break;
			}

			Log::Fatal(fmt::format("CMDBWorker: Database Operation failed, TaskNo[{}] MDBWorker[{}:{}] ActionType[{}] Message[{}]", nTaskNo, pTaskMethod->nWorkerID,objectName, nActionType, (const char*)x.ErrText()));
			ret = -2;
			try
			{
				if (!CheckDbConnection(x.ErrNativeCode()))
				{
					RollbackActions();
					nRetryTimes = 0;
				}
			}
			catch(SAException &)
			{
			}
		}
	}

	if(0 != ret)
	{
// 		Log::Fatal(fmt::format("CMDBWorker: Database Operation failed, TaskNo[{}] MDBWorker[{}:{}] ActionType[{}]", nTaskNo, pTaskMethod->nWorkerID, objectName, nActionType));
	}
	else
	{
// 		Log::Debug(fmt::format("CMDBWorker: Database Operation success, TaskNo[{}] MDBWorker[{}:{}] ActionType[{}]", nTaskNo, pTaskMethod->nWorkerID, objectName, nActionType));
	}
	return ret;
}
int CMDBWorker::DbTableOperation(CFieldDescribe* pDescirbe, void* pData, char const* pszSQLCommand, int* pKeyPosition, TMDBActionType nActionType)
{
	SACommand* pSACommand = GetSACommand(pszSQLCommand);
	assert(NULL != pSACommand);

	int nParamPosition = 1;
	if (TA_INSERT==nActionType || TA_UPDATE==nActionType)
	{
		for (int i=0; i<pDescirbe->GetMemberCount(); ++i)
		{
			BindParam(pDescirbe->GetMemberDesc(i), pData, nParamPosition, pSACommand);
			nParamPosition++;
		}
	}

	if (TA_DELETE==nActionType || TA_UPDATE==nActionType)
	{
		int* pCurrPosition = pKeyPosition;
		while (*pKeyPosition > 0)
		{
			BindParam(pDescirbe->GetMemberDesc(*pKeyPosition-1), pData, nParamPosition, pSACommand);
			pKeyPosition++;
			nParamPosition++;
		}
	}
	pSACommand->Execute();

	return 0;
}
int CMDBWorker::DbSpOperation(CFieldDescribe* pDescirbe, void* pData, char const* pszKeyName, TMDBActionType nActionType)
{
	m_pSACommand->setCommandText(pszKeyName);
	m_pSACommand->Param("i_ActionType").setAsLong() = nActionType;

	char szTemp[1000] = "i_";
	for (int i=0; i<pDescirbe->GetMemberCount(); i++)
	{
		strcpy(szTemp+2, pDescirbe->GetMemberDesc(i)->szName);
		BindParam(pDescirbe->GetMemberDesc(i), pData, szTemp, m_pSACommand);
	}

	m_pSACommand->Execute();
	m_pDbConnection->Commit();
	return m_pSACommand->Param("o_ReturnNo").asLong();
}
TTaskMethod* CMDBWorker::FindTaskMethod(char const* pszObjectName)
{
	CTaskMethodMap::iterator iter = m_taskMethodMap.find(pszObjectName);
	if (iter == m_taskMethodMap.end()) {
		return NULL;
	}
	return (*iter).second;
}
SACommand* CMDBWorker::GetSACommand(char const* pszSQL)
{
	SACommand* pSACommand = NULL;
	CSACommandCache::iterator iter = m_SACommandCache.find(pszSQL);
	if (iter == m_SACommandCache.end())
	{
		pSACommand = new SACommand(m_pDbConnection);
		pSACommand->setCommandText(pszSQL, SA_CmdSQLStmt);
		m_SACommandCache[pszSQL] = pSACommand;
	}
	else
	{
		pSACommand = (*iter).second;
	}

	return pSACommand;
}
void CMDBWorker::BindParam(TMemberDesc* pMemberDesc, void* pData, int nParamPosition, SACommand* pSACommand)
{
	char strTemp[2] = {'\0', '\0'};
	char* pMemberData = (char*)pData+pMemberDesc->nStructOffset;
	switch(pMemberDesc->nType) 
	{
	case FT_WORD:						//short
		pSACommand->Param(nParamPosition).setAsShort() = *((short*)pMemberData);
		break;
	case FT_LONG:
	case FT_DWORD:						//long
		pSACommand->Param(nParamPosition).setAsLong() = *((int*)pMemberData);
		break;
	case FT_REAL8:						//double
		if (*((REAL8 *)pMemberData) == DOUBLE_NaN)
		{
			pSACommand->Param(nParamPosition).setAsNull();
		}
		else
		{
			pSACommand->Param(nParamPosition).setAsDouble() = *((REAL8*)pMemberData);
		}
		break;
	case FT_BYTE:
		if (1 == pMemberDesc->nSize)
		{
			strTemp[0] = *pMemberData;
			pSACommand->Param(nParamPosition).setAsString() = strTemp;
		}
		else
		{
			pSACommand->Param(nParamPosition).setAsString() = pMemberData;
		}
		break;
	}
}
void CMDBWorker::BindParam(TMemberDesc* pMemberDesc, void* pData, char const* pszParamName, SACommand* pSACommand)
{
	char strTemp[2] = {'\0', '\0'};
	char* pMemberData = (char*)pData+pMemberDesc->nStructOffset;
	switch(pMemberDesc->nType)
	{
	case FT_WORD:						//short
		pSACommand->Param(pszParamName).setAsShort() = *((short*)pMemberData);
		break;
	case FT_LONG:
	case FT_DWORD:						//long
		pSACommand->Param(pszParamName).setAsLong() = *((int*)pMemberData);
		break;
	case FT_REAL8:						//double
		if (*((REAL8 *)pMemberData) == DOUBLE_NaN)
		{
			pSACommand->Param(pszParamName).setAsNull();
		}
		else
		{
			pSACommand->Param(pszParamName).setAsDouble() = *((REAL8*)pMemberData);
		}
		break;
	case FT_BYTE:
		if (1 == pMemberDesc->nSize)
		{
			strTemp[0] = *pMemberData;
			pSACommand->Param(pszParamName).setAsString() = strTemp;
		}
		else
		{
			pSACommand->Param(pszParamName).setAsString() = pMemberData;
		}
		break;
	}
}

void CMDBWorker::CommitActions()
{
	if (m_actionUncommitted.size() <= 0)
		return;

	if (TA_COMMIT != m_actionUncommitted.back().nActionType)
		return;

	try
	{
		m_pDbConnection->Commit();
	}	
	catch(SAException &x)
	{
		try
		{
			CheckDbConnection(x.ErrNativeCode());
			RollbackActions();
		}
		catch(SAException &)
		{
		}
	}

	for (vector<TMDBAction>::iterator iter=m_actionUncommitted.begin(); iter!=m_actionUncommitted.end(); ++iter)
	{
		if (NULL != iter->pAccomplishRef)
		{
			//使调度器知道该工作已经做完
			iter->pAccomplishLock->Lock();
			(*(iter->pAccomplishRef))--;
			iter->pAccomplishLock->UnLock();	
		}
	}
	m_actionUncommitted.clear();
}
void CMDBWorker::RollbackActions()
{
	try
	{
		m_pDbConnection->Rollback();
	}
	catch(SAException &x)
	{
		Log::Fatal(fmt::format("CMDBWorker: Database operation rollback error, ExceptionMsg[{}]", (char const*)x.ErrText()));
	}
	m_taskMutex.Lock();
	for (vector<TMDBAction>::reverse_iterator i=m_actionUncommitted.rbegin(); i!=m_actionUncommitted.rend(); ++i)
	{
		m_taskQueue.push_front(*i);
	}	
	m_actionUncommitted.clear();
	m_taskMutex.UnLock();
}

void CMDBWorker::DbKeepAlive()
{
	time_t tNow = time(NULL);
	if (m_nDbKeepAliveTimeout>0 && tNow-m_tLastDbActionTime>m_nDbKeepAliveTimeout)
	{
		m_pDbConnection->KeepAlive();
		m_tLastDbActionTime = time(NULL);
	}
}
bool CMDBWorker::CheckDbConnection(int nErrorNativeCode)
{
	m_pDbConnection->CheckConntectionError(nErrorNativeCode);
	if (!m_pDbConnection->isConnected())
	{
		m_nConnectTimerCount = 0;
		return false;
	}
	return true;
}
