#pragma once
#pragma once

#include "notify/Notify.h"

class Dispatcher {
public:
	void Dispatch(Data& data);
private:
	void dispatch(TradePlatformNotification& en);
	void dispatch_order_rtn(OrderRtnNotification& en);
	void dispatch_order_error(OrderErrorNotification& en);
	void dispatch_trade(InvestorTrade& trade, bool isRtn = false);
	void trigger_query_account(type::OrderStatus status, TradePlatform platform);
};
