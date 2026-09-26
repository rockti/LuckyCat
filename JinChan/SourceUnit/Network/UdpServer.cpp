#include "stdafx.h"
#include "UdpServer.h"
#include "UdpChannel.h"


CUdpServer::CUdpServer(CServiceName* pName)
:CServerBase(pName),m_ServiceName(pName->GetLocation())
,m_bConnected(false)
{
	int on=1;
	m_nID = socket(AF_INET, SOCK_DGRAM, 0); //建立UDP套接字
	setsockopt(m_nID, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	setsockopt(m_nID, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof(on));

	struct sockaddr_in servaddr;			//IPv4套接口地址定义
	memset(&servaddr, 0, sizeof(servaddr)); //地址结构清零
	servaddr.sin_family = AF_INET;			//IPv4协议
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);		//内核指定地址
	servaddr.sin_port = htons(m_ServiceName.GetPort()); //端口
	
	//分配协议地址,绑定端口
	if (0 != bind(m_nID, (sockaddr*)&servaddr, sizeof(servaddr)))
	{
		RAISE_RUNTIME_ERROR("UdpSvr bind fail");
	}
}

CUdpServer::~CUdpServer()
{
}

CChannel* CUdpServer::Accept(int wait)
{
	if (m_bConnected)
	{
		return NULL;
	}

	m_bConnected = true;
	return new CUdpChannel(m_nID, m_ServiceName.GetHost(), m_ServiceName.GetPort()+1);
}

int CUdpServer::GetId(void)
{
	if (!m_bConnected)
		return -1;
	else
		return 0;
}
