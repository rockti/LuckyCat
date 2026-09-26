#include "conn/ctp/CTPTraderHandler.h"
#include "utils/Log.h"
#include "domain/Error.h"
#include "config/Config.h"
#include "notify/ConnPublisher.h"
#include "notify/Notify.h"
#include "conn/ExchangeConn.h"
#include "model/dealing/ExchangeRequest.h"
#include "service/Service.h"
#include "service/InstrumentProgressManager.h"
#include "CTPConnection.h"
#pragma warning(disable:4100)

using namespace ztp;

int const IntervalMS = 1000;
namespace {
	inline static type::Direction ConvertDirectionFromCTP(TThostFtdcDirectionType const& direction) {
		return direction == THOST_FTDC_D_Buy ? type::Direction::Buy : type::Direction::Sell;
	}

	inline static type::Direction ConvertPosDirectionFromCTP(TThostFtdcPosiDirectionType const& direction) {
		return direction == THOST_FTDC_PD_Long ? type::Direction::Buy : type::Direction::Sell;
	}

	inline static type::OffsetFlag ConvertOperationFromCTP(TThostFtdcOffsetFlagType const& operation) {
		switch (operation) {
		case THOST_FTDC_OF_Open:
			return type::OffsetFlag::Open;
		case THOST_FTDC_OF_Close:
		case THOST_FTDC_OF_CloseYesterday:
			return type::OffsetFlag::Close;
		case THOST_FTDC_OF_CloseToday:
			return type::OffsetFlag::CloseToday;
		default:
			return type::OffsetFlag::Unknown;
		}
	}
	inline static type::OrderPriceType ConvertOrderTypeFromCTP(CThostFtdcOrderField const& orderField) {
		if (orderField.TimeCondition == THOST_FTDC_TC_GFD && orderField.VolumeCondition == THOST_FTDC_VC_AV) {
			return type::OrderPriceType::LimitPrice;
		} else if (orderField.TimeCondition == THOST_FTDC_TC_IOC && orderField.VolumeCondition == THOST_FTDC_VC_AV) {
			return type::OrderPriceType::FAK;
		} else if (orderField.TimeCondition == THOST_FTDC_TC_IOC && orderField.VolumeCondition == THOST_FTDC_VC_CV) {
			return type::OrderPriceType::FOK;
		} else {
			return type::OrderPriceType::LimitPrice;
		}
	}
	inline static ExchangeStatus ConvertExchangeStatusFromCTP(TThostFtdcInstrumentStatusType status) {
		switch (status) {
		case THOST_FTDC_IS_BeforeTrading:
			return ExchangeStatus::BEFORE_TRADING;
		case THOST_FTDC_IS_NoTrading:
			return ExchangeStatus::NO_TRADING;
		case THOST_FTDC_IS_Continous:
			return ExchangeStatus::CONTINOUS;
		case THOST_FTDC_IS_AuctionOrdering:
			return ExchangeStatus::AUCTION_ORDERING;
		case THOST_FTDC_IS_AuctionBalance:
			return ExchangeStatus::AUCTION_BALANCE;
		case THOST_FTDC_IS_AuctionMatch:
			return ExchangeStatus::AUCTION_MATCH;
		case THOST_FTDC_IS_Closed:
			return ExchangeStatus::CLOSED;
		default:
			return ExchangeStatus::UNKNOW;
		}
	}
	inline static type::OrderStatus ConvertOrderStatus(TThostFtdcOrderStatusType status) {
		switch (status) {
		case THOST_FTDC_OST_AllTraded:
			return type::OrderStatus::AllTraded;
		case THOST_FTDC_OST_PartTradedQueueing:
			return type::OrderStatus::PartTradedQueueing;
		case THOST_FTDC_OST_PartTradedNotQueueing:
			return type::OrderStatus::Canceled; 
		case THOST_FTDC_OST_NoTradeQueueing:
			return type::OrderStatus::NoTradeQueueing;
		case THOST_FTDC_OST_NoTradeNotQueueing:
			return type::OrderStatus::NoTradeNotQueueing;
		case THOST_FTDC_OST_Canceled:
			return type::OrderStatus::Canceled;
		case THOST_FTDC_OST_Unknown:
		case THOST_FTDC_OST_NotTouched:
		case THOST_FTDC_OST_Touched:
			return type::OrderStatus::Unknown;
		default:
			return type::OrderStatus::Unknown;
		}
	}
	inline static type::Exchange ConvertExchangeIDFromCTP(TThostFtdcExchangeIDType const& exchange) {
		std::string_view exchangeView = exchange;
		if (exchangeView == "SHFE") {
			return type::Exchange::SHFE;
		}
		if (exchangeView == "CFFEX") {
			return type::Exchange::CFFEX;
		}
		if (exchangeView == "DCE") {
			return type::Exchange::DCE;
		}
		if (exchangeView == "CZCE") {
			return type::Exchange::CZCE;
		}
		return type::Exchange::SHFE;

	}
}

CTPTraderHandler::CTPTraderHandler(TradeSystemConfig const& config, ConnPublisher& publisher)
	:m_config(config),m_publisher(publisher),m_pTraderApi(0) {
}

