#pragma once

#include "model/Constants.h"


enum class DataType {
	Quote = 0,			// 全行情
	Price,				// 只更新一档价格的行情（盛利）
	Dealing,			// 回报
	Order,				// 委托
	Cancel,				// 撤单
	Trade,				// 成交明细
	MarketingQuote,		// 做市报价
	MarketingCancel,	// 做市撤单

	StrategyCommand,	// 操作员指令
	TimerCommand,		// 计时器指令
	ConnCommand,		// 交易所指令

	UIMessage,			// 界面日志
	Exchange,			// 交易所信息
	QryFund,			// 查资金
	QryPosition,		// 查持仓
	UpdateAvailable,	// 界面更新资金
	CleanPosition,		// 清空持仓
	BusinessUpdate,		// 更新参数
};

const int MAX_DATA_SIZE = 512;
struct Data {
	DataType Type;
	uint16_t Size;
	std::byte Data[MAX_DATA_SIZE];
};

struct ExchangeNotification {
	enum TYPE { QUOTE_CONNECTED, QUOTE_LOGINED, TRADER_CONNECTED, TRADER_LOGINED, QUOTE_DISCONNECTED, TRADER_DISCONNECTED, EXCHANGE_STATUS, CODE_LOGIN };

	ConnPlatform Platform;
	Exchange Exchange;
	TYPE Type;
	ExchangeStatus Status;
};
struct FundsNotification {
	ConnPlatform Platform;
	money Capital;
	money Available;
	money Fee;
	money Margin;
	money CloseProfit;
};

//OrderNotification是交易所状态，Dealing是内部状态
struct OrderNotification {
	enum ORDER_STATUS { 
		E_ORDERED, 
		E_SENT_FAILED, 
		E_CANCELED, 
		E_CANCEL_FAILED, 
		E_PARTIAL, 
		E_TRADE,
		E_ACCEPT/*Just4CTP*/, 
		E_FLOW_CONTROL/*流控*/,
		E_QUANTITY_TRADE,
		E_FAK_PARTIAL_TRADE,
		E_FUCK_ESUNNY_A_CANCEL,
		E_UNKOWN
	};
	ORDER_STATUS Status;
	Exchange Exchange;		//必填字段
	InstID InstID = INVALID_INSTRUMENT_ID;
	LocalOrderID LocalId;
	Direction Direction = Direction::BUY;
	Operation Operation = Operation::UNKNOWN;
	money OrderPrice = 0.0;
	uint16_t OrderQuantity = 0;
	uint16_t DealQuantity = 0;
	money DealPrice = 0.0;
	char SystemNo[21] = { 0 };
	bool IsMarketing = false;
	bool IsQueryOrder = false;
	bool IsQueryTrade = false;
};
inline char const* Desc(OrderNotification::ORDER_STATUS s) {
	switch (s) {
	case OrderNotification::E_ORDERED:
		return "Ordered";
	case OrderNotification::E_SENT_FAILED:
		return "SentFailed";
	case OrderNotification::E_CANCELED:
		return "Canceled";
	case OrderNotification::E_CANCEL_FAILED:
		return "CancelFailed";
	case OrderNotification::E_PARTIAL:
		return "Partial";
	case OrderNotification::E_TRADE:
		return "Trade";
	case OrderNotification::E_ACCEPT:
		return "Accept";
	case OrderNotification::E_FLOW_CONTROL:
		return "FlowCtl";
	case OrderNotification::E_QUANTITY_TRADE:
		return "QuantityTrade";
	case OrderNotification::E_FAK_PARTIAL_TRADE:
		return "FakPartial";
	default:
		return "NA";
	}
}

struct PositionNotification {
	InstID InstID;
	Direction Direction;
	HedgeType HedgeType;
	uint16_t Quantity;
	uint16_t Frozen;
	uint16_t TodayQty;	// 今仓是指要用平今指令的持仓，是Quantity的子集
	money Price;
};

struct CleanPositionNotification {};

class Business;
struct StrategyCommand {
	enum TYPE { NONE = 0, ADD_BUSINESS, PAUSE, START, STOP, UPDATE_ARGUMENT, MANUAL_COMMAND/*手工下单*/ };
	uint8_t Type;
	char Data[256];
	Business* Business; // only for add business, will remove in the future
	int BusinessID;
};

struct OrderInfo;
class TriggerTimer;
struct TimerCommand {
	OrderInfo* pInfo = nullptr;
	OrderInfo* pExInfo = nullptr;
	TriggerTimer* TimerPtr = nullptr;
	Business const* BusinessPtr;
};

class ExchangeConn;
struct ConnCommand {
	enum TYPE {
		LOGIN,					// 登录
		LOGOUT,					// 录出
		SET_CODE,				// 二次认证(仅易盛使用)
		MARKET_SUBSCRIBE,		// 行情订阅

		FUND_QRY,		
		ORDER_QRY,				// 委托查询
		TRADE_QRY,				// 成交查询
		POSITION_QRY,			// 持仓查询
	};
	uint8_t Type;
	char Data[256];
	ConnPlatform Platform;
};
