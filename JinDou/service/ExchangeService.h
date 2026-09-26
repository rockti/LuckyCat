#pragma once

#include "model/Constants.h"
#include "service/Service.h"

class ExchangeService : public IExchangeService {

public:
	virtual TradePlatform toPlatform(type::Exchange exchange) override;
};