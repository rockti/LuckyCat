#pragma once
#include "Protocol.h"
#include "XMPPackage.h"

#define MSG_XMPERR_RECVHEARTBEAT		0x2001
#define MSG_XMPERR_SENDHEARTBEAT		0x2002
#define MSG_XMPERR_BADPACKAGE			0x2003
#define MSG_XMPWARNING_RECVHEARTBEAT	0x2101

#define XMPTypeNone				0x00
#define XMPTypeFTD				0x01
#define XMPTypeMDB				0x01
#define XMPTypeCompressed		0x02
#define XMPTypeZTP				0x03

DWORD const XMPHBT_SLAVE_MODE	= 0x00000001;

//外部管理协议(ExternalManagerProtocol)，协议栈容器的第二层协议，主要负责链路管理
class  CXMPProtocol : public CProtocol  
{
public:
	CXMPProtocol(CReactorThread* pReactor);
	virtual ~CXMPProtocol();

	//加入XMP报头并传至下层协议
	virtual int Push(CPackage* pPackage, CProtocol* pUpper);
	
	void EnableHeartbeat(bool bEnable);
	void SetHeartbeatMode(DWORD dwHeartbeatMode);
	void SetHeartbeatTimeout(DWORD dwReadTimeout);

protected:	
	//记录当前收到包的时间，判断是否XMP包，并上传
	virtual int Pop(CPackage* pPackage);
	virtual void OnTimer(int nIDEvent);
	//发送心跳
	int SendHeartbeat();
	//生成并发送设置心跳超时报文
	int SendWriteTimeout(DWORD dwWriteTimeout);
	//根据设置心跳超时报文，设置发送心跳超时时间
	void SetWriteTimeout(CXMPPackage* pPackage);
	virtual int OnRecvErrorPackage(CPackage* pPackage);

protected:
	bool m_bHeartEnable;		//是否接收/发送心跳
	DWORD m_LastWriteTime;		//最后发包时间
	DWORD m_LastReadTime;		//最后收包时间
	DWORD m_LastWarnTime;		//最后发出警告时间
	DWORD m_dwHeartbeatMode;	//心跳时间模式
	DWORD m_dwReadTimeout;		//接收心跳的超时时间
	DWORD m_dwWriteTimeout;		//发出心跳的时间间隔
	DWORD m_dwWarningTimeout;	//接收心跳的报警时间
};
