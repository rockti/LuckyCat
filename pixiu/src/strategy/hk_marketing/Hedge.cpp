#include "pch.h"

#include "Hedge.h"
#include "HKMarketingStrategy.h"
#include "utils/Log.h"

namespace hk {
	void Hedge::OnMajorTrade(Trade const& majorTrade) {

		if (!m_hedgeSender.IsHedge()) {
			return;
		}

		if (IsBuy(majorTrade.GetDealing().GetDirection())) {
			m_majorPos += majorTrade.GetDealQuantity();
		} else {
			m_majorPos -= majorTrade.GetDealQuantity();
		}

		int qty = CalcHedgeQuantity();
		if (qty != 0) {
			auto direct = qty > 0 ? Direction::SELL : Direction::BUY ;
			auto price = m_hedgeSender.PushHedgeOrder(direct, abs(qty));
			Business* business = majorTrade.GetDealing().GetBusiness();
			if ( business != nullptr) {
				auto const& msg = fmt::format("Business[{}] Send Hedge: Major Dir[{}] Qty[{}] Price[{}], Hedge Dir[{}] Qty[{}] Price[{}]",
					business->GetId(),
					Desc(majorTrade.GetDealing().GetDirection()), majorTrade.GetDealQuantity(), majorTrade.GetDealPrice().value(),
					Desc(direct), abs(qty), price.value());
				Log::Info(msg);
			}
			m_hedgeFreezePos += -qty;
		}
	}

	void Hedge::OnHedgeTrade(Trade const& hedgeTrade) {
		if (IsBuy(hedgeTrade.GetDealing().GetDirection())) {
			m_hedgePos += hedgeTrade.GetDealQuantity();
			m_hedgeFreezePos -= hedgeTrade.GetDealQuantity();
		} else {
			m_hedgePos -= hedgeTrade.GetDealQuantity();
			m_hedgeFreezePos += hedgeTrade.GetDealQuantity();
		}
	}

	void Hedge::OnHedgeCancel(Dealing const& dealing) {
		if (IsBuy(dealing.GetDirection())) {
			m_hedgeFreezePos -= dealing.GetCancelQuantity();
		} else {
			m_hedgeFreezePos += dealing.GetCancelQuantity();
		}

	}

	int Hedge::CalcHedgeQuantity() {
		int needHedgePos = m_majorPos + (m_hedgePos + m_hedgeFreezePos) * 3;
		return needHedgePos / 3;
	}
	std::string Hedge::GetHedgeInfo() const {
		return fmt::format("M:{} H:{} UH:{}", m_majorPos, m_hedgePos, GetUnhedgePos());
	}
}

