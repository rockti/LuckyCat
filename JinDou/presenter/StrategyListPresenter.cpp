#include "TradeListPresenter.h"
#include "StrategyListPresenter.h"
#include "runner/Framework.h"
#include "notify/UIPublisher.h"

using namespace automate;

namespace ui {
    int StrategyListPresenter::AddStrategy(AutoArgument const& arg) {
		Business* business = new AutoStrategy(arg, Framework::Instance().GetStrategyPublisher(), BusinessDealingStore::Instance());
		strategys_.emplace(business->GetId(),arg);
        Framework::Instance().GetUIPublisher().AddBusiness(business);
        return business->GetId();
    }

    void StrategyListPresenter::ModifyStrategy(int id, AutoArgument const& arg) {
		auto iter = strategys_.find(id);
		iter->second = arg;
        auto& uiPublisher = Framework::Instance().GetUIPublisher();
		StrategyCommand command;
		command.Type = StrategyCommand::UPDATE_ARGUMENT;
		int businessID = id;
		command.Business = nullptr;
		command.BusinessID = businessID;
		AutoArgument& argument = *(AutoArgument*)command.Data;
		argument = arg;
        uiPublisher.Push(command);

    }

    void StrategyListPresenter::DeleteStrategy(int id) {
		strategys_.erase(id);
		auto& uiPublisher = Framework::Instance().GetUIPublisher();
		StrategyCommand command;
		command.Type = StrategyCommand::DELETE_BUSINESS;
		int businessID = id;
		command.Business = nullptr;
		command.BusinessID = businessID;
		uiPublisher.Push(command);
    }

	void StrategyListPresenter::StartStrategy(int id) {
		auto& uiPublisher = Framework::Instance().GetUIPublisher();
		StrategyCommand command;
		command.Type = StrategyCommand::START;
		int businessID = id;
		command.Business = nullptr;
		command.BusinessID = businessID;
		uiPublisher.Push(command);
	}

	void StrategyListPresenter::PauseStrategy(int id) {
		auto& uiPublisher = Framework::Instance().GetUIPublisher();
		StrategyCommand command;
		command.Type = StrategyCommand::PAUSE;
		int businessID = id;
		command.Business = nullptr;
		command.BusinessID = businessID;
		uiPublisher.Push(command);
	}

	void StrategyListPresenter::StopStrategy(int id) {
		auto& uiPublisher = Framework::Instance().GetUIPublisher();
		StrategyCommand command;
		command.Type = StrategyCommand::STOP;
		int businessID = id;
		command.Business = nullptr;
		command.BusinessID = businessID;
		uiPublisher.Push(command);
	}
	std::optional<AutoArgument> StrategyListPresenter::GetArg(int id) const {
		auto iter = strategys_.find(id);
		if (iter != strategys_.end()) {
			return { iter->second };
		}
		return {};
	}
	std::optional<AutoSnapshot> StrategyListPresenter::GetSnapshot(int id) const {
		auto iter = snapshots_.find(id);
		if (iter != snapshots_.end()) {
			return { iter->second };
		}
		return {};
	}
    void StrategyListPresenter::OnUpdate(int id, AutoSnapshot const& snapshot) {
		snapshots_[id] = snapshot;
		view_->OnUpdate(id, snapshot);
    }
}