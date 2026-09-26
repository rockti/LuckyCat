#include "stdafx.h"
#include "ListenHandler.h"

CListenHandler::CListenHandler(CReactorThread* pReactor, CServerBase* pServer)
:CEventHandler(pReactor),m_pServer(pServer)
{
}
CListenHandler::~CListenHandler()
{
	delete m_pServer;
}

int CListenHandler::HandleInput()
{
	CChannel* pChannel = m_pServer->Accept(0);
	if (NULL == pChannel)
		return -1;
	return OnConnected(pChannel);
}

void CListenHandler::GetIds(int* pReadId, int* pWriteId)
{
	*pReadId = m_pServer->GetId();
	*pWriteId = 0;
}

CUdpListenHandler::CUdpListenHandler(CReactorThread* pReactor, CServerBase* pServer)
:CEventHandler(pReactor),m_pServer(pServer),m_pChannel(m_pServer->Accept(0))
{
}
CUdpListenHandler::~CUdpListenHandler()
{
	delete m_pChannel;
	delete m_pServer;
}

int CUdpListenHandler::HandleInput()
{
	return OnRecvDatagram(m_pChannel);
}

void CUdpListenHandler::GetIds(int *pReadId, int *pWriteId)
{
	*pReadId = m_pChannel->GetId();
	*pWriteId = 0;
}
