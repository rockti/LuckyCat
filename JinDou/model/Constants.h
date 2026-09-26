#pragma once

#include <cstdint>
#include "type/ZTPType.h"
using namespace ztp;
// enum class Exchange {
// 	SHFE	= 0,			// 上海期货
// 	HKEX	= 1,			// 港交所
// 	CFFEX	= 2,			// 中金（上海）
// 	SGE		= 3,			// 上海黄金
// 	DCE		= 4,			// 大连期货
// 	CZCE	= 5,			// 郑州期货
// 	HKEX_Q  = 13,			// 港交所行情
// 	CTP		= 15
// };



// 
// enum class Direction	//方向
// {
// 	BUY = 0,
// 	SELL
// };
// 
// enum class HedgeType	//投保(此版本暂不支持套保)
// {
// 	SPECULATE = 1,
// 	ARBITRAGE
// };
// 
// enum class OrderType	//委托方式(此版本暂不支持FOK)
// {
// 	NORMAL = 0,
// 	FAK,
// 	FOK
// };
// 
// inline const char* Desc(OrderType type) {
// 	switch (type) {
// 	case OrderType::NORMAL:
// 		return "Normal";
// 	case OrderType::FAK:
// 		return "FAK";
// 	case OrderType::FOK:
// 		return "FOK";
// 	default:
// 		return "UNKNOWN";
// 		break;
// 	}
// }
// 
// inline Direction OppositeDirection(Direction direction) {
// 	return (Direction::BUY == direction) ? Direction::SELL : Direction::BUY;
// }
// 
// inline bool IsBuy(Direction direct) { return direct == Direction::BUY; }
// 
// inline char Desc(Direction direct) {
// 	return direct == Direction::BUY ? 'B' : 'S';
// }
// 
// enum class Operation	//操作
// {
// 	OPEN = 0,
// 	OFFSET,
// 	OFFSET_TODAY,
// 	UNKNOWN				// 强减，强平等都归为未知 
// };
// 
// inline char const* Desc(Operation opt) {
// 	switch (opt) {
// 	case Operation::OPEN:
// 		return "OP";
// 	case Operation::OFFSET:
// 		return "OF";
// 	case Operation::OFFSET_TODAY:
// 		return "OT";
// 	case Operation::UNKNOWN:
// 		return "UN";
// 	default:
// 		return "UN";
// 	}
// }


enum class ExchangeStatus {
	BEFORE_TRADING = 0, 
	NO_TRADING, 
	CONTINOUS, 
	AUCTION_ORDERING, 
	AUCTION_BALANCE, 
	AUCTION_MATCH, 
	CLOSED, 
	UNKNOW = 99
};

enum class ConnectStatus {
	None,
	QuoteConnected,
	QuoteDisconnected,
	TradeConnected,
	TradeDisconnected,
};

// EXCHANGE（换手）：增仓为0
// BOTH_OPEN（双开）：增仓数量等于现手数量	新多头买入开仓和新空头卖出开仓达成的成交
// BOTH_OFFSET（双平）：增仓数量等于负的现手数量	老多头卖出平仓和老空头买入平仓达成的成交
// BEAR_OPEN（空开）：买价为成交价，且增仓为正值	空头和多头同时开仓，以多头申报价格成交，反映主动性买盘
// BULL_OPEN（多开）：卖价为成交价，且增仓为正值	空头和多头同时开仓，以多头申报价格成交，反映主动性买盘
// BEAR_OFFSET（空平）：卖价为成交价，且增仓为负值	空头主动平仓
// BULL_OFFSET（多平）：买价为成交价，且增仓为负值	多头主动平仓
enum class Behavior		//行情是500ms快照，因此只能大致估算方向
{
	UNKNOWN = 0,
	EXCHANGE,
	BOTH_OPEN,
	BOTH_OFFSET,
	BEAR_OPEN,
	BULL_OPEN,
	BEAR_OFFSET,
	BULL_OFFSET
};

enum class MarketSituation {
	BULL = 0,
	BEAR,
	SHOCK		// 震荡
};


#define ILLEGAL_MONTH						1
#define ILLEGAL_INSTRUMENT_INDEX			2 
#define ILLEGAL_TABLE_INDEX					3
#define ILLEGAL_EXCHANGE_TYPE				4
#define ILLEGAL_MESSAGE_TYPE				5
#define ILLEGAL_OPERATION_TYPE				6

const int MAX_INSTRUMENT_COUNT = 1000;		//市场合约数量


const int CTP_INDEX = 0;
const int ESUNNY_INDEX = 1;
const int FEMAS_INDEX = 2;

const int SHFE_INDEX = 0;
const int HKEX_INDEX = 1;
const int CFFEX_INDEX = 2;

inline constexpr int PlatformIndex(TradePlatform api) {
	return int(api);
}

inline constexpr TradePlatform Platform(type::Exchange exchange) {
	switch (exchange) {
	case type::Exchange::SHFE:
	case type::Exchange::CFFEX:
	case type::Exchange::DCE:
	case type::Exchange::CZCE:
		return TradePlatform::CTP;
	case type::Exchange::HKEX:
		return TradePlatform::HKEX_ESUNNY;
	case type::Exchange::SGE:
		return TradePlatform::ZTP;
	default:
		return TradePlatform::CTP;
	}
}


inline constexpr int ConnIndex(type::Exchange exchange) {
	return static_cast<int>(Platform(exchange));
}




const type::InstID INVALID_INSTRUMENT_ID = 255;

inline bool IsInvalidInstID(type::InstID instID) { return instID == INVALID_INSTRUMENT_ID; }

inline type::InstID InstIDGen(type::Exchange exchange, int index) {
	return (((uint16_t)exchange) << 12) | ((uint16_t)index);
}

inline type::Exchange InstIdExchange(type::InstID instID) {
	return static_cast<type::Exchange>((instID & 0xF000) >> 12);
}

inline int InstIDIndex(type::InstID instID) {
	return instID & 0x0FFF;
}

typedef uint32_t LocalOrderID;

const uint32_t INVALID_LOCAL_ID = 99999999;

typedef uint16_t QtyT;
typedef uint32_t TickT;