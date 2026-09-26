#pragma once
//行情相关的对象

#include "model/Instrument.h"
#include <array>
#include "type/Money.h"

// 只有盛立有简单行情
class SimpleQuote {
public:
	SimpleQuote(money price = 0.00, uint32_t quantity = 0)
		:m_price(price),m_quantity(quantity) {
	}

	SimpleQuote(SimpleQuote const& simpleQuote)
		:m_price(simpleQuote.GetPrice()),m_quantity(simpleQuote.GetQuantity()) {
	}

	money GetPrice() const { return m_price; }
	uint32_t GetQuantity() const { return m_quantity; }

	bool operator==(SimpleQuote const& other) const {
		return m_price == other.GetPrice() && m_quantity == other.GetQuantity();
	}

	bool operator!=(SimpleQuote const& other) const {
		return !operator==(other);
	}

private:
	money m_price;
	uint32_t m_quantity;
};

inline TickT priceToTick(money price, money unitPrice) {
	return TickT((price / unitPrice + 0.1).value());
}

inline TickT priceToTick(SimpleQuote const& price, money unitPrice) {
	return priceToTick(price.GetPrice(), unitPrice);
}

inline money tickToPrice(TickT tick, money unitPrice) {
	return tick * unitPrice;
}

typedef std::array<SimpleQuote,5> SimpleQuoteArray;

class Quote {
public:
	Quote()
		:m_instID(-1), m_instrument(0),
		m_buyPriceArray(), m_sellPriceArray(), m_openPrice(0.00), m_highestPrice(0.00), m_lowestPrice(0.00),
		m_upperLimit(0.00), m_lowerLimit(0.00), m_openInterest(0), m_lastPrice(0.00), m_lastSettlement(0.00),
		m_volume(0), m_turnover(0.0), m_isWholeMarket(false), m_minuteID(0), m_hidenBuyPrice(0.00), m_hidenSellPrice(0.00) {		
	}

	Quote(InstID instID, Instrument const* instrument, SimpleQuoteArray const& buyPriceArray, SimpleQuoteArray const& sellPriceArray,
		money openPrice, money highestPrice, money lowestPrice, money upperLimit, money lowerLimit, int openInterest, money lastPrice,
		money lastSettlement, int volumn, money turnover, bool isWholeMarket, int minuteID, SimpleQuote const& hidenBuy, SimpleQuote const& hidenSell)
		:m_instID(instID), m_instrument(instrument), m_buyPriceArray(buyPriceArray), m_sellPriceArray(sellPriceArray),
		m_openPrice(openPrice), m_highestPrice(highestPrice), m_lowestPrice(lowestPrice), m_upperLimit(upperLimit), m_lowerLimit(lowerLimit),
		m_openInterest(openInterest), m_lastPrice(lastPrice), m_lastSettlement(lastSettlement), m_volume(volumn), m_turnover(turnover),
		m_isWholeMarket(isWholeMarket), m_minuteID(minuteID), m_hidenBuyPrice(hidenBuy), m_hidenSellPrice(hidenSell) {
	}

	Quote(Quote const& quote)
		:m_instID(quote.GetInstID()), m_instrument(quote.m_instrument),
		m_buyPriceArray(quote.GetBuyPriceArray()), m_sellPriceArray(quote.GetSellPriceArray()),
		m_openPrice(quote.GetOpenPrice()), m_highestPrice(quote.GetHighestPrice()), m_lowestPrice(quote.GetLowestPrice()),
		m_upperLimit(quote.GetUpperLimit()), m_lowerLimit(quote.GetLowerLimit()), m_openInterest(quote.GetOpenInterest()),
		m_lastPrice(quote.GetLastPrice()), m_lastSettlement(quote.GetLastSettlement()), m_volume(quote.GetVolume()),
		m_turnover(quote.GetTurnover()), m_isWholeMarket(quote.IsWholeMarket()), m_minuteID(quote.GetMinuteID()),
		m_hidenBuyPrice(quote.GetHidenBuy()), m_hidenSellPrice(quote.GetHidenSell()) {
	}

public:
	inline InstID GetInstID() const { return m_instID; }
	inline InstCodeT const& GetInstCode() const { return m_instrument->GetInstCode(); }
	inline Instrument const& GetInstrument() const { return *m_instrument; }
	inline SimpleQuoteArray const& GetBuyPriceArray() const { return m_buyPriceArray; }
	inline SimpleQuoteArray const& GetSellPriceArray() const { return m_sellPriceArray; }
	inline SimpleQuote const& GetHidenBuy() const { return m_hidenBuyPrice; }
	inline SimpleQuote const& GetHidenSell() const { return m_hidenSellPrice; }
	inline money GetOpenPrice() const { return m_openPrice; }
	inline money GetHighestPrice() const { return m_highestPrice; }
	inline money GetLowestPrice() const { return m_lowestPrice; }
	inline money GetUpperLimit() const { return m_upperLimit; }
	inline money GetLowerLimit() const { return m_lowerLimit; }
	inline int GetOpenInterest() const { return m_openInterest; };
	inline money GetLastPrice() const { return m_lastPrice; }
	inline money GetLastSettlement() const { return m_lastSettlement; }
	inline int GetVolume() const { return m_volume; }
	inline money GetTurnover() const { return m_turnover; }
	inline int GetMinuteID() const { return m_minuteID; }
	inline money GetUnitPrice() const { return m_instrument->GetUnitPrice(); }

