#include "stdafx.h"
#include "Session.h"

static int g_sessionIDBase = 0;
CSession::CSession(CReactorThread* pReactor, CChannel* pChannel, int nMaxPackageSize)
:CEventHandler(pReactor),m_bSuspendRead(false),m_pChannel(pChannel)
{
	m_nSessionID = ++g_sessionIDBase;
	m_pChannelProtocol = new CChannelProtocol(m_pIOThread, pChannel, nMaxPackageSize);
	m_pChannelProtocol->RegisterErrorHandler(this);
	m_pSessionCallback = NULL;
}
CSession::~CSession()
{
	delete m_pChannelProtocol;
	m_pChannel->Disconnect();
	delete m_pChannel;
}

int CSession::HandleInput()
{
	return m_pChannelProtocol->HandleInput();
}
int CSession::HandleOutput()
{
	return m_pChannelProtocol->HandleOutput();
}
int CSession::HandleEvent(int nEventID, DWORD dwParam, void *pParam)
{
	switch(nEventID)
	{
	case MSG_CHANNDL_DRSTROY:
		delete this;
		return 0;
	case MSG_CHANNEL_READ_ERR:
	case MSG_CHANNEL_WRITE_ERR:
		if (dwParam != 0) {
			Disconnect(dwParam);
		} else {
			Disconnect(nEventID);
		}
		return 0;
	}
	return 0;
}	

void CSession::Disconnect(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		//主动断开，尝试将未发送的数据尽量发出
		if(m_pChannelProtocol->Flush() < 0)
		{
			return;
		}
	}
	m_pChannel->Disconnect();
	OnChannelLost(nErrorCode);
	delete this;
}
void CSession::GetIds(int* pReadId, int* pWriteId)
{
	m_pChannelProtocol->GetIds(pReadId,pWriteId);
	if (m_bSuspendRead)
	{
		*pReadId = 0;
	}
}
void CSession::OnChannelLost(int nErrorCode)
{
	if (NULL != m_pSessionCallback) 
	{
		m_pSessionCallback->OnSessionDisconnected(this, nErrorCode);
	}
}
void CSession::SuspendRead(bool bSuspend)
{
	m_bSuspendRead = bSuspend;
}
