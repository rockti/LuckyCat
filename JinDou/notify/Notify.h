#pragma once

#include "model/Constants.h"
#include "type/ZTPType.h"
#include "domain/Error.h"
using namespace ztp;

enum class DataType {
	Quote = 0,
	Order,			
	Trade,
	OrderInsert,
	OrderCancel,
	OrderRtn,
	TradeRtn,
	OrderErrorRtn,
	Error,
	Instrument,
	Margin,
	Fee,
	StrategyCommand,	
	APICommand,			
	UIMessage,
	Exchange,			
	TradePlatform,
	Account,
	Position,
	CleanPosition,
	BusinessUpdate,
	InitStatus
};


struct TradePlatformNotification {
	enum TYPE { CONNECTED, DISCONNECTED, LOGIN, LOGIN_FAILED, LOGOUT, PASSWORD_CHANGE};

	TradePlatform Platform;
	SubSystem SubSystem;
	TYPE Type;
	Error Error;
	type::TradingDay TradingDay;
};

struct ExchangeNotification {
	TradePlatform Platform;
	type::Exchange Exchange;
	ExchangeStatus Status;
};

struct OrderRtnNotification {
	InvestorOrder Order;
};

struct TradeNotification {
	InvestorTrade Trade;
};

struct OrderErrorNotification {
	enum Type {Insert, Cancel};
	Type Type;
	TradePlatform Platform;
	type::OrderLocalID OrderLocalID;
	type::OrderLocalID CancelLocalID;
	type::OrderSysID OrderSysID;
	Error Error;
};

struct ErrorNotification {
	TradePlatform Platform;
	Error Error;
};

struct CleanPositionNotification {
	TradePlatform Platform;
};


class Business;
struct StrategyCommand
{
	enum TYPE { 
		NONE = 0, 
		ADD_BUSINESS, 
		DELETE_BUSINESS,
		ADD_MARKET_HANDLER/*增加行情处理器*/, 
		CANCEL, 
		RESET, 
		PAUSE, 
		START, 
		STOP,
		UPDATE_ARGUMENT, 
		UI_UPDATE_BUSINESS_INFO ,
		SPECIAL_COMMAND			//特殊指令，在Data中是具体Strategy::Command类型
	};
	TYPE Type;
	char Data[256];
	Business* Business; // only for add business, will remove in the future

	int BusinessID;

};

class ExchangeConn;
struct LoginCommand {
	type::InvestorID TradeInvestor;
	type::Password TradePassword;
	type::InvestorID QuoteInvestor;
	type::Password QuotePassword;
};

struct ChangePasswordCommand {
	type::Password OldPassword;
	type::Password NewPassword;
};

struct ConnCommand {
	enum TYPE {
		LOGIN,
		LOGOUT,
		MARKET_SUBSCRIBE,
		ORDER_QRY,
		TRADE_QRY,
		POSITION_QRY,
		INSTRUMENT_QRY,
		FEE_QRY,
		MARGIN_QRY,
		ACCOUNT_QRY,
		RELOGIN,
		CHANGE_PASSWORD
	};
	TYPE Type;
	TradePlatform Platform;
	SubSystem SubSystem;
	char Data[256] = "";
};



enum class InitStatus {
	New,
	Processing,
	Done,
	Fail
};

enum class InitProcess {
	QuoteConnect,
	TradeConnect,
	QuoteLogin,
	TradeLogin,
	QueryInstrument,
	QueryFee,
	QueryMargin,
	SubscribeQuote,
	QueryAccount,
	QueryPosition,
	QueryOrder,
	QueryTrade,
};

struct InitStatusNotification {
	TradePlatform Platform;
	InitProcess Process;
	InitStatus Status;
	Error Error;
};


const int MAX_DATA_SIZE = 512;
struct Data {
	DataType Type;
	uint16_t Size;
	std::byte Payload[MAX_DATA_SIZE];
};

