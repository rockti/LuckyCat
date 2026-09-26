#include "stdafx.h"
#include "SelectThread.h"
#include "Profile.h"
#include "MarketService.h"
#include "ZQuoteConnection.h"

CReadOnlyTimeType GetLocalTime() {
	SYSTEMTIME st = { 0 };
	::GetLocalTime(&st);
	return CReadOnlyTimeType(FT("%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond));
}

int main(int argc, char* argv[]) {
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

	// Init QuoteHandler
	// In QuoteHandler, FTDInstance() to add Instruemnt
	// Add put last Snapshot to FTDInstance()

	ZQuoteConnection conn(pDepthMarketFlow);

	serverThread->Join();
	serverThread->Stop();

	DBInstance()->Stop();
	DBInstance()->Join();

}
