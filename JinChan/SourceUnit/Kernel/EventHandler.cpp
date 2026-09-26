#include "stdafx.h"
#include "EventHandler.h"
#include "ReactorThread.h"

CEventHandler::CEventHandler(CReactorThread* pReactor)
:m_pIOThread(pReactor)
{
}

CEventHandler::~CEventHandler()
{
	KillTimer(0);
	m_pIOThread->RemoveIOHandler(this);
	m_pIOThread->ClearEvent(this);
}

bool CEventHandler::PostEvent(int message, DWORD dwParam, void *pParam)
{
	return m_pIOThread->PostEvent(this, message, dwParam, pParam);
}

int CEventHandler::SendEvent(int message, DWORD dwParam, void *pParam)
{
	return m_pIOThread->SendEvent(this, message, dwParam, pParam);
}

void CEventHandler::SetTimer(int nIDEvent, int nElapse)
{
	m_pIOThread->RegisterTimer(this, nIDEvent, nElapse);
}

void CEventHandler::KillTimer(int nIDEvent)
{
	m_pIOThread->RemoveTimer(this, nIDEvent);
}
