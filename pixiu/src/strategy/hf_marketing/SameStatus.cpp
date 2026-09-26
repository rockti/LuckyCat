#include "pch.h"
#include "SameStatus.h"
#include "utils/Log.h"
#include "PriceBusiness.h"
#include "NeturalSpreadStrategy.h"


namespace hf::netural {
	BasePhase* MajorPhase::OnQuote() {
		if (m_business->IsMajorPending() && m_business->IsMajorMissChance() && m_business->m_DealingStatus.MajorStatus().SystemNo.Length() > 0) {
			m_business->cancelMajor();
			return m_business->GetPhase(Phase::MajorMissChance);
		}
		return this;
	}
	int const MAX_REVERSE_CNT = 3;	// 最多只能有3套Reverse
	BasePhase* MajorPhase::OnMajorTrade(int qty) {		
		if (true == m_business->m_parent.JustBalance()) {
			if (m_business->IsBalance()) {
				m_business->SetRemoveable();
				return m_business->GetPhase(Phase::End);
			}
		}	// 平衡用Business成交，不发Reverse
		else {
			Direction majorDir = m_business->GetMajorDirection();	
			Direction reverseDir = m_business->GetReverseDirection();
			money unitPrice = m_business->m_unitPrice;
			money majorPrice = m_business->GetMajorOrderedPrice();

			int reverseTick = m_business->GetReverseTick(majorDir);		// 根据持仓因子来调整平盘点位
			money reversePrice = m_business->GetHFPricePosition().GetReversePriceByQuote(reverseTick);
			if (QuoteBias::FST == m_business->m_bias) {
				Variance var = m_business->m_parent.GetMacroVariance();
				TickT reverseTick = (Direction::SELL == reverseDir) ? var.AVGTick - var.STDevTick : var.AVGTick + var.STDevTick;
				reversePrice = tickToPrice(reverseTick, unitPrice);	// 一倍方差位置
				Log::Info(fmt::format("PriceBusiness[{}]: should send special reverse[{}:{:.3f}]", (void*)m_business, Desc(reverseDir), reversePrice));
			}	// 超价的单子ReversePrice取一倍方差

			if (m_business->m_parent.GetReverseCnt(majorDir) <= MAX_REVERSE_CNT) {
				while (m_business->m_parent.HasTheSameReverse(majorDir, reversePrice)) {
					reversePrice = (Direction::SELL == reverseDir) ? reversePrice + unitPrice : reversePrice - unitPrice;
					Log::Info(fmt::format("PriceBusiness[{}]: change reversePrice to [{}:{:.3f}], Tick[{}]", (void*)m_business, Desc(reverseDir), reversePrice, reverseTick));
				}
			}	// 平盘报价向外调整（最多四个价位）
			m_business->sendReverse(reversePrice, qty);
			return m_business->GetPhase(Phase::Reverse);
		}
		return this;
	}
	BasePhase* MajorPhase::OnMajorCancel(int qty) {
		if (m_business->IsBalance()) {
			m_business->SetRemoveable();
			return m_business->GetPhase(Phase::End);
		}
		return this;
	}
	BasePhase* MajorPhase::OnPauseCommand() {
		m_business->cancelMajor();
		return this;
	}

	BasePhase* MajorMissChancePhase::OnMajorTrade(int qty) {
		if (true == m_business->m_parent.JustBalance()) {
			if (m_business->IsBalance()) {
				m_business->SetRemoveable();
				return m_business->GetPhase(Phase::End);
			}
		}	// 平衡用Business成交，不发Reverse
		else {
			Direction reverseDir = m_business->GetReverseDirection();
			money reversePrice = m_business->GetMajorOrderedPrice();	// 撤单失败的成交通常是危险的，直接反向同价位出
			Log::Info(fmt::format("PriceBusiness[{}]: send escape reverse[{}:{:.3f}]", (void*)m_business, Desc(reverseDir), reversePrice));
			m_business->sendReverse(reversePrice, qty);
			return m_business->GetPhase(Phase::Reverse);
		}
		return this;
	}
	BasePhase* MajorMissChancePhase::OnMajorCancel(int qty) {
		if (m_business->IsBalance()) {
			m_business->SetRemoveable();
			return m_business->GetPhase(Phase::End);
		}
		return this;
	}

