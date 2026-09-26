#include "stdafx.h"
#include "TradeServiceImpl.h"
#include "ErrorDefine.h"
#include "EntityCopy.h"
#include "BaseExchApi.h"
#include "Investor.h"
#include "LinkManager.h"
#include <fmt/format.h>

extern CExchangeLinkManager* g_pExchangeLinkManager;


namespace {

	void LogOrder(char const* action, Order const& order) {
		data::COrder const& oOrder = *order.GetOriginalOrder();
		data::CInvestorAccount const& account = *oOrder.getInvestorAccount();
		data::CInvestorPosition const& pos = *oOrder.getInvestorPosition();
		char const* direction = type::IsBuy(oOrder)? "B" : "S";
		char const* operation = type::IsOpen(oOrder) ? "OPEN" : "OFFSET";
		Log::Info(fmt::format("TradeServiceImpl[{}]: InvestorID[{}] InstrumentID[{}] LocalID[{}] SysID[{}] Dir[{}] OP[{}] PC[{}] Volume[{}] Traded[{}] Remain[{}] Price[{:.2f} OrderStatus[{}] FFee[{}] FMargin[{}] TFee[{}] TMargin[{}]",
			action, oOrder.InvestorID, oOrder.InstrumentID, oOrder.OrderLocalID, oOrder.OrderSysID,
			direction, operation, oOrder.ProductClass, oOrder.Volume, oOrder.VolumeTrade,
			oOrder.VolumeRemain, oOrder.LimitPrice, oOrder.OrderStatus,
			oOrder.FrozenFee, oOrder.FrozenMargin,oOrder.TradeFee, oOrder.TradeMargin
		));

		Log::Info(fmt::format("TradeServiceImpl[{}] Account:InvestorID[{}] InstrumentID[{}] LocalID[{}] SysID[{}] Avail[{:.2f}] FFee[{:.2f}] FMargin[{:.2f}], Fee[{:.2f}] Margin[{:.2f}]",
			action, oOrder.InvestorID, oOrder.InstrumentID, oOrder.OrderLocalID, oOrder.OrderSysID,
			account.Available, account.FrozenFee, account.FrozenMargin, account.Fee, account.Margin)
		);

		Log::Info(fmt::format("TradeServiceImpl[{}] Position:InvestorID[{}] InstrumentID[{}] LocalID[{}] SysID[{}] Pos[{}] PosCost[{:.2f}] FPos[{}],YDPos[{}] YDPosPrice[{:.2f}] FFee[{:.2f}] FMargin[{:.2f}] UserdMargin[{:.2f}]",
			action, oOrder.InvestorID, oOrder.InstrumentID, oOrder.OrderLocalID, oOrder.OrderSysID,
			pos.Position, pos.PositionCost, pos.FrozenPosition,
			pos.YdPosition, pos.YdPositionPrice,
			pos.FrozenFee,pos.FrozenMargin,pos.UsedMargin
		));

	}

}

CTradeServiceImpl::CTradeServiceImpl(SystemStatusRepository& systemRepo, InvestorRepository& investorRepo, InstrumentRepository& instRepo, IFlow* pTradeResult)
:m_systemRepo(systemRepo),m_investorRepo(investorRepo), m_instRepo(instRepo) {
}

OrderResult CTradeServiceImpl::ReqOrderInsert(Investor& investor,data::COrder& inputOrder) {
	auto order = investor.CreateOrder(inputOrder);

	if (!order.IsSuccess()) {
		return order;
	}
	order->GetOriginalOrder()->TradingDay = m_systemRepo.Get()->TradingDay;
	inputOrder.FrozenMargin = order->GetFreezeMargin();
	inputOrder.FrozenFee = order->GetFreezeFee();
	inputOrder.ClientID = investor.GetClientID();
	LogOrder(__func__,order.Value.value());
	return order;
}

OrderResult CTradeServiceImpl::RspOrderInsert(data::COrder& inputOrder, data::CRspInfo const& rspInfo) {
	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, inputOrder.BrokerID, inputOrder.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("RspOrderInsert:can not find order by InvestorID [{}]", inputOrder.InvestorID));
		return OrderResult(InvalidInvestorError);
	}
	auto order = investor->FindOrder(inputOrder.OrderLocalID);
	if (!order.IsSuccess()) {
		Log::Error(fmt::format("RspOrderInsert:can not find order by OrderLocalID[{}]", inputOrder.OrderLocalID));
		return order;
	}
	if (rspInfo.ErrorID != 0) {
		order->OnInsertFailed(Error{ rspInfo.ErrorID.value(),rspInfo.ErrorMsg.value() });
		inputOrder.FrozenFee = 0.0;
		inputOrder.FrozenMargin = 0.0;
	} else {
		order->OnAccepted(inputOrder.OrderSysID.value());
	}
	inputOrder.OrderStatus = order->GetStatus();
	LogOrder(__func__, order.Value.value());
	return order;
}

