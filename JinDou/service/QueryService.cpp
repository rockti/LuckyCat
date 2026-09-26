
#include "service/QueryService.h"

void QueryService::TriggerQueryAccount(TradePlatform platform) {
	tradeSystemService_.QueryAccount(platform);
}

void QueryService::TriggerQueryPosition(TradePlatform platform) {
	tradeSystemService_.QueryPositions(platform);
}
