#include "pch.h"
#include "conn/ctp/CTPTraderHandler.h"
#include "utils/Log.h"

#include "config/Config.h"
#include "notify/ConnPublisher.h"
#include "notify/Notify.h"
#include "conn/ExchangeConn.h"
#include "model/dealing/ExchangeRequest.h"

#pragma warning(disable:4100)

extern char const* DEADLINE;
int const IntervalMS = 1000;
namespace {
	inline static Direction ConvertDirectionFromCTP(TThostFtdcDirectionType const& direction) {
		return direction == THOST_FTDC_D_Buy ? Direction::BUY : Direction::SELL;
	}
	inline static Operation ConvertOperationFromCTP(TThostFtdcOffsetFlagType const& operation) {
		switch (operation) {
		case THOST_FTDC_OF_Open:
			return Operation::OPEN;
		case THOST_FTDC_OF_Close:
		case THOST_FTDC_OF_CloseYesterday:
			return Operation::OFFSET;
		case THOST_FTDC_OF_CloseToday:
			return Operation::OFFSET_TODAY;
		default:
			return Operation::UNKNOWN;
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
}

CTPTraderHandler::CTPTraderHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher)
	:m_config(config),m_instHelper(instHelper),m_publisher(publisher)
	,m_pTraderApi(0),m_frontId(0),m_sessionId(0) {
}

void CTPTraderHandler::OnFrontConnected() {
	Log::Info("CTPTraderHandler: FrontConnected");

	ExchangeNotification notification;
	notification.Platform = ConnPlatform::CTP;
	notification.Type = ExchangeNotification::TRADER_CONNECTED;
	m_publisher.NotifyUI(notification);

	//连接成功后立即进行认证
	CThostFtdcReqAuthenticateField authenticate = {0};
	strcpy_s(authenticate.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(authenticate.UserID, m_config.Credential.UserID.c_str());
	strcpy_s(authenticate.AuthCode, m_config.Credential.TradeAuthCode.c_str());
	strcpy_s(authenticate.AppID, m_config.Credential.AppID.c_str());
	int rtnCode = m_pTraderApi->ReqAuthenticate(&authenticate, RequestIdACC::Generate());
	if (0 != rtnCode) {
		auto msg = fmt::format("CTPTraderHandler: ReqAuthenticate() Failed, RtnCode[{}]", rtnCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void CTPTraderHandler::OnFrontDisconnected(int nReason) {
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::CTP;
	notification.Type = ExchangeNotification::TRADER_DISCONNECTED;
	m_publisher.NotifyUI(notification);
	Log::Error(fmt::format("CTPTraderHandler: FrontDisconnected, Reason[{:x}]", nReason));
}
void CTPTraderHandler::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pRspAuthenticateField)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: Authenticate Failed, ErrorCode[{}] ErrorMsg[{}] BrokerID[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspAuthenticateField->BrokerID);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
	else {
		Log::Info("CTPTraderHandler: Authenticate Success");

		//断线重连后的自动重登录
		if (0 != m_frontId+m_sessionId) {
			CThostFtdcReqUserLoginField loginReq = {0};
			strcpy_s(loginReq.BrokerID, m_config.Credential.BrokerID.c_str());
			strcpy_s(loginReq.UserID, m_config.Credential.UserID.c_str());
			strcpy_s(loginReq.Password, m_config.Credential.Password.c_str());
			int rtnCode = m_pTraderApi->ReqUserLogin(&loginReq, RequestIdACC::Generate());
			if (0 != rtnCode) {
				Log::Error(fmt::format("CTPTraderHandler: ReqUserLogin() Failed, RtnCode[{}]", rtnCode));
			}
		}
	}
}
void CTPTraderHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pRspUserLogin)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: UserLogin Failed, ErrorCode[{}] ErrorMsg[{}] BrokerID[{}] UserID[{}] Password[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspUserLogin->BrokerID, pRspUserLogin->UserID, m_config.Credential.Password.c_str());
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	} 
	else {
		CheckLicense(pRspUserLogin->TradingDay, pRspUserLogin->BrokerID);
		m_frontId = pRspUserLogin->FrontID;
		m_sessionId = pRspUserLogin->SessionID;
		int maxOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		maxOrderRef += atoi(m_config.Credential.BrokerID.c_str());
		LocalIdACC::Init(maxOrderRef);

		auto msg = fmt::format("CTPTraderHandler: UserLogin Success, TradingDay[{}] MaxOrderRef[{}]", pRspUserLogin->TradingDay, maxOrderRef);			
		Log::Info(msg);
		m_publisher.NotifyUI(msg);

		SettlementInfoConfirm();		//登录成功先确认结算单

		ExchangeNotification notification;
		notification.Platform = ConnPlatform::CTP;
		notification.Type = ExchangeNotification::TRADER_LOGINED;
		m_publisher.NotifyUI(notification);
	}
}
void CTPTraderHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pUserLogout)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		Log::Warn(fmt::format("CTPTraderHandler: UserLogout Failed, ErrorCode[{}], ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	} else {
		Log::Info("CTPTraderHandler: UserLogout Success");
	}
}

void CTPTraderHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: OnRspError, ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
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
		OrderNotification notification = { 0 };
		notification.InstID = m_instHelper.GetInstID(pInputOrder->InstrumentID);
		notification.Direction = ConvertDirectionFromCTP(pInputOrder->Direction);
		notification.Operation = ConvertOperationFromCTP(pInputOrder->CombOffsetFlag[0]);
		notification.LocalId = atoi(pInputOrder->OrderRef);
		notification.OrderPrice = pInputOrder->LimitPrice;
		notification.OrderQuantity = pInputOrder->VolumeTotalOriginal;
		notification.Status = OrderNotification::E_SENT_FAILED;
		if (40==pRspInfo->ErrorID/*交易所未处理请求超过许可数*/ || 41==pRspInfo->ErrorID/*交易所每秒发送请求数超过许可数*/ || 91==pRspInfo->ErrorID/*之前日志上显示流控是91*/) {
			notification.Status = OrderNotification::E_FLOW_CONTROL;
		}
		m_publisher.PushResponse(notification);

		char const* direction = (Direction::SELL==notification.Direction) ? "S" : "B";
		auto msg = fmt::format("CTP OrderInsert Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Qty[{}] LocalID[{}]",
			pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrder->InvestorID, pInputOrder->InstrumentID, direction, pInputOrder->CombOffsetFlag, pInputOrder->LimitPrice, notification.OrderQuantity, pInputOrder->OrderRef);
		m_publisher.NotifyUI(msg);
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
			OrderNotification notification = { 0 };
			notification.InstID = m_instHelper.GetInstID(pInputOrderAction->InstrumentID);
			notification.LocalId = atoi(pInputOrderAction->OrderRef);
			strcpy_s(notification.SystemNo, pInputOrderAction->OrderSysID);
			notification.Status = OrderNotification::E_CANCEL_FAILED;
			if (40==pRspInfo->ErrorID/*交易所未处理请求超过许可数*/ || 41==pRspInfo->ErrorID/*交易所每秒发送请求数超过许可数*/ || 91==pRspInfo->ErrorID/*之前日志上显示流控是91*/) {
				notification.Status = OrderNotification::E_FLOW_CONTROL;
			}
			m_publisher.PushResponse(notification);

			auto msg = fmt::format("CTP OrderAction Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}]"
				, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrderAction->InvestorID, pInputOrderAction->InstrumentID);
			m_publisher.NotifyUI(msg);
			Log::Error(msg);
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
	} else {
		Log::Info("CTPTraderHandler: SettlementInfoConfirm Success");
		::Sleep(IntervalMS);
// 		QueryTradingCode();		//不用交易编码了，所以先不查了
		QueryTradingAccount();
	}
}
void CTPTraderHandler::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pTradingCode)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: QryTradingCode Failed, ErrorCode[{}] ErrorMsg[{}] UserID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pTradingCode->InvestorID);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	} else {
		//TODO update clientID
// 		if (0==strcmp("CFFEX", pTradingCode->ExchangeID) && pTradingCode->IsActive)
// 		{			
// 			char const* clientId = m_user.GetClientId(Exchange::CFFEX);
// 			if (0!=strcmp(clientId, pTradingCode->ClientID) && THOST_FTDC_CIDT_Arbitrage==pTradingCode->ClientIDType)
// 			{
// 				m_user.SetClientId(Exchange::CFFEX, pTradingCode->ClientID);	//有套利编码优先使用套利编码
// 			}	
// 			else if (0==strlen(clientId) && THOST_FTDC_CIDT_Speculation==pTradingCode->ClientIDType)
// 			{
// 				m_user.SetClientId(Exchange::CFFEX, pTradingCode->ClientID);
// 			}
// 		}
// 		else if (0 == strcmp("CZCE", pTradingCode->ExchangeID) && pTradingCode->IsActive)
// 		{
// 			m_user.SetClientId(Exchange::CZCE, pTradingCode->ClientID);
// 		}
// 		else if (0 == strcmp("DCE", pTradingCode->ExchangeID) && pTradingCode->IsActive)
// 		{
// 			m_user.SetClientId(Exchange::DCE, pTradingCode->ClientID);
// 		}
// 		else if (0 == strcmp("SHFE", pTradingCode->ExchangeID) && pTradingCode->IsActive)
// 		{
// 			m_user.SetClientId(Exchange::SHFE, pTradingCode->ClientID);
// 		}

		if (bIsLast) {
			Log::Info("CTPTraderHandler: QryTradingCode Success");
			::Sleep(IntervalMS);
//			QueryTradingAccount();
		}
	}
}
void CTPTraderHandler::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pTradingAccount)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: QryTradingAccount Failed, ErrorCode[{}] ErrorMsg[{}] AccountID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pTradingAccount->AccountID);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	} else {
		FundsNotification notification;
		notification.Platform = ConnPlatform::CTP;
		notification.Capital = pTradingAccount->Balance;
		notification.Available = pTradingAccount->Available;
		notification.Margin = pTradingAccount->CurrMargin;
		notification.Fee = pTradingAccount->Commission;
		notification.CloseProfit = pTradingAccount->CloseProfit;
		m_publisher.PushResponse(notification);
// 		m_publisher.NotifyUI(notification);		// 不给界面，界面资金由框架推送UpdateAvailable

		Log::Info(fmt::format("CTPTraderHandler: QryTradingAccount Success, Capital[{:.2f}] Available[{:.2f}] Margin[{:.2f}] Profit[{:.2f}] Fee[{:.2f}]"
			,pTradingAccount->Balance, pTradingAccount->Available, pTradingAccount->CurrMargin, pTradingAccount->CloseProfit, pTradingAccount->Commission));

		m_publisher.PushResponse(CleanPositionNotification());
		::Sleep(IntervalMS);
		QueryPosition();
	}
}
// CTP持仓里Position字段为同方向，总持仓YdPosition为此方向老仓
void CTPTraderHandler::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInvestorPosition) {
		Log::Info("CTPTraderHandler: There is no position info be queried");
		::Sleep(IntervalMS);
		QueryOrder();
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("CTPTraderHandler: QueryPosition Failed, ErrorCode[{}] ErrorMsg[{}] UserID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInvestorPosition->InvestorID);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	} 
	else {
		PositionNotification notification = { 0 };
		notification.InstID = m_instHelper.GetInstID(pInvestorPosition->InstrumentID);
		if (notification.InstID == INVALID_INSTRUMENT_ID) {
			Log::Warn(fmt::format("CTPTraderHandler: QueryPosition filter not support Instrument[{}]", pInvestorPosition->InstrumentID));
			return;
		}

		if (0 != pInvestorPosition->Position) {
			FillingPositionNotification(*pInvestorPosition, notification);
			m_publisher.PushResponse(notification);
			m_publisher.NotifyUI(notification);

			char const* direction = (Direction::BUY == notification.Direction) ? "B" : "S";
			Log::Info(fmt::format("CTPTraderHandler: OnRspQryInvestorPosition, UserID[{}] Instr[{}] Dir[{}] Total[{}] Today[{}] AvgPrice[{:.3f}]",
				pInvestorPosition->InvestorID, pInvestorPosition->InstrumentID, direction, pInvestorPosition->Position, notification.TodayQty, notification.Price.value()));
		}

		if (bIsLast) {
			Log::Info("CTPTraderHandler: QueryPosition Success");
 			::Sleep(IntervalMS);
			QueryOrder();
		}
	}
}
void CTPTraderHandler::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pOrder) {
		Log::Info("CTPTraderHandler: There is no order be queried");
		Sleep(IntervalMS);
		QueryTrade();
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
 		Log::Error(fmt::format("CTPTraderHandler QueryOrder Failed, RequestId[{}] UserID[{}] ErrorCode[{}] ErrorMsg[{}]",
 			nRequestID, pOrder->InvestorID, pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	} else {
		OrderNotification notification = { 0 };
		notification.InstID = m_instHelper.GetInstID(pOrder->InstrumentID);
		if (notification.InstID == INVALID_INSTRUMENT_ID) {
 			Log::Warn(fmt::format("CTPTraderHandler OnRspQryOrder received not exist Inst[{}], RequestId[{}] UserID[{}] SystemId[{}] LocalId[{}]",
 				pOrder->InstrumentID, nRequestID, pOrder->InvestorID, pOrder->OrderSysID, pOrder->OrderRef));
			return;
		}
	
		FillingOrderNotification(*pOrder, notification);	//查委托给出的是委托最终状态，因此可以调用这个函数。 OnRtnOrder里则不行，因为状态会一直变化，和OnRtnTrade互相影响
		notification.IsQueryOrder = true;
		m_publisher.PushResponse(notification);

//  		char const* direction = ConvertDirectionFromCTP(pOrder->Direction) == Direction::SELL ? "S" : "B";
//  		Log::Info(fmt::format("CTPTraderHandler OnRspQryOrder, UserID[{}] Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Info[O:{} D:{} R:{}] SystemId[{}] LocalID[{}] OrderStatus[{:c}] OrderSubmitStatus[{:c}]"
//  			,pOrder->InvestorID, pOrder->InstrumentID, direction, pOrder->CombOffsetFlag, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal,
// 			notification.SystemNo, pOrder->OrderLocalID, pOrder->OrderStatus, pOrder->OrderSubmitStatus));

		if (bIsLast) {
			Log::Info("CTPTraderHandler QueryOrder Success");
			Sleep(IntervalMS);			//延迟1秒钟才能发起下个查询（CTP独有）
// 			QueryTrade();
		}
	}
}
void CTPTraderHandler::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pTrade) {
		Log::Info("CTPTraderHandler: There is no trade be queried");
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::CTP;
		notification.Type = ExchangeNotification::CODE_LOGIN;
		m_publisher.NotifyUI(notification);
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		Log::Error(fmt::format("CTPTraderHandler QueryTrade Failed, RequestId[{}] UserID[{}] ErrorCode[{}] ErrorMsg[{}]",
			nRequestID, pTrade->InvestorID, pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	}
	else {
		OrderNotification notification = { 0 };
		notification.InstID = m_instHelper.GetInstID(pTrade->InstrumentID);
		if (notification.InstID == INVALID_INSTRUMENT_ID) {
			Log::Warn(fmt::format("CTPTraderHandler OnRspQryTrade received not support Inst[{}], UserID[{}] SystemId[{}] LocalId[{}]",
				pTrade->InstrumentID, pTrade->InvestorID, pTrade->OrderSysID, pTrade->OrderRef));
			return;
		}

//		strcpy_s(notification.contractCode, pTrade->InstrumentID);
//		strcpy_s(notification.clientId, pTrade->InvestorID);				//CTP取客户号
		notification.Direction = ConvertDirectionFromCTP(pTrade->Direction);
		notification.Operation = ConvertOperationFromCTP(pTrade->OffsetFlag);

		notification.DealQuantity = pTrade->Volume;
		notification.DealPrice = pTrade->Price;
		notification.LocalId = atoi(pTrade->OrderRef);
		strcpy_s(notification.SystemNo, pTrade->OrderSysID);
// 		strcpy_s(notification.tradeNo, pTrade->TradeID);
// 		strcpy_s(notification.tradeDate, pTrade->TradeDate);
// 		strcpy_s(notification.tradeTime, pTrade->TradeTime);
		notification.Status = OrderNotification::E_TRADE;
		notification.IsQueryTrade = true;
		m_publisher.PushResponse(notification);

// 		char const* dir = (Direction::SELL==notification.Direction) ? "S" : "B";
// 		Log::Info(fmt::format("CTPTraderHandler OnRspQryTrade, UserID[{}] Inst[{}] Dir[{}] Opt[{}] DealPrice[{:.3f}] Qty[{}] SystemId[{}] LocalId[{}]",
// 			pTrade->InvestorID, pTrade->InstrumentID, dir, pTrade->OffsetFlag, pTrade->Price, pTrade->Volume, pTrade->OrderSysID, pTrade->OrderRef));

		if (bIsLast) {
			Log::Info("CTPTraderHandler OnRspQryTrade Success");

			ExchangeNotification notification;
			notification.Platform = ConnPlatform::CTP;
			notification.Type = ExchangeNotification::CODE_LOGIN;
			m_publisher.NotifyUI(notification);
		}
	}
}

void CTPTraderHandler::OnRtnOrder(CThostFtdcOrderField *pOrder) {
	if (NULL == pOrder)
		return;
	
	OrderNotification notification = { 0 };
	notification.InstID = m_instHelper.GetInstID(pOrder->InstrumentID);
	bool isEmpty = (0 == strlen(pOrder->OrderSysID));
	if ((notification.InstID == INVALID_INSTRUMENT_ID) && (false==isEmpty)) {
		Log::Warn(fmt::format("CTPTraderHandler: OnRtnOrder filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			,pOrder->InstrumentID, pOrder->InvestorID, pOrder->OrderSysID, pOrder->OrderRef));
		return;
	}

	notification.Exchange = InstIdExchange(notification.InstID);		
	notification.Direction = ConvertDirectionFromCTP(pOrder->Direction);
	notification.Operation = ConvertOperationFromCTP(pOrder->CombOffsetFlag[0]);
	notification.OrderPrice = pOrder->LimitPrice;
	notification.OrderQuantity = pOrder->VolumeTotalOriginal;
	notification.DealQuantity = pOrder->VolumeTraded;	// 这里的成交量并非增量而是总量！
	notification.LocalId = atoi(pOrder->OrderRef);
	notification.DealPrice = pOrder->LimitPrice;
	strcpy_s(notification.SystemNo, pOrder->OrderSysID);

	if (IsAccepted(*pOrder, isEmpty)) {	
		notification.Status = OrderNotification::E_ACCEPT;
		m_publisher.PushResponse(notification);
		return;
	}

	char const* direction = (Direction::SELL == notification.Direction) ? "S" : "B";
	if (IsOrdered(*pOrder, isEmpty)) {	//普通委托才有，FAK不会有这个状态
		notification.Status = OrderNotification::E_ORDERED;
		m_publisher.PushResponse(notification);
// 		Log::Info(fmt::format("CTPTraderHandler OnRtnOrder: UserID[{}] Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Info[O:{} D:{} R:{}] SystemID[{}] LocalID[{}] OrderStatus[{:c}] OrderSubmitStatus[{:c}] NotifyStatus[{}]",
// 			pOrder->InvestorID, pOrder->InstrumentID, direction, pOrder->CombOffsetFlag, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal,
// 			pOrder->OrderSysID, pOrder->OrderRef, pOrder->OrderStatus, pOrder->OrderSubmitStatus, "E_ORDERED"));
		return;
	}

	if (IsPartial(*pOrder, isEmpty) || IsAllTraded(*pOrder, isEmpty)) {
		notification.Status = OrderNotification::E_QUANTITY_TRADE;
		m_publisher.PushResponse(notification);
// 		Log::Info(fmt::format("CTPTraderHandler OnRtnOrder: UserID[{}] Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Info[O:{} D:{} R:{}] SystemID[{}] LocalID[{}] OrderStatus[{:c}] OrderSubmitStatus[{:c}] NotifyStatus[{}] TimeCondition[{}]",
// 			pOrder->InvestorID, pOrder->InstrumentID, direction, pOrder->CombOffsetFlag, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal,
// 			pOrder->OrderSysID, pOrder->OrderRef, pOrder->OrderStatus, pOrder->OrderSubmitStatus, "E_QUANTITY_TRADE", pOrder->TimeCondition));
		return;
	} else if (THOST_FTDC_OST_Canceled == pOrder->OrderStatus) {
		std::string notifyStatus = "E_SENT_FAILED";
		if (isEmpty) {
			//系统号为空，表明是CTP系统撤单 eg:委托价格非最小单位的整数倍;价格超过涨跌停板;
			//委托请求的交易所每秒发送请求数超过许可数(不报到交易所)的情况，撤单在OnErrRtnOrderAction
			notification.Status = OrderNotification::E_SENT_FAILED;
			auto msg = fmt::format("CTPTraderHandler: Cancelled by System! StatusMsg[{}] LocalID[{}] OrderStatus[{:c}] OrderSubmitStatus[{:c}]", pOrder->StatusMsg, pOrder->OrderRef, pOrder->OrderStatus,pOrder->OrderSubmitStatus);
			m_publisher.NotifyUI(msg);
			Log::Error(msg);
		} else if (IsCanceled(*pOrder, isEmpty)) {
			notification.Status = OrderNotification::E_CANCELED;
			notifyStatus = "E_CANCELED";

			//FAK部成部撤
			if (IsFakOrder(*pOrder,isEmpty) && notification.DealQuantity > 0) {
				notification.DealPrice = notification.OrderPrice;
				notification.Status = OrderNotification::E_FAK_PARTIAL_TRADE;
				notifyStatus = "E_FAK_PARTIAL_TRADE";
			}
// 			Log::Info(fmt::format("CTPTraderHandler OnRtnOrder: UserID[{}] Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Info[O:{} D:{} R:{}] SystemID[{}] LocalID[{}] OrderStatus[{:c}] OrderSubmitStatus[{:c}] NotifyStatus[{}] TimeCondition[{}]",
// 				pOrder->InvestorID, pOrder->InstrumentID, direction, pOrder->CombOffsetFlag, pOrder->LimitPrice, pOrder->VolumeTotalOriginal, pOrder->VolumeTraded, pOrder->VolumeTotal,
// 				pOrder->OrderSysID, pOrder->OrderRef, pOrder->OrderStatus, pOrder->OrderSubmitStatus, notifyStatus, pOrder->TimeCondition));
		}
		m_publisher.PushResponse(notification);
		return;
	}
	else {
 		auto msg = fmt::format("CTPTraderHandler: OnRtnOrder received unknown status combination, Inst[{}] LocalID[{}] OrderStatus[{:c}] OrderSubmitStatus[{:c}] TimeCondition[{}]"
			,pOrder->InstrumentID, pOrder->OrderRef, pOrder->OrderStatus, pOrder->OrderSubmitStatus, pOrder->TimeCondition);
 		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		return;
	}
}
void CTPTraderHandler::OnRtnTrade(CThostFtdcTradeField *pTrade) {
	if (NULL == pTrade)
		return;

	OrderNotification notification = { 0 };
	notification.InstID = m_instHelper.GetInstID(pTrade->InstrumentID);
	if (notification.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("CTPTraderHandler: OnRtnTrade filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pTrade->InstrumentID ,pTrade->InvestorID, pTrade->OrderSysID, pTrade->OrderRef));
		return;
	}
	
	notification.Exchange = InstIdExchange(notification.InstID);
	notification.Direction = ConvertDirectionFromCTP(pTrade->Direction);
	notification.Operation = ConvertOperationFromCTP(pTrade->OffsetFlag);
	notification.DealQuantity = pTrade->Volume;		// OnRtnTrade里的成交量是增量
	notification.DealPrice = pTrade->Price;
	notification.LocalId = atoi(pTrade->OrderRef);
	strcpy_s(notification.SystemNo, pTrade->OrderSysID);
	notification.Status = OrderNotification::E_TRADE;
	m_publisher.PushResponse(notification);
	char const* direction = (Direction::SELL==notification.Direction) ? "S" : "B";
	Log::Info(fmt::format("CTPTraderHandler OnRtnTrade: UserID[{}] Inst[{}] Dir[{}] Opt[{:c}] Price[{:.3f}] Qty[{}] SystemID[{}] LocalID[{}]"
		,pTrade->InvestorID, pTrade->InstrumentID, direction, pTrade->OffsetFlag, pTrade->Price, pTrade->Volume, pTrade->OrderSysID, pTrade->OrderRef));
}

//交易所错误委托回报
void CTPTraderHandler::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {
	return;		//返回内容和OnRspOrderInsert中返回的错误相同，直接return
	if (NULL == pInputOrder)
		return;
	
	if (IsErrorRspInfo(pRspInfo)) {
		OrderNotification notification = { 0 };
		notification.InstID = m_instHelper.GetInstID(pInputOrder->InstrumentID);
		if (INVALID_INSTRUMENT_ID == notification.InstID) {
			Log::Warn(fmt::format("CTPTraderHandler: OnErrRtnOrderInsert received not support Instrument[{}], userID[{}] localID[{}] "
				,pInputOrder->InstrumentID, pInputOrder->InvestorID, pInputOrder->OrderRef));
			return;
		}

		if (40==pRspInfo->ErrorID/*交易所未处理请求超过许可数*/ || 41==pRspInfo->ErrorID/*交易所每秒发送请求数超过许可数*/ || 91==pRspInfo->ErrorID/*交易所报错91*/) {
			notification.Status = OrderNotification::E_FLOW_CONTROL;
		} else {
			notification.Status = OrderNotification::E_SENT_FAILED;
		}
		notification.LocalId = atoi(pInputOrder->OrderRef);
		m_publisher.PushResponse(notification);

		//返回内容和OnRspOrderInsert中返回的错误相同，注释之
// 		char const* direction = ConvertDirectionFromCTP(pInputOrder->Direction) == Direction::SELL ? "S" : "B";
// 		auto msg = fmt::format("Exchange OrderInsert Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] Dir[{}] Opt[{}] Info[{:.3f},{}] LocalID[{}]"
// 			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrder->InvestorID, pInputOrder->InstrumentID, direction, pInputOrder->CombOffsetFlag, pInputOrder->LimitPrice, pInputOrder->VolumeTotalOriginal, pInputOrder->OrderRef);
// 		m_publisher.NotifyUI(msg);
// 		Log::Error(msg);
	}
}
//交易所撤单错误回报
void CTPTraderHandler::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {
	return;		//返回内容和OnRspOrderAction中返回的错误相同，直接return

	if (NULL == pOrderAction)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		OrderNotification notification = { 0 };
		notification.InstID = m_instHelper.GetInstID(pOrderAction->InstrumentID);
		if (INVALID_INSTRUMENT_ID == notification.InstID) {
			Log::Warn(fmt::format("CTPTraderHandler OnErrRtnOrderAction received not exist Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}] ActionID[{}]",
				pOrderAction->InstrumentID, pOrderAction->InvestorID, pOrderAction->OrderSysID, pOrderAction->OrderLocalID, pOrderAction->ActionLocalID));
			return;
		}

		//这两种情况视为逻辑正确，其他视为撤单失败 
		if (pRspInfo->ErrorID != 24 /*重复撤单*/ && pRspInfo->ErrorID != 26/*全部成交或者已撤*/) {
			if (40==pRspInfo->ErrorID/*交易所未处理请求超过许可数*/ || 41==pRspInfo->ErrorID/*交易所每秒发送请求数超过许可数*/ || 91==pRspInfo->ErrorID/*之前日志上显示流控是91*/) {
				notification.Status = OrderNotification::E_FLOW_CONTROL;
			} else {
				notification.Status = OrderNotification::E_CANCEL_FAILED;
			}
			notification.LocalId = atoi(pOrderAction->OrderRef);
			strcpy_s(notification.SystemNo , pOrderAction->OrderSysID);
			m_publisher.PushResponse(notification);

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
	notification.Platform = ConnPlatform::CTP;
	notification.Type = ExchangeNotification::EXCHANGE_STATUS;
	notification.Status = ConvertExchangeStatusFromCTP(pInstrumentStatus->InstrumentStatus);

// 	if (0 == strcmp("CZCE", pInstrumentStatus->ExchangeID)) {
// 		notification.Exchange = Exchange::CZCE;
// 		if (m_exchangeStatusMap[Exchange::CZCE] != notification.Status) {
// 			m_exchangeStatusMap[Exchange::CZCE] = notification.Status;
// 			notifyTag = true;
// 		}
// 	}
// 	else if (0 == strcmp("DCE", pInstrumentStatus->ExchangeID)) {
// 		notification.Exchange = Exchange::DCE;
// 		if (m_exchangeStatusMap[Exchange::DCE] != notification.Status) {
// 			m_exchangeStatusMap[Exchange::DCE] = notification.Status;
// 			notifyTag = true;
// 		}
// 	}
	if (0 == strcmp("SHFE", pInstrumentStatus->ExchangeID)) {
		notification.Exchange = Exchange::SHFE;
		if (m_exchangeStatusMap[Exchange::SHFE] != notification.Status) {
			m_exchangeStatusMap[Exchange::SHFE] = notification.Status;
			notifyTag = true;
		}
	}
	if (0 == strcmp("CFFEX", pInstrumentStatus->ExchangeID)) {
		notification.Exchange = Exchange::CFFEX;
		if (m_exchangeStatusMap[Exchange::CFFEX] != notification.Status) {
			m_exchangeStatusMap[Exchange::CFFEX] = notification.Status;
			notifyTag = true;
		}
	}

	if (notifyTag) {
		m_publisher.NotifyUI(notification);
		m_publisher.PushResponse(notification);
		notifyTag = false;
		Log::Info(fmt::format("Exchange[{}] Status[{}]", Desc(notification.Exchange), Desc(notification.Status)));
	}
}

void CTPTraderHandler::SettlementInfoConfirm() {
	CThostFtdcSettlementInfoConfirmField confirmReq = { 0 };
	strcpy_s(confirmReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(confirmReq.InvestorID, m_config.Credential.UserID.c_str());
	int rtnCode = m_pTraderApi->ReqSettlementInfoConfirm(&confirmReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		auto msg = fmt::format("CTPTraderHandler: ReqSettlementInfoConfirm() Failed, RtnCode[{}]", rtnCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void CTPTraderHandler::QueryTradingCode() {
	CThostFtdcQryTradingCodeField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_config.Credential.UserID.c_str());
	int rtnCode = m_pTraderApi->ReqQryTradingCode(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
 		auto msg = fmt::format("CTPTraderHandler: QueryTradingCode() Failed, RtnCode[{}]", rtnCode);
 		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void CTPTraderHandler::QueryTradingAccount() {
	CThostFtdcQryTradingAccountField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_config.Credential.UserID.c_str());
	int rtnCode = m_pTraderApi->ReqQryTradingAccount(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		auto msg = fmt::format("CTPTraderHandler: QryTradingAccount() Failed, RtnCode[{}]", rtnCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void CTPTraderHandler::QueryPosition() {
	CThostFtdcQryInvestorPositionField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_config.Credential.UserID.c_str());
	int rtnCode = m_pTraderApi->ReqQryInvestorPosition(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		Log::Error(fmt::format("CTPTraderHandler: QueryPosition() Failed, RtnCode[{}]", rtnCode));
	}
}
void CTPTraderHandler::QueryTrade() {
	CThostFtdcQryTradeField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_config.Credential.UserID.c_str());
	int rtnCode = m_pTraderApi->ReqQryTrade(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		Log::Error(fmt::format("CTPTraderHandler: QueryTrade() Failed, RtnCode[{}]", rtnCode));
	}
}
void CTPTraderHandler::QueryOrder() {
	CThostFtdcQryOrderField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_config.Credential.UserID.c_str());
	int rtnCode = m_pTraderApi->ReqQryOrder(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		Log::Error(fmt::format("CTPTraderHandler: QueryOrder() Failed, RtnCode[{}]", rtnCode));
	}
}

void CTPTraderHandler::CheckLicense(TThostFtdcDateType tradingDay,  TThostFtdcBrokerIDType brokerId) {
	if (0 > strcmp(DEADLINE, tradingDay)) {
		MessageBox(NULL, _T("^_^!"), _T("@_@"), MB_ICONSTOP);
		exit(-1);
	}
}
void CTPTraderHandler::FillingPositionNotification(CThostFtdcInvestorPositionField const& position, PositionNotification& notification) {	
	notification.Quantity = position.Position/*现有总持仓*/;
	notification.TodayQty = 0;
	if (IsSHFE(notification.InstID) || IsINE(notification.InstID)) {
		if (0 != position.TodayPosition) {
			notification.TodayQty = position.TodayPosition/*需用平今的今仓*/;
		}
	}
	double multiplier = m_instHelper.GetMultiplier(notification.InstID);
	notification.Price = position.OpenCost / (notification.Quantity * multiplier);	// 开仓均价自己计算
	
	if (THOST_FTDC_PD_Long == position.PosiDirection) {
		notification.Direction = Direction::BUY;
		notification.Frozen = position.LongFrozen;		// 多头冻结
	} else if (THOST_FTDC_PD_Short == position.PosiDirection) {
		notification.Direction = Direction::SELL;
		notification.Frozen = position.ShortFrozen;		// 空头冻结
	}

	if (THOST_FTDC_HF_Speculation == position.HedgeFlag) {
		notification.HedgeType = HedgeType::SPECULATE;
	}
	else if (THOST_FTDC_HF_Arbitrage == position.HedgeFlag) {
		notification.HedgeType = HedgeType::ARBITRAGE;
	}
}
//仅供报单查询调用
void CTPTraderHandler::FillingOrderNotification(CThostFtdcOrderField const& order, OrderNotification& notification) {
	notification.Direction = ConvertDirectionFromCTP(order.Direction);
	notification.Operation = ConvertOperationFromCTP(order.CombOffsetFlag[0]);
	notification.OrderPrice = order.LimitPrice;
	notification.OrderQuantity = order.VolumeTotalOriginal;
	notification.DealQuantity = order.VolumeTraded;
	notification.LocalId = atoi(order.OrderRef);				//ctp接口里既有orderLocalId，又有orderRef，根据文档取orderRef
	strcpy_s(notification.SystemNo, order.OrderSysID);
	SetOrderNotificationStatus(order, notification);
}
void CTPTraderHandler::SetOrderNotificationStatus(CThostFtdcOrderField const& order, OrderNotification& notification) {
	bool isEmpty = (0==strlen(order.OrderSysID));
	if (IsOrdered(order, isEmpty)) {
		notification.Status = OrderNotification::E_ORDERED;
		return;
	} else if (IsSendFailed(order, isEmpty)) {
		notification.Status = OrderNotification::E_SENT_FAILED;
		return;
	} else if (IsCanceled(order, isEmpty)) {
		notification.Status = OrderNotification::E_CANCELED;
		return;
	} else if (IsCancelFailed(order, isEmpty)) {
		notification.Status = OrderNotification::E_CANCEL_FAILED;
		return;
	} else if (IsPartial(order, isEmpty)) {
		notification.Status = OrderNotification::E_PARTIAL;
		notification.DealPrice = order.LimitPrice;
		notification.DealQuantity = order.VolumeTraded;
		return;
	} else if (IsAllTraded(order, isEmpty)) {
		notification.Status = OrderNotification::E_TRADE;
		notification.DealPrice = order.LimitPrice;
		notification.DealQuantity = order.VolumeTraded;
		return;
	} else if (IsAccepted(order, isEmpty)) {
		notification.Status = OrderNotification::E_ACCEPT;
		return;
	} else {
// 		FT formatter("CTPTraderHandler received unknown order status combination InvestorId[%s] LocalId[%s] SystemId[%s] OrderStatus[%c] OrderSubmitStatus[%c]",
// 			order.InvestorID, order.OrderRef, order.OrderSysID, order.OrderStatus, order.OrderSubmitStatus);
// 		Log::Error(formatter);

// 		PostNotifyInfo(formatter, m_queueContext.GetInterfaceQueue());
		return;
	}
}

bool CTPTraderHandler::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {
	return ((NULL != pRspInfo) && (0 != pRspInfo->ErrorID));
}
bool CTPTraderHandler::IsOrdered(CThostFtdcOrderField const& order, bool isEmpty) const
{
	//交易所：未成交还在队列中|状态未知 有系统号 成交量为0
	return (THOST_FTDC_OST_NoTradeQueueing==order.OrderStatus || THOST_FTDC_OST_Unknown==order.OrderStatus) && !isEmpty && (0==order.VolumeTraded);
}
bool CTPTraderHandler::IsSendFailed(CThostFtdcOrderField const& order, bool isEmpty) const
{
	//CTP：报单拒绝													//系统撤单 eg:委托价格非最小单位的倍数
	return (THOST_FTDC_OSS_InsertRejected==order.OrderSubmitStatus || THOST_FTDC_OST_Canceled==order.OrderStatus) && isEmpty;	
}
bool CTPTraderHandler::IsCanceled(CThostFtdcOrderField const& order, bool isEmpty) const {
	//交易所：撤单 有系统号 委托量>成交量
	return (THOST_FTDC_OST_Canceled == order.OrderStatus) && !isEmpty && (order.VolumeTotalOriginal > order.VolumeTraded);
}
bool CTPTraderHandler::IsCancelFailed(CThostFtdcOrderField const& order, bool isEmpty) const
{
	//CTP：撤单拒绝
	return THOST_FTDC_OSS_CancelRejected==order.OrderSubmitStatus;
}
bool CTPTraderHandler::IsPartial(CThostFtdcOrderField const& order, bool isEmpty) const {
	return (THOST_FTDC_OST_PartTradedQueueing==order.OrderStatus) && !isEmpty && (order.VolumeTotalOriginal>order.VolumeTraded);
}
bool CTPTraderHandler::IsAllTraded(CThostFtdcOrderField const& order, bool isEmpty) const 
{
	return (THOST_FTDC_OST_AllTraded==order.OrderStatus) && !isEmpty && (order.VolumeTotalOriginal==order.VolumeTraded);
}
bool CTPTraderHandler::IsAccepted(CThostFtdcOrderField const& order, bool isEmpty) const {
	//CTP：报单|撤单提交 交易所：报单状态未知	(CTP应答状态)
	return (THOST_FTDC_OSS_InsertSubmitted==order.OrderSubmitStatus || THOST_FTDC_OSS_CancelSubmitted==order.OrderSubmitStatus) && (THOST_FTDC_OST_Unknown==order.OrderStatus);
}
bool CTPTraderHandler::IsFakOrder(CThostFtdcOrderField const& order, bool isEmpty) const {
	return order.TimeCondition == THOST_FTDC_TC_IOC && order.VolumeCondition == THOST_FTDC_VC_AV;
}