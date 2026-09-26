#include "stdafx.h"
#include "XMPProtocol.h"

#define XMP_HEARTBEAT_TIMER_ID		1
#define READ_TIMEOUT_DEFAULT		120
#define WRITE_TIMEOUT_DEFAULT		60
#define WARNING_TIMEOUT_DEFAULT		80

CXMPProtocol::CXMPProtocol(CReactorThread* pReactor)
:CProtocol(pReactor, XMPHLEN+XMPEXTHLEN+XMP_EXTHEAD_MAX_SIZE, new CXMPPackage(), new CXMPPackage()),m_bHeartEnable(false)
{
	m_LastWriteTime = pReactor->Time();
	m_LastReadTime = pReactor->Time();
	m_LastWarnTime = m_LastReadTime;

	m_dwHeartbeatMode = 0;

	m_dwReadTimeout = READ_TIMEOUT_DEFAULT;
	m_dwWriteTimeout = WRITE_TIMEOUT_DEFAULT;
	m_dwWarningTimeout = WARNING_TIMEOUT_DEFAULT;

	EnableHeartbeat(true);
}
CXMPProtocol::~CXMPProtocol()
{
}

int CXMPProtocol::Pop(CPackage* pPackage)
{
	//设置最新收到数据包的时间
	m_LastReadTime = m_pIOThread->Time();
	m_LastWarnTime = m_LastReadTime;

	//检查是不是心跳
	CXMPPackage* pXMPPackage = (CXMPPackage*)pPackage;
	if(XMPTypeNone==pXMPPackage->GetActiveID() && XMPTagHeartbeatTimeOut==pXMPPackage->GetExtTag())
	{
		SetWriteTimeout(pXMPPackage);
		return 0;
	}
	return CProtocol::Pop(pPackage);
}
int CXMPProtocol::Push(CPackage* pPackage, CProtocol* pUpper)
{
	((CXMPPackage*)m_pSendPackage)->GetXMPHeader()->Type = (BYTE)pUpper->GetActiveID();

	//设置最新发送数据包的时间
	m_LastWriteTime = m_pIOThread->Time();
	return CProtocol::Push(pPackage, pUpper);
}
void CXMPProtocol::OnTimer(int nIDEvent)
{
	if(XMP_HEARTBEAT_TIMER_ID == nIDEvent)
	{
		if(m_pIOThread->Time()-m_LastReadTime > m_dwReadTimeout)
		{
			//心跳读超时，通知下层Channel
			NotifyError(MSG_XMPERR_RECVHEARTBEAT, 0, this);
			return;
		}

		if(m_pIOThread->Time()-m_LastWriteTime >= m_dwWriteTimeout)
		{
			//心跳读超时，发送心跳包
			if(SendHeartbeat()<0)
			{
				NotifyError(MSG_XMPERR_SENDHEARTBEAT, 0, this);
				return;
			}
		}

		if(m_pIOThread->Time()-m_LastWarnTime > m_dwWarningTimeout)
		{
			//心跳读超时，通知下层协议
			NotifyError(MSG_XMPWARNING_RECVHEARTBEAT, m_pIOThread->Time()-m_LastWarnTime, this);
			m_LastWarnTime = m_pIOThread->Time();
			return;
		}
	}
}
void CXMPProtocol::EnableHeartbeat(bool bEnable)
{
	if (m_bHeartEnable == bEnable)
		return ;

	bEnable ? SetTimer(XMP_HEARTBEAT_TIMER_ID,1000) : KillTimer(XMP_HEARTBEAT_TIMER_ID);
	m_bHeartEnable = bEnable;
}
void CXMPProtocol::SetHeartbeatMode(DWORD dwHeartbeatMode)
{
	m_dwHeartbeatMode = dwHeartbeatMode;
}
void CXMPProtocol::SetHeartbeatTimeout(DWORD dwReadTimeout)
{
	if (m_dwReadTimeout != dwReadTimeout)
	{
		m_dwReadTimeout = dwReadTimeout;
		if (m_dwReadTimeout < 4)
		{
			m_dwReadTimeout = 4;
		}
		m_dwWarningTimeout = dwReadTimeout/2;
		DWORD dwWriteTimeout = (m_dwReadTimeout-1)/3;
		SendWriteTimeout(dwWriteTimeout);
	}
}

int CXMPProtocol::SendHeartbeat()
{
 	int len = GetHdrReserveLen();
	CXMPPackage HeartbeatPackage;
	HeartbeatPackage.ConstructAllocate(0, len);
	HeartbeatPackage.SetExtHeader(XMPTagKeepAlive, 0, NULL);

	//设置最新发送数据包的时间
	m_LastWriteTime = m_pIOThread->Time();
 	int ret = Send(&HeartbeatPackage);
 	return ret;
}

int CXMPProtocol::SendWriteTimeout(DWORD dwWriteTimeout)
{
	int len = GetHdrReserveLen();
	CXMPPackage pkgHeartbeatTimeout;
	pkgHeartbeatTimeout.ConstructAllocate(sizeof(dwWriteTimeout), len);

	CHANGE_ENDIAN(dwWriteTimeout);
	pkgHeartbeatTimeout.SetExtHeader(XMPTagHeartbeatTimeOut, sizeof(dwWriteTimeout), (BYTE*)&dwWriteTimeout);

	//设置最新发送数据包的时间
	m_LastWriteTime = m_pIOThread->Time();

	int ret = Send(&pkgHeartbeatTimeout);

	return ret;
}
void CXMPProtocol::SetWriteTimeout(CXMPPackage* pPackage)
{
	if (pPackage->GetExtTagLen() != sizeof(DWORD))
	{
		return;
	}
	memcpy(&m_dwWriteTimeout, pPackage->GetExtData(), sizeof(DWORD));
	CHANGE_ENDIAN(m_dwWriteTimeout);
	if (m_dwHeartbeatMode & XMPHBT_SLAVE_MODE)
	{
		SetHeartbeatTimeout(m_dwWriteTimeout*3+1+3);	//4×3+1+3=16s
	}
}
int CXMPProtocol::OnRecvErrorPackage(CPackage* pPackage)
{
	NotifyError(MSG_XMPERR_BADPACKAGE, 0, this);
	return 0;
}
