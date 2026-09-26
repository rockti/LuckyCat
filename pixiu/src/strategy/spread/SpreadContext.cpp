#include "pch.h"
#include "SpreadContext.h"
#include "service/QuoteService.h"
#include "model/Instrument.h"

namespace hf::spread {
	money SPInstruments::GetMultipleAvgPrice(Direction majorDir, QtyT majorVolume/*需求量*/, QtyT& actualVolume/*实际量*/) const {
		Quote const& majorQuote = WritableQuoteService().GetQuote(Major);
		SimpleQuoteArray const& quoteArrry = (Direction::SELL == majorDir) ? majorQuote.GetBuyPriceArray() : majorQuote.GetSellPriceArray();
	
		uint32_t sumQty = 0;		
		money lastPrice = 0.0;
		double turnover = 0.0;
		QtyT validVolume = (isMajorSpot) ? majorVolume * 10/*1:100*/ : majorVolume * 4/*1:40*/;	// 有效Mjaor量
		for (SimpleQuote quote : quoteArrry) {
			sumQty += quote.GetQuantity();
			if (lastPrice < 1) {
				if (sumQty >= validVolume && quote.GetPrice() > 1/*防止交割行情(有量无价)*/) {
					actualVolume = quote.GetQuantity();
					return quote.GetPrice();
				}	// 首档满足有效量, 直接返回首档价格
				turnover += (quote.GetPrice().value() * quote.GetQuantity());
			}
			else {
				money spaceDelta = Direction::SELL == majorDir ? lastPrice - quote.GetPrice() : quote.GetPrice() - lastPrice;
				if (spaceDelta > ValidSpace) {
					actualVolume = 0;
					return 0.0;
				}	// 档位间距过大，放弃

				turnover += (quote.GetPrice().value() * quote.GetQuantity());
				if (sumQty >= validVolume) {
					double tmpPrice = round((turnover / sumQty) * 100/*精确到百分位*/);
					actualVolume = majorVolume;
					return tmpPrice / 100;
				}	// 总量满足有效量，返回均价
			}
			lastPrice = quote.GetPrice();
		}

		actualVolume = 0;
		return 0.0;
	}

