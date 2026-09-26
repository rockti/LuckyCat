#include "stdafx.h"
#include "TcpChannel.h"
#include "SocketInit.h"

CTcpChannel::CTcpChannel(int socketId)
:CChannel(CT_STREAM, socketId)
,m_dwInAddr(0),m_dwLocalAddr(0)
{
	for (;;)
	{
		unsigned long on_windows=1;
		if (ioctlsocket(socketId, FIONBIO, &on_windows) < 0)
		{
			if (GET_LAST_SOCK_ERROR() == EINTR)
				continue;
			RAISE_RUNTIME_ERROR("TcpChannel can not set FIONBIO");
		}
		else
			break;
	}

	int on = 1;   
	setsockopt(socketId, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on));

	struct sockaddr name;
	SOCKADDRLEN nameLen = sizeof(name);
	if (0 == getpeername(GetId(), &name, &nameLen))
	{
		memcpy(&m_dwInAddr, &(((struct sockaddr_in*)&name)->sin_addr),sizeof(DWORD));
	}

	if (0 == getsockname(GetId(), &name, &nameLen))
	{
		memcpy(&m_dwLocalAddr, &(((struct sockaddr_in*)&name)->sin_addr), sizeof(DWORD));
	}
}
CTcpChannel::~CTcpChannel()
{
}

int CTcpChannel::ReadImp(int number, char *buffer)
{
	int nCount = ::recv(GetId(), buffer, number, 0);
	if (0 == nCount)
	{
		int nErrno = GET_LAST_SOCK_ERROR();
		return -1;
	}
	else if (-1 ==nCount)
	{
		int nErrno = GET_LAST_SOCK_ERROR();		
		if (EWOULDBLOCK==nErrno || 0==nErrno)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	return nCount;
}

int CTcpChannel::WriteImp(int number, char *buffer)
{
	int nCount = ::send(GetId(), buffer, number, 0);
	if (nCount == 0)
	{
		int nErrno = GET_LAST_SOCK_ERROR();
		return -1;
	}
	else if (-1 == nCount)
	{
		int nErrno = GET_LAST_SOCK_ERROR();
		if (EWOULDBLOCK == nErrno)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	return nCount;
}

bool CTcpChannel::AvailableImp(void)
{
	return true;
}

bool CTcpChannel::DisconnectImp(void)
{
	closesocket(GetId());
	return false;
}

bool CTcpChannel::CheckConnectionImp(void)
{
	return true;
}

char *CTcpChannel::GetRemoteName(void)
{
	struct in_addr addr;
	memcpy(&addr, &m_dwInAddr, sizeof(DWORD));
	return inet_ntoa(addr);
}

char* CTcpChannel::GetLocalName(void)
{
	struct in_addr addr;
	memcpy(&addr, &m_dwLocalAddr, sizeof(DWORD));
	return inet_ntoa(addr);
}



