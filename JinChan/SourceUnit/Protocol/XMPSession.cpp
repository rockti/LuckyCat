#include "stdafx.h"
#include "XMPSession.h"

CXMPSession::CXMPSession(CReactorThread* pReactor, CChannel* pChannel)
:CSession(pReactor, pChannel, XMP_PACKAGE_MAX_SIZE+XMPHLEN+XMPEXTHLEN+256)
{
	m_pXMPProtocol = new CXMPProtocol(m_pIOThread);
	m_pXMPProtocol->AttachLower(m_pChannelProtocol, XMPTypeNone);
	m_pXMPProtocol->RegisterErrorHandler(this);
}
CXMPSession::~CXMPSession()
{
	delete m_pXMPProtocol;
}

int CXMPSession::HandleEvent(int nEventID, DWORD dwParam, void *pParam)
{
	switch(nEventID)
	{
	case MSG_XMPERR_RECVHEARTBEAT:
	case MSG_XMPERR_SENDHEARTBEAT:
	case MSG_XMPERR_BADPACKAGE:
		Disconnect(nEventID);
		return 0;
	}

	return CSession::HandleEvent(nEventID, dwParam, pParam);
}	

void CXMPSession::SetHeartbeatTimeout(DWORD dwReadTimeout)
{
	m_pXMPProtocol->SetHeartbeatTimeout(dwReadTimeout);
}

void CXMPSession::SetHeartbeatMode(bool bSlaveMode)
{
	m_pXMPProtocol->SetHeartbeatMode(bSlaveMode);
}
