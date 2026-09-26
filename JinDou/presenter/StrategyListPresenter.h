#pragma once
#include "model/dealing/Dealing.h"
#include "service/Service.h"
#include "PresenterUtils.h"
#include <algorithm>
#include "strategy/auto/AutoStrategy.h"
using namespace automate;

namespace ui {
	class StrategyListPresenter;
	enum class StrategyViewModel {
		Detail,
		Total
	};
	using Strategys = std::vector<AutoArgument>;
	class IStrategyListView : public ViewBase<StrategyListPresenter> {
	public:
		virtual void OnUpdate(int id, AutoSnapshot const& snapshot) = 0;
	};

	class StrategyListPresenter : public IStrategyObserver {
	public:
		StrategyListPresenter(IStrategyListView* view) :view_(view) {
			view->SetPresenter(this);
		}

		int AddStrategy(AutoArgument const& arg);
		void ModifyStrategy(int id, AutoArgument const& arg);
		void DeleteStrategy(int id);
		void StartStrategy(int id);
		void PauseStrategy(int id);
		void StopStrategy(int id);
		std::optional<AutoArgument> GetArg(int id) const;
		std::optional<AutoSnapshot> GetSnapshot(int id) const;
		
		virtual void OnUpdate(int id, AutoSnapshot const& snapshot) override;

	private:
		IStrategyListView* view_;
		std::unordered_map<int, AutoArgument> strategys_;
		std::unordered_map<int, AutoSnapshot> snapshots_;
	};
}
