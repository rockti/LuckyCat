#pragma once
#include "Session.h"
#include "XMPProtocol.h"

class CXMPSession : public CSession  
{
public:
	CXMPSession(CReactorThread* pReactor, CChannel* pChannel);
	virtual ~CXMPSession();
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);
	void SetHeartbeatTimeout(DWORD dwReadTimeout);
	void SetHeartbeatMode(bool bSlaveMode);

protected:
	CXMPProtocol* m_pXMPProtocol;	//容器内的第二层协议(从下部开始),负责链路管理
};