	money SPInstruments::GetMarketSellDelta(QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume, money& minorPrice, QtyT& minorVolume) const {
		QuoteService& quoteService = WritableQuoteService();
		if (multiple > 1 && Exchange::SGE == MajorExchange) {
			if (isMajorFarther) {
				SimpleQuote const& minorFirstQuote = quoteService.GetQuote(Closer).GetSell(0);
				SimpleQuote const& minorSecondQuote = quoteService.GetQuote(Closer).GetSell(1);
				money minorFirstPrice = minorFirstQuote.GetPrice();		// 卖0
				money minorSecondPrice = minorSecondQuote.GetPrice();	// 卖1
				if (minorSecondPrice - minorFirstPrice > ValidSpace) {
					minorVolume = 0;	// 保证委托量不够，不会发单
					return 0.0;
				}	// Minor 两档价位过大直接放弃FAK

				minorPrice = minorSecondPrice/*对着2档发单*/;
				minorVolume = minorFirstQuote.GetQuantity();
				majorPrice = GetMultipleAvgPrice(Direction::SELL, maxVolume * multiple, majorVolume);	// SGE major 看五档均价和总量					
				return majorPrice/*对价抛远*/ - minorFirstPrice/*对价买近*/;	// 价差对着一档判断
			}
			else {
				SimpleQuote const& minorFirstQuote = quoteService.GetQuote(Farther).GetBuy(0);
				SimpleQuote const& minorSecondQuote = quoteService.GetQuote(Farther).GetBuy(1);
				minorPrice = minorFirstQuote.GetPrice();				// 买0
				money minorSecondPrice = minorSecondQuote.GetPrice();	// 买1
				if (minorPrice - minorSecondPrice > ValidSpace) {
					minorVolume = 0;
					return 0.0;
				}	// Minor 两档价位过大直接放弃FAK

				majorPrice = GetMultipleAvgPrice(Direction::BUY, maxVolume * multiple, majorVolume);	// SGE major 看五档均价和总量	
				minorVolume = minorFirstQuote.GetQuantity();
				return minorPrice/*对价抛远*/ - majorPrice/*对价买近*/;
			}
		}
		else {
			if (isMajorFarther) {
				majorPrice = quoteService.GetBuyPrice(Farther);
				majorVolume = quoteService.GetBuyVolumn(Farther);
				minorPrice = quoteService.GetSellPrice(Closer);
				minorVolume = quoteService.GetSellVolumn(Closer);				
				return majorPrice/*对价抛远*/ - minorPrice/*对价买近*/;
			}
			else {
				minorPrice = quoteService.GetBuyPrice(Farther);
				minorVolume = quoteService.GetBuyVolumn(Farther);
				majorPrice = quoteService.GetSellPrice(Closer);
				majorVolume = quoteService.GetSellVolumn(Closer);				
				return minorPrice/*对价抛远*/ - majorPrice/*对价买近*/;
			}
		}
	}	
	money SPInstruments::GetMarketBuyDelta(QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume, money& minorPrice, QtyT& minorVolume) const {
		QuoteService& quoteService = WritableQuoteService();
		if (multiple > 1 && Exchange::SGE == MajorExchange) {
			if (isMajorFarther) {
				SimpleQuote const& minorFirstQuote = quoteService.GetQuote(Closer).GetBuy(0);
				SimpleQuote const& minorSecondQuote = quoteService.GetQuote(Closer).GetBuy(1);
				money minorFirstPrice = minorFirstQuote.GetPrice();
				money minorSecondPrice = minorSecondQuote.GetPrice();
				if (minorPrice - minorFirstPrice > ValidSpace) {
					minorVolume = 0;	// 保证委托量不够，不会发单
					return 0.0;
				}	// Minor 两档价位过大直接放弃FAK

				minorPrice = minorSecondPrice/*对着2档发单*/;
				minorVolume = minorFirstQuote.GetQuantity();
				majorPrice = GetMultipleAvgPrice(Direction::BUY, maxVolume * multiple, majorVolume);	// SGE major 看五档均价和总量	
				return majorPrice/*对价买远*/ - minorFirstPrice/*对价抛近*/;
			}
			else {
				SimpleQuote const& minorFirstQuote = quoteService.GetQuote(Closer).GetSell(0);
				SimpleQuote const& minorSecondQuote = quoteService.GetQuote(Closer).GetSell(1);
				minorPrice = minorFirstQuote.GetPrice();
				money minorSecondPrice = minorSecondQuote.GetPrice();
				if (minorSecondPrice - minorPrice > ValidSpace) {
					minorVolume = 0;
					return 0.0;
				}	// Minor 两档价位过大直接放弃FAK

				majorPrice = GetMultipleAvgPrice(Direction::SELL, maxVolume * multiple, majorVolume);	// major 看五档均价和总量	
				minorVolume = minorFirstQuote.GetQuantity();
				return minorPrice/*对价买远*/ - majorPrice/*对价抛近*/;
			}
		}
		else {
			if (isMajorFarther) {
				majorPrice = quoteService.GetSellPrice(Farther);
				majorVolume = quoteService.GetSellVolumn(Farther);
				minorPrice = quoteService.GetBuyPrice(Closer);
				minorVolume = quoteService.GetBuyVolumn(Closer);
				return majorPrice/*对价买远*/ - minorPrice/*对价抛近*/;
			}
			else {
				minorPrice = quoteService.GetSellPrice(Farther);
				minorVolume = quoteService.GetSellVolumn(Farther);
				majorPrice = quoteService.GetBuyPrice(Closer);
				majorVolume = quoteService.GetBuyVolumn(Closer);
				return minorPrice/*对价买远*/ - majorPrice/*对价抛近*/;
			}
		}
	}