	BasePhase* ReversePhase::OnQuote() {
		if (m_business->IsReversePending() && m_business->IsReverseNeedRetry()) {
			m_business->cancelReverse();
			m_business->m_retryTag = true;
		}
		if (m_business->IsMajorPending() && m_business->IsMajorMissChance()) {	// Major 部分成交时需要撤单
			m_business->cancelMajor();
		}
		return this;
	}
	BasePhase* ReversePhase::OnMajorTrade(int qty) {
		// Major 部分成交，要再议，现在都是1手委托不会有这个情况
		Direction majorDir = m_business->GetMajorDirection();
		if (true == m_business->m_parent.JustBalance()) {
			if (m_business->IsBalance()) {
				m_business->SetRemoveable();
				return m_business->GetPhase(Phase::End);
			}
		}
		else {
			Direction reverseDir = m_business->GetReverseDirection();
			money reversePrice = m_business->GetHFPricePosition().GetReversePriceByQuote(m_business->GetGainTick());
			m_business->sendReverse(reversePrice, qty);
		}
		return this;
	}
	BasePhase* ReversePhase::OnMajorCancel(int qty) {
		if (m_business->IsBalance()) {
			m_business->SetRemoveable();
			return m_business->GetPhase(Phase::End);
		}
		return this;	// 未成交的Major撤单，可能还有Rvs挂单
	}
	BasePhase* ReversePhase::OnReverseTrade(int qty) {
		if (m_business->IsBalance()) {
			m_business->SetRemoveable();
			money costPrice = m_business->m_unitPrice * m_business->GetCostTick();
			money majorPrice = m_business->GetMajorOrderedPrice();
			money reversePrice = m_business->GetReverseOrderedPrice();
			if (Direction::SELL == m_business->GetMajorDirection()) {
				if (reversePrice + costPrice < majorPrice) {
					m_business->m_parent.UpdateGainAmt();
				}
				else if (reversePrice + costPrice > majorPrice) {
					m_business->m_parent.UpdateLossAmt();
				}
				else {
					m_business->m_parent.UpdateHedgeAmt();
				}
			}
			else {
				if (reversePrice - costPrice > majorPrice) {
					m_business->m_parent.UpdateGainAmt();
				}
				else if (reversePrice - costPrice < majorPrice) {
					m_business->m_parent.UpdateLossAmt();
				}
				else {
					m_business->m_parent.UpdateHedgeAmt();
				}
			}
			return m_business->GetPhase(Phase::End);
		}
		return this;
	}
	BasePhase* ReversePhase::OnReverseCancel(int qty) {
		if (false == m_business->m_parent.JustBalance() && true == m_business->m_retryTag && m_business->m_maxFailedCnt > 0) {
			HFPricePosition const& pricePosition = m_business->GetHFPricePosition();
			Direction majorDir = m_business->GetMajorDirection();
			Operation majorOpt = m_business->GetMajorOperation();
			Direction reverseDir = m_business->GetReverseDirection();
			Operation reverseOpt = m_business->GetReverseOperation();
			money currentRvsPrice = m_business->GetReverseOrderedPrice();

			money majorPrice = m_business->GetMajorOrderedPrice()/*开仓价*/;
			money costPrice = m_business->m_unitPrice * m_business->GetCostTick();
			money retryRvsPrice = (Direction::SELL == reverseDir) ? majorPrice + costPrice : majorPrice - costPrice;	// 收敛：成本价

			int baseTick = (Direction::SELL == reverseDir) ? m_business->m_parent.GetArg().Ticks.BidBaseTick : m_business->m_parent.GetArg().Ticks.OfrBaseTick;
			int reverseTick = m_business->GetGainTick();
			if (m_business->m_escapeTag) {
				money retryZone = m_business->m_unitPrice * (baseTick + reverseTick);
				retryRvsPrice = pricePosition.GetFirstReversePrice();
				(Direction::SELL == reverseDir) ? retryRvsPrice += retryZone : retryRvsPrice -= retryZone;
			}	// 躲避：RVS倒退(base+gain)个价位
			else if (m_business->m_stopTag) {
				retryRvsPrice = pricePosition.GetFirstReversePrice();
			}	// 止损：直接对价盘口

			money quotePrice = m_business->GetHFPricePosition().GetFirstReversePrice();	// 当前RVS方向盘口价
			retryRvsPrice = (Direction::SELL == reverseDir) ? std::max(quotePrice, retryRvsPrice) : std::min(quotePrice, retryRvsPrice);
			Log::Info(fmt::format("PriceBusiness[{}]: OnReverseCancel, major[{} {}: {:.3f}] retry to [{} {}: {:.3f}]",
				(void*)m_business, Desc(majorDir), Desc(majorOpt), majorPrice, Desc(reverseDir), Desc(reverseOpt), retryRvsPrice));
			m_business->sendReverse(retryRvsPrice, qty);
			return this;
		}
		else {
			if (m_business->IsBalance()) {	
				m_business->SetRemoveable();
// 				Log::Info(fmt::format("PriceBusiness[{}]: OnReverseCancel to 'End'", (void*)m_business));
				return m_business->GetPhase(Phase::End);
			}
		}
		return this;
	}
	BasePhase* ReversePhase::OnPauseCommand() {
		if (m_business->IsMajorPending()) {
			m_business->cancelMajor();
		}
		if (m_business->IsReversePending()) {
			m_business->cancelReverse();
		}
		return this;
	}
}