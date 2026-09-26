#pragma once
#include "OrderPresenter.h"

namespace ui {

	void OrderPresenter::InsertOrder(OrderInsertRequest const& order) {
		ServiceSinglton::Services().TradeSystem->Insert(order);
	}
	void OrderPresenter::CancelOrder(InvestorOrder const& order) {
		OrderCancelRequest request;
		request.OrderLocalID = order.OrderLocalID;
		request.CancelLocalID = LocalIdACC::NextNormal();
		request.Exchange = order.Exchange;
		request.InstID = order.InstID;
		request.InstrumentCode = order.InstrumentCode;
		request.OrderSysID = order.OrderSysID;

		ServiceSinglton::Services().TradeSystem->Cancel(request);
	}
}
