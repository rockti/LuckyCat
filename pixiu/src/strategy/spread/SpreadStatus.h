#pragma once

class Quote;
class Dealing;
class Trade;

namespace hf::spread {
	enum class Status {
		Start = 0,
		MinorPending,
		Balancing,
		Pausing,
		End,
	};

	class SpreadBusiness;
	struct BasePhase {				
		BasePhase() {}
		BasePhase(SpreadBusiness* business, Status status, char const* name) : m_pBusiness(business), m_status(status), m_name(name) {}
		virtual BasePhase* OnCommand() { return this; }
		virtual BasePhase* OnQuote(Quote const& quote) { return this; }

		virtual BasePhase* OnMinorInsertFailed(Dealing const& dealing) { return this; }
		virtual BasePhase* OnMinorCancel(int qty) { return this; }
		virtual BasePhase* OnMinorTrade(Dealing const& dealing, int qty) { return this; }

		virtual BasePhase* OnMajorInsertFailed(Dealing const& dealing) { return this; }
		virtual BasePhase* OnMajorCancel(int qty) { return this; }
		virtual BasePhase* OnMajorTrade(Dealing const& dealing, int qty) { return this; }

		SpreadBusiness* m_pBusiness = nullptr;
		Status m_status = Status::Start;
		char const* m_name = "N/A";
	};

	struct EndPhase : public BasePhase {
		EndPhase(SpreadBusiness* business) : BasePhase(business, Status::End, "End") {}
	};

	struct StartPhase : public BasePhase {
		StartPhase(SpreadBusiness* business) : BasePhase(business, Status::Start, "Start") {}
		virtual BasePhase* OnCommand() override;
		virtual BasePhase* OnQuote(Quote const& quote) override;
	};

	struct MinorPendingPhase : public BasePhase {
		MinorPendingPhase(SpreadBusiness* business) : BasePhase(business, Status::MinorPending, "Pending") {}
		virtual BasePhase* OnMinorInsertFailed(Dealing const& dealing) override;

		virtual BasePhase* OnCommand() override;
		virtual BasePhase* OnQuote(Quote const& quote) override;

		virtual BasePhase* OnMinorCancel(int qty) override;
		virtual BasePhase* OnMinorTrade(Dealing const& dealing, int qty) override;
	};

	struct BalancingPhase : public BasePhase {
		BalancingPhase(SpreadBusiness* business) : BasePhase(business, Status::Balancing, "Balancing") {}
		virtual BasePhase* OnMinorInsertFailed(Dealing const& dealing) override;
		virtual BasePhase* OnMajorInsertFailed(Dealing const& dealing) override;

		virtual BasePhase* OnCommand() override;
		virtual BasePhase* OnQuote(Quote const& quote) override;

		virtual BasePhase* OnMinorCancel(int qty) override;
		virtual BasePhase* OnMinorTrade(Dealing const& dealing, int qty) override;

		virtual BasePhase* OnMajorCancel(int qty) override;
		virtual BasePhase* OnMajorTrade(Dealing const& dealing, int qty) override;
	};

	struct PausingPhase : public BasePhase {
		PausingPhase(SpreadBusiness* context) :BasePhase(context, Status::Pausing, "Pausing") {}
		virtual BasePhase* OnQuote(Quote const& quote) override;
		virtual BasePhase* OnMinorCancel(int qty) override;
		virtual BasePhase* OnMajorCancel(int qty) override;
		virtual BasePhase* OnMinorTrade(Dealing const& dealing, int qty) override;
		virtual BasePhase* OnMajorTrade(Dealing const& dealing, int qty) override;
	};

	struct SpreadMachine : public BasePhase {
		SpreadMachine(SpreadBusiness* pBusiness) {
			m_phases[int(Status::Start)] = new StartPhase(pBusiness);
			m_phases[int(Status::MinorPending)] = new MinorPendingPhase(pBusiness);
			m_phases[int(Status::Balancing)] = new BalancingPhase(pBusiness);
			m_phases[int(Status::Pausing)] = new PausingPhase(pBusiness);
			m_phases[int(Status::End)] = new EndPhase(pBusiness);
		}

		std::array<BasePhase*, 8> m_phases;
		BasePhase* GetPhase(Status phase) { return m_phases[int(phase)]; }
	};
}