#pragma once

#include "model/Constants.h"
#include "model/dealing/OrderCommand.h"
#include "service/QuoteService.h"
#include <optional>
#include "utils/Log.h"


#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>

class Instrument;
namespace hf::netural {
	struct HFOrderCommands {		//委托组合
		OrderCommand Major;			//主动腿
		OrderCommand Reverse;		//反向腿
		bool operator==(HFOrderCommands const& other) const {
			return other.Major == Major && other.Reverse == Reverse;
		}
	};
	
	const HFOrderCommands MajorSellOpenOrders{ SellOpenNormal , BuyOpenNormal};
	const HFOrderCommands MajorSellOffsetOrders{ SellOffsetNormal , BuyOffsetNormal};

	const HFOrderCommands MajorBuyOpenOrders{ BuyOpenNormal , SellOpenNormal};
	const HFOrderCommands MajorBuyOffsetOrders{ BuyOffsetNormal , SellOffsetNormal};

	// 合约组合：Orders，Delta，Price
	class HFInstruments {
	public:
		HFInstruments(int major, int hedge) :Major(major), Hedge(hedge) {
			Farther = std::max(major, hedge);
			Closer = std::min(major, hedge);
			m_is_f = (Farther == Major) ? true : false;
		}
		HFInstruments(HFInstruments const& other) = default;
		HFInstruments& operator=(HFInstruments const& other) = default;
		bool operator==(HFInstruments const& other) const {
			return other.Major == Major && other.Hedge == Hedge;
		}

		//卖价差Orders：买近抛远->[远月卖Orders，近月买Orders] [只看买卖方向，无所谓开仓还是平仓]
		HFOrderCommands GetSellSpreadOpenOrders() const { return m_is_f ? MajorSellOpenOrders : MajorBuyOpenOrders; }
		HFOrderCommands GetSellSpreadOffsetOrders() const { return m_is_f ? MajorSellOffsetOrders : MajorBuyOffsetOrders; }

		//买价差Orders：抛近买远->[远月买开Orders，近月卖开Orders]，[只看买卖方向，无所谓开仓还是平仓]
		HFOrderCommands GetBuySpreadOpenOrders() const { return m_is_f ? MajorBuyOpenOrders : MajorSellOpenOrders; }
		HFOrderCommands GetBuySpreadOffsetOrders() const { return m_is_f ? MajorBuyOffsetOrders : MajorSellOffsetOrders; }

		//行情计算的价差
		money QuoteSellDelta() const;	
		money QuoteBuyDelta() const;
		//委托价计算的价差
		money PriceSellDelta(money majorOrderPrice) const;
		money PriceBuyDelta(money majorOrderPrice) const;
		
		// VOI(Volume Order Imbalance)量价不平衡
		// 订单流不平衡测量了在研究的特定时间内，最优买卖价格上委托量的增量之差，反映了投资行为在最优买卖价格上的供需情况
		int GetVOI(SimpleQuote const& lastAsk, SimpleQuote const& lastBid);

		//根据设定价差倒算出来的委托价
		money GetMajorPriceBySellLine(money sellLine) const;
		money GetMajorPriceByBuyLine(money buyLine) const;

		// 有等号，判断条件为：rangeTick+1
		bool IsPriceFit(Direction quoteDir, money price, int rangeTick) const;

		bool IsMajorFarther() const { return m_is_f; }
	public:
		InstID Major;
		InstID Hedge;
		InstID Farther;
		InstID Closer;
	private:
		bool m_is_f;
	};

	// 带方向和操作的合约组合
	struct HFSpread {
		HFSpread(HFInstruments const& instruments, HFOrderCommands const& orders, Direction spreadDir)
			:Instruments(instruments), Orders(orders), SpreadDir(spreadDir) {
		}
		HFSpread& operator=(HFSpread const& other) = default;
		bool operator==(HFSpread const& other) const {
			return other.Instruments == Instruments && other.Orders == Orders && SpreadDir == other.SpreadDir;
		}
		HFInstruments Instruments;
		HFOrderCommands Orders;
		Direction SpreadDir;		//价差方向

