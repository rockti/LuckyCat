
#include "stdafx.h"
#include "TradeCore.h"
#include "Database.h"
#include <thread>


char const* version = "2.4.29";

int _tmain(int argc, _TCHAR* argv[]) {
	Log::Init("Trade", "./Trade.log");
	printNlog("Trade: START... Version[{}]", version);

	//初始化新小本地委托号
	int localIdPrefix = theProfile().GetPrivateProfile("SGE", "LocalIDPrefix", 8888);
	if (localIdPrefix == 8888) {
		printNlog("###############loading localIdPrefix failed!########### Exit!");
		exit(-1);
	}
	if (localIdPrefix > 100 || localIdPrefix < 10) {
		printNlog("###############Incorrect localIdPrefix [{}]!########### Exit!",localIdPrefix);
		exit(-1);
	}
	MaxOrderLocalIDSinglton::InitMinLocalID(localIdPrefix * 1'000'000);

// 	CSelectThread* pConnectThread = new CSelectThread;
// 	pConnectThread->Create();

	//创建初始化线程
	CSelectThread* pInitThread = new CSelectThread;
	pInitThread->Create();

	//创建交易核心
	CTradeCore tradeCore(/*pConnectThread,*/ pInitThread);
	tradeCore.Create();
	while (!tradeCore.IsInited()) {
		::Sleep(500);	//等待交易核心初始化结束
	}


	auto t = std::thread([&] {
		tradeCore.ZTPRun();
		});

	//这里直接启动前置，开始提供服务
	tradeCore.GetTradeFront()->FrontStartWork();
	tradeCore.Join();
//	pConnectThread->Join();
	t.join();

	return 0;
}

