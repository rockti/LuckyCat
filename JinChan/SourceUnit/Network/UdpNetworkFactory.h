#pragma once

#include "NetworkFactory.h"

class CUdpNetworkFactory : public CNetworkFactory  
{
public:
	CUdpNetworkFactory();
	virtual ~CUdpNetworkFactory();
	
	virtual CServerBase* CreateServer(CServiceName* pName);	
	virtual CClientBase* CreateClient(CServiceName* pName);

private:
	static CUdpNetworkFactory m_instance;
};