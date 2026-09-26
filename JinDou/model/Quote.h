#pragma once
//行情相关的对象

#include "model/Instrument.h"
#include <array>
#include "type/ZTPType.h"
using namespace ztp;

// 只有盛立有简单行情
class SimpleQuote
{
public:
	SimpleQuote(type::Price price = 0.00, uint32_t quantity = 0)
		:m_price(price),m_quantity(quantity)
	{
	}

	SimpleQuote(SimpleQuote const& simpleQuote)
		:m_price(simpleQuote.GetPrice()),m_quantity(simpleQuote.GetQuantity())
	{
	}

	type::Price GetPrice() const { return m_price; }
	uint32_t GetQuantity() const { return m_quantity; }

	bool operator==(SimpleQuote const& other) const
	{
		return m_price == other.GetPrice() && m_quantity == other.GetQuantity();
	}

	bool operator!=(SimpleQuote const& other) const 
	{
		return !operator==(other);
	}

private:
	type::Price m_price;
	uint32_t m_quantity;
};


typedef std::array<SimpleQuote,5> SimpleQuoteArray;

class Quote
{
public:
	Quote()
		:m_instID(-1)
		,m_buyPriceArray(),m_sellPriceArray()
		,m_openPrice(0.00),m_highestPrice(0.00),m_lowestPrice(0.00)
		,m_upperLimit(0.00),m_lowerLimit(0.00),m_openInterest(0)
		,m_lastPrice(0.00),m_lastSettlement(0.00),m_volume(0)
		,m_isWholeMarket(false)
		,m_behavior(Behavior::UNKNOWN)
		,m_settlementUpperLimit(0),m_settlementLowerLimit(0)
		,m_hidenBuyPrice(),m_hidenSellPrice()
	{
	}

	Quote(type::InstID instID, SimpleQuoteArray const& buyPriceArray, SimpleQuoteArray const& sellPriceArray
		,type::Price openPrice, type::Price highestPrice, type::Price lowestPrice, type::Price upperLimit, type::Price lowerLimit, int openInterest, type::Price lastPrice, type::Price lastSettlement
		,int volumn, bool isWholeMarket, SimpleQuote const& hidenBuy, SimpleQuote const& hidenSell)
		:m_instID(instID),m_buyPriceArray(buyPriceArray),m_sellPriceArray(sellPriceArray)
		,m_openPrice(openPrice),m_highestPrice(highestPrice),m_lowestPrice(lowestPrice),m_upperLimit(upperLimit),m_lowerLimit(lowerLimit)
		,m_openInterest(openInterest),m_lastPrice(lastPrice),m_lastSettlement(lastSettlement),m_volume(volumn),m_isWholeMarket(isWholeMarket)
		,m_hidenBuyPrice(hidenBuy),m_hidenSellPrice(hidenSell)
	{
		m_behavior = Behavior::UNKNOWN;
	}

	Quote(Quote const& quote)
		:m_instID(quote.GetInstID())
		,m_buyPriceArray(quote.GetBuyPriceArray()),m_sellPriceArray(quote.GetSellPriceArray())
		,m_openPrice(quote.GetOpenPrice())
		,m_highestPrice(quote.GetHighestPrice()),m_lowestPrice(quote.GetLowestPrice())
		,m_upperLimit(quote.GetUpperLimit()),m_lowerLimit(quote.GetLowerLimit())
		,m_openInterest(quote.GetOpenInterest()),m_lastPrice(quote.GetLastPrice()),m_lastSettlement(quote.GetLastSettlement())
		,m_volume(quote.GetVolumn())
		,m_isWholeMarket(quote.IsWholeMarket()),m_behavior(quote.GetBehavior())
		,m_hidenBuyPrice(quote.GetHidenBuy()),m_hidenSellPrice(quote.GetHidenSell())
	{
	}

public:
	inline type::InstID GetInstID() const { return m_instID; }
	inline SimpleQuoteArray const& GetBuyPriceArray() const { return m_buyPriceArray; }
	inline SimpleQuoteArray const& GetSellPriceArray() const { return m_sellPriceArray; }
	inline SimpleQuote const& GetHidenBuy() const { return m_hidenBuyPrice; }
	inline SimpleQuote const& GetHidenSell() const { return m_hidenSellPrice; }
	inline type::Price GetOpenPrice() const { return m_openPrice; }
	inline type::Price GetHighestPrice() const { return m_highestPrice; }
	inline type::Price GetLowestPrice() const { return m_lowestPrice; }
	inline type::Price GetUpperLimit() const { return m_upperLimit; }
	inline type::Price GetLowerLimit() const { return m_lowerLimit; }
	inline int GetOpenInterest() const { return m_openInterest; };
	inline type::Price GetLastPrice() const { return m_lastPrice; }
	inline type::Price GetLastSettlement() const { return m_lastSettlement; }
	inline int GetVolumn() const { return m_volume; }

