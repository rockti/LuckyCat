#include "pch.h"

#include "QuotingBusiness.h"
#include "HKMarketingStrategy.h"
#include "model/dealing/OrderCommand.h"
#include "model/factory/InstrumentFactory.h"
#include "utils/Log.h"

namespace hk {

	QuotingBusiness::QuotingBusiness(HKMarketingStrategy* parent)
		:m_parent(parent), m_machine(this) {
	}

	void QuotingBusiness::Quoting() {
		InstID instID = m_parent->GetArguments().InstIDs.Major;
		QtyT quantity = m_parent->GetArguments().QuoteQuantity;
		money buyPrice = m_lastTicks.BuyTick * m_parent->GetArg().UnitPrice;
		money sellPrice = m_lastTicks.SellTick * m_parent->GetArg().UnitPrice;
		auto orderInfo = m_parent->CreateMarketingOrder(instID, quantity, buyPrice, sellPrice);
 		MarketingDealing dealing = m_parent->PushMarketingOrder(orderInfo);
 		dealing.GetBuyDealing().SetHookPtr(this);
 		dealing.GetSellDealing().SetHookPtr(this);
 		m_machine.SetDealing(dealing);
		m_quotingTicks = m_lastTicks;
		m_quotingQuantity = quantity;
		m_currentDealings = dealing;


// 		if (m_parent->GetRemainBalanceQty() != 0) {
// 			// balance normal order
// 			auto& arg = m_parent->GetArguments();
// 			OrderInfo order;
// 			QtyT balanceQty = std::min(m_parent->GetRemainBalanceQty(), arg.InitPosition.BalanceQty);
// 			if (IsBuy(arg.InitPosition.Direct)) {
// 				order = m_parent->CreateOrder(instID, SellNormalUnknown, balanceQty, (m_lastTicks.SellTick - 1) * arg.UnitPrice);
// 			} else {
// 				order = m_parent->CreateOrder(instID, BuyNormalUnknown, balanceQty, (m_lastTicks.BuyTick + 1) * arg.UnitPrice);
// 			}
// 
// 			auto& balanceDealing = m_parent->PushOrder(order);
// 			balanceDealing.SetHookPtr(this);
// 			m_machine.SetBalanceDealing(balanceDealing);
// 		}
	}

	void QuotingBusiness::Cancel() {
		InstID instID = m_parent->GetArguments().InstIDs.Major;
		if (!m_machine.GetBuyDealing().IsFinished() && !m_machine.GetSellDealing().IsFinished()) {
			MarketingOrderCancel cancel;
			cancel.InstID = m_parent->GetArguments().InstIDs.Major;
			cancel.BuyLocalId = m_machine.GetBuyDealing().GetLocalId();
			cancel.BuySystemNo = m_machine.GetBuyDealing().GetSystemNo();
			cancel.SellLocalId = m_machine.GetSellDealing().GetLocalId();
			cancel.SellSystemNo = m_machine.GetSellDealing().GetSystemNo();
			cancel.Exchange = InstIdExchange(cancel.InstID);

			m_parent->PushMarketingAction(cancel);
			return;
		}
		if (!m_machine.GetBuyDealing().IsFinished()) {
			OrderCancel cancel(
				InstIdExchange(instID), 
				m_machine.GetBuyDealing().GetSystemNo(),
				InstCodeT("ESunny No Needed"),
				instID,
				m_machine.GetBuyDealing().GetLocalId(),
				0);
			m_parent->PushAction(cancel);
		} else {
			OrderCancel cancel(
				InstIdExchange(instID),
				m_machine.GetSellDealing().GetSystemNo(),
				InstCodeT("ESunny No Needed"),
				instID,
				m_machine.GetSellDealing().GetLocalId(),
				0);
			m_parent->PushAction(cancel);
		}

	}

	bool QuotingBusiness::IsMyMarketDealing(LocalOrderID localId) const {
		return m_currentDealings.GetBuyDealing().GetLocalId() == localId || m_currentDealings.GetSellDealing().GetLocalId() == localId;
	}

	bool QuotingBusiness::IsMyDealing(LocalOrderID localId) const {
		bool isMarketDealing = m_currentDealings.GetBuyDealing().GetLocalId() == localId || m_currentDealings.GetSellDealing().GetLocalId() == localId;
		return isMarketDealing;
	}


	void QuotingBusiness::OnQuoteChange(QuoteTicks const& newTicks, TickChanges const& tickChanges) {
		m_lastTicks = newTicks;
		m_machine.OnQuoteChange();
	}

	void QuotingBusiness::OnBuyOrder(bool isBalance) {
		m_machine.OnBuyOrder(isBalance);
	}

	void QuotingBusiness::OnSellOrder(bool isBalance) {
		m_machine.OnSellOrder(isBalance);
	}
	void QuotingBusiness::OnBuyTrade(bool isBalance) {
		m_machine.OnBuyTrade(isBalance);
	}

	void QuotingBusiness::OnBuyCancel(bool isBalance) {
		m_machine.OnBuyCancel(isBalance);
	}

	void QuotingBusiness::OnSellTrade(bool isBalance) {
		m_machine.OnSellTrade(isBalance);
	}

	void QuotingBusiness::OnSellCancel(bool isBalance) {
		m_machine.OnSellCancel(isBalance);
	}

	void QuotingBusiness::OnConnect() {
		m_machine.OnConnect();
	}

	void QuotingBusiness::OnDisconnect() {
		m_machine.OnDisconnect();
	}

}


