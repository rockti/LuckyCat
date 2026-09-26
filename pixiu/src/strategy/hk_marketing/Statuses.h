#pragma once

#include "model/Constants.h"
#include "model/dealing/MarketingDealing.h"

namespace hk {

	class QuotingBusiness;

	enum class Status {
		Start = 0,
		Sent,
		Quoting,
		Cancelling,
		TradeCancelling,
		End,
		Fail,
	};

	struct QuotingMachine;

	struct BaseStatus {
		BaseStatus(QuotingMachine* machine, Status status, char const* name) :m_machine(machine), m_index(status),Name(name) {}

		virtual BaseStatus* OnQuoteChange() { return this; }

		virtual BaseStatus* OnBuyOrder(bool isBalance = false) { return this; }
		virtual BaseStatus* OnSellOrder(bool isBalance = false) { return this; }
		virtual BaseStatus* OnBuyTrade(bool isBalance = false) { return this; }
		virtual BaseStatus* OnSellTrade(bool isBalance = false) { return this; }

		virtual BaseStatus* OnBuyCancel(bool isBalance = false) { return this; }
		virtual BaseStatus* OnSellCancel(bool isBalance = false) { return this; }

		virtual BaseStatus* OnCancelCommand() { return this; }

		virtual BaseStatus* OnConnect() { return this; }
		virtual BaseStatus* OnDisconnect();

		QuotingMachine* m_machine;
		Status m_index;
		char const* Name;
	};

	struct StartStatus : public BaseStatus {
		StartStatus(QuotingMachine* machine) :BaseStatus(machine, Status::Start, "Start") {}
		virtual BaseStatus* OnQuoteChange() override;
	};

	struct SentStatus : public BaseStatus {
		SentStatus(QuotingMachine* machine) :BaseStatus(machine, Status::Sent,"Sent") {}

		virtual BaseStatus* OnBuyOrder(bool isBalance = false) override;
		virtual BaseStatus* OnSellOrder(bool isBalance = false) override;

	};
	struct QuotingStatus : public BaseStatus {
		QuotingStatus(QuotingMachine* machine) :BaseStatus(machine, Status::Quoting,"Quoting") {}
		virtual BaseStatus* OnQuoteChange() override;
		virtual BaseStatus* OnBuyTrade(bool isBalance = false) override;
		virtual BaseStatus* OnSellTrade(bool isBalance = false) override;
		virtual BaseStatus* OnCancelCommand() override;
	};

	struct CancellingStatus : public BaseStatus {
		CancellingStatus(QuotingMachine* machine) :BaseStatus(machine, Status::Cancelling,"Cancelling") {}

		virtual BaseStatus* OnBuyOrder(bool isBalance = false) override;
		virtual BaseStatus* OnSellOrder(bool isBalance = false) override;

		virtual BaseStatus* OnBuyTrade(bool isBalance = false) override;
		virtual BaseStatus* OnSellTrade(bool isBalance = false) override;
		virtual BaseStatus* OnBuyCancel(bool isBalance = false) override;
		virtual BaseStatus* OnSellCancel(bool isBalance = false) override;

	};

	struct TradeCancellingStatus : public BaseStatus {
		TradeCancellingStatus(QuotingMachine* machine) :BaseStatus(machine, Status::TradeCancelling, "TradeCancelling") {}

		virtual BaseStatus* OnBuyOrder(bool isBalance = false) override;
		virtual BaseStatus* OnSellOrder(bool isBalance = false) override;

		virtual BaseStatus* OnBuyTrade(bool isBalance = false) override;
		virtual BaseStatus* OnSellTrade(bool isBalance = false) override;
		virtual BaseStatus* OnBuyCancel(bool isBalance = false) override;
		virtual BaseStatus* OnSellCancel(bool isBalance = false) override;
	};


	struct EndStatus : public BaseStatus {
		EndStatus(QuotingMachine* machine) :BaseStatus(machine, Status::End, "End") {}
	};

	struct FailStatus : public BaseStatus {
		FailStatus(QuotingMachine* machine) :BaseStatus(machine, Status::Fail, "Fail") {}

