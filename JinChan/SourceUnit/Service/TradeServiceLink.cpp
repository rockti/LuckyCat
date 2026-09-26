#include "stdafx.h"
#include "TradeServiceLink.h"
#include "LinkManager.h"
#include "ZtpDataStruct.h"
#include "ErrorDefine.h"
#include "Investor.h"
#include "MaxOrderLocalIDSinglton.h"
#include "ZTPType.h"

using namespace ztp;

extern CExchangeLinkManager* g_pExchangeLinkManager;

void Fill(data::COrder& field, Order const& order) {
	CopyZTPOrderEntity(&field, order.GetOriginalOrder());
}
void Fill(data::CTrade& field, Trade const& trade) {
	CopyTradeEntity(&field, trade.GetOriginalTrade());
}

template<class Field>
void CTradeServiceLink::PushRspToResultFlow(uint32_t tID, CZTPPackage& package, Field& field, Error error) {
	m_ztpPackage.PreparePublish(tID);
	m_ztpPackage.GetZTPHeader()->SessionID = package.GetSessionID();
	m_ztpPackage.GetZTPHeader()->RequestID = package.GetRequestID();
	data::CRspInfo rspField{ error.Code,  type::ErrorMsg(error.Msg)};
	m_ztpPackage.AddField(rspField);
	m_ztpPackage.AddField(field);
	m_ztpPackage.MakePackage();
	m_pTradeResultFlow->Append(m_ztpPackage.GetAddress(), m_ztpPackage.GetLength());
}

void CTradeServiceLink::PushErrorRspToResultFlow(uint32_t tID, CZTPPackage& package, Error error) {
	m_ztpPackage.PreparePublish(tID);
	m_ztpPackage.GetZTPHeader()->SessionID = package.GetSessionID();
	m_ztpPackage.GetZTPHeader()->RequestID = package.GetRequestID();
	data::CRspInfo rspField{ type::ErrorID(error.Code),  type::ErrorMsg(error.Msg) };
	m_ztpPackage.AddField(rspField);
	m_ztpPackage.MakePackage();
	m_pTradeResultFlow->Append(m_ztpPackage.GetAddress(), m_ztpPackage.GetLength());
}
void CTradeServiceLink::NotifyOrderToResultFlow(Order const& order) {
	data::COrder field;
	Fill(field, order);
	m_ztpPackage.PreparePublish(TID_NotifyOrder);
	m_ztpPackage.AddField(field);
	m_ztpPackage.MakePackage();
	m_pTradeResultFlow->Append(m_ztpPackage.GetAddress(), m_ztpPackage.GetLength());
}
void CTradeServiceLink::NotifyTradeToResultFlow(Trade const& trade) {
	data::CTrade field;
	Fill(field, trade);
	NotifyTradeToResultFlow(field);
}
void CTradeServiceLink::NotifyTradeToResultFlow(data::CTrade const& field) {
	m_ztpPackage.PreparePublish(TID_NotifyTrade);
	m_ztpPackage.AddField(field);
	m_ztpPackage.MakePackage();
	m_pTradeResultFlow->Append(m_ztpPackage.GetAddress(), m_ztpPackage.GetLength());
}

CTradeServiceLink::CTradeServiceLink(SessionRepository& sessionRepo,SystemStatusRepository& systemStatusRepo, BaseInfoRepository& baseInfoRepo, InvestorRepository& investorRepo, InstrumentRepository& instRepo, IFlow* pTradeResultFlow)
:m_sessionRepo(sessionRepo),m_systemStatusRepo(systemStatusRepo), m_baseInfoRepo(baseInfoRepo),m_investorRepo(investorRepo),
m_tradeService(systemStatusRepo,investorRepo, instRepo, pTradeResultFlow),m_pTradeResultFlow(pTradeResultFlow){
	m_ztpPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
}
CTradeServiceLink::~CTradeServiceLink() {
	m_ztpPackage.Release();
}

