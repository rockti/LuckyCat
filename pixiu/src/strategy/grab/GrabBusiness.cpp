#include "pch.h"
#include "GrabBusiness.h"
#include "GrabStrategy.h"
#include "model/factory/InstrumentFactory.h"

namespace tg::grab {
	GrabBusiness::GrabBusiness(GrabStrategy& pParent, InstID inst, QtyT openQty, QtyT holdQty, money offsetPrice, money stopPrice)
		:m_parent(pParent), m_machine(this), m_instID(inst), m_openQty(openQty), m_holdQty(holdQty), m_offsetPrice(offsetPrice), m_stopPrice(stopPrice) {
		m_currentStatusPtr = m_machine.GetStatus(Phase::Active);
	}

	void GrabBusiness::OnActiveOrder(Dealing const& dealing) {
		// 只有SGE的交割和中立仓才会到这里
		if (m_lastSystemID.compare(dealing.GetSystemNo().Str())) {
			return;	// 相同的系统号不处理
		}
		m_lastSystemID = dealing.GetSystemNo().Str();
		m_pendingAmt += dealing.GetOrderQuantity();
		if (m_pendingAmt > m_holdQty) {
			CancelSGEActive(dealing);
			Log::Info(fmt::format("GrabBusiness[{}]: OnActiveOrder SGE PendingAmt[{}] > HoldQty[{}], autoCancel. LocalID[{}] SystemID[{}]",
				m_parent.GetId(), m_pendingAmt, m_holdQty, dealing.GetLocalId(), dealing.GetSystemNo().Str()));
		}	// 委托量超过保留量，自动撤单
	}	
	void GrabBusiness::OnActiveTrade(int qty) {
		m_position += qty;
		m_currentStatusPtr = m_currentStatusPtr->OnActiveTrade(qty);
	}
	void GrabBusiness::OnActiveCancel(int qty) {
		m_currentStatusPtr = m_currentStatusPtr->OnActiveCancel(qty);
	}
	void GrabBusiness::OnQuote() {
		m_currentStatusPtr = m_currentStatusPtr->OnQuote();
	}
	void GrabBusiness::OnPause() {
		m_currentStatusPtr = m_currentStatusPtr->OnPauseCommand();
	}
	void GrabBusiness::OnPassiveTrade(int qty) {
		m_position -= qty;
		m_passiveSent -= qty;
		m_currentStatusPtr = m_currentStatusPtr->OnPassiveTrade(qty);
	}
	void GrabBusiness::OnPassiveCancel(int qty) {
		m_passiveSent -= qty;
		m_currentStatusPtr = m_currentStatusPtr->OnPassiveCancel(qty);
	}
	
	void GrabBusiness::CancelSGEActive(Dealing const& dealing) {
		InstCodeT const& instCode = InstrumentFactory::GetInstruments().Get(m_instID).GetInstCode();
		OrderCancel action(InstIdExchange(m_instID), dealing.GetSystemNo(), instCode, m_instID, dealing.GetLocalId(), LocalIdACC::NextNormal(), dealing.GetProductClass());
		m_parent.PushAction(action);
	}	// SGE 的交割或中立仓超过最大保有量后自动撤单

