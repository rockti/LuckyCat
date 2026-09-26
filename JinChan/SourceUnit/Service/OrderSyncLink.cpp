#include "stdafx.h"
#include "OrderSyncLink.h"
#include "LinkManager.h"
#include "ZtpDataStruct.h"
#include "ErrorDefine.h"
#include "Investor.h"
#include "MaxOrderLocalIDSinglton.h"
#include "ZTPType.h"

using namespace ztp;

extern CExchangeLinkManager* g_pExchangeLinkManager;

COrderSyncLink::COrderSyncLink(SystemStatusRepository& systemStatusRepo, 
	InstrumentRepository& instRepo,
	InvestorRepository& investorRepo,
	InitOrderStore& orderStore)
:m_systemStatusRepo(systemStatusRepo), 
m_instRepo(instRepo),
m_investorRepo(investorRepo),
m_orderStore(orderStore){
}
COrderSyncLink::~COrderSyncLink() {
}

void LogSpotPositionNotify(data::CSpotPositionNotify const& spotPos);

bool COrderSyncLink::HandleMessage(CZTPPackage& package) {
	int errorCode = 0;
	char* errorMsg = NULL;
	switch (package.GetTID()) {
	case TID_NotifySeatLoginSuccess: {	//席位登录成功通知
		data::CSeatLogin loginInfoField;
		package.GetSingleField(loginInfoField);

		//更新交易日期
		auto* pStatus = m_systemStatusRepo.Get();
		if (MaxOrderLocalIDSinglton::Get() < loginInfoField.OrderLocalID && loginInfoField.OrderLocalID < 40000) {
			MaxOrderLocalIDSinglton::InitMaxOrderLocalID(loginInfoField.OrderLocalID);
		}
		if (pStatus->TradingDay != loginInfoField.TradingDay) {
			printNlog("system tradingday [{}], login response tradingday [{}]", pStatus->TradingDay, loginInfoField.TradingDay);
			::MessageBoxA(nullptr, "初始化错误，交易日期不符，请检查是否正确初始化", "初始化错误", MB_OK | MB_ICONERROR);
			exit(-1);
		}
		pStatus->TradingDay = loginInfoField.TradingDay;
		pStatus->SystemStatus = type::SystemStatus::Ready;

		printf("Start Trading\n");
		Log::Info("Start Trading\n");
	}
	break;
	case TID_NotifySeatDisconnected: {	//席位断开连接通知
		data::CSeatInfo seatInfoField;
		package.GetSingleField(seatInfoField);

// 		//更新席位状态及最大本地报单号
// 		CSeatInfo* pSeatInfo = m_memoryDB.m_pSeatInfoFactory->getFirst();
// 		m_memoryDB.m_pSeatInfoFactory->endGet();
// 		CSeatInfo theSeatInfo;
// 		theSeatInfo.init();
// 		m_memoryDB.m_pSeatInfoFactory->retrieve(pSeatInfo, &theSeatInfo);
// 		theSeatInfo.OrderLocalID = seatInfoField.OrderLocalID;
// 		theSeatInfo.SeatStatus = ZTP_SS_Logout;
// 		m_memoryDB.m_pSeatInfoFactory->update(pSeatInfo, &theSeatInfo, nullptr);
// 
// 		m_errorEngine.getLastError(&errorCode, &errorMsg);
	}
	break;
	case TID_NotifyInstrumentStatus: {	//合约交易状态通知
		data::CInstrumentStatus instrumentStatusField;
		package.GetSingleField(instrumentStatusField);
		auto* pSystemStatus = m_systemStatusRepo.Get();
		pSystemStatus->TradingStatus = convertToTradingStatus(instrumentStatusField.InstrumentStatus);
	}
	break;
	case TID_NotifyInstrumentInfo: {	//更新合约信息
		data::CInstrument instrumentField;
		auto iter = package.GetFieldTypeIterator();
		while (!iter.IsEnd()) {
			iter.Retrieve(instrumentField);
			m_instRepo.AddOrUpdate(instrumentField);
			iter.Next();
		}
	}
	break;
	case TID_NotifyOrder: 
		syncOrder(package);
		break;
	case TID_NotifyTrade: 
		syncTrade(package);
		break;
	case TID_NotifyInitFinish: {			//反演结束
//			DoAfterDataSync(pTransaction);	//连接交易账户
//			toExchTradingStatus(); //TODO OWEN 状态管理
			return true;
		}
	break;
	case TID_NotifySpotPosition: {
		data::CSpotPositionNotify positionNotify;
		if (!package.GetSingleField(positionNotify)) {
			return false;
		}
		LogSpotPositionNotify(positionNotify);

		auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, positionNotify.BrokerID, positionNotify.InvestorID);
		if (!investor) {
			Log::Error(fmt::format("OrderSync SpotPositionNotify:can not find investor by InvestorID [{}] Error[{}]", positionNotify.InvestorID, investor.Error.Msg));
			return false;
		}

		data::CPositionChange positionChange;
		positionChange.BrokerID = positionNotify.BrokerID;
		positionChange.InvestorID = positionNotify.InvestorID;
		positionChange.ExchangeID = positionNotify.ExchangeID;
		positionChange.ChangeType = type::PositionChangeFlag::Spot;
		positionChange.InstrumentID = positionNotify.InstrumentID.value();
		positionChange.ProductClass = type::ProductClass::Spot;
		positionChange.Direction = type::Direction::Buy; //现货永远是买仓
		positionChange.OffsetFlag = positionNotify.OffsetFlag;
		positionChange.Volume = positionNotify.Volume;
		positionChange.ExchangeSequenceNo = m_investorRepo.GetLastSGESeqNo();
		positionChange.TradingDay = m_systemStatusRepo.Get()->TradingDay;
		auto* pSavedChange = m_investorRepo.Add(positionChange);

		Saver saver(m_investorRepo, *pSavedChange, true);

		investor->OnPosition(*pSavedChange);		
		break;
	}
	default:	break;
	}

    return true;
}

