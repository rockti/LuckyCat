#include "conn/ExchangeConn.h"
#include "model/Constants.h"
#include "model/dealing/Dealing.h"
#include "utils/Log.h"
#include <fmt/format.h>

void ExchangeConn::LogOrderInsert(int rtn, OrderInsertRequest const& order, int requestId) {
	char const* direction = IsBuy(order.Direction) ? "B" : "S";
	char const* condition = (order.OrderPriceType==type::OrderPriceType::LimitPrice) ? "Normal" : "FAK";
	Log::Info(fmt::format("**OrderInsert**: rtn[{}] RequestID[{}] LocalID[{}] Instrument[{}] Flag[{}|{}] Price[{:.3f},{}] Condition[{}]",
		rtn, requestId, order.OrderLocalID, order.InstCode, direction, order.OffsetFlag, order.Price.value(), order.Volume, condition));
}
void ExchangeConn::LogOrderCancel(int rtn, OrderCancelRequest const& orderAction, int requestId) {
	Log::Info(fmt::format("**OrderCancel**: rtn[{}] RequestId[{}] LocalID[{}] Instrument[{}] CancelID[{}] SystemId[{}]",
		rtn, requestId, orderAction.OrderLocalID, orderAction.InstrumentCode, orderAction.CancelLocalID, orderAction.OrderSysID));
}

char const* GetExchangeDescribe(type::Exchange flag)
{
	switch (flag)
	{
	case type::Exchange::CFFEX:	return "CFFEX";
	case type::Exchange::CZCE:	return "CZCE";
	case type::Exchange::DCE:		return "DCE";
	case type::Exchange::SHFE:	return "SHFE";
	case type::Exchange::SGE:		return "SGE";
	case type::Exchange::HKEX:	return "HKEX";
	default:	return "BROKER";
	}
}