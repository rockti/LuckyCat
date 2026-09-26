#include "stdafx.h"
#include "UdpNetworkFactory.h"
#include "UdpClient.h"
#include "UdpServer.h"

const char* UDP_NETWORK_NAME = "udp";

CUdpNetworkFactory CUdpNetworkFactory::m_instance;
CUdpNetworkFactory::CUdpNetworkFactory()
{
	RegisterFactory();
}

CUdpNetworkFactory::~CUdpNetworkFactory()
{
}

CServerBase* CUdpNetworkFactory::CreateServer(CServiceName *pName)
{
	if (0 == strcmp(pName->GetChannel(),UDP_NETWORK_NAME))
	{
		return new CUdpServer(pName);
	}
	return CNetworkFactory::CreateServer(pName);
}

CClientBase* CUdpNetworkFactory::CreateClient(CServiceName *pName)
{
	if (0 == strcmp(pName->GetChannel(),UDP_NETWORK_NAME))
	{
		return new CUdpClient();
	}
	return CNetworkFactory::CreateClient(pName);
}