Error CTradeServiceLink::CheckSession(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID, type::SessionID const& SessionID) {
	type::BrokerID seatBrokerID(m_baseInfoRepo.Get()->ParticipantID.value());
	if (BrokerID != seatBrokerID) {
		return BrokerIDMismatchError;
	}
	auto* pUserSession = m_sessionRepo.Get(SessionID);
	if (NULL == pUserSession) {
		return SessionNotFoundError;
	}
	if (pUserSession->UserType == type::UserType::Operator) {
		//OWEN TODO, 目前Operator可以管理所有Investor，以后再说
	} else {
		if (pUserSession->LoginID != InvestorID) {
			return SessionNotFoundError;
		}
	}
	return NoError;
}

void LogOrderErrRsp(data::CRspInfo const& rspInfo, data::COrder const& inputOrder) {
	char const* direction = type::IsBuy(inputOrder) ? "B" : "S";
	char const* operation = type::IsOpen(inputOrder) ? "OPEN" : "OFFSET";
	Log::Info(fmt::format("OrderErrRsp: ErrorCode{} ErrorMessage[{}],LocalID[{}] InstrumentID[{}] Flag[{}|{}] Price[{:.2f},{}] Condition[{}]",
		rspInfo.ErrorID, rspInfo.ErrorMsg,
		inputOrder.OrderLocalID, inputOrder.InstrumentID, direction, operation,
		inputOrder.LimitPrice, inputOrder.Volume, inputOrder.OrderPriceType));
}

void LogCancelErrRsp(data::CRspInfo const& rspInfo, data::COrderAction const& orderAction) {
	Log::Info(fmt::format("CancelErrRsp: ErrorCode{} ErrorMessage[{}],OrderLocalID[{}] ActionLocalID[{}] SysID[{}]",
		rspInfo.ErrorID, rspInfo.ErrorMsg,
		orderAction.OrderLocalID,orderAction.ActionLocalID,	orderAction.OrderSysID));
}

void LogNotifyOrder(data::COrder const& order) {
	char const* direction = type::IsBuy(order) ? "B" : "S";
	char const* operation = type::IsOpen(order) ? "OPEN" : "OFFSET";
	Log::Info(fmt::format("NotifyOrder: InvestorID[{}] InstrumentID[{}] LocalID[{}] SysID[{}] Dir[{}] OP[{}] PC[{}] Volume[{}] Traded[{}] Remain[{}] Price[{:.2f} OrderStatus[{}] SeqNo[{}]",
		order.InvestorID, order.InstrumentID, order.OrderLocalID,order.OrderSysID,
		direction, operation,order.ProductClass, order.Volume, order.VolumeTrade, 
		order.VolumeRemain, order.LimitPrice, order.OrderStatus, order.SequenceNo
	));
}

void LogNotifyTrade(data::CTrade const& trade) {
	char const* direction = type::IsBuy(trade) ? "B" : "S";
	char const* operation = type::IsOpen(trade) ? "OPEN" : "OFFSET";
	Log::Info(fmt::format("NotifyTrade: InvestorID[{}] InstrumentID[{}] LocalID[{}] SysID[{}] TradeID[{}] Dir[{}] OP[{}] PC[{}] Traded[{}] Price[{:.2f} SeqNo[{}]",
		trade.InvestorID, trade.InstrumentID, trade.OrderLocalID, trade.OrderSysID,trade.TradeID,
		direction, operation, trade.ProductClass, trade.TradeVolume, trade.TradePrice,trade.SequenceNo));
}

void LogInputOrder(data::COrder const& inputOrder) {
	char const* direction = type::IsBuy(inputOrder) ? "B" : "S";
	char const* operation = type::IsOpen(inputOrder) ? "OPEN" : "OFFSET";
	Log::Info(fmt::format("InputOrder: UserLocalID[{}] InstrumentID[{}] Flag[{}|{}] Price[{:.2f},{}] Condition[{}]",
		inputOrder.UserOrderLocalID, inputOrder.InstrumentID, direction, operation,
		inputOrder.LimitPrice, inputOrder.Volume, inputOrder.OrderPriceType));
}

