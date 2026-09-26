#pragma once
#include "OrderListPresenter.h"

namespace ui {
	void OrderListPresenter::OnAdd(InvestorOrder const& order) {
		switch (model_) {
		case ui::OrderViewModel::All:
			view_->AddOrder(order);
			break;
		case ui::OrderViewModel::Pending:
			if (order.IsPending()) {
				view_->AddOrder(order);
			}
			break;
		case ui::OrderViewModel::Traded:
			if (order.IsAllTraded()) {
				view_->AddOrder(order);
			}
			break;
		case ui::OrderViewModel::Cancelled:
			if (order.IsCancelled()) {
				view_->AddOrder(order);
			}
			break;
		default:
			break;
		}
	}
	void OrderListPresenter::OnChange(InvestorOrder const& order, int errorCode) {
		if (!match(model_, order)) {
			view_->DeleteOrder(order);
		} else {
			view_->UpdateOrder(order);
		}
	}
	void OrderListPresenter::ViewModel(OrderViewModel model) {
		model_ = model;
		auto orders = getOrders(model);
		sort(orders.begin(), orders.end(), [&](auto& l, auto& r)->bool {
			return l.Index < r.Index;
		});
		view_->Reload(orders);
	}
	Orders OrderListPresenter::GetCancelableOrders(OrderViewModel model) const {
		if (model == OrderViewModel::All || model == OrderViewModel::Pending) {
			return getOrders(OrderViewModel::Pending);
		}
		return {};
	}
	Orders OrderListPresenter::getOrders(OrderViewModel model) const {
		auto orders = ServiceSinglton::Services().Order->GetAll();
		Orders result;
		for (auto const& order : orders) {
			if (match(model, order)) {
				result.push_back(order);
			}
		}
		return result;
	}
	bool OrderListPresenter::match(OrderViewModel model, InvestorOrder const& order) const {
		switch (model)
		{
		case ui::OrderViewModel::All:
			return true;
		case ui::OrderViewModel::Pending:
			return order.IsPending();
		case ui::OrderViewModel::Traded:
			return order.IsAllTraded();
		case ui::OrderViewModel::Cancelled:
			return order.IsCancelled();
		default:
			return true;
			break;
		}
	}
}