	void GrabBusiness::SendPassive(money price, int qty) {
		m_passiveSent += qty;
		if (!IsSHFE(m_instID)) {
			OrderCommand const& command = (m_offsetPrice > m_stopPrice) ? SellOffsetNormal : BuyOffsetNormal;	// Active用FAK，Passive用Normal
			auto orderInfo = m_parent.CreateOrder(m_instID, command, qty, price, SGE_OC_Defer/*默认延期*/);
			m_passiveDealingPtr = &m_parent.PushPassiveOrderWithHook(orderInfo, this);
		}
		else {
			OrderCommand const& command = (m_offsetPrice > m_stopPrice) ? SellOffsetTodayNormal : BuyOffsetTodayNormal;	// Active用FAK，Passive用Normal
			auto orderInfo = m_parent.CreateOrder(m_instID, command, qty, price, SGE_OC_Defer/*默认延期*/);
			m_passiveDealingPtr = &m_parent.PushPassiveOrderWithHook(orderInfo, this);
		}
	}	// SGE 的抢单只有Defer会自动平仓
	void GrabBusiness::CancelPassive() {
		if (m_passiveDealingPtr->GetSystemNo().Length() > 0) {
			InstCodeT const& instCode = InstrumentFactory::GetInstruments().Get(m_instID).GetInstCode();
			OrderCancel action(InstIdExchange(m_instID), m_passiveDealingPtr->GetSystemNo(), instCode, m_instID, m_passiveDealingPtr->GetLocalId(), LocalIdACC::NextNormal(), SGE_OC_Defer);
			m_parent.PushAction(action);
		}
	}	// SGE 的抢单只有Defer会自动平仓
	bool GrabBusiness::IsPassiveOrdered() {
		if (nullptr != m_passiveDealingPtr) {
			return m_passiveDealingPtr->IsOrdered();
		}
		return false;
	}
	bool GrabBusiness::MissPassiveChance() {
		Quote const& quote = WritableQuoteService().GetQuote(m_instID);
		money ofrQuote = quote.GetSell().GetPrice();
		money bidQuote = quote.GetBuy().GetPrice();

		if (true == m_stopTag) {
			if (Direction::SELL == m_passiveDealingPtr->GetDirection()) {
				if (m_passiveDealingPtr->GetOrderPrice() > quote.GetSell().GetPrice()) {
					Log::Info(fmt::format("GrabBusiness[{}]: MissPassiveChance, Quote[{:.3f}, {:.3f}] StopPrice[{}:{:.3f}]",
						m_parent.GetId(), ofrQuote, bidQuote, Desc(m_passiveDealingPtr->GetDirection()), m_passiveDealingPtr->GetOrderPrice()));
					return true;
				}
				return false;
			}
			else {
				if (m_passiveDealingPtr->GetOrderPrice() < quote.GetBuy().GetPrice()) {
					Log::Info(fmt::format("GrabBusiness[{}]: MissPassiveChance, Quote[{:.3f}, {:.3f}] StopPrice[{}:{:.3f}]",
						m_parent.GetId(), ofrQuote, bidQuote, Desc(m_passiveDealingPtr->GetDirection()), m_passiveDealingPtr->GetOrderPrice()));
					return true;
				}
				return false;
			}
		}	// 止损Dealing，非最优价撤单重发
		else {
			if (Direction::SELL == m_passiveDealingPtr->GetDirection()) {
				if (quote.GetLastPrice() < m_stopPrice) {
					Log::Info(fmt::format("GrabBusiness[{}]: MissPassiveChance, Quote[{:.3f},{:.3f}] LastPrice[{:.3f}] < StopPrice[{:.3f}]",
						m_parent.GetId(), ofrQuote, bidQuote, quote.GetLastPrice(), m_stopPrice));
					m_stopTag = quote.GetLastPrice() < m_stopPrice;
				}
			}
			else {
				if (quote.GetLastPrice() > m_stopPrice) {
					Log::Info(fmt::format("GrabBusiness[{}]: MissPassiveChance, Quote[{:.3f},{:.3f}] LastPrice[{:.3f}] > StopPrice[{:.3f}]",
						m_parent.GetId(), ofrQuote, bidQuote, quote.GetLastPrice(), m_stopPrice));
					m_stopTag = quote.GetLastPrice() > m_stopPrice;
				}
			}
		}	// 平仓Dealing，最新价差于止损价撤单改发止损
		return m_stopTag;
	}
	money GrabBusiness::GetStopPrice() {
		Quote const& quote = WritableQuoteService().GetQuote(m_instID);
		money unitPrice = quote.GetUnitPrice();
		return (Direction::SELL == m_passiveDealingPtr->GetDirection()) ? quote.GetSell().GetPrice() - unitPrice : quote.GetBuy().GetPrice() + unitPrice;
	}
}