#pragma once

#include "model/Constants.h"
#include "service/Service.h"

class QueryService : public IQueryService {
public:
	QueryService(ITradeSystemService& tradeSystemService) :tradeSystemService_(tradeSystemService) {}
	virtual void TriggerQueryAccount(TradePlatform platform) override;
	virtual void TriggerQueryPosition(TradePlatform platform) override;
private:
	ITradeSystemService& tradeSystemService_;
};