	money SPInstruments::GetMinorPriceByUpperLine(money ofrLine, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const {
		if (multiple > 1 && Exchange::SGE == MajorExchange) {
			majorPrice = isMajorFarther ? GetMultipleAvgPrice(Direction::SELL/*抛远*/, maxVolume * multiple, majorVolume)
				: GetMultipleAvgPrice(Direction::BUY/*买近*/, maxVolume * multiple, majorVolume);
		}
		else {
			QuoteService& quoteService = WritableQuoteService();
			majorPrice = isMajorFarther ? quoteService.GetBuyPrice(Farther) : quoteService.GetSellPrice(Closer);
			majorVolume = isMajorFarther ? quoteService.GetBuyVolumn(Farther) : quoteService.GetSellVolumn(Closer);
		}
		return isMajorFarther ? majorPrice - ofrLine : majorPrice + ofrLine;
	}
	money SPInstruments::GetMinorPriceByLowerLine(money bidLine, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const {
		if (multiple > 1 && Exchange::SGE == MajorExchange) {
			majorPrice = isMajorFarther ? GetMultipleAvgPrice(Direction::BUY/*买远*/, maxVolume * multiple, majorVolume)
				: GetMultipleAvgPrice(Direction::SELL/*抛近*/, maxVolume * multiple, majorVolume);
		}
		else {
			QuoteService& quoteService = WritableQuoteService();
			majorPrice = isMajorFarther ? quoteService.GetSellPrice(Farther) : quoteService.GetBuyPrice(Closer);
			majorVolume = isMajorFarther ? quoteService.GetSellVolumn(Farther) : quoteService.GetBuyVolumn(Closer);
		}
		return isMajorFarther ? majorPrice - bidLine : majorPrice + bidLine;
	}

	money SPInstruments::GetSellDeltaByMinorPrice(money minorPrice, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const {
		if (multiple > 1 && Exchange::SGE == MajorExchange) {
			majorPrice = isMajorFarther ? GetMultipleAvgPrice(Direction::SELL/*抛远*/, maxVolume, majorVolume)
				: GetMultipleAvgPrice(Direction::BUY/*买近*/, maxVolume * multiple, majorVolume);
		}
		else {
			QuoteService& quoteService = WritableQuoteService();
			majorPrice = isMajorFarther ? quoteService.GetBuyPrice(Farther) : quoteService.GetSellPrice(Closer);
			majorVolume = isMajorFarther ? quoteService.GetBuyVolumn(Farther) : quoteService.GetSellVolumn(Closer);
		}
		return isMajorFarther ? majorPrice - minorPrice : minorPrice - majorPrice;
	}
	money SPInstruments::GetBuyDeltaByMinorPrice(money minorPrice, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const {
		if (multiple > 1 && Exchange::SGE == MajorExchange) {
			majorPrice = isMajorFarther ? GetMultipleAvgPrice(Direction::BUY/*买远*/, maxVolume, majorVolume)
				: GetMultipleAvgPrice(Direction::SELL/*抛近*/, maxVolume * multiple, majorVolume);
		}
		else {
			QuoteService& quoteService = WritableQuoteService();
			majorPrice = isMajorFarther ? quoteService.GetSellPrice(Farther) : quoteService.GetBuyPrice(Closer);
			majorVolume = isMajorFarther ? quoteService.GetSellVolumn(Farther) : quoteService.GetBuyVolumn(Closer);
		}
		return isMajorFarther ? majorPrice - minorPrice : minorPrice - majorPrice;
	}

	bool SPInstruments::IsPriceFit(InstID instID, Direction quoteDir, money price, bool isDeepQuote, int rangeTick) const {
		QuoteService& quoteService = WritableQuoteService();
		money unitPrice = quoteService.GetQuote(instID).GetUnitPrice();

		if (rangeTick < 0)	rangeTick = 0;
		money validZone = rangeTick * unitPrice;

		if (isDeepQuote) {
			if (Direction::SELL == quoteDir) {
				return price <= quoteService.GetQuote(instID).GetSell(rangeTick).GetPrice();
			}
			else {
				return price >= quoteService.GetQuote(instID).GetBuy(rangeTick).GetPrice();						
			}
		}
		else {
			if (Direction::SELL == quoteDir) {
				return price - validZone <= quoteService.GetSellPrice(instID);
			}
			else {
				return price >= quoteService.GetBuyPrice(instID) - validZone;
			}
		}
	}

	money DirectionalOrders::TryCalcOpenFreezeTotal(int quantity) const {
		QuoteService& quoteService = WritableQuoteService();
		auto const& major = InstrumentFactory::GetInstruments().Get(Instruments.Major);
		auto const& minor = InstrumentFactory::GetInstruments().Get(Instruments.Minor);

		money majorTotal = major.CalculateTotal(quoteService.GetSellPrice(Instruments.Major), quantity, Operation::OPEN);
		money minorTotal = minor.CalculateTotal(quoteService.GetSellPrice(Instruments.Minor), quantity, Operation::OPEN);
		return majorTotal + minorTotal;
	}
	money DirectionalOrders::CalcOpenFreezeMargin(int quantity) const {
		QuoteService& quoteService = WritableQuoteService();
		auto const& major = InstrumentFactory::GetInstruments().Get(Instruments.Major);
		auto const& minor = InstrumentFactory::GetInstruments().Get(Instruments.Minor);

		money majorFreeze = major.CalculateMargin(quoteService.GetSellPrice(Instruments.Major), quantity, Operation::OPEN);
		money minorFreeze = minor.CalculateMargin(quoteService.GetSellPrice(Instruments.Minor), quantity, Operation::OPEN);
		return std::max(majorFreeze, minorFreeze);
	}
	money DirectionalOrders::CalcOpenFreezeFee(int quantity) const {
		QuoteService& quoteService = WritableQuoteService();
		auto const& major = InstrumentFactory::GetInstruments().Get(Instruments.Major);
		auto const& minor = InstrumentFactory::GetInstruments().Get(Instruments.Minor);
		money majorFee = major.CalculateFee(quoteService.GetSellPrice(Instruments.Major), quantity, Operation::OPEN);
		money minorFee = minor.CalculateFee(quoteService.GetSellPrice(Instruments.Minor), quantity, Operation::OPEN);
		return majorFee + minorFee;
	}
}