	inline bool IsValid() const { return m_lastPrice > 0.1; }

	inline SimpleQuote const& GetBuy(int i=0) const { return m_buyPriceArray[i]; }
	inline SimpleQuote const& GetSell(int i=0) const { return m_sellPriceArray[i]; }

	//盛立行情使用接口
	inline bool IsWholeMarket() const { return m_isWholeMarket; }

	inline Behavior GetBehavior() const { return m_behavior; }
	inline 	void SetBehavior(Behavior behavior) { m_behavior = behavior; }

	//集合竞价使用接口
	inline type::Price GetSettlementUpperLimit() const { return m_settlementUpperLimit; }
	inline type::Price GetSettlementLowerLimit() const { return m_settlementLowerLimit; }
	inline void SetSettlementUpperLimit(type::Price settlementUpperLimit) { m_settlementUpperLimit = settlementUpperLimit; }
	inline void SetSettlementLowerLimit(type::Price settlementLowerLimit) { m_settlementLowerLimit = settlementLowerLimit; }

public:
	void UpdateQuote(Quote const& quote)
	{
		m_instID = quote.GetInstID();
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
		m_volume = quote.GetVolumn();
		m_isWholeMarket = true;
		m_hidenBuyPrice = quote.GetHidenBuy();
		m_hidenSellPrice = quote.GetHidenSell();
		m_behavior = quote.GetBehavior();
	}
	Quote& UpdatePrice(SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, type::Money lastPrice)
	{
		m_buyPriceArray = buyArray;
		m_sellPriceArray = sellArray;
		m_lastPrice = lastPrice;
		return *this;		//更新完价格返回完整的行情
	}
	bool IsCloseToLimit() const
	{
		const type::Money CLOSE_TO_PERCENT = 0.003;
		bool isCloseToUpperLimit = GetLastPrice() >= (GetUpperLimit()-GetUpperLimit()*CLOSE_TO_PERCENT);
		bool isCloseToLowerLimit = GetLastPrice() <= (GetLowerLimit()+GetLowerLimit()*CLOSE_TO_PERCENT);
		return isCloseToUpperLimit || isCloseToLowerLimit;
	}


private:
	type::InstID m_instID;
	SimpleQuoteArray m_buyPriceArray;
	SimpleQuoteArray m_sellPriceArray;
	SimpleQuote m_hidenBuyPrice;
	SimpleQuote m_hidenSellPrice;
	type::Price m_openPrice;
	type::Price m_highestPrice;
	type::Price m_lowestPrice;
	type::Price m_upperLimit;
	type::Price m_lowerLimit;
	int m_openInterest;				//持仓量
	type::Price m_lastPrice;				//最新价
	type::Price m_lastSettlement;			//昨结算
	int m_volume;					//成交量
	Behavior m_behavior;
	bool m_isWholeMarket;

	type::Price m_settlementUpperLimit;
	type::Price m_settlementLowerLimit;
};

//只针对上海进行过测试，中金所的跌停算法和上海不同
class LimitCalculator4Shfe
{
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
struct KLine
{
	KLine()
	{
		Init();
	}

	void Init()
	{
		memset(this, 0, sizeof(KLine));
	}

	type::Money minPrice;
	type::Money maxPrice;
	type::Money openPrice;
	type::Money closePrice;
	type::Money averageGrowth;
	int volume;
	int direction;		//-1:Bear;	0:Stable;	1:Bull
};