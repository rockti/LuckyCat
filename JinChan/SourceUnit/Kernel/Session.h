#pragma once
#include "EventHandler.h"
#include "ChannelProtocol.h"
#include "ZTPType.h"

using namespace ztp;

class CSession;
class CSessionCallback
{
public:
	virtual void OnSessionConnected(CSession* pSession) = 0;
	virtual void OnSessionDisconnected(CSession* pSession, int nReason) = 0;
	virtual void OnSessionWarning(CSession* pSession, int nReason, int nParam) = 0;
	virtual ~CSessionCallback(){};
};

//该类做基类用，不能实例化
class CSession : public CEventHandler {
public:
	CSession(CReactorThread* pReactor, CChannel* pChannel, int MaxPackageSize);
	virtual ~CSession();
	
	inline type::SessionID GetSessionID()	{ return m_nSessionID; }
	inline CChannel* GetChannel() { return m_pChannel; };
	inline void RegisterSessionCallback(CSessionCallback* pSessionCallback)	{ m_pSessionCallback = pSessionCallback; }

	//当网络有可读数据包时被调用
	virtual int HandleInput();
	
	//当Reactor发现需要向网络发送数据包时被调用
	virtual int HandleOutput();
	
	//处理一条消息
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);

	//断开网络连接
	virtual void Disconnect(int nErrorCode);

	//获取socket，供select使用
	virtual void GetIds(int* pReadId, int* pWriteId);

	//挂起/恢复读操作
	void SuspendRead(bool bSuspend);

protected:	
	//网络断路消息处理程序，每当发现网络断路时叫用此函数
	virtual void OnChannelLost(int nErrorCode);

protected:
	type::SessionID m_nSessionID;
	bool m_bSuspendRead;		//挂起恢复标志

	CChannel* m_pChannel;		//容器捆绑的Channel
	CChannelProtocol* m_pChannelProtocol;	//容器内的底层协议
	CSessionCallback* m_pSessionCallback;	//回调指针
};
