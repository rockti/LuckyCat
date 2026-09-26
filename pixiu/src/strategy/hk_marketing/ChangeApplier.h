#pragma once
#include "HKMarketingStrategy.h"

namespace hk {


	inline QuoteTicks CovertQuoteToTick(Quote const& quote) {
		QuoteTicks ticks;
		SimpleQuote const& buy = quote.GetBuy(0);
		SimpleQuote const& sell = quote.GetSell(0);
		SimpleQuote const& hidenBuy = quote.GetHidenBuy();
		SimpleQuote const& hidenSell = quote.GetHidenSell();
		money unitPrice = quote.GetUnitPrice();

		ticks.BuyTick = priceToTick(buy, unitPrice);
		ticks.SellTick = priceToTick(sell, unitPrice);
		if (hidenBuy.GetQuantity() > 0) {
			TickT hidenBuyTick = priceToTick(hidenBuy, unitPrice);
			ticks.BuyTick = hidenBuyTick > ticks.BuyTick ? hidenBuyTick : ticks.BuyTick;
		} 
		if (hidenSell.GetQuantity() > 0) {
			TickT hidenSellTick = priceToTick(hidenSell, unitPrice);
			ticks.SellTick = hidenSellTick < ticks.SellTick ? hidenSellTick : ticks.SellTick;
		}

		if (ticks.SellTick <= ticks.BuyTick) {
			ticks.SellTick = ticks.BuyTick + 2;
		}
		return ticks;
	}

	inline char const* getModel(InstrumentModel model) {
		switch (model) {
		case hk::InstrumentModel::CME_GDU:
			return "CME_GDU";
			break;
		case hk::InstrumentModel::CME_GDR:
			return "CME_GDR";
			break;
		default:
			return "N/A";
			break;
		}
	}
	double const OUNCE_RATE = 31.1035;

	class GCTicksSimulator {
		const TickT GDUSpread = 2;
	public:
		GCTicksSimulator(Arguments const& arg) :m_arg(arg) {
		}

		inline TickT floor(TickT gduTick) const {
			return gduTick / 100 * 100;
		}

		inline TickT ceiling(TickT gduTick) const {
			return floor(gduTick + 99);
		}

		// Buy Bias 指 sell 偏不安全，让Buy成交
		inline bool calcBuySafty(TickT buyDeviation, TickT sellDeviation) const {

			switch (m_arg.PriceModel) {
			case QuotingPriceModel::BuySafe:
				return true;
			case QuotingPriceModel::SellSafe:
				return false;
			case QuotingPriceModel::Netural:
			default:
				break;
			}
			return buyDeviation <= sellDeviation;
		}
		// multiple 10000 to move 50.8131 to 508131
		inline QuoteTicks Convert2GDU(QuoteTicks const& gcTicks) {
			QuoteTicks ticks;
			
			TickT buyTick = int(double(gcTicks.BuyTick * 1000) / 31.1035);
			TickT sellTick = int(double(gcTicks.SellTick * 1000) / 31.1035);
			TickT middle = (buyTick + sellTick) / 2;

			TickT newBuyTick = floor(buyTick);
			TickT buyDeviation = buyTick - newBuyTick;

			TickT newSellTick = ceiling(sellTick);
			TickT sellDeviation = newSellTick - sellTick;

			TickT newSpread = (newSellTick - newBuyTick);
			bool isBuySafe = calcBuySafty(buyDeviation, sellDeviation);
			if (newSpread == 100) {
				if (isBuySafe) {
					newBuyTick -= 100;
				} else {
					newSellTick += 100;
				}
			}
			
			QuoteTicks result{ newBuyTick / 100, newSellTick / 100 };
			result.BuyTick += m_arg.InstCostTick + m_arg.QualityTick;
			result.SellTick += m_arg.InstCostTick + m_arg.QualityTick;

			if (m_arg.SpreadTick > 2) {
				TickT spread = std::max(m_arg.SpreadTick, result.Spread());
				switch (m_arg.PriceModel) {
				case QuotingPriceModel::BuySafe:
					result.BuyTick = result.SellTick - spread;
					break;
				case QuotingPriceModel::SellSafe:
					result.SellTick = result.BuyTick + spread;
					break;
				case QuotingPriceModel::Netural:{
					TickT adjustTick = (m_arg.SpreadTick - 1) / 2;
					result.BuyTick -= adjustTick;
					result.SellTick += adjustTick;
					break;
				}
				}
			}
			return result;
		}

		QuoteTicks ticksAdjust(QuoteTicks const& ticks, const hk::Position& pos) {
			QuoteTicks result = ticks;
			bool isSmallSpread = result.Spread() <= m_arg.SpreadTick;

			if (pos.IsBuyBias()) {
				if (pos.IsBuyOneTick()) {
					result.BuyTick -= 1;
				}
			} else if (pos.IsSellBias()) {
				if (pos.IsSellOneTick()) {
					result.BuyTick += 1;
				}
			}

			if (isSmallSpread) {
				result.SellTick = result.BuyTick + m_arg.SpreadTick;
			}
			result.BuyTick += m_arg.InstCostTick;
			result.SellTick += m_arg.InstCostTick;
			return result;
		}



		Arguments const& m_arg;

	};

	class QuoteChangeApplier {
	public:

		QuoteChangeApplier(HKMarketingStrategy& parent) :
			m_parent(parent), m_arg(m_parent.GetArg()), m_business(parent.GetQuotingBusiness()),
			m_position(parent.GetPosition()), m_gcTickSimulator(m_arg) {
		}

