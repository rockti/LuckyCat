#pragma once
#include "serverBase.h"

class CUdpServer : public CServerBase 
{
public:
	CUdpServer(CServiceName* pName);
	virtual ~CUdpServer();
	
public:
	//接收客户端连接
	virtual CChannel* Accept(int wait);
	virtual int GetId(void);

private:
	CServiceName m_ServiceName;
	int m_nID;	//套接字
	bool m_bConnected;
};
