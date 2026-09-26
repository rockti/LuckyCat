#pragma once


namespace tg::grab {
	enum class Phase {
		End,
		Active,
		Passive,
	};

	class GrabBusiness;
	struct BaseStatus {
		BaseStatus(GrabBusiness* business, Phase index) :m_business(business), m_index(index) {}
		virtual BaseStatus* OnQuote() { return this; }
		virtual BaseStatus* OnPauseCommand() { return this; }

		virtual BaseStatus* OnActiveTrade(int qty) { return this; }
		virtual BaseStatus* OnActiveCancel(int qty) { return this; }
		virtual BaseStatus* OnPassiveTrade(int qty) { return this; }
		virtual BaseStatus* OnPassiveCancel(int qty) { return this; }

		Phase m_index = Phase::End;
		GrabBusiness* m_business;
	};

	struct EndStatus : public BaseStatus {
		EndStatus(GrabBusiness* business) : BaseStatus(business, Phase::End) {}
	};
	struct GrabActive : public BaseStatus {
		GrabActive(GrabBusiness* business) : BaseStatus(business, Phase::Active) {}
		virtual BaseStatus* OnQuote()  override;			// for no ordered
		virtual BaseStatus* OnActiveTrade(int qty) override;
		virtual BaseStatus* OnActiveCancel(int qty) override;
		virtual BaseStatus* OnPauseCommand() override;
	};
	struct GrabPassive : public BaseStatus {
		GrabPassive(GrabBusiness* business) : BaseStatus(business, Phase::Passive) {}
		virtual BaseStatus* OnQuote()  override;			// for passive & stop
		virtual BaseStatus* OnPauseCommand() override;

		virtual BaseStatus* OnPassiveTrade(int qty) override;
		virtual BaseStatus* OnPassiveCancel(int qty) override;
	};

	struct GrabMachine {
		GrabMachine(GrabBusiness* business)
			:m_activePhase(business), m_passivePhase(business), m_End(business) {
			m_statuses[int(m_activePhase.m_index)] = &m_activePhase;
			m_statuses[int(m_passivePhase.m_index)] = &m_passivePhase;
			m_statuses[int(m_End.m_index)] = &m_End;
		}
		BaseStatus* GetStatus(Phase status) { return m_statuses[int(status)]; }
		GrabActive m_activePhase;
		GrabPassive m_passivePhase;
		EndStatus m_End;
		std::array<BaseStatus*, 5> m_statuses;
	};
}