void LogInputCancel(data::COrderAction const& actionField) {
	Log::Info(fmt::format("OrderAction: UserLocalID[{}] InstrumentID[{}] SystemID[{}]",
		actionField.UserOrderLocalID, actionField.InstrumentID, actionField.OrderSysID));
}

void LogSpotPositionNotify(data::CSpotPositionNotify const& spotPos) {
	Log::Info(fmt::format("SpotPositionNotify: InvestorID[{}] InstrumentID[{}] OffsetFlag[{}] Volume[{}]",
		spotPos.InvestorID, spotPos.InstrumentID, spotPos.OffsetFlag,spotPos.Volume));
}

void CTradeServiceLink::HandleMessage(CZTPPackage& package) {
	switch (package.GetTID()) {
	case TID_ReqOrderInsert:				//报单请求
		handleOrderInsert(package);
		break;
	case TID_ReqOrderAction:				//撤单请求
		handleCancel(package);
		break;
	case TID_RspOrderInsert:				//报单应答(失败应答)
		handleOrderInsertError(package);
		break;
	case TID_RspOrderAction:				//撤单应答(失败应答)
		handleCancelError(package);
		break;
	case TID_NotifyOrder:					//委托回报
		handelNotifyOrder(package);
		break;
	case TID_NotifyTrade:					//成交回报
		handleNotifyTrade(package);
		break;
	case TID_NotifySeatLoginSuccess: {		//席位登录成功通知
		data::CSeatLogin loginInfoField;
		package.GetSingleField(loginInfoField);

		auto* pSeatInfo = m_baseInfoRepo.Get();
		pSeatInfo->SeatStatus = type::SeatStatus::Login;
		pSeatInfo->OrderLocalID = loginInfoField.OrderLocalID;
	}
		break;
	case TID_NotifySeatDisconnected: {		//席位断开连接通知
		data::CSeatInfo seatInfoField;
		package.GetSingleField(seatInfoField);

		//更新席位状态及最大本地报单号
		auto* pSeatInfo = m_baseInfoRepo.Get();
		pSeatInfo->SeatStatus = type::SeatStatus::Logout;
		pSeatInfo->OrderLocalID = seatInfoField.OrderLocalID;
	}
		break;
// 	case TID_NotifyInstrumentStatus: {		//合约交易状态改变
// 		data::CInstrumentStatus instrumentStatusField;
// 		package.GetSingleField(instrumentStatusField);
// 		auto* pSystemStatus = m_systemStatusRepo.Get();
// 		pSystemStatus->TradingStatus = convertToTradingStatus(instrumentStatusField.InstrumentStatus);
// 
// 		m_pZtpackage.PreparePublish(TID_NotifyInstrumentStatus);
// 		m_pZtpackage.AddField(instrumentStatusField);
// 		m_pZtpackage.MakePackage();
// 		m_pTradeResultFlow->Append(m_pZtpackage.GetAddress(), m_pZtpackage.GetLength());
// 	}
// 		break;
// 	case TID_NotifyDynamicRiskCalculate: {		//动态计算浮动盈亏
// 		m_tradeService.DynamicProfitCalculate();
// 	}
		break;
	case TID_NotifySpotPosition: {	
		handleSpotPositionNotify(package);
		break;
	}
	case TID_RtnDeferFee: {
		handleDeferFee(package);
		break;
	}
	default:
		break;
	}
    return;
}


