#pragma once

#include "EventHandler.h"
#include "DBWorkThread.h"
#include "DBScheduler.h"

class CMarketScheduler : public CEventHandler, public IDBSyncChecker
{
public:
	CMarketScheduler(CReactorThread* pReactorThread);
	virtual ~CMarketScheduler()	{}

	bool Init(char const* DatabaseName, char const* UserName, char const* Password);

	void AssignTask(DWORD nTaskNo);
	void SetCommitPoint(DWORD nTaskNo);
	void Dispatch(char const* ObjectName, CFieldDescribe* pDescirbe, void const* pData, TMDBActionType nActionType);

	virtual void OnTimer(int nIDEvent);
	virtual bool CheckTaskSync(int nTaskNo);

private:
	void RefreshCurrentTaskNo();
	void PrepareTaskMethod(TTaskMethod* pTaskMethod);		//预先把要操作表的语句都准备好

private:
	int m_nCurrTaskNo;
	CMDBWorker* m_pWorkThread;

	typedef std::deque<MDBTask> CTaskQueue;
	CTaskQueue m_taskQueue;

	CMutex m_mutex;
};
