#pragma once

#include "Mutex.h"
#include "Semaphore.h"
#include "EventHandler.h"

struct CSyncEvent
{
	CEvent event;
	CSemaphore sem;
	int nRetval;
	CSyncEvent* pNext;
};

//消息队列
class  CEventQueue  
{
public:
	CEventQueue(int nQueueSize);
	virtual ~CEventQueue();

	bool IsEmpty() { return (NULL==m_pFirstSyncEvent) && (m_nReadPos==m_nWritePos); }

	bool AddPostEvent(CEventHandler* pEventHandler, int nEventID, DWORD dwParam, void* pParam);
	bool AddSyncEvent(CSyncEvent* pEvent);
	bool PeekEvent(CEvent& event);
	void ClearEvent(CEventHandler* pEventHandler);
	void RemoveAll();

protected:
	CEvent* m_pPostEvent;
	CSyncEvent* m_pFirstSyncEvent;
	CSyncEvent* m_pLastSyncEvent;

	int m_nEventSize;
	int m_nReadPos;
	int m_nWritePos;
	CMutex m_lock;
};