		bool IsMajorBuy() const { return Orders.Major.Direction == Direction::BUY; }
		inline Direction GetMajorDirection() const { return Orders.Major.Direction; }
		inline Operation GetMajorOperation() const { return Orders.Major.Operation; }

		money GetUnitPrice() const;
		money GetLastPrice() const;
		money GetFirstMajorQuote() const;					// 最优本方价
		money GetFirstReverseQuote() const;					// 最优对方价
		money GetBestMajorPrice(int selfVolume) const;		// 最优市场价(优于等于盘口)

		money GetMajorPriceByTick(Quote const& quote, int baseTick, int selfVolume, bool isDef) const;		// 根据对价确定委托价
		//这里直接算掉，不再丢到Instruments中取了
		money GetReversePriceByQuote(int gainTick) const {
			// 盘口平仓是比对价盘口让一跳
			money worstReversePrice = IsMajorBuy() ? GetFirstReverseQuote() - GetUnitPrice() : GetFirstReverseQuote() + GetUnitPrice();
			// 预计平仓是本方盘口顶开
			money predictReversePrice = IsMajorBuy() ? GetFirstMajorQuote() + (GetUnitPrice() * gainTick) : GetFirstMajorQuote() - (GetUnitPrice() * gainTick);
			if (IsMajorBuy()) {
				return std::max(worstReversePrice, predictReversePrice);;
			}
			else {
				return std::min(worstReversePrice, predictReversePrice);
			}
		}	// 根据Major最新盘口价算出Rvs价
		money GetReversePriceByTrade(money tradePrice, int gainTick) const {
			return IsMajorBuy() ? tradePrice + (GetUnitPrice() * gainTick) : tradePrice - (GetUnitPrice() * gainTick);
		}	// 根据Major成交价算出Rvs报价（盈利最大，未必成交）

		money CalcFreezeFunds(Instrument const& instrument, money majorPrice, int quantity) const;

		// 有等号，判断条件为：tolerantTick+1
		bool IsMajorPriceMeaningful(money majorPrice, int tolerantTick) const;					// 是否有效报价
	};

	class HFLines {
	public:
		HFLines(HFInstruments const& instruments, money sellDelta, money sellHedgeDelta, money buyHedgeDelta, money buyDelta)
			:m_instruments(instruments), sellDelta(sellDelta), sellHedgeDelta(sellHedgeDelta), buyDelta(buyDelta), buyHedgeDelta(buyHedgeDelta) {
		}
		HFLines& operator=(HFLines const& other) = default;
		money sellDelta;
		money sellHedgeDelta;
		money buyHedgeDelta;
		money buyDelta;
		
		HFSpread GetSellOpenSpread() const { return HFSpread{ m_instruments, m_instruments.GetSellSpreadOpenOrders(), Direction::SELL}; }
		HFSpread GetSellOffsetSpread() const { return HFSpread{ m_instruments, m_instruments.GetSellSpreadOffsetOrders(), Direction::SELL}; }
		HFSpread GetBuyOpenSpread() const { return HFSpread(m_instruments, m_instruments.GetBuySpreadOpenOrders(), Direction::BUY); }
		HFSpread GetBuyOffsetSpread() const { return HFSpread(m_instruments, m_instruments.GetBuySpreadOffsetOrders(), Direction::BUY); }

		money CurrentSellDelta(money majorPrice) const { return m_instruments.PriceSellDelta(majorPrice); }	//根据价格算的价差
		money CurrentBuyDelta(money majorPrice) const { return m_instruments.PriceBuyDelta(majorPrice); }	//根据价格算的价差

		money SellLineMajorPrice() const { return m_instruments.GetMajorPriceBySellLine(sellDelta); }		//根据价差算的价格
		money BuyLineMajorPrice() const { return m_instruments.GetMajorPriceByBuyLine(buyDelta); }			//根据价差算的价格

		bool IsPriceFit(Direction quoteDir, money price, int tick) const { return m_instruments.IsPriceFit(quoteDir, price, tick); }
	private:
		HFInstruments m_instruments;
	};
}

