#include "StatusMachine.h"
#include "Context.h"
#include "model/dealing/Dealing.h"
#include "model/Quote.h"
#include "strategy/Business.h"
#include "utils/Log.h"

namespace automate {
	struct PausedStatus : public BaseStatus {
		PausedStatus(Context* context) :BaseStatus(context, RunningStatus::Paused, "Paused") {}
	};

	struct FailStatus : public BaseStatus {
		FailStatus(Context* context) :BaseStatus(context, RunningStatus::Fail, "Fail") {}
	};

	namespace fak {
		struct StartStatus : public BaseStatus {
			StartStatus(Context* context) :BaseStatus(context, RunningStatus::Start, "Start") {}
			virtual BaseStatus* OnQuote(Quote const& quote) override {
				Changed = true;
				auto running = ContextPtr->TryRunning();
				if (running.Mode != RunningMode::None) {
					ContextPtr->FireMinor(running);
					return ContextPtr->GetStatus(RunningStatus::Balancing);
				}
				Changed = false;
				return this;
			}

			virtual BaseStatus* OnCancelCommand() override {
				Changed = true;
				ContextPtr->Reset();
				return ContextPtr->GetStatus(RunningStatus::Paused);
			}
		};

		struct BalancingStatus : public BaseStatus {
			BalancingStatus(Context* context) :BaseStatus(context, RunningStatus::Balancing, "Balancing") {}

			virtual BaseStatus* OnMinorFinished(Dealing const& dealing) override {
				Changed = true;
				type::Volume tradeVolume = dealing.GetDealQuantity();
				if (tradeVolume > 0) {
					ContextPtr->GetRunning().MinorTrade(tradeVolume);
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				} else {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Start);
				}

				return this;
			}

			virtual BaseStatus* OnMajorFinished(Dealing const& dealing) override {
				Changed = true;
				ContextPtr->GetRunning().FinishMajor(dealing.GetLocalId(), dealing.GetOrderQuantity(), dealing.GetDealQuantity());
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Start);
				}

