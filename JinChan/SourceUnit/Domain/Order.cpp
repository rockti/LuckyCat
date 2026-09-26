#include "stdafx.h"
#include "Order.h"
#include "Investor.h"

Trade Order::CreateTrade(data::CTrade const& notifyTrade) {
	return m_investor->CreateTrade(*this,notifyTrade);
}

void Order::OnAccepted(char const* orderSysID) {
	m_order->OrderSysID = orderSysID;
	m_order->OrderStatus = type::OrderStatus::NoTradeQueueing;
	m_investor->AddSysIDIndex(*this);
	m_investor->UpdateOrder(*this);
}

void Order::OnInsertFailed(Error error, bool unfreeze) {
	m_insertErrorCode = error.Code;
	m_order->InsertErrorCode = error.Code;
	m_order->OrderStatus = type::OrderStatus::InsertFail;
	if (unfreeze) {
		m_investor->Unfreeze(*this);
	}
	m_investor->UpdateOrder(*this);
}
void Order::OnCancelSending(type::UserOrderLocalID UserOrderActionLocalID, type::OrderLocalID ActionLocalID) {
	m_order->UserActionLocalID = UserOrderActionLocalID;
	m_order->ActionLocalID = ActionLocalID;
	m_order->OrderStatus = type::OrderStatus::CancelQueueing;
	m_investor->UpdateOrder(*this);
}

void Order::OnCancelFailed(Error error) {
	m_order->OrderStatus = type::OrderStatus::CancelFail;
	m_order->CancelErrorCode = error.Code;
	m_investor->UpdateOrder(*this);
}

void Order::OnCancelled() {
	m_order->OrderStatus = type::OrderStatus::Canceled;
	m_investor->Unfreeze(*this);
	m_order->FrozenFee = 0.0;
	m_order->FrozenMargin = 0.0;
	m_investor->UpdateOrder(*this);
}

void Order::OnOrder(data::COrder& notifyOrder) {
// 	m_order->VolumeTrade = notifyOrder.VolumeTrade;
// 	m_order->VolumeRemain = notifyOrder.VolumeRemain; 
	m_order->OrderStatus = notifyOrder.OrderStatus;
	m_investor->UpdateOrder(*this);
}


void Order::OnTrade(Trade const& trade) {
	double unitFee = m_order->FrozenFee / m_order->VolumeRemain;
	double unitMargin = m_order->FrozenMargin / m_order->VolumeRemain;
	double unfreezeFee = unitFee * trade.GetVolume();
	double unfreezeMargin = unitMargin * trade.GetVolume();

	m_order->VolumeTrade += trade.GetVolume();
	m_order->VolumeRemain -= trade.GetVolume();

	if (m_order->VolumeRemain > 0) {
		m_order->OrderStatus = type::OrderStatus::PartTradedQueueing;
	} else {
		m_order->OrderStatus = type::OrderStatus::AllTraded;
	}

	m_order->FrozenFee -= unfreezeFee;
	m_order->FrozenMargin -= unfreezeMargin;

	m_order->TradeFee += trade.GetTradeFee();
	m_order->TradeMargin += trade.GetMargin();
	m_investor->OnTrade(trade, unfreezeMargin, unfreezeFee);
//	m_investor->ModifyMoney(trade, unfreezeMargin, unfreezeFee);
	m_investor->UpdateOrder(*this);
	m_investor->UpdateTrade(trade);
}
bool Order::IsLong() const {
	return ::IsLong(m_order);
}

bool Order::IsShort() const {
	return ::IsShort(m_order);
}

bool Order::IsSpot() const {
	return m_order->ProductClass == type::ProductClass::Spot;
}

Trade::Trade(Order& order, data::CTrade* pTrade) :
	m_order(order), m_trade(pTrade) {
	data::CInstrument const& instrument = order.GetInstrument();
	data::CInvestorMargin const& margin = order.GetInvestorMargin();
	data::CInvestorFee const& fee = order.GetInvestorFee();
	type::Money usedMargin = 0.0;
	if (m_order.IsLong()) {
		usedMargin = pTrade->TradeVolume * pTrade->TradePrice * instrument.VolumeMultiple * margin.LongMarginRate + pTrade->TradeVolume * margin.LongMarginAmt;
	} else if (m_order.IsShort()){
		usedMargin = pTrade->TradeVolume * pTrade->TradePrice * instrument.VolumeMultiple * margin.ShortMarginRate + pTrade->TradeVolume * margin.ShortMarginAmt;
	}
	pTrade->UsedMargin = usedMargin;
	pTrade->UsedFee = pTrade->TradeVolume * pTrade->TradePrice * instrument.VolumeMultiple * fee.OpenFeeRate + pTrade->TradeVolume * fee.OpenFeeAmt;
}

int Trade::GetMultiple() const {
	return m_order.GetInstrument().VolumeMultiple;
}

bool Trade::IsSpot() const {
	return m_order.IsSpot();
}