		void Handle(Quote const& quote) {
			HandleFollow(quote);
		}

		void HandleFollow(Quote const& quote) {
			QuoteTicks newTicks = CovertQuoteToTick(quote);
			if (!checkFollowQuote(quote, newTicks)) return;;
			TickChanges changes = m_lastSourceTicks.Update(newTicks);
			if (changes.IsBigChange()) {
				Log::Warn("FUCK GC - BIGCHANGE");
				m_parent.Pause("BIG GC SPREAD");
			}
			if (changes.IsAnyChanged(1)) {
				auto newDestTicks = convert(newTicks);
				auto destChanges = applyChange(newDestTicks);
				logFollow(changes, newDestTicks, destChanges);
			}
		}

		void Reapply() {
			QuoteTicks const& quotingTicks = m_business.GetQuotingTicks();
			QuoteTicks newTicks = convert(m_lastSourceTicks);

			auto newChanges = m_lastDestTicks - newTicks;
			if (newChanges.IsAnyChanged(m_arg.SensitivityTick)) {
				m_lastDestTicks.Update(newTicks);
			}

			TickChanges changes = m_lastDestTicks - quotingTicks;
			if (changes.IsAnyChanged(1)) {
				m_business.OnQuoteChange(m_lastDestTicks, changes);
//				logReapply(quotingTicks);
			}
		}

		TickT GetLastBuyTick() const {
			return m_lastDestTicks.BuyTick;
		}

		TickT GetLastSellTick() const {
			return m_lastDestTicks.SellTick;
		}

	private:
		QuoteTicks convert(QuoteTicks newSourceTicks) {
			switch (m_arg.Model) {
			case InstrumentModel::CME_GDU:
				return m_gcTickSimulator.Convert2GDU(newSourceTicks);
			default:
				throw std::logic_error("unknown model");
				break;
			}
		}

		TickChanges applyChange(QuoteTicks newTicks) {
// 			if (m_arg.Model != InstrumentModel::CME_GDU) {
// 				if (m_lastRateTick < 60000 || m_lastRateTick > 80000) {
// 					return TickChanges{ 0,0 };
// 				}
// 			}
			TickChanges destChanges = m_lastDestTicks - newTicks;
			if (destChanges.IsAnyChanged(m_arg.SensitivityTick)) {
				m_lastDestTicks.Update(newTicks);
				m_business.OnQuoteChange(m_lastDestTicks, destChanges);
			}
			return destChanges;
		}

		inline bool isExceptionGCQuote(Quote const& quote, QuoteTicks const& ticks) {
			return ticks.Spread() > 15 || ticks.BuyTick < 100 || ticks.SellTick < 100 || ticks.BuyTick > 100000 || ticks.SellTick > 100000;
		}

		bool checkFollowQuote(const Quote& quote, QuoteTicks& sourceTicks) {
			if (isExceptionGCQuote(quote, sourceTicks)) {
				auto msg = fmt::format("HKStrategy: Exception Source Tick [{} {}]", sourceTicks.BuyTick, sourceTicks.SellTick);
				Log::Warn(msg);
//				m_parent.NotifyUI(msg);
				msg = fmt::format("HKStrategy: Exception Source Quote buy [{:.1f} {}, {:.1f} {}] sell[{:.1f} {}, {:.1f} {}]",
					quote.GetBuy().GetPrice().value(), quote.GetBuy().GetQuantity(), quote.GetBuy(1).GetPrice().value(), quote.GetBuy(1).GetQuantity(),
					quote.GetSell().GetPrice().value(), quote.GetSell().GetQuantity(), quote.GetSell(1).GetPrice().value(), quote.GetSell(1).GetQuantity()
				);
				Log::Warn(msg);
				return false;
			}
			return true;
		}


		void logFollow(hk::TickChanges& changes, hk::QuoteTicks& newDestTicks, hk::TickChanges& destChanges) {
			char const* instCode = InstrumentFactory::GetInstruments().Get(m_arg.InstIDs.Major).GetInstCode().Str();
			auto msg = fmt::format("HKStrategy[{}]:Model[{}] Inst:[{}] Follow: O:[{}:{} {}]-[{}:{}] D:[{}:{} {}]-[{}{}], Last D:[{}:{} {}]]",
				m_parent.GetId(), getModel(m_arg.Model), instCode,
				m_lastSourceTicks.BuyTick, m_lastSourceTicks.SellTick, m_lastSourceTicks.Spread(), changes.BuyChange, changes.SellChange,
				newDestTicks.BuyTick, newDestTicks.SellTick, newDestTicks.Spread(), destChanges.BuyChange, destChanges.SellChange,
				m_lastDestTicks.BuyTick, m_lastDestTicks.SellTick, m_lastDestTicks.Spread());
			Log::Info(msg);
		}

		void logReapply(const hk::QuoteTicks& quotingTicks) {
			Log::Info(fmt::format("HKStrategy ApplyQuoteChange New Ticks:[{}:{}], Quoting Ticks:[{}:{}]",
				m_lastDestTicks.BuyTick, m_lastDestTicks.SellTick,
				quotingTicks.BuyTick, quotingTicks.SellTick));
		}

	private:
		HKMarketingStrategy& m_parent;
		Arguments const& m_arg;
		QuotingBusiness& m_business;
		Position const& m_position;
		QuoteTicks m_lastSourceTicks;
		QuoteTicks m_lastDestTicks;
//		TickT m_lastRateTick = 0;
		GCTicksSimulator m_gcTickSimulator;
	};
}
