#pragma once

//在Windows下，所有的socket的错误号全部以WSA开始
#ifdef WIN32
#undef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif
#define GET_LAST_SOCK_ERROR() h_errno
#define SOCKADDRLEN	int


class CSocketInit  
{
public:
	CSocketInit()
	{
		WSADATA initData;
		initData.wVersion=0;
		initData.wHighVersion=2;
		strcpy(initData.szDescription, "");
		strcpy(initData.szSystemStatus, "");
		initData.iMaxSockets = 100;
		initData.lpVendorInfo = 0;
		initData.lpVendorInfo = "";
		if (0 != WSAStartup(2, &initData))	RAISE_RUNTIME_ERROR("Can not init socket");
	}
	virtual ~CSocketInit() {}
};
