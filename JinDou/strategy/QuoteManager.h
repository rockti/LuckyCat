#pragma once
#include "model/Constants.h"
#include "model/Quote.h"
namespace quote {
	struct Price {
		TickT Tick = 0;
		QtyT Position = 0;
	};

	struct QuoteTicks {
		Price Buy;
		Price Sell;
	};

	struct TwoTicks {
		Price Buy2;
		Price Buy;
		Price Sell;
		Price Sell2;
	};


	inline TickT priceToTick(SimpleQuote const& price, money unitPrice) {
		return int((price.GetPrice() / unitPrice + 0.1).value());
	}

	inline TickT priceToTick(money price, money unitPrice) {
		return int((price / unitPrice + 0.1).value());
	}

	inline Price price(SimpleQuoteArray const& priceArray, money unitPrice) {
		SimpleQuote const& quote1 = priceArray[0];
		SimpleQuote const& quote2 = priceArray[1];
		Price p;
		if (quote1.GetQuantity() < 3 && quote2.GetQuantity() > 0) {
			p.Tick = priceToTick(quote2, unitPrice);
			p.Position = quote2.GetQuantity();
		} else {
			p.Tick = priceToTick(quote1, unitPrice);
			p.Position = quote1.GetQuantity();
		}
		return p;
	}

	inline QuoteTicks CovertQuoteToTick(Quote const& quote) {
		QuoteTicks ticks;
		ticks.Buy = price(quote.GetBuyPriceArray(), quote.GetUnitPrice());
		ticks.Sell = price(quote.GetSellPriceArray(), quote.GetUnitPrice());
		return ticks;
	}

	inline std::tuple<Price, Price> twoTicks(SimpleQuoteArray const& priceArray, money unitPrice) {
		SimpleQuote const& quote1 = priceArray[0];
		SimpleQuote const& quote2 = priceArray[1];
		Price p1;
		Price p2;
		p2.Tick = priceToTick(quote2, unitPrice);
		p2.Position = quote2.GetQuantity();
		p1.Tick = priceToTick(quote1, unitPrice);
		p1.Position = quote1.GetQuantity();
		return { p1,p2 };
	}

	inline TwoTicks CovertQuoteToTwoTicks(Quote const& quote) {
		QuoteTicks ticks;
		auto [buy1, buy2] = twoTicks(quote.GetBuyPriceArray(), quote.GetUnitPrice());
		auto [sell1, sell2] = twoTicks(quote.GetSellPriceArray(), quote.GetUnitPrice());
		return { buy2, buy1 ,sell1, sell2 };
	}

}

