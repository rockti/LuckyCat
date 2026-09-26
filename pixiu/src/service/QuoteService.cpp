#include "pch.h"

#include "service/QuoteService.h"
#include "utils/Log.h"


// void QuoteService::UpdateQuote(Quote const& quote) {
// 	int instIndex = InstIDIndex(quote.GetInstID());
// 	assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
// 	m_quotes[instIndex].UpdateQuote(quote);
// }
// Quote const& QuoteService::UpdatePrice(InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, money lastPrice) {
// 	int instIndex = InstIDIndex(instID);
// 	assert(instIndex >= 0 && instIndex < MAX_INSTRUMENT_COUNT);
// 	return m_quotes[instIndex].UpdatePrice(buyArray, sellArray, lastPrice);
// }

//确定多空的依据是最新价和买卖价比较，确定开平仓和换手的依据是持仓变化
//接近于买卖价与最新价涨跌无关，可以接近买价不断提升，也可以卖价不断下压
Behavior QuoteService::GetBehavior(Quote const& preMarket, Quote const& currentMarket) {
	if (0 == currentMarket.GetVolume()) {
		return Behavior::UNKNOWN;
	}
	else {
		money sellPrice = currentMarket.GetSell().GetPrice();
		money lastPrice = currentMarket.GetLastPrice();
		money buyPrice = currentMarket.GetBuy().GetPrice();

		money ofrSpread = sellPrice - lastPrice;	// OFR 离最新的距离
		money bidSpread = lastPrice - buyPrice;		// BID 离最新的距离
		int volumnChange = currentMarket.GetVolume() - preMarket.GetVolume();
		int positionChange = currentMarket.GetOpenInterest() - preMarket.GetOpenInterest();
		if (positionChange == 0) {
			return Behavior::EXCHANGE;			// EXCHANGE（换手）：持仓不变
		}	// 换手
		else if (positionChange > 0) {
			if (ofrSpread < bidSpread) {
				return Behavior::BID_OPEN;		// BID_OPEN（多开）：增仓为正值且最新价接近卖价
			}
			else if (bidSpread < ofrSpread) {
				return Behavior::BEAR_OPEN;		// BEAR_OPEN（空开）：增仓为正值且最新价接近买价
			} 
			else if (ofrSpread == bidSpread || positionChange == volumnChange) {
				return Behavior::BOTH_OPEN;		// BOTH_OPEN（双开）：增仓数量等于现手数量 或 最新价在买卖价当中
			}
		}	// 增仓
		else {
			if (ofrSpread < bidSpread) {
				return Behavior::BID_OFFSET;	// BULL_OFFSET（多平）：增仓为负值且最新价接近卖价
			}
			else if (bidSpread < ofrSpread) {
				return Behavior::BEAR_OFFSET;	// BEAR_OFFSET（空平）：增仓为负值且最新价接近买价
			}
			else if (ofrSpread == bidSpread || -positionChange == volumnChange) {
				return Behavior::BOTH_OFFSET;	// BOTH_OFFSET（双平）：增仓数量等于负的现手数量 或 最新价在买卖价当中
			}
		}	// 减仓
	}
	/*
	fmt::memory_buffer out;
	format_to(out, "MarketStorage::GetBehavior() UNKNOWN! LastPrice[%.2f],SellPrice[%.2f],BuyPrice[%.2f],VolumnChanged[%d],PositionChanged[%.2f],PreMarketTime[%s],MarketTime[%s]",
		currentMarket.GetLastPrice(),
		currentMarket.GetSell().GetPrice(),
		currentMarket.GetBuy().GetPrice(),
		(currentMarket.GetVolumn() - preMarket.GetVolumn()),
		(currentMarket.GetOpenInterest() - preMarket.GetOpenInterest()),
		preMarket.GetTime().Format("%H::%M::%S"),
		currentMarket.GetTime().Format("%H:%M:%S"));
	;

	spdlog::warn(out.data());
	*/
	return Behavior::UNKNOWN;
}


QuoteService& WritableQuoteService() {
	static QuoteService* s_quoteSnapshot;
	if (NULL == s_quoteSnapshot) {
		s_quoteSnapshot = new QuoteService();
	}
	return *s_quoteSnapshot;
}
