#include "pch.h"

#include "Marketing.h"
#include "model/Instrument.h"
#include "model/factory/InstrumentFactory.h"

namespace hf::netural {
	money HFInstruments::QuoteSellDelta() const {
		QuoteService& quoteService = WritableQuoteService();
		if (m_is_f) {
			return quoteService.GetSellPrice(Farther) - quoteService.GetSellPrice(Closer);
		} else {
			return quoteService.GetBuyPrice(Farther) - quoteService.GetBuyPrice(Closer);
		}
	}
	money HFInstruments::PriceSellDelta(money majorOrderPrice) const {
		QuoteService& quoteService = WritableQuoteService();
		if (m_is_f) {
			return majorOrderPrice - quoteService.GetSellPrice(Closer);
		} else {
			return quoteService.GetBuyPrice(Farther) - majorOrderPrice;
		}
	}
	money HFInstruments::QuoteBuyDelta() const {
		QuoteService& quoteService = WritableQuoteService();
		if (m_is_f) {
			return quoteService.GetBuyPrice(Farther) - quoteService.GetBuyPrice(Closer);
		} else {
			return quoteService.GetSellPrice(Farther) - quoteService.GetSellPrice(Closer);
		}
	}
	money HFInstruments::PriceBuyDelta(money majorOrderPrice) const {
		QuoteService& quoteService = WritableQuoteService();
		if (m_is_f) {
			return majorOrderPrice - quoteService.GetBuyPrice(Closer);
		} else {
			return quoteService.GetSellPrice(Farther) - majorOrderPrice;
		}
	}

	int HFInstruments::GetVOI(SimpleQuote const& lastAsk, SimpleQuote const& lastBid) {
		Quote const& majorQuote = WritableQuoteService().GetQuote(Major);

		int askDeltaVolumn = 0;
		auto currentAskPrice = majorQuote.GetSell().GetPrice();
		auto currentAskQty = majorQuote.GetSell().GetQuantity();
		auto lastAskPrice = lastAsk.GetPrice();
		auto lastAskQty = lastAsk.GetQuantity();
		if (currentAskPrice < lastAskPrice) {
			askDeltaVolumn = currentAskQty;
		}
		else if (currentAskPrice > lastAskPrice) {
			askDeltaVolumn = 0;
		}
		else {
			askDeltaVolumn = currentAskQty - lastAskQty;
		}

		int bidDeltaVolumn = 0;
		auto currentBidPrice = majorQuote.GetBuy().GetPrice();
		auto currentBidQty = majorQuote.GetBuy().GetQuantity();
		auto lastBidPrice = lastBid.GetPrice();
		auto lastBidQty = lastBid.GetQuantity();
		if (currentBidPrice < lastBidPrice) {
			bidDeltaVolumn = 0;
		}
		else if (currentAskPrice > lastAskPrice) {
			bidDeltaVolumn = currentBidQty;
		}
		else {
			bidDeltaVolumn = currentBidQty - lastBidQty;
		}

		return (bidDeltaVolumn - askDeltaVolumn);
	}
	money HFInstruments::GetMajorPriceBySellLine(money sellLine) const {
		QuoteService& quoteService = WritableQuoteService();
		auto unitPrice = quoteService.GetUnitPrice(Major);
		if (m_is_f) {
			auto majorSellPrice = quoteService.GetSellPrice(Closer) + sellLine;
			auto worstSellPrice = quoteService.GetBuyPrice(Farther) + unitPrice;	//最差卖价是对手买价+1跳
			return std::max(majorSellPrice, worstSellPrice);	//保证不与对价Cross
		}
		else {
			auto majorBuyPrice = quoteService.GetBuyPrice(Farther) - sellLine;
			auto worstBuyPrice = quoteService.GetSellPrice(Closer) - unitPrice;		//最差买价是对手卖价-1跳
			return std::min(majorBuyPrice, worstBuyPrice);		//保证不与对价Cross
		}
	}
	money HFInstruments::GetMajorPriceByBuyLine(money buyLine) const {
		QuoteService& quoteService = WritableQuoteService();
		auto unitPrice = quoteService.GetUnitPrice(Major);
		if (m_is_f) {
			auto majorBuyPrice = quoteService.GetBuyPrice(Closer) + buyLine;
			auto worstBuyPrice = quoteService.GetSellPrice(Farther) - unitPrice;
			return std::min(majorBuyPrice, worstBuyPrice);		//保证不与对价Cross
		}
		else {
			auto majorSellPrice = quoteService.GetSellPrice(Farther) - buyLine;
			auto worstSellPrice = quoteService.GetBuyPrice(Closer) + unitPrice;
			return std::max(majorSellPrice, worstSellPrice);	//保证不与对价Cross
		}
	}
	bool HFInstruments::IsPriceFit(Direction quoteDir, money price, int rangeTick) const {
		QuoteService& quoteService = WritableQuoteService();
		money unitPrice = quoteService.GetQuote(Major).GetUnitPrice();

		if (rangeTick < 0)	rangeTick = 0;
		money validZone = rangeTick * unitPrice;

		if (Direction::SELL == quoteDir) {	
			return price - validZone <= quoteService.GetSellPrice(Major);
		}
		else if (Direction::BUY == quoteDir) {	
			return price >= quoteService.GetBuyPrice(Major) - validZone;
		}		
		return true;
	}

