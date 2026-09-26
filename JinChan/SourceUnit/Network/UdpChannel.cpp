#include "stdafx.h"
#include "UdpChannel.h"
#include "SocketInit.h"


CUdpChannel::CUdpChannel(int id, char* pAddress, WORD port)
:CChannel(CT_DATAGRAM, id)
{
	for (;;)
	{
		unsigned long on_windows = 1;
		if (ioctlsocket(id, FIONBIO, &on_windows) < 0)
		{
			if (GET_LAST_SOCK_ERROR() == EINTR)
				continue;
			RAISE_RUNTIME_ERROR("UdpChannel can not set FIONBIO");
		}
		else
			break;
	}

	int rcvbufsize = 1*1024*1024;
	SOCKADDRLEN paramlen = sizeof(rcvbufsize);
	int nRtn = setsockopt(id, SOL_SOCKET, SO_RCVBUF, (char const*)&rcvbufsize, paramlen);
	if(nRtn != 0)
	{
		RAISE_RUNTIME_ERROR("Can not setsockopt revbuf");
	}

	m_pTargetAddr = new sockaddr_in;
	memset(m_pTargetAddr, 0, sizeof(sockaddr_in));
	m_pTargetAddr->sin_family = AF_INET; //IPv4
	m_pTargetAddr->sin_port = htons(port); 
	m_pTargetAddr->sin_addr.s_addr = inet_addr(pAddress); 
}

CUdpChannel::~CUdpChannel()
{
}

int CUdpChannel::ReadImp(int number, char* buffer)
{
	sockaddr_in fromAddr;
	SOCKADDRLEN nFromLen = sizeof(sockaddr_in);
	int nCount = recvfrom(GetId(), buffer, number, 0, (sockaddr*)&fromAddr, &nFromLen);

	if (nCount == 0)
		return -1;

	if (nCount == -1)
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

int CUdpChannel::WriteImp(int number, char* buffer)
{
	int nCount = ::sendto(GetId(), buffer, number, 0, (sockaddr*)m_pTargetAddr, sizeof(sockaddr_in));

	if (nCount == 0)
	{
		return -1;
	}

	if (nCount == -1)
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

bool CUdpChannel::AvailableImp(void)
{
	return true;
}

bool CUdpChannel::DisconnectImp(void)
{
	closesocket(GetId());
	return false;
}

bool CUdpChannel::CheckConnectionImp(void)
{
	return true;
}