		virtual BaseStatus* OnConnect() override;
	};



	struct QuotingMachine {
		QuotingMachine(QuotingBusiness* business)
			:m_business(business),
			m_Start(this),m_Sent(this),m_Quoting(this), m_Cancel(this),
			m_TradeCancelling(this),m_End(this),m_Disconnect(this) {
			
			m_statuses[int(Status::Start)] = &m_Start;
			m_statuses[int(Status::Sent)] = &m_Sent;
			m_statuses[int(Status::Quoting)] = &m_Quoting;
			m_statuses[int(Status::Cancelling)] = &m_Cancel;
			m_statuses[int(Status::TradeCancelling)] = &m_TradeCancelling;
			m_statuses[int(Status::End)] = &m_End;
			m_statuses[int(Status::Fail)] = &m_Disconnect;

			m_currentStatus = &m_Start;
		}
		inline BaseStatus* GetStatus(Status status) {
			return m_statuses[int(status)];
		}
		void Quoting();
		void Cancel();
		void Hedge();
		bool IsAllCancelled() const {
			return m_currentDealing.IsAllCancelled();
		}
		bool IsAllFinished() const {
			return m_currentDealing.IsAllFinished();
		}

		void Jump(BaseStatus* newStatus);
		
		void Start() {
			m_paused = false;
			if (m_currentStatus->m_index == Status::End || m_currentStatus->m_index == Status::Fail) {
				Jump(GetStatus(Status::Start));
			}
		}
		void Pause() {
			m_paused = true;
			if (m_currentStatus==&m_Sent || m_currentStatus==&m_Quoting) {
				Jump(m_currentStatus->OnCancelCommand());
			}
		}

		void Fail() {
			m_paused = true;
			Jump(GetStatus(Status::Fail));
		}

		inline bool IsPaused() const { return m_paused; }

		inline void OnQuoteChange() { 
			if (m_paused) return;
			Jump(m_currentStatus->OnQuoteChange());
		}

		inline void OnBuyOrder(bool isBalance = false) {
			Jump(m_currentStatus->OnBuyOrder(isBalance));
		}

		inline void OnSellOrder(bool isBalance = false) {
			Jump(m_currentStatus->OnSellOrder(isBalance));
		}

		inline void OnBuyTrade(bool isBalance = false) {
			Jump(m_currentStatus->OnBuyTrade(isBalance));
		}

		inline void OnSellTrade(bool isBalance = false) {
			Jump(m_currentStatus->OnSellTrade(isBalance));
		}

		inline void OnBuyCancel(bool isBalance = false) {
			Jump(m_currentStatus->OnBuyCancel(isBalance));
		}

		inline void OnSellCancel(bool isBalance = false) {
			Jump(m_currentStatus->OnSellCancel(isBalance));
		}

		inline void OnConnect() {
			Jump(m_currentStatus->OnConnect());
		}

		inline void OnDisconnect() {
			Jump(m_currentStatus->OnDisconnect());
		}

		inline void SetDealing(MarketingDealing const& dealing) {
			m_currentDealing = dealing;
		}

		void NoQuotingTicks();
		inline Dealing const& GetBuyDealing() const {
			return m_currentDealing.GetBuyDealing();
		}
		inline Dealing const& GetSellDealing() const {
			return m_currentDealing.GetSellDealing();
		}

		inline MarketingDealing const& GetDealing() const {
			return m_currentDealing;
		}

		inline bool IsAllOrdered() const {
			return m_currentDealing.IsAllOrdered();
		}

		Status GetCurrentStatus() const { return m_currentStatus->m_index; }
	private:
		QuotingBusiness* m_business;
		StartStatus m_Start;
		SentStatus m_Sent;
		QuotingStatus m_Quoting;
		CancellingStatus m_Cancel;
		TradeCancellingStatus m_TradeCancelling;
		EndStatus m_End;
		FailStatus m_Disconnect;

		std::array<BaseStatus*, 10> m_statuses;
		BaseStatus* m_currentStatus = nullptr;
		bool m_paused = false;
		MarketingDealing m_currentDealing;

	};
}