void CTradeServiceLink::handelNotifyOrder(CZTPPackage& package) {
	data::COrder orderField;
	if (!package.GetSingleField(orderField)) {
		return;
	}
	LogNotifyOrder(orderField);
	auto result = m_tradeService.OnRtnOrder(orderField);

	if (orderField.SequenceNo > 0) {
		m_investorRepo.SetLastSGESeqNo(orderField.SequenceNo);
	}

	if (result) {
		Saver com(m_investorRepo, *result.Value->GetOriginalOrder());
		if (result->GetStatus() == type::OrderStatus::Canceled) {
			m_localIdRepository.Remove(result->GetCancelLocalID());
		}
		if (result->GetStatus() == type::OrderStatus::NoTradeQueueing) {
			m_localIdRepository.Remove(orderField.OrderLocalID);
		}
		m_ztpPackage.PreparePublish(TID_NotifyOrder);
		m_ztpPackage.AddField(*result->GetOriginalOrder());
		m_ztpPackage.MakePackage();
		m_pTradeResultFlow->Append(m_ztpPackage.GetAddress(), m_ztpPackage.GetLength());
	}
}
void CTradeServiceLink::handleNotifyTrade(CZTPPackage& package) {
	data::CTrade tradeField;
	if (!package.GetSingleField(tradeField)) {
		return;
	}
	if (tradeField.SequenceNo > 0) {
		m_investorRepo.SetLastSGESeqNo(tradeField.SequenceNo);
	}

	LogNotifyTrade(tradeField);

	auto trade = m_tradeService.OnRtnTrade(tradeField);
	if (trade.IsSuccess()) {
		Saver saver(m_investorRepo, *trade.Value->GetOriginalTrade(),true);
		NotifyTradeToResultFlow(*trade->GetOriginalTrade());
	}

}

Error checkProductClass(data::COrder const& inputOrder) {
	if (type::IsSpot(inputOrder) || type::IsDefer(inputOrder)) {
		return NoError;
	}
	return InvalidProductClassError;
}

Error checkOffsetFlag(data::COrder const& inputOrder) {
	//非延期没有开平，只有买卖
	if (type::IsSpot(inputOrder)) {
		if (type::IsBuy(inputOrder)) {
			if (!type::IsOpen(inputOrder)) {
				return InvalidOffsetFlagError;
			}
		} else {
			if (!type::IsClose(inputOrder)) {
				return InvalidOffsetFlagError;
			}
		}
	}
	return NoError;
}

void CTradeServiceLink::handleOrderInsert(CZTPPackage& package) {
	data::COrder inputOrder;
	if (!package.GetSingleField(inputOrder)) {
		return;
	}	
	LogInputOrder(inputOrder);
	inputOrder.TradingDay = m_systemStatusRepo.Get()->TradingDay;

	//找不到对应Session，直接返回
	if (auto error = CheckSession(inputOrder.BrokerID, inputOrder.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder, error);
		return;
	}

	if (auto error = checkProductClass(inputOrder); !error.IsSuccess()) {
		PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder, error);
		return;

	}

	if (auto error = checkOffsetFlag(inputOrder); !error.IsSuccess()) {
		PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder, error);
		return;
	}

	inputOrder.OrderLocalID = MaxOrderLocalIDSinglton::GetNextOrderLocalID();
	auto investor = m_tradeService.GetInvestor(inputOrder.BrokerID, inputOrder.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("ReqOrderInsert:can not find order by InvestorID [{}]", inputOrder.InvestorID));
		PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder, investor.Error);
		return;
	}
	auto result = m_tradeService.ReqOrderInsert(investor.Value.value(),inputOrder);
	if (!result) {
		PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder, result.Error);
		return;
	}

	CBaseExchApi* pExchApi = g_pExchangeLinkManager->GetSGEHandler();
	if (!pExchApi->ReqOrderInsert(inputOrder)) {
		result->OnInsertFailed(SystemError);
		PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder, SystemError);
		return;
	}

	Saver com(m_investorRepo, *result.Value->GetOriginalOrder(),true);

	LocalIdMapping mapping{ inputOrder.OrderLocalID,inputOrder.UserOrderLocalID,package.GetSessionID(), package.GetRequestID() };
	m_localIdRepository.Add(mapping);
	PushRspToResultFlow(TID_RspOrderInsert, package, inputOrder);
}
void CTradeServiceLink::handleOrderInsertError(CZTPPackage& package) {
	data::CRspInfo rspInfo;
	data::COrder order;
	auto iter = package.GetFieldTypeIterator();
	if (!iter.Retrieve(rspInfo)) return;
	iter.Next();
	if (!iter.Retrieve(order)) return;
	LogOrderErrRsp(rspInfo, order);

	auto mapping = m_localIdRepository.Find(order.OrderLocalID);
	if (!mapping) {
		Log::Warn(fmt::format("handleOrderInsertError: can not find the order by [{}]", order.OrderLocalID));
		return;
	}
	auto result = m_tradeService.RspOrderInsert(order, rspInfo);
	if (!result) {
		return;
	}
	Saver com(m_investorRepo, *result.Value->GetOriginalOrder());

	order.UserOrderLocalID = mapping->UserLocalId;
	PushRspToResultFlow(TID_ErrOrderInsert, package, order, Error{ rspInfo.ErrorID.value(),rspInfo.ErrorMsg.value() });
	m_localIdRepository.Remove(order.OrderLocalID);
}