void COrderSyncLink::syncTrade(CZTPPackage& package) {
	data::CTrade notifyTrade;
	if (!package.GetSingleField(notifyTrade)) {
		return;
	}
	m_investorRepo.SetLastSGESeqNo(notifyTrade.SequenceNo);
	Log::Info(fmt::format("Trade Sync: SysID[{}] TradeID{} SeqNo{}", notifyTrade.OrderSysID,notifyTrade.TradeID, notifyTrade.SequenceNo));
	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, notifyTrade.BrokerID, notifyTrade.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("Trade Sync :can not find order by InvestorID [{}]", notifyTrade.InvestorID));
		return;
	}
	auto order = investor->FindOrder(notifyTrade.OrderSysID);
	if (!order) {
		Log::Error(fmt::format("Trade Sync:can not find order by OrderSysID [{}] Error[{}]", notifyTrade.OrderSysID,order.Error.Code));
	} else {
		auto trade = order->CreateTrade(notifyTrade);
		trade.GetOriginalTrade()->SequenceNo = notifyTrade.SequenceNo;
		Saver com(m_investorRepo, *trade.GetOriginalTrade());
		order->OnTrade(trade);
	}
	investor->ReplayChanges(notifyTrade.SequenceNo);
}

void COrderSyncLink::syncOrder(CZTPPackage& package) {
	data::COrder notifyOrder;
	if (!package.GetSingleField(notifyOrder)) {
		return;
	}
	Log::Info(fmt::format("Order Sync Start: SysID{} SeqNo{}", notifyOrder.OrderSysID, notifyOrder.SequenceNo));
	m_investorRepo.SetLastSGESeqNo(notifyOrder.SequenceNo);

	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, notifyOrder.BrokerID, notifyOrder.InvestorID);
	if (!investor) {
		return;
	}
	bool isNew = false;
	auto result = investor->GetOrNewOrder(notifyOrder, isNew);
	if (!result) {
		Log::Info(fmt::format("Order Sync failed: SysID{} SeqNo{} Error[{}]", notifyOrder.OrderSysID, notifyOrder.SequenceNo, result.Error.Code));
		return;
	}
	Saver saver(m_investorRepo, *result.Value->GetOriginalOrder()/*, isNew*/);
	data::COrder& oOrder = *result->GetOriginalOrder();

	oOrder.SequenceNo = notifyOrder.SequenceNo;
	oOrder.TradingDay = notifyOrder.TradingDay;

	auto dbOrder = m_orderStore.Get(notifyOrder.OrderLocalID);
	if (dbOrder) {
		oOrder.UserOrderLocalID = dbOrder->UserOrderLocalID;
		oOrder.UserActionLocalID = dbOrder->UserActionLocalID;
		oOrder.UserCustom = dbOrder->UserCustom;
		oOrder.OrderOperatorID = dbOrder->OrderOperatorID;
		oOrder.CancelOperatorID = dbOrder->CancelOperatorID;
		oOrder.InsertErrorCode = dbOrder->InsertErrorCode;
		oOrder.CancelErrorCode = dbOrder->CancelErrorCode;
	} else {
		Log::Warn(fmt::format("warning OrderSync: can not find order user localID in database. OrderLocalID[{}]", result->GetOrderLocalID()));
	}
	if (MaxOrderLocalIDSinglton::IsMine(oOrder.OrderLocalID)) {
		MaxOrderLocalIDSinglton::InitMaxOrderLocalID(oOrder.OrderLocalID);
	}
	if (MaxOrderLocalIDSinglton::IsMine(oOrder.ActionLocalID)) {
		MaxOrderLocalIDSinglton::InitMaxOrderLocalID(oOrder.ActionLocalID);
	}

	switch (notifyOrder.OrderStatus) {
	case type::OrderStatus::NoTradeQueueing: {		//委托成功 尚未成交
		oOrder.InsertTime = notifyOrder.InsertTime;
		result->OnAccepted(notifyOrder.OrderSysID.value());
		break;
	}
	case type::OrderStatus::Canceled: {				//Normal撤单或者FAK自动撤单
		oOrder.CancelTime = notifyOrder.CancelTime;
		result->OnCancelled();
		break;
	}
	case type::OrderStatus::PartTradedQueueing: {	//Normal 挂单部分成交
		result->OnOrder(notifyOrder);
		break;
	}
	case type::OrderStatus::AllTraded: {			//Normal或者FAK 全部成交
		result->OnOrder(notifyOrder);
		break;
	}
	default:
		break;
	}

	investor->ReplayChanges(notifyOrder.SequenceNo);

}
