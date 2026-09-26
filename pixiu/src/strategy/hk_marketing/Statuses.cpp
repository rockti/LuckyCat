#include "pch.h"
#include "Statuses.h"
#include "QuotingBusiness.h"
#include "utils/Log.h"

namespace hk {
	BaseStatus* BaseStatus::OnDisconnect() { return m_machine->GetStatus(Status::Fail); }

	BaseStatus* StartStatus::OnQuoteChange() {
		if (!m_machine->IsPaused()) {
			m_machine->Quoting();
			return m_machine->GetStatus(Status::Sent);
		}
		return this;
	}

	BaseStatus* SentStatus::OnBuyOrder(bool isBalance) {
		if (m_machine->IsAllOrdered()) {
			if (m_machine->IsPaused()) {
				m_machine->Cancel();
				return m_machine->GetStatus(Status::Cancelling);
			}
			return m_machine->GetStatus(Status::Quoting);
		}
		return this;
	}

	BaseStatus* SentStatus::OnSellOrder(bool isBalance) {
		if (m_machine->IsAllOrdered()) {
			if (m_machine->IsPaused()) {
				m_machine->Cancel();
				return m_machine->GetStatus(Status::Cancelling);
			}
			return m_machine->GetStatus(Status::Quoting);
		}
		return this;
	}

	BaseStatus* QuotingStatus::OnQuoteChange() {
		m_machine->Cancel();
		return m_machine->GetStatus(Status::Cancelling);
	}

	BaseStatus* QuotingStatus::OnBuyTrade(bool isBalance) {
		if (isBalance) {
			return this;
		}
		m_machine->Cancel();
		return m_machine->GetStatus(Status::TradeCancelling);
	}

	BaseStatus* QuotingStatus::OnSellTrade(bool isBalance) {
		if (isBalance) {
			return this;
		}
		m_machine->Cancel();
		return m_machine->GetStatus(Status::TradeCancelling);
	}

	BaseStatus* QuotingStatus::OnCancelCommand() {
		m_machine->Cancel();
		return m_machine->GetStatus(Status::Cancelling);
	}

	//////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	/////////////////Cancelling//////////////////////////////
	/////////////////////////////////////////////////////////

	BaseStatus* CancellingStatus::OnBuyOrder(bool isBalance) {
		//重新撤单, ESUNNY特殊SB逻辑
		m_machine->Cancel();
		return this;
	}
	BaseStatus* CancellingStatus::OnSellOrder(bool isBalance) {
		//重新撤单, ESUNNY特殊SB逻辑
		m_machine->Cancel();
		return this;
	}
	BaseStatus* CancellingStatus::OnBuyTrade(bool isBalance) {
		if (m_machine->IsAllFinished()) {
			m_machine->NoQuotingTicks();
			if (m_machine->IsPaused()) {
				return m_machine->GetStatus(Status::End);
			}
			m_machine->Quoting();
			return m_machine->GetStatus(Status::Sent);
		}
		return m_machine->GetStatus(Status::TradeCancelling);
	}
	BaseStatus* CancellingStatus::OnSellTrade(bool isBalance) {
		if (m_machine->IsAllFinished()) {
			m_machine->NoQuotingTicks();
			if (m_machine->IsPaused()) {
				return m_machine->GetStatus(Status::End);
			}
			m_machine->Quoting();
			return m_machine->GetStatus(Status::Sent);
		}
		return m_machine->GetStatus(Status::TradeCancelling);
	}
	BaseStatus* CancellingStatus::OnBuyCancel(bool isBalance) {
		if (m_machine->IsAllFinished()) {
			m_machine->NoQuotingTicks();
			if (m_machine->IsPaused()) {
				return m_machine->GetStatus(Status::End);
			}
			m_machine->Quoting();
			return m_machine->GetStatus(Status::Sent);
		}
		return this;
	}
	BaseStatus* CancellingStatus::OnSellCancel(bool isBalance) {
		if (m_machine->IsAllFinished()) {
			m_machine->NoQuotingTicks();
			if (m_machine->IsPaused()) {
				return m_machine->GetStatus(Status::End);
			}
			m_machine->Quoting();
			return m_machine->GetStatus(Status::Sent);
		}
		return this;
	}

	//////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	/////////////////TradeCancelling/////////////////////////
	/////////////////////////////////////////////////////////

	BaseStatus* TradeCancellingStatus::OnBuyOrder(bool isBalance) {
		//重新撤单
		m_machine->Cancel();
		return this;
	}
	BaseStatus* TradeCancellingStatus::OnSellOrder(bool isBalance) {
		//重新撤单
		m_machine->Cancel();
		return this;
	}
	BaseStatus* TradeCancellingStatus::OnBuyTrade(bool isBalance) {
		return OnBuyCancel(isBalance);
	}
	BaseStatus* TradeCancellingStatus::OnSellTrade(bool isBalance) {
		return OnBuyCancel(isBalance);
	}
	BaseStatus* TradeCancellingStatus::OnBuyCancel(bool isBalance) {
		if (m_machine->IsAllFinished()) {
			m_machine->NoQuotingTicks();
			if (m_machine->IsPaused()) {
				return m_machine->GetStatus(Status::End);
			}
			m_machine->Quoting();
			return m_machine->GetStatus(Status::Sent);
		}
		return this;
	}
	BaseStatus* TradeCancellingStatus::OnSellCancel(bool isBalance) {
		return OnBuyCancel(isBalance);
	}

	inline void QuotingMachine::Quoting() {
		m_business->Quoting();
	}

	inline void QuotingMachine::Cancel() {
		m_business->Cancel();
	}

	void QuotingMachine::Jump(BaseStatus* newStatus) {
		if (newStatus != m_currentStatus) {
			Log::Info(fmt::format("HKMachine[{}] Jump [{}] -> [{}]", (void*)m_business, m_currentStatus->Name, newStatus->Name));
		}
		m_currentStatus = newStatus;
	}

	void QuotingMachine::NoQuotingTicks() {
		m_business->NoQuotingTicks();
	}

	BaseStatus* FailStatus::OnConnect() { return m_machine->GetStatus(Status::Start); }


}
