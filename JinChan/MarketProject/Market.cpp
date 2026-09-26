// Market.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MarketCore.h"

int _tmain(int argc, _TCHAR* argv[]) {
	Log::Init("Market", "./Market.log");
	Log::Info("Market: START...");

	CSelectThread* pInitThread = new CSelectThread;
	pInitThread->Create();


	CMarketCore marketCore(pInitThread);
	marketCore.Create();
	marketCore.Join();
	Log::Info("Market: END");

	return 0;
}

