#include "pch.h"
#include "GrabStatus.h"
#include "GrabBusiness.h"
#include "GrabStrategy.h"
#include "model/factory/InstrumentFactory.h"

namespace tg::grab {
	BaseStatus* GrabActive::OnQuote() {	
		Log::Info(fmt::format("GrabBusiness[{}]: OnQuote, goto 'Passive' Phase", m_business->m_parent.GetId()));
		return m_business->GetStatus(Phase::Passive);
	}
	BaseStatus* GrabActive::OnActiveTrade(int qty) {
		Log::Info(fmt::format("GrabBusiness[{}]: ActiveTrade, goto 'Passive' Phase, Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
		return m_business->GetStatus(Phase::Passive);
	}
	BaseStatus* GrabActive::OnActiveCancel(int qty) {
		Log::Info(fmt::format("GrabBusiness[{}]: ActiveCancel, goto 'Passive' Phase, Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
		return m_business->GetStatus(Phase::Passive);
	}
	BaseStatus* GrabActive::OnPauseCommand() {
		m_business->m_endTag = true;
		Log::Info(fmt::format("GrabBusiness[{}]: paused by 'PAUSE', Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
		return m_business->GetStatus(Phase::End);
	}

	BaseStatus* GrabPassive::OnQuote() {
		int currentPosition = m_business->m_position;			// 当前持仓量
		int offsetableQty = m_business->CalcPassiveQty();		// 可平持仓量
		if (m_business->IsPassiveOrdered()) {
			if (offsetableQty > 0) {
				if (m_business->m_position > 50) {
					Log::Info(fmt::format("GrabBusiness[{}]: OnQuote, OverPos[{}] waiting for the deal, LocalID[{}]"
						, m_business->m_parent.GetId(), currentPosition, m_business->m_passiveDealingPtr->GetLocalId()));
					return this;
				}	// 品种持仓超过50手，不重发平仓单
				m_business->CancelPassive();
				m_business->m_retryTag = true;
				Log::Info(fmt::format("GrabBusiness[{}]: OnQuote, CancelPassive for retry newOne[{}: {:.3f},{}], Pos[{}] LocalID[{}]"
					, m_business->m_parent.GetId(), Desc(m_business->m_passiveDealingPtr->GetDirection()), m_business->m_offsetPrice
					, offsetableQty, currentPosition, m_business->m_passiveDealingPtr->GetLocalId()));
			}	// 少于50手的持仓下，有新的active成交，撤掉原有平仓单后重发
			else if (m_business->MissPassiveChance()) {
				m_business->CancelPassive();
				Log::Info(fmt::format("GrabBusiness[{}]: OnQuote, CancelPassive by 'STOP' quote, Pos[{}] LocalID[{}]"
					, m_business->m_parent.GetId(), currentPosition, m_business->m_passiveDealingPtr->GetLocalId()));
			}	// 无新的active成交，但是行情已到撤单止损
		}	// 已经发过平仓委托
		else {
			if (offsetableQty > 0) {
				if (IsCFFEX(m_business->m_instID)) {
					offsetableQty = std::min(offsetableQty, 50);	// 中金所品种单次最大平仓量不得超过50
				}
				else if (IsSGE(m_business->m_instID)) {
					Instruments const& instruments = InstrumentFactory::GetInstruments();
					if (0 == strcmp("Au(T+D)", instruments.Get(m_business->m_instID).GetContractCode())) {
						offsetableQty = std::min(offsetableQty, 200);
					}	// Au(T+D)最大平仓手数200（一般大量成交都会留平仓空间，拆单能最大限度保证平仓价格更优）
					else {
						offsetableQty = std::min(offsetableQty, 2000);
					}	// 其他SGE品种最大平仓手数2000（起到拆单的作用）
				}
				m_business->SendPassive(m_business->m_offsetPrice, offsetableQty);
				Log::Info(fmt::format("GrabBusiness[{}]: OnQuote, SendPassive[{}: {:.3f},{}] Pos[{}] LocalID[{}]", m_business->m_parent.GetId()
					, Desc(m_business->m_passiveDealingPtr->GetDirection()), m_business->m_offsetPrice
					, offsetableQty, currentPosition, m_business->m_passiveDealingPtr->GetLocalId()));
			}
			else {
				if (--m_business->m_confirmCnt <= 0) {
					Log::Info(fmt::format("GrabBusiness[{}]: OnQuote, confirm no offsetable then SetEndTag, Pos[{}]", m_business->m_parent.GetId(), currentPosition));
					m_business->m_endTag = true;
					return m_business->GetStatus(Phase::End);
				}	// 等两轮行情，确认没有成交，结束状态
			}
		}	// 尚未发成平仓委托(流控视为发送失败，等下次行情来时重试)
		return this;
	}
	BaseStatus* GrabPassive::OnPauseCommand() {
		if (m_business->IsPassiveOrdered()) {
			m_business->CancelPassive();
			Log::Info(fmt::format("GrabBusiness[{}]: CancelPassive by 'PAUSE', Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
		}
		return this;
	}
	BaseStatus* GrabPassive::OnPassiveTrade(int qty) {
		if (nullptr != m_business->m_passiveDealingPtr && m_business->m_passiveDealingPtr->IsAllTrade()) {
			Log::Info(fmt::format("GrabBusiness[{}]: OnPassiveTrade then SetEndTag, Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
			m_business->m_endTag = true;
			return m_business->GetStatus(Phase::End);
		}
// 		Log::Info(fmt::format("GrabBusiness[{}]: OnPassiveTrade, Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
		return this;
	}
	BaseStatus* GrabPassive::OnPassiveCancel(int qty) {
		if (m_business->m_stopTag) {
			money stopPrice = m_business->GetStopPrice();
			m_business->SendPassive(stopPrice/*最优价止损不打对价*/, m_business->m_position/*全部止损，无视保留*/);
			Log::Info(fmt::format("GrabBusiness[{}]: OnPassiveCancel, StopPassive[{:.3f},{}]", m_business->m_parent.GetId(), stopPrice, m_business->m_position));
			return this;
		}	// 止损撤单
		if (m_business->m_retryTag) {
			int offsetQty = m_business->CalcPassiveQty();
			m_business->SendPassive(m_business->m_offsetPrice, offsetQty);
			m_business->m_retryTag = false;
			Log::Info(fmt::format("GrabBusiness[{}]: OnPassiveCancel, RetryPassive[{:.3f},{}]", m_business->m_parent.GetId(), m_business->m_offsetPrice, offsetQty));
			return this;
		}	// 新成交撤单

		m_business->m_endTag = true;
		m_business->m_parent.SetStop();
		Log::Info(fmt::format("GrabBusiness[{}]: OnPassiveCancel then SetEndTag, Pos[{}]", m_business->m_parent.GetId(), m_business->m_position));
		return m_business->GetStatus(Phase::End);		// 界面手动撤单，状态结束
	}
}