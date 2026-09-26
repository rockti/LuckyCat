#include "stdafx.h"
#include "NotifyHandler.h"
#include "SelectThread.h"
#include "SocketInit.h"
#include "ServiceName.h"
#include "TcpServer.h"

#define FlUX_GENE   40       //流控因子
#define FlUX_CTL_INTER   15  //流控时间粒度(毫秒)，表示检查流控的时间间隔。

CNotifyHandler::CNotifyHandler(CSelectThread* pReactor, int pipeNo)
:CEventHandler(pReactor),m_pSelectThread(pReactor),m_pipeNo(pipeNo)
,m_nReadPipe(0),m_nWritePipe(0)
,m_nPreClock(m_pSelectThread->GetClock()),m_nPerCnt(0),m_SendFlag(false)
,m_nReadCnt(0),m_nWriteCnt(0)
{
	CreatePipe(m_nReadPipe, m_nWritePipe);
	m_pSelectThread->RegisterIOHandler(this);	//将自身对象加入到通知线程的列表中
	memset(m_buffer,0,sizeof(m_buffer));
}

CNotifyHandler::~CNotifyHandler()
{
	closesocket(m_nWritePipe);
	closesocket(m_nReadPipe);
}

void CNotifyHandler::GetIds(int* pReadId, int* pWriteId)
{
	*pReadId = m_nReadPipe;
	*pWriteId = 0;	//writePipe不返回
}

//从管道中读取数据,只读一个字节，保证活着，不需要全部收完，协议层会保证传输递达
int CNotifyHandler::HandleInput()
{
	int nRtn=::recv(m_nReadPipe, m_buffer, 1, 0);
	if (1 != nRtn)
	{
		perror("read::");
		return 0;
	}
	m_nReadCnt++;
	return 0;
}

//检查流控的时间间隔是 FlUX_CTL_INTER 毫秒。如果在时间间隔内，触发次数过多，则关闭发送开关。
//每次流控检查，触发次数的计数都重新清零。
//1、只有在发送开关打开的时候才发送通知 
//2、只有上次发送的通知已经被读取，才发送通知
void CNotifyHandler::Notify()
{	
	m_pSelectThread->SetSpeciaFlag(true);

	//流控策略：超限请求关闭发送标志
	++m_nPerCnt;	//每个流控周期的计数
	DWORD inter = m_pSelectThread->GetClock()-m_nPreClock;
	if (inter > FlUX_CTL_INTER)
	{		
		m_nPreClock = m_pSelectThread->GetClock();	
		if (m_nPerCnt > inter*FlUX_GENE)
		{
			m_SendFlag = false;		
			m_nPerCnt=0;
			return;
		}
		else
		{
			m_SendFlag = true;		
			m_nPerCnt=0;
		}
	}

	if (!m_SendFlag)
	{
		return;
	}

	//如果有尚未处理的通知信号，则不再发送信号
	if (m_nWriteCnt > m_nReadCnt)
		return;

	int nRtn = ::send(m_nWritePipe, m_buffer, 1, 0);
	if (1 != nRtn)
	{
		perror("write::");
		return;
	}
	m_nWriteCnt++;
	return ;
}

void CNotifyHandler::CreatePipe(int& pRead, int& pWrite)
{
	pRead = 0;
	pWrite = 0;

	m_pipeNo = m_pipeNo*10+(_getpid()%50);	//随机端口
	char* pSvc = new char[100];
	sprintf(pSvc, "tcp://127.0.0.1:%d", m_pipeNo);

	CServiceName svc(pSvc);
	CTcpServer localServer(&svc);		
	auto pipeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pipeSocket < 0)
	{
		RAISE_RUNTIME_ERROR("CNotifyHandler: can not create socket for Pipe");
	}

	//非阻塞绑定
	int on = 1;
	setsockopt(pipeSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	for (;;)
	{	
		unsigned long on_windows=1;
		if (ioctlsocket(pipeSocket, FIONBIO, &on_windows) < 0)
		{
			if (GET_LAST_SOCK_ERROR() == EINTR)
				continue;
			RAISE_RUNTIME_ERROR("CNotifyHandler can not set FIONBIO");
		}
		else
			break;
	}

	char* hostname = "127.0.0.1";
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = htons(svc.GetPort());
	dest.sin_addr.s_addr=inet_addr(hostname); 
	int nRtn = ::connect(pipeSocket, (struct sockaddr*)&dest, sizeof(dest));
	if (0 == nRtn)
	{
		pWrite = (int)pipeSocket;
	}

	Sleep(100);

	struct sockaddr_in it;
	SOCKADDRLEN nameLen = sizeof(it);
	auto newId = ::accept(localServer.GetId(), (struct sockaddr *)&it, &nameLen);
	if (newId < 0)
	{
		RAISE_RUNTIME_ERROR("create pipe_server failed!!");
	}
	else
	{
		pRead = (int)newId;
	}

	//检查客户端是否创建成功
	if (pWrite>0)	return;

	//检查是否创建完成
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(pipeSocket, &writefds);
	struct timeval timeout;
	timeout.tv_sec=5;
	timeout.tv_usec=0;
	nRtn = select(int(pipeSocket+1), NULL, &writefds, NULL, &timeout);
	if (0 == nRtn)
	{
		RAISE_RUNTIME_ERROR("create pipe_client failed!!");
		return;
	}

	struct sockaddr name;
	if ((nRtn>0)&&FD_ISSET(pipeSocket,&writefds))
	{
		SOCKADDRLEN nameLen=sizeof(name);
		if (getpeername(pipeSocket,&name,&nameLen)==0)
		{
			pWrite=(int)pipeSocket;
			return;
		}
	}

	closesocket(pipeSocket);
	RAISE_RUNTIME_ERROR("create pipe_client failed!!");
}
