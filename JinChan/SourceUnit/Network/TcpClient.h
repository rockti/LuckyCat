#pragma once

#include "ClientBase.h"

//CTcpClient是一个基于TCP的网络客户端
class CTcpClient : public CClientBase
{
public:
	CTcpClient();	
	virtual ~CTcpClient();

	virtual CChannel* Connect(CServiceName* pName);
	
protected:
	virtual CChannel* CreateChannel(int id);
};

