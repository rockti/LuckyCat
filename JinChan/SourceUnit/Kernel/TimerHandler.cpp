#include "stdafx.h"
#include "TimerHandler.h"
#include "EventHandler.h"

const DWORD MAX_TIMER_HEAP_TIMEOUT = 3600*24*1000;	//定时器最大不能起过一天
CTimerHandler::CTimerHandler(DWORD nClock)
:m_nClock(0),m_nClockAlter(nClock)
{
}

CTimerHandler::~CTimerHandler()
{
}

void CTimerHandler::RegisterTimer(CEventHandler* pEventHandler, int nIDEvent, int nElapse)
{
	CTimerHeapNode node;
	node.nElapse = nElapse;
	node.nIDEvent = nIDEvent;
	node.pTimer = pEventHandler;
	node.nExpire = m_nClock + nElapse;
	push(node);
}

void CTimerHandler::RemoveTimer(CEventHandler* pEventHandler, int nIDEvent)
{
	for (size_t i=0; i<size(); i++)
	{
		CTimerHeapNode &node = c[i];
		if (node.pTimer==pEventHandler && (0==nIDEvent || node.nIDEvent==nIDEvent))
		{
			node.pTimer = NULL;
		}
	}
}

bool CTimerHandler::CheckExpire(DWORD nClock)
{
	if (size() > 0)
	{
		CTimerHeapNode node = top();
		return (node.nExpire <= m_nClock) ? true : false;
	}
	return false;
}

void CTimerHandler::Expire(DWORD nClock)
{
	SyncTime(nClock);
	while (size() > 0)
	{
		CTimerHeapNode node = top();
		if (node.nExpire > m_nClock)
		{
			break;	//最近的定时器尚未到期，不需要做更多的检查了
		}

		pop();

		if (node.pTimer != NULL)
		{
			node.nExpire = m_nClock + node.nElapse;
			push(node);
			node.pTimer->OnTimer(node.nIDEvent);
		}		
	}
}

void CTimerHandler::SyncTime(DWORD nClock)
{
	m_nClock = nClock - m_nClockAlter;
	if (m_nClock > MAX_TIMER_HEAP_TIMEOUT)
	{
		vector<CTimerHeapNode> tmpNodes;
		while (size() > 0)
		{
			tmpNodes.push_back(top());
			pop();
		}
		for (size_t i=0; i<tmpNodes.size(); i++)
		{
			if (tmpNodes[i].nExpire <= m_nClock)
			{
				tmpNodes[i].nExpire = 0;
			}
			else
			{
				tmpNodes[i].nExpire -= m_nClock;
			}
			push(tmpNodes[i]);
		}
		m_nClockAlter += m_nClock;
		m_nClock = 0;
	}
}

