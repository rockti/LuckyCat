#pragma once
#include "BaseThread.h"
#include "DBConnection.h"
#include "Mutex.h"
#include "CacheList.h"

enum TMDBActionType
{
	TA_INSERT,
	TA_UPDATE,
	TA_DELETE,
	TA_COMMIT,
};
struct TMDBAction
{
	CMutex* pAccomplishLock;
	int* pAccomplishRef;
	int nTaskNo;
	char const* pszObjectName;
	CFieldDescribe* pDescirbe;
	void* pData;
	TMDBActionType nActionType;
};

const int MAX_TASK_COMMAND_LEN	= 4000;
enum TTaskMethodType
{
	TMT_TABLE,
	TMT_SP
};
struct TTaskMethod
{
	int nWorkerID;
	CFieldDescribe* pDescirbe;
	TTaskMethodType nMethodType;
	char const* pszObjectName;
	char const* pszKeyName;
	char szInsertCommand[MAX_TASK_COMMAND_LEN];
	char szUpdateCommand[MAX_TASK_COMMAND_LEN];
	char szDeleteCommand[MAX_TASK_COMMAND_LEN];
	int keyPosition[10];
};

struct ltstr;
class IDBSyncChecker
{
public:
	virtual bool CheckTaskSync(int nTaskNo) = 0;

protected:
	virtual ~IDBSyncChecker()	{};
};

class CMDBWorker : public CThreadBase
{
public:
	CMDBWorker(int nWorkerID, IDBSyncChecker* pSyncChecker);
	virtual ~CMDBWorker();

	bool ConnectDB(char const* DBName, char const* UserName, char const* Password)	{ return m_pDbConnection->ConnectDB(DBName, UserName, Password); }

	void Reset();
	void Stop();
	void EnableDbKeepAlive(int nTimeout)	{ m_nDbKeepAliveTimeout = nTimeout; }

	void Init(TTaskMethod* pTaskMethod);
	void AssignTask(CMutex* pAccomplisLock, int* pAccomplishRef, int nTaskNo, char const* pszObjectName, CFieldDescribe* pDescirbe, void const* pData, TMDBActionType nActionType);

private:
	virtual void Run();
	void Work(); 

	int DbOperation(char const* objectName, void* pData, CFieldDescribe* pDescirbe, TMDBActionType nAction, int nTaskNo);
	int DbSpOperation(CFieldDescribe* pDescirbe, void* pData, char const* pszKeyName, TMDBActionType nActionType);
	int DbTableOperation(CFieldDescribe* pDescirbe, void* pData, char const* pszSQLCommand, int* pKeyPosition, TMDBActionType nActionType);

	//根据表名找到表
	TTaskMethod* FindTaskMethod(const char *pszObjectName);
	//根据SQL语句找到SACommand
	SACommand* GetSACommand(char const* pszSQL);

	//绑定参数
	void BindParam(TMemberDesc* pMemberDesc, void* pData, int nParamPosition, SACommand* pCommand);
	void BindParam(TMemberDesc* pMemberDesc, void* pData, char const* pszParamName, SACommand* pSACommand);

	void CommitActions();
	void RollbackActions();
	void DbKeepAlive();
	bool CheckDbConnection(int nErrorNativeCode);

private:
	int m_nWorkerID;
	IDBSyncChecker* m_pSyncChecker;
	bool m_bShouldRun;

	int m_nActionCount;
	int m_nConnectTimerCount;
	int m_nDbKeepAliveTimeout;
	time_t m_tLastDbActionTime;

	CDbConnection* m_pDbConnection;
	SACommand* m_pSACommand;

	CCacheList m_taskDataList;

	typedef std::deque<TMDBAction> CTaskQueue;
	CTaskQueue m_taskQueue;

	typedef std::map<char const*, TTaskMethod*, ltstr> CTaskMethodMap;
	CTaskMethodMap m_taskMethodMap;		//操作表Map

	typedef std::map<char const*, SACommand*> CSACommandCache;
	CSACommandCache m_SACommandCache;	//SQL语句缓存

	std::vector<TMDBAction> m_actionUncommitted;	//本地保存的任务缓存，尚未commit

	CMutex m_taskMutex;

};