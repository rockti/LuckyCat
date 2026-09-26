#pragma once

#include "model/Quote.h"
#include "service/Service.h"

class QuoteService : public ServiceBase<IQuoteObserver,IQuoteService>
{
public:
	QuoteService() {}
	virtual ~QuoteService(void) {}

	Quote const& Initialize(Quote const& quote)
	{
		int instIndex = InstIDIndex(quote.GetInstID());
		assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
		quotes_[instIndex] = quote;
		return quotes_[instIndex];
	}

	virtual Quote const& Update(Quote const& quote) override;
	virtual Quote const& UpdatePrice(type::InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, type::Money lastPrice) override;

	virtual QuoteResult Get(type::InstID instID) const override {
		int instIndex = InstIDIndex(instID);
		assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
		return quotes_[instIndex];
	}



private:
	Quote quotes_[MAX_INSTRUMENT_COUNT];
};

IQuoteService& WritableQuoteService();