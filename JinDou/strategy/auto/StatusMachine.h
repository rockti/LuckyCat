#pragma once
#include <array>
#include "type/ZTPType.h"
class Dealing;
class Quote;

namespace automate {
	using namespace ztp;
	enum class RunningStatus {
		Start = 0,
		MinorSent,
		Pausing,
		Balancing,
		Paused,
		Fail,
	};

	struct Context;

	struct BaseStatus {
		BaseStatus() {}
		BaseStatus(Context* context, RunningStatus status, char const* name) :ContextPtr(context), SelfStatus(status), Name(name) {}

		virtual BaseStatus* OnQuote(Quote const& quote) { Changed = false;  return this; }

		virtual BaseStatus* OnMajorFinished(Dealing const& dealing) { return this; }
		virtual BaseStatus* OnMinorFinished(Dealing const& dealing) { return this; }

		virtual BaseStatus* OnMinorOrder(Dealing const& dealing) { return this; }
		virtual BaseStatus* OnMinorTrade(Dealing const& dealing, InvestorTrade const& trade) { return this; }
		virtual BaseStatus* OnMinorCancel(Dealing const& dealing) { return this; }


		virtual BaseStatus* OnMajorInsertFailed(Dealing const& dealing) { return this; }
		virtual BaseStatus* OnMinorInsertFailed(Dealing const& dealing) { return this; }
		virtual BaseStatus* OnMinorOppositeInsertFailed(Dealing const& dealing) { return this; }

		virtual BaseStatus* OnMinorOppositeFinished(Dealing const& dealing) { return this; }


		virtual BaseStatus* OnCancelCommand() { return this; }

		virtual BaseStatus* OnConnect() { return this; }
		virtual BaseStatus* OnDisconnect() { return this; }

		Context* ContextPtr = nullptr;
		RunningStatus SelfStatus = RunningStatus::Start;
		char const* Name = nullptr;
		bool Changed = true;
	};

	struct StatusMachine : public BaseStatus {
		StatusMachine() {}
		void Init(Context* context);

		void LogStatus(BaseStatus* status);

		BaseStatus* GetStatus(RunningStatus status) const {
			return (*currentStatuses_)[int(status)];
		}

		void Start();
		void Pause();

		bool IsPausing() const { return isPausing_; }

		BaseStatus* Jump(BaseStatus* status) {
			LogStatus(status);
			current_ = status;
			return status;
		}

		virtual BaseStatus* OnQuote(Quote const& quote) override {
			return Jump(current_->OnQuote(quote));
		}


		virtual BaseStatus* OnMinorOrder(Dealing const& dealing) override {
			return Jump(current_->OnMinorOrder(dealing));
		}


		virtual BaseStatus* OnMajorFinished(Dealing const& dealing) override {
			return Jump(current_->OnMajorFinished(dealing));
		}

		virtual BaseStatus* OnMinorFinished(Dealing const& dealing) override {
			return Jump(current_->OnMinorFinished(dealing));
		}

		virtual BaseStatus* OnMinorTrade(Dealing const& dealing, InvestorTrade const& trade) override {
			return Jump(current_->OnMinorTrade(dealing, trade));
		}

		virtual BaseStatus* OnMinorCancel(Dealing const& dealing) override {
			return Jump(current_->OnMinorCancel(dealing));
		}

		virtual BaseStatus* OnMajorInsertFailed(Dealing const& dealing) override {
			return Jump(current_->OnMajorInsertFailed(dealing));
		}

		virtual BaseStatus* OnMinorInsertFailed(Dealing const& dealing) override {
			return Jump(current_->OnMinorInsertFailed(dealing));
		}
		virtual BaseStatus* OnMinorOppositeInsertFailed(Dealing const& dealing) override {
			return Jump(current_->OnMinorInsertFailed(dealing));
		}


		virtual BaseStatus* OnMinorOppositeFinished(Dealing const& dealing) override {
			return Jump(current_->OnMinorOppositeFinished(dealing));
		}


		virtual BaseStatus* OnCancelCommand() override {
			return Jump(current_->OnCancelCommand());
		}


		virtual BaseStatus* OnConnect() override {
			return Jump(current_->OnConnect());
		}

		virtual BaseStatus* OnDisconnect() override {
			return Jump(current_->OnDisconnect());
		}

		BaseStatus const* GetCurrentStatus() const {
			return current_;
		}
	private:
		std::array<BaseStatus*, 20> pendingStatuses_;
		std::array<BaseStatus*, 20> fakStatuses_;
		std::array<BaseStatus*, 20>* currentStatuses_;
		BaseStatus* current_ = nullptr;
		bool isPausing_ = false;
	public:

	};
}