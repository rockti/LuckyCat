#pragma once
#include "model/Constants.h"
#include "model/Quote.h"

namespace quoteArray {
	struct Price {
		TickT Tick = 0;
		QtyT Volume = 0;

		bool operator==(Price const& other) const {
			return Tick == other.Tick && Volume == other.Volume;
		}
		bool operator!=(Price const& other) const {
			return Tick != other.Tick || Volume != other.Volume;
		}
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

	inline Price price(SimpleQuoteArray const& priceArray, money unitPrice) {
		SimpleQuote const& quote1 = priceArray[0];
		SimpleQuote const& quote2 = priceArray[1];
		Price p;
		if (quote1.GetQuantity() < 3 && quote2.GetQuantity() > 0) {
			p.Tick = priceToTick(quote2, unitPrice);
			p.Volume = quote2.GetQuantity();
		} else {
			p.Tick = priceToTick(quote1, unitPrice);
			p.Volume = quote1.GetQuantity();
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
		p2.Volume = quote2.GetQuantity();
		p1.Tick = priceToTick(quote1, unitPrice);
		p1.Volume = quote1.GetQuantity();
		return { p1,p2 };
	}

	inline TwoTicks CovertQuoteToTwoTicks(Quote const& quote) {
		QuoteTicks ticks;
		auto [buy1, buy2] = twoTicks(quote.GetBuyPriceArray(), quote.GetUnitPrice());
		auto [sell1, sell2] = twoTicks(quote.GetSellPriceArray(), quote.GetUnitPrice());
		return { buy2, buy1 ,sell1, sell2 };
	}

}