				if (ContextPtr->IsAllBalanceFinished() && !ContextPtr->IsBalanced()) {
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				}
				return this;
			}

			virtual BaseStatus* OnMajorInsertFailed(Dealing const& dealing) override {
				Changed = true;
				if (ContextPtr->IsOverFailedLimit()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}
				return OnMajorFinished(dealing);
			}

			virtual BaseStatus* OnCancelCommand() override {
				Changed = true;
				return ContextPtr->GetStatus(RunningStatus::Pausing);
			}
		};

		struct PausingStatus : public BaseStatus {
			PausingStatus(Context* context) :BaseStatus(context, RunningStatus::Pausing, "Pausing") {}

			virtual BaseStatus* OnMinorFinished(Dealing const& dealing) override {
				Changed = true;
				type::Volume tradeVolume = dealing.GetDealQuantity();
				if (tradeVolume > 0) {
					ContextPtr->GetRunning().MinorTrade(tradeVolume);
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				} else {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}
				return this;
			}

			virtual BaseStatus* OnMajorFinished(Dealing const& dealing) override {
				Changed = true;
				ContextPtr->GetRunning().FinishMajor(dealing.GetLocalId(), dealing.GetOrderQuantity(), dealing.GetDealQuantity());
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}

				if (ContextPtr->IsAllBalanceFinished() && !ContextPtr->IsBalanced()) {
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				}
				return this;
			}

			virtual BaseStatus* OnMajorInsertFailed(Dealing const& dealing) override {
				Changed = true;
				if (ContextPtr->IsOverFailedLimit()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}
				return OnMajorFinished(dealing);
			}
		};
	}

	namespace pending {
		struct StartStatus : public BaseStatus {
			StartStatus(Context* context) :BaseStatus(context, RunningStatus::Start, "Start") {}
			virtual BaseStatus* OnQuote(Quote const& quote) override {
				Changed = true;
				auto running = ContextPtr->TryRunning();
				if (running.Mode != RunningMode::None) {
					ContextPtr->FireMinor(running);
					return ContextPtr->GetStatus(RunningStatus::MinorSent);
				}
				Changed = false;
				return this;
			}

			virtual BaseStatus* OnCancelCommand() override {
				Changed = true;
				ContextPtr->Reset();
				return ContextPtr->GetStatus(RunningStatus::Paused);
			}
		};

		struct MinorSentStatus : public BaseStatus {
			MinorSentStatus(Context* context) :BaseStatus(context, RunningStatus::MinorSent, "MinorSent") {}

			virtual BaseStatus* OnMinorOrder(Dealing const& dealing) override {
				Changed = true;
				if (ContextPtr->IsPausing()) {
					ContextPtr->CancelMinor();
					ContextPtr->GetStatus(RunningStatus::Pausing);
				}
				if (ContextPtr->LostOpportunity()) {
					ContextPtr->CancelMinor();
				}
				return ContextPtr->GetStatus(RunningStatus::Balancing);
			}

			virtual BaseStatus* OnMinorInsertFailed(Dealing const& dealing) override {
				//todo add log
				Changed = true;
				return ContextPtr->GetStatus(RunningStatus::Paused);
			}
		};

		struct BalancingStatus : public BaseStatus {
			BalancingStatus(Context* context) :BaseStatus(context, RunningStatus::Balancing, "Balancing") {}

			virtual BaseStatus* OnQuote(Quote const& quote) override {
				Changed = true;
				if (ContextPtr->LostOpportunity() && ContextPtr->GetMinorDealing().IsPending()) {
					ContextPtr->CancelMinor();
					return this;
				}
				Changed = false;
				return this;
			}
			virtual BaseStatus* OnMinorCancel(Dealing const& dealing) override {
				Changed = true;

				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Start);
				}
				Changed = false;
				return this;
			}
			virtual BaseStatus* OnMinorTrade(Dealing const& dealing, InvestorTrade const& trade) override {
				Changed = true;
				type::Volume tradeVolume = trade.TradeVolume;
				ContextPtr->GetRunning().MinorTrade(tradeVolume);

				if (ContextPtr->LostOpportunity() && !ContextPtr->IsMinorFinished()) {
					ContextPtr->CancelMinor();
				}
				auto balanceVolume = ContextPtr->GetBalanceVolume();
				if (balanceVolume.Mode == BalanceMode::MajorBalance) {
					ContextPtr->OpenMajor(balanceVolume.Volume);
				} else {
					ContextPtr->OppositeMinor(balanceVolume.Volume);
				}
				return this;
			}

			virtual BaseStatus* OnMajorFinished(Dealing const& dealing) override {
				Changed = true;
				ContextPtr->GetRunning().FinishMajor(dealing.GetLocalId(), dealing.GetOrderQuantity(), dealing.GetDealQuantity());
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Start);
				}

				if (ContextPtr->LostOpportunity() && !ContextPtr->IsMinorFinished()) {
					ContextPtr->CancelMinor();
				}
				if (ContextPtr->IsAllBalanceFinished() && !ContextPtr->IsBalanced()) {
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				}
				return this;
			}

			virtual BaseStatus* OnMinorOppositeFinished(Dealing const& dealing) override {
				Changed = true;
				ContextPtr->GetRunning().FinishMinorOpposite(dealing.GetLocalId(), dealing.GetOrderQuantity(), dealing.GetDealQuantity());
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Start);
				}

				if (ContextPtr->LostOpportunity() && !ContextPtr->IsMinorFinished()) {
					ContextPtr->CancelMinor();
				}
				if (ContextPtr->IsAllBalanceFinished()) {
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				}
				return this;
			}


			virtual BaseStatus* OnMajorInsertFailed(Dealing const& dealing) override {
				Changed = true;
				if (ContextPtr->IsOverFailedLimit()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}
				return OnMajorFinished(dealing);
			}

			virtual BaseStatus* OnMinorOppositeInsertFailed(Dealing const& dealing) override {
				Changed = true;
				return OnMajorInsertFailed(dealing);
			}


			virtual BaseStatus* OnCancelCommand() override {
				Changed = true;
				if (ContextPtr->GetMinorDealing().IsPending() && !ContextPtr->GetMinorDealing().IsCancelSent()) {
					ContextPtr->CancelMinor();
				}
				return ContextPtr->GetStatus(RunningStatus::Pausing);
			}

			virtual BaseStatus* OnConnect() override {
				//todo 
				return this;
			}

			virtual BaseStatus* OnDisconnect() override {
				//todo 
				return this;
			}

		};

		struct PausingStatus : public BaseStatus {
			PausingStatus(Context* context) :BaseStatus(context, RunningStatus::Pausing, "Pausing") {}

			virtual BaseStatus* OnMinorCancel(Dealing const& dealing) override {
				Changed = true;
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}
				Changed = false;
				return this;
			}
			virtual BaseStatus* OnMinorTrade(Dealing const& dealing, InvestorTrade const& trade) override {
				Changed = true;
				type::Volume tradeVolume = trade.TradeVolume;
				ContextPtr->GetRunning().MinorTrade(tradeVolume);

				if (ContextPtr->LostOpportunity() && !ContextPtr->IsMinorFinished()) {
					ContextPtr->CancelMinor();
				}
				auto balanceVolume = ContextPtr->GetBalanceVolume();
				if (balanceVolume.Mode == BalanceMode::MajorBalance) {
					ContextPtr->OpenMajor(balanceVolume.Volume);
				} else {
					ContextPtr->OppositeMinor(balanceVolume.Volume);
				}
				return this;
			}

			virtual BaseStatus* OnMajorFinished(Dealing const& dealing) override {
				Changed = true;
				ContextPtr->GetRunning().FinishMajor(dealing.GetLocalId(), dealing.GetOrderQuantity(), dealing.GetDealQuantity());
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}

				if (ContextPtr->LostOpportunity() && !ContextPtr->IsMinorFinished()) {
					ContextPtr->CancelMinor();
				}
				if (ContextPtr->IsAllBalanceFinished() && !ContextPtr->IsBalanced()) {
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				}
				return this;
			}

			virtual BaseStatus* OnMinorOppositeFinished(Dealing const& dealing) override {
				Changed = true;
				ContextPtr->GetRunning().FinishMinorOpposite(dealing.GetLocalId(), dealing.GetOrderQuantity(), dealing.GetDealQuantity());
				if (ContextPtr->IsDone()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}

				if (ContextPtr->LostOpportunity() && !ContextPtr->IsMinorFinished()) {
					ContextPtr->CancelMinor();
				}
				if (ContextPtr->IsAllBalanceFinished()) {
					auto balanceVolume = ContextPtr->GetBalanceVolume();
					if (balanceVolume.Mode == BalanceMode::MajorBalance) {
						ContextPtr->OpenMajor(balanceVolume.Volume);
					} else {
						ContextPtr->OppositeMinor(balanceVolume.Volume);
					}
				}
				return this;
			} 


			virtual BaseStatus* OnMajorInsertFailed(Dealing const& dealing) override {
				Changed = true;
				if (ContextPtr->IsOverFailedLimit()) {
					ContextPtr->Reset();
					return ContextPtr->GetStatus(RunningStatus::Paused);
				}
				return OnMajorFinished(dealing);
			}

			virtual BaseStatus* OnMinorOppositeInsertFailed(Dealing const& dealing) override {
				Changed = true;
				return OnMajorInsertFailed(dealing);
			}
		};
	}


	void StatusMachine::Init(Context* context) {
		pendingStatuses_[int(RunningStatus::Start)] = new pending::StartStatus(context);
		pendingStatuses_[int(RunningStatus::MinorSent)] = new pending::MinorSentStatus(context);
		pendingStatuses_[int(RunningStatus::Balancing)] = new pending::BalancingStatus(context);
		pendingStatuses_[int(RunningStatus::Pausing)] = new pending::PausingStatus(context);
		pendingStatuses_[int(RunningStatus::Paused)] = new PausedStatus(context);
		pendingStatuses_[int(RunningStatus::Fail)] = new FailStatus(context);

		fakStatuses_[int(RunningStatus::Start)] = new fak::StartStatus(context);
		fakStatuses_[int(RunningStatus::Balancing)] = new fak::BalancingStatus(context);
		fakStatuses_[int(RunningStatus::Pausing)] = new fak::PausingStatus(context);
		fakStatuses_[int(RunningStatus::Paused)] = new PausedStatus(context);
		fakStatuses_[int(RunningStatus::Fail)] = new FailStatus(context);

		switch (context->GetArg().PriceMode) 	{
		case PriceMode::Pending: {
			this->current_ = pendingStatuses_[int(RunningStatus::Paused)];
			this->currentStatuses_ = &pendingStatuses_;
			break;
		}
		case PriceMode::FAK: {
			this->current_ = fakStatuses_[int(RunningStatus::Paused)];
			this->currentStatuses_ = &fakStatuses_;
			break;
		}
		default:
			this->current_ = pendingStatuses_[int(RunningStatus::Paused)];
			this->currentStatuses_ = &pendingStatuses_;
			break;
		}
		ContextPtr = context;
	}

	char const* name(type::Direction direction) {
		if (direction == type::Direction::Buy) {
			return "B";
		}
		return "S";
	}
	void StatusMachine::LogStatus(BaseStatus* status) {
		bool statusChanged = status != current_;

		if (!statusChanged && !status->Changed) {
			return;
		}
		auto* minorDealing = ContextPtr->GetMinorDealingPtr();
		type::Direction minorDirection = type::Direction::Buy;
		type::Price minorPrice = 0;
		type::Volume minorVolume = 0;
		bool cancelSent = false;
		if (minorDealing) {
			minorDirection = minorDealing->GetDirection();
			minorPrice = minorDealing->GetOrderPrice();
			minorVolume = minorDealing->GetDealQuantity();
			cancelSent = minorDealing->IsCancelSent();
		}
		auto const& majorQuote = ContextPtr->GetMajorQuote();
		auto const& minorQuote = ContextPtr->GetMinorQuote();
		auto const& runningPos = ContextPtr->GetRunning().Position;
		auto msg = fmt::format("Business[{}] Inst[{}-{}] Status[{}->{}] Pos[{}:{}-{}:{}] Quote[{:.3f}:{},{:.3f}:{} - {:.3f}:{},{:.3f}:{}] MinorOrder[{}:{:.3f}:{} c:{}] Running[{}:{}:{}:{}:{}]",
			ContextPtr->GetBusiness()->GetId(), ContextPtr->GetMinorInst().Code, ContextPtr->GetMajorInst().Code,
			current_->Name, status->Name,
			name(ContextPtr->GetMinorPosition().Direction), ContextPtr->GetMinorPosition().Volume,
			name(ContextPtr->GetMajorPosition().Direction), ContextPtr->GetMajorPosition().Volume,
			minorQuote.GetSell().GetPrice(), minorQuote.GetSell().GetQuantity(),
			minorQuote.GetBuy().GetPrice(), minorQuote.GetBuy().GetQuantity(),
			majorQuote.GetSell().GetPrice(),majorQuote.GetSell().GetQuantity(),
			majorQuote.GetBuy().GetPrice(), majorQuote.GetBuy().GetQuantity(),
			name(minorDirection), minorPrice, minorVolume, cancelSent,
			runningPos.Minor, runningPos.Major, runningPos.MajorFrozen, runningPos.OppositeMinor, runningPos.OppositeMinorFrozen
			);
		Log::Info(msg);
	}
	void StatusMachine::Start() {
		isPausing_ = false;
		if (current_->SelfStatus == RunningStatus::Paused) {
			ContextPtr->Reset();
			Jump(ContextPtr->GetStatus(RunningStatus::Start));
		}
	}
	void StatusMachine::Pause() {
		isPausing_ = true;
		Jump(current_->OnCancelCommand());
	}
}