void CTPTraderHandler::OnFrontConnected() {
	Log::Info("CTPTraderHandler: FrontConnected");
	TradePlatformNotification tn;
	tn.Platform = TradePlatform::CTP;
	tn.Type = TradePlatformNotification::CONNECTED;
	tn.SubSystem = SubSystem::Trade;
	m_publisher.NotifyUI(tn);
	m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::TradeConnect);
}
void CTPTraderHandler::OnFrontDisconnected(int nReason) {
	TradePlatformNotification tn;
	tn.Platform = TradePlatform::CTP;
	tn.Type = TradePlatformNotification::DISCONNECTED;
	tn.SubSystem = SubSystem::Trade;
	m_publisher.NotifyUI(tn);
	Log::Error(fmt::format("CTPTraderHandler: FrontDisconnected, Reason[{:x}]", nReason));
}
void CTPTraderHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pRspAuthenticateField) {
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::TradeLogin, AuthFailed);
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGIN_FAILED;
		tn.SubSystem = SubSystem::Trade;
		tn.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(tn);
		auto msg = fmt::format("CTPTraderHandler: Authenticate Failed, ErrorCode[{}] ErrorMsg[{}] BrokerID[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspAuthenticateField->BrokerID);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::TradeLogin, tn.Error);
		Log::Error(msg);
	} else {
		Log::Info("CTPTraderHandler: Authenticate Success");
		::_sleep(IntervalMS); // CTP 1 second limitation
		CThostFtdcReqUserLoginField loginReq = {0};
		strcpy_s(loginReq.BrokerID, m_config.Credential.BrokerID.c_str());
		strcpy_s(loginReq.UserID, m_investor.value());
		strcpy_s(loginReq.UserProductInfo, m_config.Credential.UserProductInfo.c_str());
		strcpy_s(loginReq.Password, m_password.value());
		int rtnCode = m_pTraderApi->ReqUserLogin(&loginReq, RequestIdACC::Generate());
		if (0 != rtnCode) {
			TradePlatformNotification tn;
			tn.Platform = TradePlatform::CTP;
			tn.Type = TradePlatformNotification::LOGIN_FAILED;
			tn.SubSystem = SubSystem::Trade;
			tn.Error = SystemError;
			m_publisher.NotifyUI(tn);
			Log::Error(fmt::format("CTPTraderHandler: ReqUserLogin() Failed, RtnCode[{}]", rtnCode));
			m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::TradeLogin, SystemError);
		}
	}
}
void CTPTraderHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pRspUserLogin) {
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::TradeLogin, SystemError);
	}

	if (IsErrorRspInfo(pRspInfo)) {
		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGIN_FAILED;
		tn.SubSystem = SubSystem::Trade;
		tn.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(tn);
		auto msg = fmt::format("CTPTraderHandler: UserLogin Failed, ErrorCode[{}] ErrorMsg[{}] BrokerID[{}] UserID[{}] Password[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->BrokerID, pRspUserLogin->UserID, m_config.Credential.Password.c_str());
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::TradeLogin, tn.Error);
	} else {
		Log::Info(fmt::format("CTPTraderHandler: UserLogin Success, MaxOrderRef[{}]", pRspUserLogin->MaxOrderRef));
		_sleep(IntervalMS); // CTP 1 second limitation
		SettlementInfoConfirm();		//登录成功先确认结算单
	}
}
void CTPTraderHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pUserLogout)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		Log::Warn(fmt::format("CTPTraderHandler: UserLogout Failed, ErrorCode[%d], ErrorMsg[%s]", pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	} else {
		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGOUT;
		tn.SubSystem = SubSystem::Trade;
		m_publisher.NotifyUI(tn);
		Log::Info("CTPTraderHandler: UserLogout Success");
	}
}

void CTPTraderHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		Log::Error(fmt::format("CTPTraderHandler: OnRspError, ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg));

		ErrorNotification noti;
		noti.Platform = TradePlatform::CTP;
		noti.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(noti);
	}
}
// CTP委托参数校验错误应答，详见《综合交易平台交易API特别说明》
// CTP收到报单指令，如果没有通过参数校验(资金不足等)，拒绝接受报单指令。用户就会收到OnRspOrderInsert消息，其中包含了错误编码和错误消息。
// 如果CTP接受了报单指令，用户不会收到OnRspOrderInser，而会收到OnRtnOrder，用来更新委托状态。
// 交易所收到报单后，如果通过校验则用户会收到OnRtnOrder、OnRtnTrade， 如果交易所认为报单错误，用户就会收到OnErrRtnOrder*。
void CTPTraderHandler::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInputOrder)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		InvestorOrder order;
		order.Platform = TradePlatform::CTP;
		auto instCode = type::InstrumentCode(pInputOrder->InstrumentID);
		auto instID = ServiceSinglton::Services().Instrument->toID(instCode);
		if (instID == INVALID_INSTRUMENT_ID) {
			Log::Warn(fmt::format("CTPTraderHandler: OnRspOrderAction filter not support Instrument[{}], UserID[{}] LocalID[{}]"
				, pInputOrder->InstrumentID, pInputOrder->InvestorID, pInputOrder->OrderRef));
			return;
		}

		OrderErrorNotification noti;
		noti.Platform = TradePlatform::CTP;
		noti.Type = OrderErrorNotification::Insert;
		noti.OrderLocalID = atoi(pInputOrder->OrderRef);
		noti.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(noti);
		m_publisher.PushResponse(noti);

		char const* direction = IsBuy(ConvertDirectionFromCTP(pInputOrder->Direction)) ? "B" : "S";
		auto msg = fmt::format("CTP OrderInsert Failed:  ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] Dir[{}] Opt[{}] Info[{:.3f},{}] LocalID[{}]"
			,pRspInfo->ErrorID,pRspInfo->ErrorMsg,pInputOrder->InvestorID,pInputOrder->InstrumentID,direction,pInputOrder->CombOffsetFlag,pInputOrder->LimitPrice,order.Volume,pInputOrder->OrderRef);
		Log::Error(msg);
	}
}
// CTP撤单参数校验错误应答，详见《综合交易平台交易API特别说明》	注意：由于已经成交导致的撤单失败不会再这里给出应答
void CTPTraderHandler::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInputOrderAction)
		return;

	if (THOST_FTDC_AF_Delete != pInputOrderAction->ActionFlag)		//目前只支持撤单
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		if (pRspInfo->ErrorID != 24 /*不允许重复撤单*/ && pRspInfo->ErrorID != 26/*报单已全成交或已撤销*/) {
			auto instCode = type::InstrumentCode(pInputOrderAction->InstrumentID);
			auto instID = ServiceSinglton::Services().Instrument->toID(instCode);
			if (instID == INVALID_INSTRUMENT_ID) {
				Log::Warn(fmt::format("CTPTraderHandler: OnRspOrderAction filter not support Instrument[{}], UserID[{}] LocalID[{}]"
					, pInputOrderAction->InstrumentID, pInputOrderAction->InvestorID, pInputOrderAction->OrderRef));
				return;
			}
			OrderErrorNotification noti;
			noti.Platform = TradePlatform::CTP;
			noti.Type = OrderErrorNotification::Cancel;
			noti.OrderLocalID = atoi(pInputOrderAction->OrderRef);
			noti.CancelLocalID = pInputOrderAction->OrderActionRef;
			noti.OrderSysID = pInputOrderAction->OrderSysID;
			noti.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
			m_publisher.NotifyUI(noti);
			m_publisher.PushResponse(noti);

			auto msg = fmt::format("CTP OrderAction Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] ActionRef[{}] OrderRef[{}] SysID[{}]"
				, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrderAction->InvestorID, pInputOrderAction->InstrumentID,
				pInputOrderAction->OrderActionRef, pInputOrderAction->OrderRef, pInputOrderAction->OrderSysID);
			Log::Info(msg);
		}
	}
}
void CTPTraderHandler::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pSettlementInfoConfirm)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: SettlementInfoConfirm Failed, ErrorCode[{}] ErrorMsg[{}] UserID[{}]"
			,pRspInfo->ErrorID, pRspInfo->ErrorMsg, pSettlementInfoConfirm->InvestorID);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);

		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGIN_FAILED;
		tn.SubSystem = SubSystem::Trade;
		tn.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::TradeLogin, tn.Error);
	} else {
		Log::Info("CTPTraderHandler: SettlementInfoConfirm Success");
		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGIN;
		tn.SubSystem = SubSystem::Trade;
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::TradeLogin);
	}
}
void CTPTraderHandler::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pTradingCode)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: QryTradingCode Failed, ErrorCode[{}] ErrorMsg[{}] UserID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pTradingCode->InvestorID);
		Log::Error(msg);
	}
}

void CTPTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pTradingAccount) {
		auto msg = fmt::format("CTPTraderHandler: QryTradingAccount empty");
		Log::Warn(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryAccount, SystemError);
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: QryTradingAccount Failed, ErrorCode[{}] ErrorMsg[{}] AccountID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pTradingAccount->AccountID);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryAccount, Error{pRspInfo->ErrorID, pRspInfo->ErrorMsg});
	} else {
		InvestorAccount account;
		account.Platform = TradePlatform::CTP;
		account.InvestorID = pTradingAccount->AccountID;
		account.Balance = pTradingAccount->Balance;
		account.Available = pTradingAccount->Available;
		account.Margin = pTradingAccount->CurrMargin;
		account.Fee = pTradingAccount->Commission;
		account.CloseProfit = pTradingAccount->CloseProfit;
		account.FrozenFee = pTradingAccount->FrozenCommission;
		account.FrozenMargin = pTradingAccount->FrozenMargin;
		account.PreBalance = pTradingAccount->PreBalance;
		account.Platform = TradePlatform::CTP;
		m_publisher.NotifyUI(account);

		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryAccount);
		Log::Error(fmt::format("CTPTraderHandler: QryTradingAccount Success, Capital[{:.2f}] Available[{:.2f}] Margin[{:.2f}] Profit[{:.2f}] Fee[{:.2f}]"
			,pTradingAccount->Balance, pTradingAccount->Available, pTradingAccount->CurrMargin, pTradingAccount->CloseProfit, pTradingAccount->Commission));
	}
}