	money HFSpread::GetUnitPrice() const {
		return InstrumentFactory::GetInstruments().Get(Instruments.Major).GetUnitPrice();
	}
	money HFSpread::GetLastPrice() const {
		return WritableQuoteService().GetLastPrice(Instruments.Major);
	}
	money HFSpread::GetFirstMajorQuote() const {
		return WritableQuoteService().GetPrice(Instruments.Major, Orders.Major.Direction);
	}
	money HFSpread::GetFirstReverseQuote() const {
		return WritableQuoteService().GetPrice(Instruments.Major, Orders.Reverse.Direction);
	}
	money HFSpread::GetBestMajorPrice(int selfVolume) const {
		QuoteService& quote = WritableQuoteService();
		money unitPrice = GetUnitPrice();
		money bestPrice = quote.GetPrice(Instruments.Major, Orders.Major.Direction);	// 本方盘口价
		money worstPrice = quote.GetPrice(Instruments.Major, Orders.Reverse.Direction);	// 对方盘口价
		QtyT bestVolume = quote.GetVolumn(Instruments.Major, Orders.Major.Direction);	// 本方盘口量
		if (IsMajorBuy()) {
			if (worstPrice - bestPrice > unitPrice/*盘口间有加价空间*/ && bestVolume > selfVolume/*防止盘口是自己的报价*/) {
				bestPrice += unitPrice;	// 盘口买价加一跳
			}
		} else {
			if (bestPrice - worstPrice > unitPrice/*盘口间有降价空间*/ && bestVolume > selfVolume/*防止盘口是自己的报价*/) {
				bestPrice -= unitPrice;	// 盘口卖价减一跳
			}
		}
		return bestPrice;
	}

	money HFSpread::GetMajorPriceByTick(Quote const& quote, int baseTick, int selfVolume, bool isDef) const {
		if (baseTick < 0) {
			return GetBestMajorPrice(selfVolume);	// 抢价需要防止First上的报价是自己
		}	// best

		money orderPrice = 0;
		money unitPrice = quote.GetUnitPrice();
		money ofrQuote = quote.GetSell().GetPrice();
		money bidQuote = quote.GetBuy().GetPrice();
		money QuoteSpacing = unitPrice * baseTick;
		if (isDef) {
			orderPrice = IsMajorBuy() ? bidQuote - unitPrice : ofrQuote + unitPrice;
		}
		else if (ofrQuote - bidQuote > QuoteSpacing) {
			orderPrice = IsMajorBuy() ? bidQuote + unitPrice : ofrQuote - unitPrice;
		}	// 盘口间距超出基础宽度，用Best报价
		else {
			orderPrice = IsMajorBuy() ? bidQuote : ofrQuote;
		}	// 盘口间距在基础宽度内，跟随盘口报价
		return orderPrice;
	}
	bool HFSpread::IsMajorPriceMeaningful(money majorPrice, int tolerantTick) const {
		if (0 == majorPrice) {
			return false;
		}

		QuoteService& quote = WritableQuoteService();
		money unitPrice = GetUnitPrice();
		money tolerantZone = unitPrice * tolerantTick;
		money firstPrice = quote.GetPrice(Instruments.Major, Orders.Major.Direction);	
		if (IsMajorBuy()) {
			return majorPrice >= firstPrice - tolerantZone;
		}
		else {
			return majorPrice <= firstPrice + tolerantZone;
		}
		return false;
	}

	money HFSpread::CalcFreezeFunds(Instrument const& instrument, money majorPrice, int quantity) const {
		return instrument.CalculateTotal(majorPrice, quantity, Orders.Major.Operation);	
	}
}
