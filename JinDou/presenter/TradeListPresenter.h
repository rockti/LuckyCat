#pragma once
#include "model/dealing/Dealing.h"
#include "service/Service.h"
#include "PresenterUtils.h"
#include <algorithm>

namespace ui {
	class TradeListPresenter;
	enum class TradeViewModel {
		Detail,
		Total
	};
	using Trades = std::vector<InvestorTrade>;
	class ITradeListView : public ViewBase<TradeListPresenter> {
	public:
		virtual void AddTrade(InvestorTrade const& trade) = 0;
		virtual void Reload(Trades const& trade) = 0;
	};
	class TradeListPresenter : public ITradeObserver {
	public:
		TradeListPresenter(ITradeListView* view) :view_(view) {
			view->SetPresenter(this);
		}
		virtual void OnAdd(const InvestorTrade& trade) override;
		virtual void Reload(TradeViewModel model);
		void ViewModel(TradeViewModel model);

	private:
		Trades getTrades(TradeViewModel model);
		Trades getTradesByTradeID();

		Trades getTradesByInstID();

		ITradeListView* view_;
		TradeViewModel model_ = TradeViewModel::Detail;
		Deque<InvestorTrade> trades_;
	};
}
