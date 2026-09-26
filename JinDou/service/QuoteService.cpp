#include "service/QuoteService.h"
#include "utils/Log.h"


Quote const& QuoteService::Update(Quote const& quote) {
	int instIndex = quote.GetInstID();
	assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
	quotes_[instIndex].UpdateQuote(quote);
	for (auto& observer : observers_) {
		observer->OnQuote(quotes_[instIndex]);
	}
	return quotes_[instIndex];
}
Quote const& QuoteService::UpdatePrice(type::InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, type::Money lastPrice) {
	int instIndex = instID;

	assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
	quotes_[instIndex].UpdatePrice(buyArray, sellArray, lastPrice);
	for (auto& observer : observers_) {
		observer->OnQuote(quotes_[instIndex]);
	}
	return quotes_[instIndex];
}


IQuoteService& WritableQuoteService() {
	static IQuoteService* s_quoteSnapshot;
	if (NULL == s_quoteSnapshot) {
		s_quoteSnapshot = new QuoteService();
	}
	return *s_quoteSnapshot;
}