char* trimWhitespace(char* str) {
	char* end;

	// Trim leading space
	while (isspace((unsigned char)*str)) str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;

	// Write new null terminator character
	end[1] = '\0';

	return str;
}
// CTP持仓里Position字段为同方向，总持仓YdPosition为此方向老仓
void CTPTraderHandler::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	if (NULL == pOrder)
		return;
	InvestorOrder order;
	order.Platform = TradePlatform::CTP;
	order.InstrumentCode = type::InstrumentCode(pOrder->InstrumentID);
	order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
	bool isEmpty = (0 == strlen(pOrder->OrderSysID));
	if ((order.InstID == INVALID_INSTRUMENT_ID) && (false==isEmpty)) {
		Log::Warn(fmt::format("CTPTraderHandler: OnRtnOrder filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			,pOrder->InstrumentID, pOrder->InvestorID, pOrder->OrderSysID, pOrder->OrderRef));
		return;
	}
	order.Exchange = ConvertExchangeIDFromCTP(pOrder->ExchangeID);
	order.Direction = ConvertDirectionFromCTP(pOrder->Direction);
	order.OffsetFlag = ConvertOperationFromCTP(pOrder->CombOffsetFlag[0]);
	order.OrderPriceType = ConvertOrderTypeFromCTP(*pOrder);
	order.LimitPrice = pOrder->LimitPrice;
	order.Volume = pOrder->VolumeTotalOriginal;
 	order.VolumeRemain = pOrder->VolumeTotal;		
 	order.VolumeTrade = pOrder->VolumeTraded;
	order.OrderLocalID = atoi(pOrder->OrderRef);
	order.OrderSysID = pOrder->OrderSysID;
	order.OrderStatus = ConvertOrderStatus(pOrder->OrderStatus);
	order.UniqueID = pOrder->OrderRef;
	if (order.UniqueID.empty()) {
		order.UniqueID = pOrder->OrderSysID;
	}
	order.InsertTime = pOrder->InsertTime;
	order.CancelTime = pOrder->CancelTime;

	//交易所强制撤单（参数不对）
	if (order.OrderSysID.empty() && pOrder->OrderStatus == THOST_FTDC_OST_Canceled) { 
		OrderErrorNotification oen;
		oen.Type = OrderErrorNotification::Insert;
		oen.OrderLocalID = order.OrderLocalID;
		oen.Error = SystemError;
		oen.Error.Msg = pOrder->StatusMsg;
		m_publisher.NotifyUI(oen);
		m_publisher.PushResponse(oen);
	} else {
		m_publisher.NotifyUI(OrderRtnNotification{ order });
		m_publisher.PushResponse(order);
	}

	char const* direction = IsBuy(order.Direction) ? "B" : "S";
	Log::Info(fmt::format("CTPTraderHandler OnRtnOrder: UserID[{}] Instrument[{}] Dir[{}] Opt[{:c}] Price[{:.3f}] Quantity[{}] Deal[{}] Remain[{}] SystemID[{}] LocalID[{}] Type[{:c}] OrderStatus[{:c}] SubmitStatus[{:c}]",
		pOrder->InvestorID, pOrder->InstrumentID, direction, pOrder->CombOffsetFlag[0], pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal, 
		pOrder->OrderSysID, pOrder->OrderRef, order.OrderPriceType, pOrder->OrderStatus, pOrder->OrderSubmitStatus));
}
void CTPTraderHandler::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	if (NULL == pTrade)
		return;

	InvestorTrade trade;
	trade.Platform = TradePlatform::CTP;
	trade.InstrumentCode = type::InstrumentCode(pTrade->InstrumentID);
	trade.InstID = ServiceSinglton::Services().Instrument->toID(trade.InstrumentCode);
	if (trade.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("CTPTraderHandler: OnRtnTrade filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pTrade->InstrumentID ,pTrade->InvestorID, pTrade->OrderSysID, pTrade->OrderLocalID));
		return;
	}
	
	trade.Exchange = ConvertExchangeIDFromCTP(pTrade->ExchangeID);
	trade.Direction = ConvertDirectionFromCTP(pTrade->Direction);
	trade.OffsetFlag = ConvertOperationFromCTP(pTrade->OffsetFlag);
	trade.TradeVolume = pTrade->Volume;		//OnRtnTrade里的成交量是增量
	trade.TradeTime = pTrade->TradeTime;
	trade.TradePrice = pTrade->Price;
	trade.OrderLocalID = atoi(pTrade->OrderRef);
	trade.OrderSysID =  pTrade->OrderSysID;
	trade.TradeID = pTrade->TradeID;
	m_publisher.NotifyUI(TradeNotification{ trade });
	m_publisher.PushResponse(trade);

	char const* direction = IsBuy(trade.Direction) ? "B" : "S";
	Log::Info(fmt::format("CTPTraderHandler OnRtnTrade: UserID[{}] Instrument[{}] Dir[{}] Opt[{:c}] Price[{:.3f}] Quantity[{}] SystemID[{}] LocalID[{}]",
		pTrade->InvestorID, pTrade->InstrumentID, direction, pTrade->OffsetFlag, pTrade->Price, pTrade->Volume, pTrade->OrderSysID, pTrade->OrderRef));
}

//交易所错误委托回报
void CTPTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {
	if (NULL == pInputOrder)
		return;
	
	if (IsErrorRspInfo(pRspInfo)) {
		InvestorOrder order;
		order.Platform = TradePlatform::CTP;
		order.InstrumentCode = type::InstrumentCode(pInputOrder->InstrumentID);
		order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
		if (INVALID_INSTRUMENT_ID == order.InstID) {
			Log::Warn(fmt::format("CTPTraderHandler: OnErrRtnOrderInsert received not support Instrument[{}], userID[{}] localID[{}] "
				,pInputOrder->InstrumentID, pInputOrder->InvestorID, pInputOrder->OrderRef));
			return;
		}

		OrderErrorNotification noti;
		noti.Platform = TradePlatform::CTP;
		noti.Type = OrderErrorNotification::Insert;
		noti.OrderLocalID = atoi(pInputOrder->OrderRef);
		noti.Error = { pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(noti);
		m_publisher.PushResponse(noti);
		char const* direction = ConvertDirectionFromCTP(pInputOrder->Direction) == type::Direction::Sell ? "S" : "B";
		auto msg = fmt::format("Exchange OrderInsert Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] Dir[{}] Opt[{}] Info[{:.3f},{}] LocalID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrder->InvestorID, pInputOrder->InstrumentID, direction, pInputOrder->CombOffsetFlag, pInputOrder->LimitPrice, pInputOrder->VolumeTotalOriginal, pInputOrder->OrderRef);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
//交易所撤单错误回报
void CTPTraderHandler::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {
	if (NULL == pOrderAction)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		InvestorOrder order;
		order.Platform = TradePlatform::CTP;
		order.InstrumentCode = type::InstrumentCode(pOrderAction->InstrumentID);
		order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
		if (INVALID_INSTRUMENT_ID == order.InstID) {
			Log::Warn(fmt::format("CTPTraderHandler OnErrRtnOrderAction received not exist Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}] ActionID[{}]",
				pOrderAction->InstrumentID, pOrderAction->InvestorID, pOrderAction->OrderSysID, pOrderAction->OrderLocalID, pOrderAction->ActionLocalID));
			return;
		}

		//这两种情况视为逻辑正确，其他视为撤单失败 
		if (pRspInfo->ErrorID != 24 /*重复撤单*/ && pRspInfo->ErrorID != 26/*全部成交或者已撤*/) {
			OrderErrorNotification noti;
			noti.Platform = TradePlatform::CTP;
			noti.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
			noti.OrderLocalID = atoi(pOrderAction->OrderRef);
			noti.CancelLocalID = pOrderAction->OrderActionRef;
			noti.OrderSysID = pOrderAction->OrderSysID;
			m_publisher.NotifyUI(noti);
			m_publisher.PushResponse(noti);

			auto msg = fmt::format("Exchange OrderAction Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] SystemID[{}] LocalID[{}] "
				,pRspInfo->ErrorID,pRspInfo->ErrorMsg,pOrderAction->InvestorID,pOrderAction->InstrumentID,pOrderAction->OrderSysID,pOrderAction->OrderLocalID);
			m_publisher.NotifyUI(msg);
			Log::Error(msg);
		}
	}
}

void CTPTraderHandler::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus) {
	if (NULL == pInstrumentStatus)
		return;

	bool notifyTag = false;
	ExchangeNotification notification;
	notification.Platform = TradePlatform::CTP;
	notification.Status = ConvertExchangeStatusFromCTP(pInstrumentStatus->InstrumentStatus);
	type::VarietyCode varityCode{ pInstrumentStatus->InstrumentID };
	if (0 == strcmp("SHFE", pInstrumentStatus->ExchangeID) && 
		ServiceSinglton::Services().Instrument->HasVariety(TradePlatform::CTP, varityCode)) {
		notification.Exchange = ConvertExchangeIDFromCTP(pInstrumentStatus->ExchangeID);
		if (m_exchangeStatusMap[notification.Exchange] != notification.Status) {
			m_exchangeStatusMap[notification.Exchange] = notification.Status;
			notifyTag = true;
		}
	}

	if (notifyTag) {
		m_publisher.NotifyUI(notification);
		m_publisher.PushResponse(notification);
		notifyTag = false;
		Log::Info(fmt::format("Exchange[{}] Status[{}] Inst[{}]", GetExchangeDescribe(notification.Exchange), pInstrumentStatus->InstrumentStatus, pInstrumentStatus->InstrumentID));
	}
}

void CTPTraderHandler::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo != nullptr && pRspInfo->ErrorID != ERROR_NONE) {
		Log::Error(fmt::format("CTPTraderHandler: OnRspQryInstrument failed Error[{}] Msg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg));
		return;
	}
	if (pInstrument == nullptr) {
		Log::Error("CTPTraderHandler: OnRspQryInstrument No Instrument");
		return;
	}

	Instrument inst;
	inst.Platform = TradePlatform::CTP;
	inst.Exchange = ConvertExchangeIDFromCTP(pInstrument->ExchangeID);
	inst.Code = pInstrument->InstrumentID;
	inst.LongPosLimit = 100000;
	inst.ShortPosLimit = 100000;
	inst.MaxOrderVolume = pInstrument->MaxLimitOrderVolume;
	inst.MinOrderVolume = pInstrument->MinLimitOrderVolume;
	inst.Name = pInstrument->InstrumentName;
	inst.ProductClass = type::ProductClass::Future;
	inst.UnitPrice = pInstrument->PriceTick;
	inst.VolumeMultiple = pInstrument->VolumeMultiple;
	inst.VarityCode = pInstrument->ProductID;
	auto instService = ServiceSinglton::Services().Instrument;
	if (instService->HasVariety(TradePlatform::CTP,inst.VarityCode)) {
		instService->Add(inst);
		m_publisher.NotifyUI(inst);
	}

	if (bIsLast) {
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryInstrument);

		std::vector<type::InstrumentCode> codes;
		auto insts = ServiceSinglton::Services().Instrument->GetAll(TradePlatform::CTP);
		for (auto const& inst : insts) {
			codes.push_back(inst.Code);
		}
		InstrumentProgressManager::CTP().InitInstruments(codes);
		for (auto const& inst : insts) {
			InstrumentProgressManager::CTP().AddInstDone(inst.Code);
		}
		Log::Info(fmt::format("CTPTraderHandler: OnRspQryInstrument Finished Total[{}]", insts.size()));
	}
}
void CTPTraderHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInvestorPosition) {
		Log::Info("CTPTraderHandler: There is no position info be queried");
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryPosition);
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: QueryPosition Failed, ErrorCode[{}] ErrorMsg[{}] UserID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInvestorPosition->InvestorID);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryPosition, Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg });
	} 
	else {
		auto instID = ServiceSinglton::Services().Instrument->toID(type::InstrumentCode(pInvestorPosition->InstrumentID));
		InvestorPosition position;
		position.Platform = TradePlatform::CTP;
		if (0 != pInvestorPosition->Position) {
			FillingPositionNotification(*pInvestorPosition, position, instID);
			if (position.InstID != INVALID_INSTRUMENT_ID) {
				m_publisher.PushResponse(position);
				m_publisher.NotifyUI(position);
			}

			char const* direction = (type::Direction::Buy == position.Direction) ? "B" : "S";
			Log::Info(fmt::format("CTPTraderHandler：OnRspQryInvestorPosition: UserID[{}] Instrument[{}] Dir[{}] Total[{}] Today[{}] History[{}] AvgPrice[{:.3f}] YDAvg[{:.3f}]",
				pInvestorPosition->InvestorID, pInvestorPosition->InstrumentID, direction, pInvestorPosition->Position, position.Position, position.YdPosition, position.PositionCost, position.YdPositionPrice));
		}

		if (bIsLast) {
			Log::Info("CTPTraderHandler: QueryPosition Success");
			m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryPosition);
		}
	}
}

