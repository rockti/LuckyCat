#include "pch.h"
#include "conn/ExchangeConn.h"
#include "model/Constants.h"
#include "model/dealing/Dealing.h"
#include "utils/Log.h"
#include "utils/Uri.h"


char const* DEADLINE = "20261230";

//char const* LEGAL_BROKERID = "3040";		//东证CTP
// char const* LEGAL_BROKERID = "0135";		//东证飞马
char const* LEGAL_BROKERID = "9999";		//海证CTP
char const* LEGAL_BROKERID_SIM = "6666";	//仿真
char const* LEGAL_BROKERID_TEST = "9999";	//测试

// char const* LLEGAL_USERID_SIM = "66609899";	// 仿真客户号
char const* LLEGAL_USERID_SIM = "66609012";		// 仿真客户号(东方)
// char const* LLEGAL_USERID_TEST = "049830";	// 测试客户号

void ExchangeConn::LogOrderInsert(int rtn, OrderInfo const& order, int requestId) {
	char const* direction = (order.Direction==Direction::SELL) ? "S" : "B";
	char const* condition = (order.OrderType==OrderType::NORMAL) ? "Normal" : "FAK";
	Log::Info(fmt::format("**OrderInsert**: rtn[{}] RequestID[{}] LocalID[{}] Instrument[{}] Flag[{}|{}] Price[{:.3f},{}] Condition[{}]"
		,rtn, requestId, order.LocalId, order.InstCode.Str(), direction, (int)order.Operation, order.Price.value(), order.Quantity, condition));
}
void ExchangeConn::LogOrderCancel(int rtn, OrderCancel const& orderAction, int requestId) {
	Log::Info(fmt::format("**OrderCancel**: rtn[{}] LocalID[{}] SystemID[{}]", rtn, orderAction.GetLocalId(), orderAction.GetSystemNo().Str()));
}

std::pair<std::string, int> SeparatePort(std::string const& hostUrl) {
	Uri uri = Uri::Parse(hostUrl);
	return make_pair(uri.Host, atoi(uri.Port.c_str()));
}