void CTradeServiceLink::handleCancel(CZTPPackage& package) {
	data::COrderAction actionField;
	if (!package.GetSingleField(actionField)) {
		return;
	}
	LogInputCancel(actionField);

	if (auto error = CheckSession(actionField.BrokerID, actionField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(TID_RspOrderAction, package, actionField, error);
		return;
	}
	actionField.ActionLocalID = MaxOrderLocalIDSinglton::GetNextOrderLocalID();

	//在这里创建Investor是为了解决OrderResult返回时Order对象的Investor已经失效的问题
	auto investor = m_tradeService.GetInvestor(actionField.BrokerID, actionField.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("ReqOrderInsert:can not find order by InvestorID [{}]", actionField.InvestorID));
		PushRspToResultFlow(TID_RspOrderAction, package, actionField, investor.Error);
		return;
	}
	actionField.ClientID = investor->GetClientID();

	auto result = m_tradeService.ReqOrderAction(investor.Value.value(), actionField);
	if (!result) {
		PushRspToResultFlow(TID_RspOrderAction, package, actionField, result.Error);
		return;
	}

	Saver com(m_investorRepo, *result.Value->GetOriginalOrder());

	actionField.OrderLocalID = result->GetOrderLocalID();

	CBaseExchApi* pExchApi = g_pExchangeLinkManager->GetSGEHandler();
	if (!pExchApi->ReqOrderAction(actionField)) {
		result->OnCancelFailed(SystemError);
		PushRspToResultFlow(TID_RspOrderAction, package, actionField, SystemError);
		return;
	}
	LocalIdMapping mapping{ actionField.ActionLocalID,actionField.UserOrderActionLocalID,package.GetSessionID(),package.GetRequestID() };
	m_localIdRepository.Add(mapping);
	PushRspToResultFlow(TID_RspOrderAction, package, actionField);
}
void CTradeServiceLink::handleCancelError(CZTPPackage& package) {
	data::COrderAction actionField;
	data::CRspInfo rspInfoField;

	auto iter = package.GetFieldTypeIterator();
	if (!iter.Retrieve(rspInfoField)) return;
	iter.Next();
	if (!iter.Retrieve(actionField)) return;

	LogCancelErrRsp(rspInfoField, actionField);
	auto result = m_tradeService.RspOrderAction(actionField, rspInfoField);
	if (!result) {
		return;
	}

	Saver com(m_investorRepo, *result.Value->GetOriginalOrder());


	auto mapping = m_localIdRepository.Find(result->GetCancelLocalID());
	if (!mapping) {
		Log::Info(fmt::format("Can not find localID mapping, OrderLocalID[{}] ActionLocalID[{}]", actionField.UserOrderLocalID, actionField.UserOrderActionLocalID));
		return;
	}
	actionField.UserOrderLocalID = result->GetOriginalOrder()->UserOrderLocalID;
	actionField.UserOrderActionLocalID = mapping->UserLocalId;
	Log::Info(fmt::format("TID_RspOrderAction: GetReqHeaderAndOrderAction, OrderLocalID[{}] ActionLocalID[{}]", actionField.UserOrderLocalID, actionField.UserOrderActionLocalID));

	PushRspToResultFlow(TID_ErrOrderAction, package, actionField, Error{ rspInfoField.ErrorID.value(), rspInfoField.ErrorMsg.value() });
	m_localIdRepository.Remove(actionField.ActionLocalID);
}