	inline bool IsValid() const { return m_lastPrice > 0.1 && m_lastPrice < 9999999; }

	inline SimpleQuote const& GetBuy(int i=0) const { return m_buyPriceArray[i]; }
	inline SimpleQuote const& GetSell(int i=0) const { return m_sellPriceArray[i]; }

	//盛立行情使用接口
	inline bool IsWholeMarket() const { return m_isWholeMarket; }

public:
	void UpdateQuote(Quote const& quote) {
		m_openPrice = quote.GetOpenPrice();
		m_buyPriceArray = quote.GetBuyPriceArray();
		m_sellPriceArray = quote.GetSellPriceArray();
		m_highestPrice = quote.GetHighestPrice();
		m_lowestPrice = quote.GetLowestPrice();
		m_upperLimit = quote.GetUpperLimit();
		m_lowerLimit = quote.GetLowerLimit();
		m_openInterest = quote.GetOpenInterest();
		m_lastPrice = quote.GetLastPrice();
		m_lastSettlement = quote.GetLastSettlement();
		m_volume = quote.GetVolume();
		m_turnover = quote.GetTurnover();
		m_isWholeMarket = true;
		m_hidenBuyPrice = quote.GetHidenBuy();
		m_hidenSellPrice = quote.GetHidenSell();
		m_minuteID = quote.GetMinuteID();
	}
	void UpdatePrice(InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, money lastPrice, int volume, int interest, int id) {
		m_buyPriceArray = buyArray;
		m_sellPriceArray = sellArray;
		m_lastPrice = lastPrice;
		m_volume = volume;
		m_openInterest = interest;
		m_minuteID = id;
	}
	bool IsCloseToLimit() const {
		const money CLOSE_TO_PERCENT = 0.003;
		bool isCloseToUpperLimit = GetLastPrice() >= (GetUpperLimit()-GetUpperLimit()*CLOSE_TO_PERCENT);
		bool isCloseToLowerLimit = GetLastPrice() <= (GetLowerLimit()+GetLowerLimit()*CLOSE_TO_PERCENT);
		return isCloseToUpperLimit || isCloseToLowerLimit;
	}

	money CalculateTotalFreeze(money price, int quantity, Operation operation) const {
		return CalculateMargin(price, quantity, operation) + CalculateFee(price, quantity, operation);
	}
	money CalculateMargin(money price, int quantity, Operation operation) const {
		return GetInstrument().CalculateMargin(price, quantity, operation);
	}
	money CalculateFee(money price, int quantity, Operation operation) const {
		return GetInstrument().CalculateFee(price, quantity, operation);
	}

private:
	InstID m_instID;
 	Instrument const* m_instrument;
	SimpleQuoteArray m_buyPriceArray;
	SimpleQuoteArray m_sellPriceArray;
	SimpleQuote m_hidenBuyPrice;
	SimpleQuote m_hidenSellPrice;
	money m_openPrice;
	money m_highestPrice;			// 最高价
	money m_lowestPrice;			// 最低价
	money m_upperLimit;
	money m_lowerLimit;
	int m_openInterest;				// 持仓量
	money m_lastPrice;				// 最新价
	money m_lastSettlement;			// 昨结算
	int m_volume;					// 成交量
	money m_turnover;				// 成交额
	bool m_isWholeMarket;
	int m_minuteID;					// 分钟数
};

//只针对上海进行过测试，中金所的跌停算法和上海不同
class LimitCalculator4Shfe {
public:
	explicit LimitCalculator4Shfe(int preSettlementPrice, int unit, double percent)
		:m_upperLimit(0),m_lowerLimit(0)
	{
		int upperTemp = int(preSettlementPrice * (1+percent));
		m_upperLimit = upperTemp - upperTemp%unit;

		int lowerTemp = int(preSettlementPrice * (1-percent));
		m_lowerLimit = lowerTemp - lowerTemp%unit;
	}

	int GetUpperLimit() const { return m_upperLimit; }
	int GetLowerLimit() const { return m_lowerLimit; }
private:
	int m_upperLimit;
	int m_lowerLimit;
};

//*******************************************K线******************************************//
struct KLine {
	KLine() {
		Init();
	}

	void Init() {
		memset(this, 0, sizeof(KLine));
	}

	money minPrice;
	money maxPrice;
	money openPrice;
	money closePrice;
	money averageGrowth;
	int volume;
	int direction;		//-1:Bear;	0:Stable;	1:Bull
};