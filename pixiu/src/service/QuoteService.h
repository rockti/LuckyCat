#pragma once

#include "model/Quote.h"
#include "utils/Log.h"

class QuoteService {
public:
	QuoteService(void) {}
	~QuoteService(void) {}

	Quote const& Initialize(Quote const& quote) {
		int instIndex = InstIDIndex(quote.GetInstID());
		assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
		m_quotes[instIndex] = quote;
		return m_quotes[instIndex];
	}
	inline Quote const& GetQuote(InstID instID) const {
		int instIndex = InstIDIndex(instID);
		assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
		return m_quotes[instIndex];
	}
	inline Quote& GetQuote(InstID instID) {
		int instIndex = InstIDIndex(instID);
		assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
		return m_quotes[instIndex];
	}

	money GetSellPrice(InstID instID) const {
		return GetQuote(instID).GetSell().GetPrice();
	}
	int GetSellVolumn(InstID instID) const {
		return GetQuote(instID).GetSell().GetQuantity();
	}

	money GetBuyPrice(InstID instID) const {
		return GetQuote(instID).GetBuy().GetPrice();
	}
	int GetBuyVolumn(InstID instID) const {
		return GetQuote(instID).GetBuy().GetQuantity();
	}

	money GetUnitPrice(InstID instID) const {
		return GetQuote(instID).GetUnitPrice();
	}
	money GetLastPrice(InstID instID) const {
		return GetQuote(instID).GetLastPrice();
	}
	
	int GetVolumn(InstID instID) const {
		return GetQuote(instID).GetVolume();
	}
	
	money GetPrice(InstID instID, Direction direct) const {
		return direct == Direction::BUY ? GetBuyPrice(instID) : GetSellPrice(instID);
	}
	int GetVolumn(InstID instID, Direction direct) const {
		return direct == Direction::BUY ? GetBuyVolumn(instID) : GetSellVolumn(instID);
	}

// 	void UpdateQuote(Quote const& quote);
// 	Quote const& UpdatePrice(InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, money lastPrice);

private:
	Behavior GetBehavior(Quote const& prev, Quote const& current);

private:
	Quote m_quotes[MAX_INSTRUMENT_COUNT];
};

QuoteService& WritableQuoteService();