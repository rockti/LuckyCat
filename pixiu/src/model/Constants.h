#pragma once

#include "type/Money.h"
#include "type/CharArray.h"


enum class Exchange {
	SHFE	= 0,			// 上海期货
	HKEX	= 1,			// 港交所
	CFFEX	= 2,			// 中国金融
	SGE		= 3,			// 上海黄金
	DCE		= 4,			// 大连期货
	CZCE	= 5,			// 郑州期货
	INE		= 6,			// 上海能源
	HKEX_Q  = 13,			// 港交所行情
	CTP		= 15			// 柜台交易
};
inline bool IsSGE(Exchange exchange) {
	return Exchange::SGE == exchange;
}
inline bool IsINE(Exchange exchange) {
	return Exchange::INE == exchange;
}
inline bool IsSHFE(Exchange exchange) {
	return Exchange::SHFE == exchange;
}
inline bool IsCFFEX(Exchange exchange) {
	return Exchange::CFFEX == exchange;
}
inline const char* Desc(Exchange exch) {
	switch (exch) {
	case Exchange::CFFEX: return "CFFEX";
	case Exchange::CZCE: return "CZCE";
	case Exchange::DCE: return "DCE";
	case Exchange::INE: return "INE";
	case Exchange::SHFE: return "SHFE";
	case Exchange::SGE:	return "SGE";
	case Exchange::HKEX: return "HKEX";
	case Exchange::HKEX_Q: return "HKEX_Q";
	default:	return "BROKER";
	}
}

enum class Direction {
	BUY = 0,
	SELL
};	// 方向
inline char Desc(Direction dir) {
	return dir == Direction::BUY ? 'B' : 'S';
}
inline bool IsBuy(Direction direct) { return direct == Direction::BUY; }
inline bool IsSell(Direction direct) { return direct == Direction::SELL; }
inline Direction OppositeDirection(Direction direction) {
	return (Direction::BUY == direction) ? Direction::SELL : Direction::BUY;
}

enum class Operation {
	OPEN = 0,
	OFFSET,
	OFFSET_TODAY,
	UNKNOWN				// 强减，强平等都归为未知 
};	//操作
inline char const* Desc(Operation opt) {
	switch (opt) {
	case Operation::OPEN:			return "OPEN";
	case Operation::OFFSET:			return "OFST";
	case Operation::OFFSET_TODAY:	return "OFTD";
	default:	return "Unknown";
	}
}
enum class OrderType {
	NORMAL = 0,
	FAK,
	FOK
};	//委托方式(不支持FOK)
inline const char* Desc(OrderType type) {
	switch (type) {
	case OrderType::NORMAL:	return "Normal";
	case OrderType::FAK:	return "FAK";
	case OrderType::FOK:	return "FOK";
	default:	return "UNKNOWN";
	}
}

enum class ExchangeStatus {
	BEFORE_TRADING = 0, 
	NO_TRADING, 
	CONTINOUS, 
	AUCTION_ORDERING, 
	AUCTION_BALANCE, 
	AUCTION_MATCH, 
	CLOSED, 
	DELIVERY_APPEND,
	DELIVERY_END,
	MID_APPEND,
	MID_BALANCE,
	UNKNOW = 99
};
inline char const* Desc(ExchangeStatus status) {
	switch (status) {
	case ExchangeStatus::BEFORE_TRADING:	return "开盘前";
	case ExchangeStatus::NO_TRADING:		return "非交易";
	case ExchangeStatus::CONTINOUS:			return "连续交易";
	case ExchangeStatus::AUCTION_ORDERING:	return "竞价报单";
	case ExchangeStatus::AUCTION_BALANCE:	return "竞价平衡";
	case ExchangeStatus::AUCTION_MATCH:		return "竞价撮合";
	case ExchangeStatus::DELIVERY_APPEND:	return "交割申报";
	case ExchangeStatus::DELIVERY_END:		return "交割结束";
	case ExchangeStatus::MID_APPEND:		return "中立仓申报";
	case ExchangeStatus::MID_BALANCE:		return "中立仓平衡";
	case ExchangeStatus::CLOSED:			return "收盘";
	default:	return "未知";
	}
}

