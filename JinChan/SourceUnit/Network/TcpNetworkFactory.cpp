#include "stdafx.h"
#include "TcpNetworkFactory.h"
#include "TcpClient.h"
#include "TcpServer.h"

const char* TCP_NETWORK_NAME = "tcp";

CTcpNetworkFactory CTcpNetworkFactory::m_instance;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpNetworkFactory::CTcpNetworkFactory()
{
	RegisterFactory();
}

CTcpNetworkFactory::~CTcpNetworkFactory()
{
}

CServerBase* CTcpNetworkFactory::CreateServer(CServiceName* pName)
{
	if (0 == strcmp(pName->GetChannel(),TCP_NETWORK_NAME))
	{
		return new CTcpServer(pName);
	}
	return CNetworkFactory::CreateServer(pName);
}

CClientBase* CTcpNetworkFactory::CreateClient(CServiceName* pName)
{
	if (0 == strcmp(pName->GetChannel(),TCP_NETWORK_NAME))
	{
		return new CTcpClient();
	}
	return CNetworkFactory::CreateClient(pName);
}


