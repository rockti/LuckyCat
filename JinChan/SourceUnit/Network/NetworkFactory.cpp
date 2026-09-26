#include "stdafx.h"
#include "NetworkFactory.h"

CNetworkFactory g_NetworkFactory;
CNetworkFactory* CNetworkFactory::m_pInstance = NULL;
CNetworkFactory::CNetworkFactory()
:m_pNext(NULL)
{
}
CNetworkFactory::~CNetworkFactory()
{
}

CNetworkFactory* CNetworkFactory::GetInstance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = &g_NetworkFactory;
	}
	return m_pInstance;
}

void CNetworkFactory::RegisterFactory()
{
	m_pNext = GetInstance();
	m_pInstance = this;
}

CServerBase* CNetworkFactory::CreateServer(CServiceName* pName)
{
	if (NULL != m_pNext)
	{
		return m_pNext->CreateServer(pName);
	}
	RAISE_RUNTIME_ERROR("CreateServer failed");
	return NULL;
}

CChannel* CNetworkFactory::CreateChannel(CServiceName* pName)
{
	CClientBase* pClient = CreateClient(pName);
	if (NULL == pClient)
	{
		return NULL;
	}

	CChannel* pChannel = pClient->Connect(pName);
	delete pClient;
	return pChannel;
}

CClientBase* CNetworkFactory::CreateClient(CServiceName *pName)
{
	if (NULL != m_pNext)
	{
		return m_pNext->CreateClient(pName);
	}
	RAISE_RUNTIME_ERROR("CreateClient failed");
	return NULL;
}