void CTradeServiceLink::handleDeferFee(CZTPPackage& package) {
	data::CDeferFee deferFee;

	auto iter = package.GetFieldTypeIterator();
	if (!iter.Retrieve(deferFee)) return;

	Log::Info(fmt::format("CTradeServiceLink: HandleDeferFee Instrument[{}] Direction[{}] Rate[{}]",
		deferFee.InstrumentID, deferFee.Direction,deferFee.FeeRate));

	m_ztpPackage.PreparePublish(TID_RtnDeferFee);
	m_ztpPackage.AddField(deferFee);
	m_ztpPackage.MakePackage();
	m_pTradeResultFlow->Append(m_ztpPackage.GetAddress(), m_ztpPackage.GetLength());
}



void CTradeServiceLink::handleSpotPositionNotify(CZTPPackage& package) {

	data::CSpotPositionNotify positionNotify;
	if (!package.GetSingleField(positionNotify)) {
		return;
	}
// 	if (!ZTP_GET_SINGLE_FIELD(package, &positionNotify)) {
// 		return;
// 	}
	LogSpotPositionNotify(positionNotify);

	auto investor = m_tradeService.GetInvestor(positionNotify.BrokerID, positionNotify.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("SpotPositionNotify:can not find investor by InvestorID [{}] Error[{}]", positionNotify.InvestorID, investor.Error.Msg));
		return;
	}

	data::CPositionChange positionChange; 
	positionChange.BrokerID = positionNotify.BrokerID.value();
	positionChange.InvestorID = positionNotify.InvestorID.value();
	positionChange.ExchangeID = positionNotify.ExchangeID.value();
	positionChange.ChangeType = type::PositionChangeFlag::Spot;
	positionChange.InstrumentID = positionNotify.InstrumentID.value();
	positionChange.ProductClass = type::ProductClass::Spot;
	positionChange.Direction = type::IsOpen(positionNotify) ? type::Direction::Buy : type::Direction::Sell; //现货永远是买仓
	positionChange.OffsetFlag = positionNotify.OffsetFlag;
	positionChange.Volume = positionNotify.Volume;
	positionChange.ExchangeSequenceNo = m_investorRepo.GetLastSGESeqNo();
	positionChange.TradingDay = m_systemStatusRepo.Get()->TradingDay;
	positionChange.NotifyTime = positionNotify.NotifyTime;
	auto* pSavedChange = m_investorRepo.Add(positionChange);

	Saver saver(m_investorRepo, *pSavedChange, true);

	auto* pos = investor->OnPosition(*pSavedChange);
	if (pos) {
		Log::Info(fmt::format("TradeServiceLink: SpotNotify Position:InvestorID[{}] InstrumentID[{}] Pos[{}] PosCost[{:.2f}] FPos[{}],YDPos[{}] YDPosPrice[{:.2f}] FFee[{:.2f}] FMargin[{:.2f}] UserdMargin[{:.2f}]",
			pos->InvestorID, pos->InstrumentID, 
			pos->Position, pos->PositionCost, pos->FrozenPosition,
			pos->YdPosition, pos->YdPositionPrice,
			pos->FrozenFee, pos->FrozenMargin, pos->UsedMargin
		));

	}
}


