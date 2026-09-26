#pragma once
#include "service/Service.h"
#include "PresenterUtils.h"
#include <algorithm>

namespace ui {
	class OrderListPresenter;
	enum class OrderViewModel {
		All,
		Pending,
		Traded,
		Cancelled,
	};
	using Orders = std::vector<InvestorOrder>;
	class IOrderListView : public ViewBase<OrderListPresenter> {
	public:
		virtual void AddOrder(InvestorOrder const& order) = 0;
		virtual void UpdateOrder(InvestorOrder const& order) = 0;
		virtual void DeleteOrder(InvestorOrder const& order) = 0;
		virtual void Reload(Orders const& orders) = 0;
	};
	class OrderListPresenter : public IOrderObserver {
	public:
		OrderListPresenter(IOrderListView* view) :view_(view) {
			view->SetPresenter(this);
		}
		virtual void OnAdd(InvestorOrder const& order) override;
		virtual void OnChange(InvestorOrder const& order, int errorCode = 0) override;
		virtual void ViewModel(OrderViewModel model);
		Orders GetCancelableOrders(OrderViewModel model) const;
	private:
		Orders getOrders(OrderViewModel model) const;
		bool match(OrderViewModel model, InvestorOrder const& order) const;

		IOrderListView* view_;
		OrderViewModel model_ = ui::OrderViewModel::All;
	};

}
