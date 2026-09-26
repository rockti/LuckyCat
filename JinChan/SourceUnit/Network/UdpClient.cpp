#include "stdafx.h"
#include "UdpClient.h"
#include "SocketInit.h"
#include "UdpChannel.h"

CUdpClient::CUdpClient()
{
}
CUdpClient::~CUdpClient()
{
}

CChannel* CUdpClient::Connect(CServiceName* pServiceName)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0); //建立UDP套接字
	/* set reuse and non block for this socket */
	int on=1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on));

	struct sockaddr_in servaddr;			//IPv4套接口地址定义
	memset(&servaddr, 0, sizeof(servaddr));	//地址结构清零
	servaddr.sin_family = AF_INET;			//IPv4协议
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY/*INADDR_BROADCAST*/);	//内核指定地址
	servaddr.sin_port = htons(pServiceName->GetPort()+1);	//端口

	//分配协议地址,绑定端口
	if (0 != bind(sockfd, (sockaddr *) &servaddr, sizeof(servaddr)))
	{
		RAISE_RUNTIME_ERROR("UdpClient bind fail");
	}

	CChannel* pChannel = new CUdpChannel(sockfd, pServiceName->GetHost(), pServiceName->GetPort());
	return pChannel;
}
