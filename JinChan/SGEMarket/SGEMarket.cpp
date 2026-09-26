#include "stdafx.h"
#include "SelectThread.h"
#include "Profile.h"
#include "MarketService.h"
#include "SGEQuoteConnection.h"
#include "MockSGEConnection.h"
CReadOnlyTimeType GetLocalTime() {
	SYSTEMTIME st = { 0 };
	::GetLocalTime(&st);
	return CReadOnlyTimeType(FT("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond));
}

int main(int argc, char* argv[]) {

	Log::Init("Market", "./SGEMarket.log");
	printNlog("Market: START...");
	// load config
	std::string marketFrontUrl = theProfile().GetPrivateProfile("Front", "MarketFrontUrl", "tcp://127.0.0.1:13823");
	std::string marketBrokerID = theProfile().GetPrivateProfile("Broker", "MarketBrokerID", "");

	//check marketBrokerID config
	if (marketBrokerID.empty()) {
		printNlog("load market broker id failed, exit");
		exit(-1);
	}
	// init data from DB
	DBInit init;
	init.Init(marketBrokerID);

	//create listen thread
	auto* serverThread = new CSelectThread();
	serverThread->Create();

	// create depth market flow
	auto* pDepthMarketFlow = new CCachedFlow(false, 2000000, 0x100000);

	//create market service
	MarketService front(serverThread, pDepthMarketFlow);
	front.RegisterListener(marketFrontUrl.c_str());
	string flowPath = theProfile().GetPrivateProfile("Common", "FlowPath", "./");
	auto pMarketFlowManager = new CMarketFlowManager(flowPath.c_str(), pDepthMarketFlow);	//生成行情流管理器，负责给Channel和Flow发送数据

#ifdef JINCHAN_MOCK
	static MockSGEConnection connection(SgeLinkType::QUOTE, pMarketFlowManager, 1000, "", 0, "", 0);
	connection.InitInstance(serverThread);
#else
	// Init QuoteHandler
	// In QuoteHandler, FTDInstance() to add Instruemnt
	// Add put last Snapshot to FTDInstance()
	string sgeAddr = theProfile().GetPrivateProfile("SGE", "SGEAddress", "188.31.25.50");
	int sgePort = theProfile().GetPrivateProfile("SGE", "SGEPort", 7777);
	string safeAddr = theProfile().GetPrivateProfile("SGE", "SafeAddress", "127.0.0.1");
	int safePort = theProfile().GetPrivateProfile("SGE", "SafePort", 6666);

	auto* seatInfo = ReposInstance().BaseRepo.Get();
	if (seatInfo) {
		static SGEQuoteConnection conn(*seatInfo,pDepthMarketFlow, sgeAddr, sgePort, safeAddr, safePort);
	} else {
		printNlog("seat info is null");
		exit(-2);
	}
#endif
	serverThread->Join();
	serverThread->Stop();

	DBInstance()->Join();
	DBInstance()->Stop();

}