void CTPTraderHandler::OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
		auto msg = fmt::format("CTPTraderHandler: OnRspQryOrder Failed ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryOrder, Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg });
		return;
	}

	if (pOrder == nullptr) {
		auto msg = fmt::format("CTPTraderHandler: OnRspQryOrder No orders");
		Log::Info(msg);
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryOrder);
		return;
	}

	InvestorOrder order;
	order.Platform = TradePlatform::CTP;
	order.InstrumentCode = type::InstrumentCode(pOrder->InstrumentID);
	order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
	order.Exchange = ConvertExchangeIDFromCTP(pOrder->ExchangeID);
	bool isEmpty = (0 == strlen(pOrder->OrderSysID));
	if ((order.InstID == INVALID_INSTRUMENT_ID) && (false == isEmpty)) {
		Log::Warn(fmt::format("CTPTraderHandler: OnRspQryOrder filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pOrder->InstrumentID, pOrder->InvestorID, pOrder->OrderSysID, pOrder->OrderLocalID));
		if (bIsLast) {
			Log::Info("CTPTraderHandler: QueryOrders Success");
			m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryOrder);
		}
		return;
	}
	order.InvestorID = pOrder->InvestorID;
	order.Direction = ConvertDirectionFromCTP(pOrder->Direction);
	order.OffsetFlag = ConvertOperationFromCTP(pOrder->CombOffsetFlag[0]);
	order.OrderPriceType = ConvertOrderTypeFromCTP(*pOrder);
	order.InsertTime = pOrder->InsertTime;
	order.CancelTime = pOrder->CancelTime;
	order.LimitPrice = pOrder->LimitPrice;
	order.Volume = pOrder->VolumeTotalOriginal;
 	order.VolumeTrade = pOrder->VolumeTraded;
 	order.VolumeRemain = pOrder->VolumeTotal;
	order.OrderLocalID = atoi(pOrder->OrderLocalID);
	order.OrderSysID = pOrder->OrderSysID;
	order.OrderStatus = ConvertOrderStatus(pOrder->OrderStatus);
	order.UniqueID = pOrder->OrderRef;
	if (order.UniqueID.empty()) {
		order.UniqueID = pOrder->OrderSysID;
	}

	m_publisher.NotifyUI(order);
	if (bIsLast) {
		Log::Info("CTPTraderHandler: QueryOrders Success");
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryOrder);
	}

	char const* direction = IsBuy(order.Direction) ? "B" : "S";
	Log::Info(fmt::format("CTPTraderHandler OnRspQryOrder: UserID[{}] Instrument[{}] Dir[{}] Opt[{:c}] Price[{:.3f}] Quantity[{}] Deal[{}] Remain[{}] SystemID[{}] LocalID[{}] Type[{:c}] OrderStatus[{:c}] SubmitStatus[{:c}]",
		pOrder->InvestorID, pOrder->InstrumentID, direction, pOrder->CombOffsetFlag[0], pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal,
		pOrder->OrderSysID, pOrder->OrderRef, order.OrderPriceType, pOrder->OrderStatus, pOrder->OrderSubmitStatus));

}
void CTPTraderHandler::OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
		auto msg = fmt::format("CTPTraderHandler: OnRspQryTrade Failed ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryTrade, Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg });
		return;
	}

	if (pTrade == nullptr) {
		auto msg = fmt::format("CTPTraderHandler: OnRspQryTrade No orders");
		Log::Info(msg);
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryTrade);
		return;
	}

	InvestorTrade trade;
	trade.Platform = TradePlatform::CTP;
	trade.InstrumentCode = type::InstrumentCode(pTrade->InstrumentID);
	trade.InstID = ServiceSinglton::Services().Instrument->toID(trade.InstrumentCode);
	trade.Exchange = ConvertExchangeIDFromCTP(pTrade->ExchangeID);
	bool isEmpty = (0 == strlen(pTrade->OrderSysID));
	if ((trade.InstID == INVALID_INSTRUMENT_ID) && (false == isEmpty)) {
		Log::Warn(fmt::format("CTPTraderHandler: OnRspQryTrade filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pTrade->InstrumentID, pTrade->InvestorID, pTrade->OrderSysID, pTrade->OrderLocalID));
		if (bIsLast) {
			Log::Info("CTPTraderHandler: QueryTrades Success");
			m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryTrade);
		}
		return;
	}
	trade.InvestorID = pTrade->InvestorID;
	trade.Direction = ConvertDirectionFromCTP(pTrade->Direction);
	trade.OffsetFlag = ConvertOperationFromCTP(pTrade->OffsetFlag);
	trade.OrderLocalID = atoi(pTrade->OrderLocalID);
	trade.OrderSysID = pTrade->OrderSysID;
	trade.ProductClass = type::ProductClass::Future;
	trade.TradeID = pTrade->TradeID;
	trade.TradePrice = pTrade->Price;
	trade.TradeVolume = pTrade->Volume;
	trade.TradeTime = pTrade->TradeTime;
	m_publisher.NotifyUI(trade);
	if (bIsLast) {
		Log::Info("CTPTraderHandler: QueryTrades Success");
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryTrade);
	}
}
void CTPTraderHandler::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pInstrumentCommissionRate == nullptr) {
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryFee, SystemError);
		auto msg = fmt::format("CTPTraderHandler: OnRspQryInstrumentCommissionRate No Fees");
		return;
	}
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryFee, Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg });

		auto msg = fmt::format("CTPTraderHandler: OnRspQryInstrumentCommissionRate failed Error[{}] Msg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		return;
	}

	if (InstrumentProgressManager::CTP().IsFeeDone()) {
		return;
	}

	InvestorFee fee;
	fee.Platform = TradePlatform::ZTP;
	fee.BrokerID = pInstrumentCommissionRate->BrokerID;
	fee.InvestorID = m_investor;
	fee.InstrumentCode = pInstrumentCommissionRate->InstrumentID;
	auto instService = ServiceSinglton::Services().Instrument;
	auto iter = m_queryFeeMapping.find(nRequestID);
	if (iter == m_queryFeeMapping.end()) {
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryFee, Error{ SystemError.Code, "未找到对应的RequestID的InstCode" });
		auto msg = fmt::format("CTPTraderHandler: QueryFee failed with RequestID RequestID[{}] InstCode[{}]", nRequestID, pInstrumentCommissionRate->InstrumentID);
		Log::Error(msg);
		return;
	}
	auto inst = instService->Get(iter->second);

	if (inst) {
		fee.Exchange = inst->Exchange;
		fee.VarityCode = inst->VarityCode;
		fee.InstrumentCode = inst->Code;
		fee.InstID = inst->InstID;
		fee.OffsetFeeAmt = pInstrumentCommissionRate->CloseRatioByVolume;
		fee.OffsetFeeRate = pInstrumentCommissionRate->CloseRatioByMoney;
		fee.OffsetTodayFeeAmt = pInstrumentCommissionRate->CloseTodayRatioByVolume;
		fee.OffsetTodayFeeRate = pInstrumentCommissionRate->CloseTodayRatioByMoney;
		fee.OpenFeeAmt = pInstrumentCommissionRate->OpenRatioByVolume;
		fee.OpenFeeRate = pInstrumentCommissionRate->OpenRatioByMoney;
		instService->AddVarietyFeeByPresentInstrument(TradePlatform::CTP, fee.VarityCode, fee);
		InstrumentProgressManager::CTP().AddVarietyFeeDone(fee.VarityCode);
		m_publisher.NotifyUI(fee);
	}
	if (InstrumentProgressManager::CTP().IsFeeDone()) {
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryFee);
	}
	m_queryFeeMapping.erase(iter);
}
void CTPTraderHandler::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pInstrumentMarginRate == nullptr) {
		auto msg = fmt::format("CTPTraderHandler: OnRspQryInstrumentMarginRate No margins");
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryMargin, SystemError);
		return;
	}
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
		auto msg = fmt::format("CTPTraderHandler: OnRspQryInstrumentMarginRate failed Error[{}] Msg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QueryMargin, Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg });
		return;
	}

	if (InstrumentProgressManager::CTP().IsMarginDone()) {
		return;
	}
	auto* instService = ServiceSinglton::Services().Instrument;
	auto inst = instService->Get(type::InstrumentCode{ pInstrumentMarginRate->InstrumentID });
	if (inst) {
		InvestorMargin margin;
		margin.Platform = TradePlatform::ZTP;
		margin.Exchange = inst->Exchange;
		margin.BrokerID = pInstrumentMarginRate->BrokerID;
		margin.InvestorID = pInstrumentMarginRate->InvestorID;
		margin.VarityCode = inst->VarityCode;
		margin.InstrumentCode = inst->Code;
		margin.InstID = inst->InstID;
		margin.LongMarginRate = pInstrumentMarginRate->LongMarginRatioByMoney;
		margin.LongMarginAmt = pInstrumentMarginRate->LongMarginRatioByVolume;
		margin.ShortMarginRate = pInstrumentMarginRate->ShortMarginRatioByMoney;
		margin.ShortMarginAmt = pInstrumentMarginRate->ShortMarginRatioByVolume;
		m_publisher.NotifyUI(margin);
		instService->AddVarietyMarginByPresentInstrument(TradePlatform::CTP, margin.VarityCode, margin);
		InstrumentProgressManager::CTP().AddVarietyMarginDone(margin.VarityCode);
	}
	if (InstrumentProgressManager::CTP().IsMarginDone()) {
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QueryMargin);
	}
}

