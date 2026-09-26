#include "stdafx.h"
#include "TcpServer.h"
#include "SocketInit.h"
#include "TcpChannel.h"

CTcpServer::CTcpServer(CServiceName* pName) 
:CServerBase(pName)
{
	struct sockaddr_in me;
	m_nID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_nID<0)
		RAISE_RUNTIME_ERROR("Can not create socket for TcpServer");
	
	int on = 1;
	setsockopt(m_nID, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	memset(&me, 0, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_port = htons(pName->GetPort());
	if (bind(m_nID, (struct sockaddr *)&me, sizeof(me)) < 0)
	{
		//´òÒ»ÏÂÆÁ
		printf("TcpServer can not bind port:%d \n", pName->GetPort());
		RAISE_RUNTIME_ERROR("TcpServer can not bind port");
	}

	for (;;)
	{
		unsigned long on_windows = 1;
		if (ioctlsocket(m_nID, FIONBIO, &on_windows) < 0)
		{
			if (GET_LAST_SOCK_ERROR() == EINTR)
				continue;
			RAISE_RUNTIME_ERROR("TcpServer can not set FIONBIO for socket");
			closesocket(m_nID);
			return;
		}
		break;
	}

	if (listen(m_nID, 5)<0)
	{
// 		Log::Debug(FT("CTcpServer::listen failed[%d] error[%d]", m_nID, GET_LAST_SOCK_ERROR()));
		RAISE_RUNTIME_ERROR("TcpServer can not listen");
	}
}

CTcpServer::~CTcpServer()
{
	closesocket(m_nID);
}

CChannel* CTcpServer::Accept(int wait)
{
	struct sockaddr_in it;
	SOCKADDRLEN nameLen = sizeof(it);
	int newId = ::accept(m_nID, (struct sockaddr*)&it, &nameLen);

	if (newId < 0) 	{
		return NULL;
	}
	return CreateChannel(newId);
}

int CTcpServer::GetId(void)
{
	return m_nID;
}

CChannel* CTcpServer::CreateChannel(int id)
{
	return new CTcpChannel(id);
}
