#pragma once


namespace hf::netural {
	enum class Phase {
		MajorOrdered = 0,
		MajorMissChance,
		Reverse,
		ReverseStop,
		End,
	};

	class PriceBusiness;
	struct BasePhase {
		BasePhase(PriceBusiness* business, Phase index) :m_business(business), m_index(index) {}
		virtual BasePhase* OnQuote() { return this; }
		virtual BasePhase* OnMajorTrade(int qty) { return this; }
		virtual BasePhase* OnMajorCancel(int qty) { return this; }
		virtual BasePhase* OnReverseTrade(int qty) { return this; }
		virtual BasePhase* OnReverseCancel(int qty) { return this; }
		virtual BasePhase* OnPauseCommand() { return this; }
		Phase m_index;
		PriceBusiness* m_business;
	};

	struct EndPhase : public BasePhase {
		EndPhase(PriceBusiness* business) :BasePhase(business, Phase::End) {}
	};

	struct MajorPhase : public BasePhase {
		MajorPhase(PriceBusiness* business) :BasePhase(business, Phase::MajorOrdered) {}
		BasePhase* OnQuote() override;
		BasePhase* OnMajorTrade(int qty) override;
		BasePhase* OnMajorCancel(int qty) override;
		BasePhase* OnPauseCommand() override;
	};

	struct MajorMissChancePhase : public BasePhase {
		MajorMissChancePhase(PriceBusiness* business) :BasePhase(business, Phase::MajorMissChance) {}
		BasePhase* OnMajorTrade(int qty) override;
		BasePhase* OnMajorCancel(int qty) override;
	};

	struct ReversePhase : public BasePhase {
		ReversePhase(PriceBusiness* business) :BasePhase(business, Phase::Reverse) {}
		BasePhase* OnQuote() override;
		BasePhase* OnMajorTrade(int qty) override;
		BasePhase* OnMajorCancel(int qty) override;
		BasePhase* OnReverseCancel(int qty) override;
		BasePhase* OnReverseTrade(int qty) override;
		BasePhase* OnPauseCommand() override;
	};

	struct Machine {
		Machine(PriceBusiness* business)
			:m_MajorOrdered(business),m_MajorChancel(business),m_ReverseOrdered(business),m_End(business) {
			m_phases[int(m_MajorOrdered.m_index)] = &m_MajorOrdered;
			m_phases[int(m_MajorChancel.m_index)] = &m_MajorChancel;
			m_phases[int(m_ReverseOrdered.m_index)] = &m_ReverseOrdered;
			m_phases[int(m_End.m_index)] = &m_End;
		}
		BasePhase* GetPhase(Phase phase) { return m_phases[int(phase)]; }
		MajorPhase m_MajorOrdered;
		MajorMissChancePhase m_MajorChancel;
		ReversePhase m_ReverseOrdered;		
		EndPhase m_End;

		std::array<BasePhase*, 10> m_phases;
	};
}
