#pragma once
#pragma warning(disable:4996)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif

#define	RCV_BUF_SIZE						65535			// 最大的接收缓冲区


enum SOCKET_EVENT {
	EVENT_CONNECT,				//连接成功事件
	EVENT_DISCONNECT,			//连接断开事件
	EVENT_RECEIVE,				//数据接收事件
};

class EESQuoteSpi {
public:
	virtual ~EESQuoteSpi() {}
	virtual void OnConnect() = 0;
	virtual void OnDisconnect() = 0;
	virtual void OnRtnDepthMarketData(int id, char const* buff, unsigned int len) = 0;
};

class EESQuoteApi {
public:
	static EESQuoteApi* CreateEESQuoteApi();
	void Init();
	void Release();

	int RegisterSocket(std::string const& remoteIP, unsigned short remotePort, std::string const& localIP, unsigned short localPort, int id);	//初始化socket
	void RegisterHandler(EESQuoteSpi* pSpi);

protected:
	virtual ~EESQuoteApi();
	static unsigned __stdcall socket_server_thread(LPVOID lpParameter);

private:
	EESQuoteApi();
	void DoProcess();
	void DoCallback(SOCKET_EVENT eventType, char const* buff=NULL, unsigned int size=0);

private:
	bool m_hQuit;			//线程退出
	int m_id;
	unsigned short m_localPort;
	unsigned short m_remotePort;

	SOCKET m_socket;
	HANDLE m_threadHandle;
	EESQuoteSpi* m_spiPtr;

	std::string m_localAddr;
	std::string m_remoteAddr;
};

