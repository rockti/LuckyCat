#pragma once
#include "IFlow.h"
#include "EventHandler.h"
#include "DBWorkThread.h"

struct MDBTask
{
	int nTaskNo;
	int nActionRef;
};

class CDBScheduler : public CEventHandler, public IDBSyncChecker
{
public:
	CDBScheduler(CReactorThread* pReactorThread, IFlow* pAccomplishFlow);
	virtual ~CDBScheduler();

	bool Init(char const* DatabaseName, char const* UserName, char const* Password);
	void AssignTask(DWORD nTaskNo);
	void Dispatch(char const* ObjectName, CFieldDescribe* pDescirbe, void const* pData, TMDBActionType nActionType);
	void SetCommitPoint(DWORD nTaskNo);
	void SetAccomplishCommPhaseNo(WORD wCommPhaseNo);

	virtual void OnTimer(int nIDEvent);
	virtual bool CheckTaskSync(int nTaskNo);

	CMDBWorker* GetWorkerFromWorkerMap(int nWorkerID);

private:
	CMDBWorker* GetWorkerFromDispatchMap(char const* ObjectName);

	void RefreshCurrentTaskNo();
	void PrepareTaskMethod(TTaskMethod* pTaskMethod);		//预先把要操作表的语句都准备好

private:
	IFlow* m_pAccomplishFlow;
	int m_nCurrTaskNo;

	typedef std::map<int, CMDBWorker*> CWorkerMap;
	CWorkerMap m_workerMap;		//线程列表

	typedef std::deque<MDBTask> CTaskQueue;
	CTaskQueue m_taskQueue;

	typedef std::map<char const*, CMDBWorker*, ltstr> CDispatchMap;
	CDispatchMap m_dispatchMap;

	CMutex m_accomplishCountMutex;
};
