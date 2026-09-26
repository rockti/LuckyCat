#include "stdafx.h"
#include "TcpClient.h"
#include "TcpChannel.h"
#include "SocketInit.h"

CTcpClient::CTcpClient()
{
}
CTcpClient::~CTcpClient()
{
}

CChannel* CTcpClient::Connect(CServiceName* pName)
{
	struct sockaddr_in dest;
	char* hostname;
	fd_set writefds;
	struct timeval timeout;

	/* create socket myself */
	int id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (id<0)
	{
		RAISE_RUNTIME_ERROR("Can not create socket for TcpClient");
	}

	/* set reuse and non block for this socket */
	int on = 1;
	setsockopt(id, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

	for (;;)
	{
		unsigned long on_windows=1;
		if (ioctlsocket(id, FIONBIO, &on_windows) < 0)
		{
			if (EINTR == GET_LAST_SOCK_ERROR())
				continue;
			RAISE_RUNTIME_ERROR("TcpClient can not set FIONBIO for socket");
			closesocket(id);
			return NULL;
		}
		break;
	}

	if (strcmp(pName->GetChannel(),"tcp"))
		RAISE_RUNTIME_ERROR("TcpClient can not connect to none tcp service");


	/* create dest socket address */
	dest.sin_family = AF_INET;
	if (0 == pName->GetPort())
		RAISE_RUNTIME_ERROR("TcpClient invalid port");
	dest.sin_port = htons(pName->GetPort());

	if (NULL != pName->GetHost())
		hostname = pName->GetHost();
	else
		hostname = "127.0.0.1"; 
	dest.sin_addr.s_addr=inet_addr(hostname);

	/* try to connect*/
	int nRtn=::connect(id, (struct sockaddr*)&dest, sizeof(dest));
	if (0 == nRtn)
	{
		SetErrorMsg(NULL);
		return CreateChannel(id);
	}

	FD_ZERO(&writefds);
	FD_SET(id, &writefds);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	nRtn = select(id+1, NULL, &writefds, NULL, &timeout);
	if (0 == nRtn)
	{
		SetErrorMsg("Can not connect to Server due to timeout");
		closesocket(id);
		return NULL;
	}

	struct sockaddr name;
	if ((nRtn>0) && FD_ISSET(id,&writefds))
	{
		SOCKADDRLEN nameLen=sizeof(name);
		if (getpeername(id,&name,&nameLen)==0)
		{
			SetErrorMsg(NULL);
			return CreateChannel(id);
		}
	}
	closesocket(id);
	SetErrorMsg("Can not connect to Server");

	return NULL;
}

CChannel *CTcpClient::CreateChannel(int id)
{
	return new CTcpChannel(id);
}
