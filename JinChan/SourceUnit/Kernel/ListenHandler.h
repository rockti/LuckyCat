#pragma once

#include "EventHandler.h"
#include "Channel.h"
#include "ServerBase.h"

//是一个基于TCP的网络服务器端，是EventHandler的子类，响应客户端的连结请求
class  CListenHandler : public CEventHandler  
{
public:
	CListenHandler(CReactorThread* pReactor, CServerBase* pServer);
	virtual ~CListenHandler();
	
	//由Reactor调用，处理连结请求事件	0 成功	-1 失败
	virtual int HandleInput();
	
	//获得监听id
	virtual void GetIds(int* pReadId, int* pWriteId);

protected:
	//当有新的连结时，调用的方法，子类必须实现该方法
	virtual int OnConnected(CChannel* pChannel) = 0;

protected:
	CServerBase* m_pServer;		//一个Server指针
};

//是一个基于UDP的网络服务器端，是EventHandler的子类，监听UDP数据报广播。
class  CUdpListenHandler : public CEventHandler  
{
public:
	CUdpListenHandler(CReactorThread* pReactor, CServerBase* pServer);
	virtual ~CUdpListenHandler();
	
	//由Reactor调用，处理连结请求事件	0 成功	-1 失败
	virtual int HandleInput();
	
	//获得监听id
	virtual void GetIds(int* pReadId, int* pWriteId);

protected:
	//当有新的udp 数据报到达时，调用的方法，子类必须实现该方法
	virtual int OnRecvDatagram(CChannel* pChannel) = 0;

protected:
	CChannel* m_pChannel;	//一个Channel指针
	CServerBase* m_pServer;	//一个Server指针
};