enum class HedgeType {	//投保(此版本暂不支持套保)
	SPECULATE = 1,
	ARBITRAGE
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
enum class Behavior	{	//行情是500ms快照，因此只能大致估算方向
	UNKNOWN = 0,
	EXCHANGE,
	BOTH_OPEN,
	BOTH_OFFSET,
	BEAR_OPEN,
	BID_OPEN,
	BEAR_OFFSET,
	BID_OFFSET
};

money const OFROPEN = 9'999;
money const BIDOFFSET = 8'888;
money const OFROFFSET = -8'888;
money const BIDOPEN = -9'999;

#define ILLEGAL_MONTH						1
#define ILLEGAL_INSTRUMENT_INDEX			2 
#define ILLEGAL_TABLE_INDEX					3
#define ILLEGAL_EXCHANGE_TYPE				4
#define ILLEGAL_MESSAGE_TYPE				5
#define ILLEGAL_OPERATION_TYPE				6

const int MAX_FUTURE_CANCEL_COUNT = 1950;		// 期货最大撤单次数
const int MAX_SPOT_CANCEL_COUNT = 650;			// 现货最大撤单次数
const int MAX_INSTRUMENT_COUNT = 1000;			// 市场合约数量

enum class ConnPlatform {
	CTP = 0,			// CTP(与易达互斥)
	HFT = 1,			// 易达(与CTP互斥)
	SGE = 2,			// SGE
	HKEX_ESUNNY = 3,	// HK ESUNNY
};	// 注意，最多只能支持三个连接	std::array<ExchangeConn*, 3> m_conns;

inline constexpr int PlatformIndex(ConnPlatform api) {
	return int(api);
}

// 这里根据品种来决定平台设计有问题
inline constexpr ConnPlatform Platform(Exchange exchange) {
	switch (exchange) {
	case Exchange::SHFE:
	case Exchange::CFFEX:
#ifdef _DEBUG
	return ConnPlatform::HFT;		// 易达测试环境
// 	return ConnPlatform::CTP;		// CTP测试环境
#else
	return ConnPlatform::HFT;
#endif
	case Exchange::HKEX:
	case Exchange::HKEX_Q:
		return ConnPlatform::HKEX_ESUNNY;
	case Exchange::SGE:
		return ConnPlatform::SGE;
	default:
		return ConnPlatform::CTP;
	}
}

inline constexpr int ConnIndex(Exchange exchange) {
	return static_cast<int>(Platform(exchange));
}

typedef uint16_t InstID;
const InstID INVALID_INSTRUMENT_ID = 255;

typedef uint32_t LocalOrderID;
const uint32_t INVALID_LOCAL_ID = 99999999;

inline bool IsInvalidInstID(InstID instID) { return instID == INVALID_INSTRUMENT_ID; }
inline constexpr InstID InstIDGen(Exchange exchange, int index) {
	return (((uint16_t)exchange) << 12) | ((uint16_t)index);
}
inline constexpr Exchange InstIdExchange(InstID instID) {
	return static_cast<Exchange>((instID & 0xF000) >> 12);
}
inline constexpr int InstIDIndex(InstID instID) {
	return instID & 0x0FFF;
}
inline bool IsINE(InstID instID) {
	return IsINE(InstIdExchange(instID));
}
inline bool IsSGE(InstID instID) {
	return IsSGE(InstIdExchange(instID));
}
inline bool IsSHFE(InstID instID) {
	return IsSHFE(InstIdExchange(instID));
}
inline bool IsCFFEX(InstID instID) {
	return IsCFFEX(InstIdExchange(instID));
}

typedef CharArray<31> InstCodeT;
typedef CharArray<40> SystemNoT;

typedef uint16_t QtyT;
typedef uint32_t TickT;
using UnitPrice = money;
