#include "pch.h"
#include "EESQuoteApi.h"
#include "EES\EESQuoteApiStruct.h"


EESQuoteApi::EESQuoteApi()
:m_hQuit(false),m_id(0),m_localPort(0),m_remotePort(0)
,m_socket(SOCKET_ERROR),m_threadHandle(nullptr),m_spiPtr(nullptr) {
	WSADATA wsa_data = {0};
	WSAStartup(MAKEWORD(2,2), &wsa_data);
}
EESQuoteApi::~EESQuoteApi() {
	WSACleanup();
}

EESQuoteApi* EESQuoteApi::CreateEESQuoteApi() {
	return new EESQuoteApi();
}

void EESQuoteApi::Init() {
	m_threadHandle = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, socket_server_thread, this, 0, NULL));
	if (NULL == m_threadHandle) {
		::ResumeThread(m_threadHandle);
	}
}
void EESQuoteApi::Release() {
	m_hQuit = true;
	closesocket(m_socket);
	DoCallback(EVENT_DISCONNECT);
}

int EESQuoteApi::RegisterSocket(std::string const& remoteIP, unsigned short remotePort, std::string const& localIP, unsigned short localPort, int id) {
	m_remoteAddr = remoteIP;
	m_remotePort = remotePort;
	m_localAddr = localIP;
	m_localPort = localPort;
	m_id = id;
	int errCode = 0;
	m_socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED|WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF);
	if (SOCKET_ERROR == m_socket) {
		errCode = GetLastError();
		return errCode;
	}

	u_long flag = 1;
	if(INVALID_SOCKET == ioctlsocket(m_socket, FIONBIO, &flag)) {		//设置为非阻塞模式
		errCode = GetLastError();
		closesocket(m_socket);
		return errCode;
	}

	int ttl = 64;
	if (INVALID_SOCKET == setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl))) {	//设置提交组播报文的TTL(跳数)
		errCode = GetLastError();
		closesocket(m_socket);
		return errCode;
	}

	BOOL bTrue = TRUE;
	if(INVALID_SOCKET == setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bTrue, sizeof(BOOL))) {		//独占端口
		errCode = GetLastError();
		closesocket(m_socket);
		return errCode;
	}

	int newSize = 5242880;
	if(INVALID_SOCKET == setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&newSize, sizeof(newSize))) {	//重置Buffer大小
		errCode = GetLastError();
		closesocket(m_socket);
		return errCode;
	}

	sockaddr_in addrLocal = {0};
	addrLocal.sin_family = AF_INET;
	addrLocal.sin_addr.S_un.S_addr = inet_addr(m_localAddr.c_str());
	addrLocal.sin_port = htons(m_remotePort);
	if (0 != ::bind(m_socket, (sockaddr*)&addrLocal, sizeof(addrLocal))) {
		errCode = GetLastError();
		closesocket(m_socket);
		return errCode;
	}

	sockaddr_in addrMulticast = {0};
	addrMulticast.sin_family = AF_INET;
	addrMulticast.sin_addr.S_un.S_addr = inet_addr(m_remoteAddr.c_str());
	addrMulticast.sin_port = htons(m_remotePort);

	DWORD dw_flag;
	dw_flag = JL_RECEIVER_ONLY;
	if (INVALID_SOCKET == WSAJoinLeaf(m_socket, (sockaddr*)&addrMulticast, sizeof(addrMulticast), 0, 0, 0, 0, dw_flag)) {
		errCode = GetLastError();
		closesocket(m_socket);
		return errCode;
	}

	DoCallback(EVENT_CONNECT);		//OnConnect()
	return 0;
}
void EESQuoteApi::RegisterHandler(EESQuoteSpi* pSpi) {
	m_spiPtr = pSpi;
}

unsigned __stdcall EESQuoteApi::socket_server_thread(void* lpParameter) {
	EESQuoteApi* pThis = (EESQuoteApi*)lpParameter;
	if (NULL == pThis)	return NULL;

	pThis->DoProcess();
	_endthreadex(0);
	return 0;
}

void EESQuoteApi::DoProcess() {
	char line[RCV_BUF_SIZE] = "";

	struct sockaddr_in muticast_addr;
	memset(&muticast_addr, 0, sizeof(muticast_addr));
	muticast_addr.sin_family = AF_INET;
	muticast_addr.sin_addr.s_addr = inet_addr(m_remoteAddr.c_str());	
	muticast_addr.sin_port = htons(m_remotePort);

	while (true) {
		int len = sizeof(sockaddr_in);
		int n_rcved = recvfrom(m_socket, line, RCV_BUF_SIZE, 0, (struct sockaddr*)&muticast_addr, &len);
		if (n_rcved > 0)  {
			DoCallback(EVENT_RECEIVE, line, n_rcved);		//收到数据		
		} 

		if (m_hQuit) {
			break;
		}		
	}
}
void EESQuoteApi::DoCallback(SOCKET_EVENT eventType, const char* buff, unsigned int size) {
	if(NULL == m_spiPtr)
		return;

	switch(eventType) {
	case EVENT_CONNECT: {
			m_spiPtr->OnConnect();
		}
		break;
	case EVENT_DISCONNECT: {
			m_spiPtr->OnDisconnect();
		}
		break;
	case EVENT_RECEIVE: {
			m_spiPtr->OnRtnDepthMarketData(m_id, buff, size);
		}
		break;
	default:
		break;
	}
}