OrderResult CTradeServiceImpl::ReqOrderAction(Investor& investor, data::COrderAction& orderAction) {

	auto order = investor.FindOrder(orderAction.OrderSysID);
	if (order) {
		if (order->GetOriginalOrder()->UserOrderLocalID == orderAction.UserOrderLocalID) {
			order->OnCancelSending(orderAction.UserOrderActionLocalID, orderAction.ActionLocalID);
		} else {
			Log::Info(fmt::format("ReqOrderAction: ID doesn't match ReqOrderSysID [{}] ReqUserLocalID[{}] Mem UserLocalID[{}]",
				orderAction.OrderSysID, orderAction.UserOrderLocalID, order->GetOrderLocalID()));
			return { OrderActionLocalIDSysIDNotMatchError };
		}
		LogOrder(__func__, order.Value.value());
	} else {
		Log::Info(fmt::format("ReqOrderAction:can not find order by OrderSysID [{}]", orderAction.OrderSysID));
	}
	return order;
}

OrderResult CTradeServiceImpl::RspOrderAction(data::COrderAction& orderAction, data::CRspInfo const& rspInfo) {
	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, orderAction.BrokerID, orderAction.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("RspOrderAction:can not find order by InvestorID [{}]", orderAction.InvestorID));
		return { InvalidInvestorError };
	}
	auto order = investor->FindOrder(orderAction.OrderSysID);
	if (!order) {
		Log::Error(fmt::format("RspOrderAction:can not find order by OrderLocalID [{}]", orderAction.OrderSysID));
		return order;
	}
	if (rspInfo.ErrorID != 0) {
		order->OnCancelFailed(Error{ rspInfo.ErrorID.value(), rspInfo.ErrorMsg.value() });
	} else {
//		result.Order.OnCancelAccepted();
	}
	LogOrder(__func__, order.Value.value());
	return order;
}

OrderResult CTradeServiceImpl::OnRtnOrder(data::COrder& notifyOrder) {
	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, notifyOrder.BrokerID, notifyOrder.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("OnRtnOrder:can not find order by InvestorID [{}]", notifyOrder.InvestorID));
		return { InvalidInvestorError };
	}
	
	auto order = investor->FindOrder(notifyOrder.OrderLocalID);
	if (!order) {
		Log::Info(fmt::format("OnRtnOrder:can not find order by OrderLocalID [{}], create new one", notifyOrder.OrderLocalID));
		order = investor->CreateOrder(notifyOrder,false);
		if (!order) {
			Log::Error(fmt::format("OnRtnOrder:can not create external NotifyOrder by OrderLocalID [{}], error[{}]", notifyOrder.OrderLocalID,order.Error.Code));
			return order;
		}
	}

// 	notifyOrder.UserOrderLocalID = order->GetOriginalOrder()->UserOrderLocalID;
// 	notifyOrder.UserActionLocalID = order->GetOriginalOrder()->UserActionLocalID;

	if (notifyOrder.SequenceNo > 0) {
		order->GetOriginalOrder()->SequenceNo = notifyOrder.SequenceNo;
	}
	switch (notifyOrder.OrderStatus) {
	case type::OrderStatus::NoTradeQueueing: {
		order->GetOriginalOrder()->InsertTime = notifyOrder.InsertTime;
		order->GetOriginalOrder()->TradingDay = notifyOrder.TradingDay;
		order->OnAccepted(notifyOrder.OrderSysID.value());
		break;
	}
	case type::OrderStatus::Canceled: {
		order->GetOriginalOrder()->CancelTime = notifyOrder.CancelTime;
		order->OnCancelled();
		break;
	}
	case type::OrderStatus::PartTradedQueueing: {
		order->OnOrder(notifyOrder);
		break;
	}
	case type::OrderStatus::AllTraded: {
		order->OnOrder(notifyOrder);
		break;
	}
	default:
		break;
	}
	LogOrder(__func__, order.Value.value());

	return order;
}

Result<Trade> CTradeServiceImpl::OnRtnTrade(data::CTrade& notifyTrade) {
	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, notifyTrade.BrokerID, notifyTrade.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("OnRtnTrade:can not find order by InvestorID [{}]", notifyTrade.InvestorID));
		return { InvalidInvestorError };
	}
	auto order = investor->FindOrder(notifyTrade.OrderSysID);
	if (!order) {
		Log::Error(fmt::format("OnRtnTrade:can not find order by OrderSysID [{}] Error[{}]", notifyTrade.OrderSysID,order.Error.Code));
	} else {
		auto trade = order->CreateTrade(notifyTrade);
		order->OnTrade(trade);
		LogOrder(__func__, order.Value.value());
		return trade;
	}
	return order.Error;
}


bool CTradeServiceImpl::NotifyInstrumentStatus(data::CInstrumentStatus* pInstrumentStatus) {
	return true;
// 	CInstrument* pContract = m_memoryDB.m_pInstrumentFactory->findByExchangeIDAndInstrumentID(pInstrument->ExchangeID, pInstrument->InstrumentID);
// 	m_memoryDB.m_pInstrumentFactory->addOrUpdate(pContract, pInstrument, pTransaction);

//	return m_pHandleNotify.HandleInstrumentStatusUpdate(pInstrumentStatus,  pTransaction);
}

void CTradeServiceImpl::DynamicProfitCalculate()
{
//	m_pHandleNotify.HandleDynamicCalculate();
}

Result<Investor> CTradeServiceImpl::GetInvestor(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID) {
	return Investors::GetInvestor(m_investorRepo, m_instRepo, BrokerID, InvestorID);
}

