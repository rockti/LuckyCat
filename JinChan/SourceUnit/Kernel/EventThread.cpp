#include "stdafx.h"
#include "EventThread.h"
#include "SessionFactory.h"
#include "DBLoadThread.h"


const int SM_DISPATCHER_STOP = 0x0001;
CEventThread::CEventThread()
:m_nCurrTime(0),m_nCurrMilTime(0),m_nCurrClockTick(0),m_bShouldRun(false)
,m_queueEvent(2048),m_timerHandler(m_nCurrClockTick) {
	SyncTime();
}
CEventThread::~CEventThread() {
}

void CEventThread::Run() {
	m_bShouldRun = true;
	while(m_bShouldRun) {
		DispatchIOs();		//分派IO事件(IO调度线程处理)
		SyncTime();			//同步时间
		DispatchEvents();	//分派信号事件(Event调度线程处理)
	}
}
int CEventThread::HandleEvent(int nEventID, DWORD dwParam, void* pParam) {
	if (nEventID==SM_DISPATCHER_STOP && IsCurrentThread()) {
		m_lockActive.Lock();
		m_bShouldRun = false;
		m_lockActive.UnLock();
	}
	return 0;
}

void CEventThread::RegisterTimer(CEventHandler* pEventHandler, int nIDEvent, int nElapse) {
	m_timerHandler.RemoveTimer(pEventHandler, nIDEvent);
	m_timerHandler.RegisterTimer(pEventHandler, nIDEvent, nElapse);
}
void CEventThread::RemoveTimer(CEventHandler* pEventHandler, int nIDEvent) {
	m_timerHandler.RemoveTimer(pEventHandler, nIDEvent);
}

bool CEventThread::PostEvent(CEventHandler* pEventHandler, int message, DWORD dwParam, void* pParam) {
	return m_queueEvent.AddPostEvent(pEventHandler, message, dwParam, pParam);
}
int CEventThread::SendEvent(CEventHandler* pEventHandler, int nEventID, DWORD dwParam, void* pParam) {
	m_lockActive.Lock();
	if(IsCurrentThread() || !m_bShouldRun) {	//自己线程的消息，自己直接处理	
		int nRtn = (NULL!=pEventHandler) ? pEventHandler->HandleEvent(nEventID, dwParam, pParam) : HandleEvent(nEventID, dwParam, pParam);
		m_lockActive.UnLock();
		return nRtn;
	}

	CSyncEvent syncEvent;
	syncEvent.event.pEventHandler = pEventHandler;
	syncEvent.event.nEventID = nEventID;
	syncEvent.event.dwParam = dwParam;
	syncEvent.event.pParam = pParam;
	syncEvent.pNext = NULL;
	syncEvent.sem.Lock();					//将信号量减为0
	m_queueEvent.AddSyncEvent(&syncEvent);
	m_lockActive.UnLock();

	syncEvent.sem.Lock();					//等待另一线程将消息处理完后释放信号量
	return syncEvent.nRetval;
}
void CEventThread::ClearEvent(CEventHandler* pEventHandler) {
	m_queueEvent.ClearEvent(pEventHandler);
}
void CEventThread::Stop(DWORD nStopCode) {
	//向本线程发出退出信号
	SendEvent(NULL, SM_DISPATCHER_STOP, 0, NULL);
}

//同步当前系统时间，检查Timer
inline void CEventThread::SyncTime() {
	struct _timeb timebuffer;
	_ftime(&timebuffer);
	m_nCurrTime = (DWORD)timebuffer.time;
	m_nCurrMilTime = timebuffer.millitm;
	m_nCurrClockTick = m_nCurrTime * 1000 + timebuffer.millitm;
	m_timerHandler.Expire(m_nCurrClockTick);
}

void CEventThread::DispatchEvents() {
	CEvent event;
	while(m_queueEvent.PeekEvent(event)) {
		int nRtn = 0;
		if (NULL != event.pEventHandler) {
			nRtn = event.pEventHandler->HandleEvent(event.nEventID, event.dwParam, event.pParam);
		}
		else {
			//处理本线程的退出消息
			nRtn = HandleEvent(event.nEventID, event.dwParam, event.pParam);
		}

		if(NULL != event.pAdd) {		//同步消息
			CSyncEvent* pSyncEvent = (CSyncEvent*)event.pAdd;
			pSyncEvent->nRetval = nRtn;
			pSyncEvent->sem.UnLock();
		}
	}
}
// 
// string CEventThread::GetEventDesc(int nEventID)
// {
// 	string desc = "";
// 	switch (nEventID) {
// 	case UM_INIT_RESULT:
// 		desc = FT("初始化结果返回-0x%x", nEventID);
// 		break;
// 	case UM_INIT_APPEND_DATA:
// 		desc = FT("初始化数据发送-0x%x", nEventID);
// 		break;
// 	case UM_INIT_APPEND_END:
// 		desc = FT("初始化完成-0x%x", nEventID);
// 		break;
// 	case UM_LISTEN_RESULT:
// 		desc = FT("监听结果-0x%x", nEventID);
// 		break;
// 	case UM_CONNECT_RESULT:
// 		desc = FT("连接结果-0x%x", nEventID);
// 		break;
// 	case UM_TRY_CONNECT:
// 		desc = FT("尝试连接-0x%x", nEventID);
// 		break;
// 	case UM_CONNECT_LOOP:
// 		desc = FT("重复连接-0x%x", nEventID);
// 		break;
// 	default:
// 		desc = FT("未知-0x%x", nEventID);
// 	}
// 	return desc;
// }