void CTPTraderHandler::SettlementInfoConfirm() {
	CThostFtdcSettlementInfoConfirmField confirmReq = { 0 };
	strcpy_s(confirmReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(confirmReq.InvestorID, m_investor.value());
	int rtnCode = m_pTraderApi->ReqSettlementInfoConfirm(&confirmReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		auto msg = fmt::format("CTPTraderHandler: ReqSettlementInfoConfirm() Failed, RtnCode[{}]", rtnCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}

void CTPTraderHandler::FillingPositionNotification(CThostFtdcInvestorPositionField const& position, InvestorPosition& pos, type::InstID instID) {
	if (instID == INVALID_INSTRUMENT_ID) return;
	auto inst = ServiceSinglton::Services().Instrument->Get(instID);
	pos.Exchange = inst->Exchange;
	pos.InstrumentCode = position.InstrumentID;
	pos.InstID = instID;
	pos.Direction = ConvertPosDirectionFromCTP(position.PosiDirection);
	pos.ProductClass = type::ProductClass::Future;
	pos.UsedMargin = position.UseMargin;

	pos.Position = position.TodayPosition;
	if (pos.Position > 0) {
		pos.PositionCost = position.PositionCost / pos.Position / inst->VolumeMultiple;
	}
	// 不能直接用YDPosition，YDPosition是结算后的值，不反应当天变化
	// CTP的老仓和今仓是分两条数据给的，所以只会有TodayOnly或HistoryOnly
	pos.YdPosition = position.Position - position.TodayPosition;
	if (position.TodayPosition > 0) {
		pos.PositionType = type::PositionType::TodayOnly;
	}
	if (pos.YdPosition > 0) {
		pos.PositionType = type::PositionType::HistoryOnly;
		pos.YdPositionPrice = position.PreSettlementPrice;
	}
}

bool CTPTraderHandler::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {
	return ((NULL != pRspInfo) && (0 != pRspInfo->ErrorID));
}
bool CTPTraderHandler::IsOrdered(CThostFtdcOrderField const& order, bool isEmpty) const {
	//交易所：未成交还在队列中|状态未知 有系统号 成交量为0
	return (THOST_FTDC_OST_NoTradeQueueing==order.OrderStatus || THOST_FTDC_OST_Unknown==order.OrderStatus) && !isEmpty && (0==order.VolumeTraded);
}
bool CTPTraderHandler::IsSendFailed(CThostFtdcOrderField const& order, bool isEmpty) const {
	//CTP：报单拒绝													//系统撤单 eg:委托价格非最小单位的倍数
	return (THOST_FTDC_OSS_InsertRejected==order.OrderSubmitStatus || THOST_FTDC_OST_Canceled==order.OrderStatus) && isEmpty;	
}
bool CTPTraderHandler::IsCanceled(CThostFtdcOrderField const& order, bool isEmpty) const {
	//交易所：撤单 有系统号 委托量>成交量
	return (THOST_FTDC_OST_Canceled==order.OrderStatus) && !isEmpty && (order.VolumeTotalOriginal>order.VolumeTraded);
}
bool CTPTraderHandler::IsCancelFailed(CThostFtdcOrderField const& order, bool isEmpty) const {
	//CTP：撤单拒绝
	return THOST_FTDC_OSS_CancelRejected==order.OrderSubmitStatus;
}
bool CTPTraderHandler::IsPartial(CThostFtdcOrderField const& order, bool isEmpty) const {
	return (THOST_FTDC_OST_PartTradedQueueing==order.OrderStatus) && !isEmpty && (order.VolumeTotalOriginal>order.VolumeTraded);
}
bool CTPTraderHandler::IsAllTraded(CThostFtdcOrderField const& order, bool isEmpty) const  {
	return (THOST_FTDC_OST_AllTraded==order.OrderStatus) && !isEmpty && (order.VolumeTotalOriginal==order.VolumeTraded);
}
bool CTPTraderHandler::IsAccepted(CThostFtdcOrderField const& order, bool isEmpty) const {
	//CTP：报单|撤单提交 交易所：报单状态未知	(CTP应答状态)
	return (THOST_FTDC_OSS_InsertSubmitted==order.OrderSubmitStatus || THOST_FTDC_OSS_CancelSubmitted==order.OrderSubmitStatus) && (THOST_FTDC_OST_Unknown==order.OrderStatus);
}
bool CTPTraderHandler::IsFakOrder(CThostFtdcOrderField const& order, bool isEmpty) const {
	return order.TimeCondition == THOST_FTDC_TC_IOC && order.VolumeCondition == THOST_FTDC_VC_AV;
}
