#include "conn/ztp/ZTPTraderHandler.h"
#include "utils/Log.h"
#include "domain/Error.h"
#include "config/Config.h"
#include "notify/ConnPublisher.h"
#include "notify/Notify.h"
#include "conn/ExchangeConn.h"
#include "model/dealing/ExchangeRequest.h"
#include "service/Service.h"

#pragma warning(disable:4100)

using namespace ztp;

int const IntervalMS = 1000;
namespace {
	inline static type::Direction ConvertDirectionFromZTP(TZtpApiDirectionType const& direction) {
		return direction == ZTP_API_D_Buy ? type::Direction::Buy : type::Direction::Sell;
	}
	inline static type::OffsetFlag ConvertOperationFromZTP(TZtpApiOffsetFlagType const& operation) {
		switch (operation) {
		case ZTP_API_OF_Open:
			return type::OffsetFlag::Open;
		case ZTP_API_OF_Close:
			return type::OffsetFlag::Close;
		case ZTP_API_OF_CloseToday:
			return type::OffsetFlag::CloseToday;
		default:
			return type::OffsetFlag::Unknown;
		}
	}

	inline static type::OrderPriceType ConvertOrderTypeFromZTP(TZtpApiOrderPriceType const& operation) {
		switch (operation) {
		case ZTP_API_LimitPrice:
			return type::OrderPriceType::LimitPrice;
		case ZTP_API_LimitPrice_FOK:
			return type::OrderPriceType::FOK;
		case ZTP_API_LimitPrice_FAK:
			return type::OrderPriceType::FAK;
		default:
			return type::OrderPriceType::LimitPrice;
		}
	}
	inline static ExchangeStatus ConvertExchangeStatusFromZTP(TZtpApiInstrumentStatusType status) {
		switch (status) {
		case ZTP_API_IS_BeforeTrading:
			return ExchangeStatus::BEFORE_TRADING;
		case ZTP_API_IS_Pause:
			return ExchangeStatus::NO_TRADING;
		case ZTP_API_IS_Continous:
			return ExchangeStatus::CONTINOUS;
		case ZTP_APi_IS_AuctionOrdering:
			return ExchangeStatus::AUCTION_ORDERING;
// 		case ZTP_API_IS_AuctionBalance:
// 			return ExchangeStatus::AUCTION_BALANCE;
		case ZTP_API_IS_AuctionMatch:
			return ExchangeStatus::AUCTION_MATCH;
		case ZTP_API_IS_Closed:
			return ExchangeStatus::CLOSED;
		default:
			return ExchangeStatus::UNKNOW;
		}
	}

	inline static type::OrderStatus ConvertOrderStatus(TZtpApiOrderStatusType status) {
		switch (status) {
		case ZTP_API_IS_AllTraded:
			return type::OrderStatus::AllTraded;
		case ZTP_API_IS_PartTraded:
			return type::OrderStatus::PartTradedQueueing;
		case ZTP_API_IS_NoTrade:
			return type::OrderStatus::NoTradeQueueing;
		case ZTP_API_IS_Canceled:
			return type::OrderStatus::Canceled;
		case ZTP_API_IS_Queueing:
			return type::OrderStatus::InsertQueueing;
		case ZTP_API_IS_Failed:
			return type::OrderStatus::InsertFail;
		default:
			return type::OrderStatus::Unknown;
			break;
		}

	}

	inline static type::ProductClass ConvertProductClassFromZTP(TZtpApiProductClassType type) {
		switch (type)
		{
		case ZTP_API_PCT_Unknown: {
			return type::ProductClass::Unknown;
		}
		case ZTP_API_PCT_Defer: {
			return type::ProductClass::Defer;
		}
		case ZTP_API_PCT_Spot: {
			return type::ProductClass::Spot;
		} 
		case ZTP_API_PCT_Delivery: {
			return type::ProductClass::Delivery;
		}
		case ZTP_API_PCT_Neutral: {
			return type::ProductClass::Neutral;
		}
		default:
			return type::ProductClass::Unknown;
			break;
		}
	}

	inline Error from(CZtpApiRspInfoField* p) {
		if (p)
			return Error{ p->ErrorID,p->ErrorMsg };
		else
			return NoError;
	}
}

ZTPTraderHandler::ZTPTraderHandler(TradeSystemConfig const& config, ConnPublisher& publisher)
	:m_config(config),m_publisher(publisher),m_pTraderApi(nullptr) {
}



void ZTPTraderHandler::OnFrontConnected()
{
	Log::Info("ZTPTraderHandler: FrontConnected");

	//连接成功后立即进行认证
// 	CZtpApiReqAuthenticateField authenticate = {0};
// 	strcpy_s(authenticate.BrokerID, m_config.Credential.BrokerID.c_str());
// 	strcpy_s(authenticate.UserID, m_config.Credential.UserID.c_str());
// 	strcpy_s(authenticate.UserProductInfo, m_config.Credential.UserProductInfo.c_str());
// 	strcpy_s(authenticate.AuthCode, m_config.Credential.TradeAuthCode.c_str());
// 	strcpy_s(authenticate.AppID, m_config.Credential.AppID.c_str());
// 	int rtnCode = m_pTraderApi->ReqAuthenticate(&authenticate, RequestIdACC::Generate());
// 	if (0 != rtnCode) {
// 		auto msg = fmt::format("ZTPTraderHandler: ReqAuthenticate() Failed, RtnCode[{}]", rtnCode);
// 		m_publisher.NotifyUI(msg);
// 		Log::Error(msg);
// 	}
	//测试环境，无需认证，测试商品期货用
	TradePlatformNotification en;
	en.Platform = TradePlatform::ZTP;
	en.Type = TradePlatformNotification::CONNECTED;
	en.SubSystem = SubSystem::Trade;
	m_publisher.NotifyUI(en);

	m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::TradeConnect);
	Log::Info("ZTPTraderHandler: Authenticate Success");
}
void ZTPTraderHandler::OnFrontDisconnected(int nReason) {
	TradePlatformNotification en;
	en.Platform = TradePlatform::ZTP;
	en.Type = TradePlatformNotification::DISCONNECTED;
	en.SubSystem = SubSystem::Trade;
	m_publisher.NotifyUI(en);
	Log::Error(fmt::format("ZTPTraderHandler: FrontDisconnected, Reason[{:x}]", nReason));
}

void ZTPTraderHandler::OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		TradePlatformNotification en;
		en.Platform = TradePlatform::ZTP;
		en.Type = TradePlatformNotification::LOGIN_FAILED;
		en.SubSystem = SubSystem::Quote;
		en.Error = from(pRspInfo);
		m_publisher.NotifyUI(en);

		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::TradeLogin, en.Error);
		auto msg = fmt::format("ZTPTraderHandler: UserLogin Failed, ErrorCode[{}] ErrorMsg[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	} else {
		TradePlatformNotification en;
		en.Platform = TradePlatform::ZTP;
		en.Type = TradePlatformNotification::LOGIN;
		en.SubSystem = SubSystem::Trade;
		en.TradingDay = pRspUserLogin->TradeDate;
		m_publisher.NotifyUI(en);

		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::TradeLogin);
		auto msg = fmt::format("ZTPTraderHandler: UserLogin Success");			
		Log::Info(msg);
	}
}
void ZTPTraderHandler::OnRspUserLogout(CZtpApiRspLogoutField* pUserLogout, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pUserLogout)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		Log::Warn(fmt::format("ZTPTraderHandler: UserLogout Failed, ErrorCode[{}], ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	} else {
		Log::Info("ZTPTraderHandler: UserLogout Success");
		TradePlatformNotification en;
		en.Platform = TradePlatform::ZTP;
		en.Type = TradePlatformNotification::LOGOUT;
		en.SubSystem = SubSystem::Trade;
		m_publisher.NotifyUI(en);
	}
}

void ZTPTraderHandler::OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pRspUserPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		TradePlatformNotification tpn;
		tpn.Error = from(pRspInfo);
		tpn.Platform = TradePlatform::ZTP;
		tpn.Type = TradePlatformNotification::PASSWORD_CHANGE;
		tpn.SubSystem = SubSystem::Trade;
		m_publisher.NotifyUI(tpn);
		auto msg = fmt::format("ZTPTraderHandler: OnRspUserPasswordUpdate, ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
	} else {
		TradePlatformNotification tpn;
		tpn.Error = NoError;
		tpn.Platform = TradePlatform::ZTP;
		tpn.Type = TradePlatformNotification::PASSWORD_CHANGE;
		tpn.SubSystem = SubSystem::Trade;
		m_publisher.NotifyUI(tpn);
		auto msg = fmt::format("ZTPTraderHandler: OnRspUserPasswordUpdate Success");
		Log::Info(msg);

	}

}

void ZTPTraderHandler::OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		ErrorNotification noti;
		noti.Error = from(pRspInfo);
		noti.Platform = TradePlatform::ZTP;
		m_publisher.NotifyUI(noti);

		auto msg = fmt::format("ZTPTraderHandler: OnRspError, ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
	}
}
void ZTPTraderHandler::OnRspOrderInsert(CZtpApiInputOrderField *pInputOrder, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInputOrder)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		InvestorOrder order;
		order.InstrumentCode = type::InstrumentCode(pInputOrder->InstrumentID);

		order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
		order.Platform = TradePlatform::ZTP;
		if ((order.InstID == INVALID_INSTRUMENT_ID)) {
			Log::Warn(fmt::format("ZTPTraderHandler: OnRspOrderAction filter not support Instrument[{}], UserID[{}] LocalID[{}]"
				, pInputOrder->InstrumentID, pInputOrder->InvestorID, pInputOrder->OrderLocalID));
			return;
		}

		order.Direction = ConvertDirectionFromZTP(pInputOrder->Direction);
		order.OffsetFlag = ConvertOperationFromZTP(pInputOrder->OffsetFlag);
		order.OrderLocalID = atoi(pInputOrder->OrderLocalID);
		order.LimitPrice = pInputOrder->OrderPrice;
		order.Volume = pInputOrder->OrderQuantity;
		order.OrderStatus = type::OrderStatus::InsertFail;
		order.InsertError = from(pRspInfo);
		if (40==pRspInfo->ErrorID/*交易所未处理请求超过许可数*/ || 41==pRspInfo->ErrorID/*交易所每秒发送请求数超过许可数*/ || 91==pRspInfo->ErrorID/*之前日志上显示流控是91*/) {
			order.InsertError = FLowControlError;
		}

		OrderErrorNotification errorNotify;
		errorNotify.Type = OrderErrorNotification::Insert;
		errorNotify.Platform = TradePlatform::ZTP;
		errorNotify.OrderLocalID = atoi(pInputOrder->OrderLocalID);
		errorNotify.Error = order.InsertError;
		m_publisher.NotifyUI(errorNotify);
		m_publisher.PushResponse(errorNotify);

		char const* direction = IsBuy(order.Direction) ? "B" : "S";
		auto msg = fmt::format("ZTP OrderInsert Failed:  ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] Dir[{}] Opt[{}] Info[{:.3f},{}] LocalID[{}]"
			,pRspInfo->ErrorID,pRspInfo->ErrorMsg,pInputOrder->InvestorID,pInputOrder->InstrumentID,direction,pInputOrder->OffsetFlag,pInputOrder->OrderPrice,order.Volume,pInputOrder->OrderLocalID);
		Log::Error(msg);
	}
}
// ZTP撤单参数校验错误应答，详见《综合交易平台交易API特别说明》	注意：由于已经成交导致的撤单失败不会再这里给出应答
void ZTPTraderHandler::OnRspOrderAction(CZtpApiInputOrderActionField *pInputOrderAction, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInputOrderAction)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		if (pRspInfo->ErrorID != 24 /*不允许重复撤单*/ && pRspInfo->ErrorID != 26/*报单已撤销*/) {
			InvestorOrder order;
			order.InstrumentCode = type::InstrumentCode(pInputOrderAction->InstrumentID);
			order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
			if ((order.InstID == INVALID_INSTRUMENT_ID)) {
				Log::Warn(fmt::format("ZTPTraderHandler: OnRspOrderAction filter not support Instrument[{}], UserID[{}] LocalID[{}]"
					, pInputOrderAction->InstrumentID, pInputOrderAction->InvestorID, pInputOrderAction->OrderLocalID));
				return;
			}
			OrderErrorNotification errorNotify;
			errorNotify.Platform = TradePlatform::ZTP;
			errorNotify.Type = OrderErrorNotification::Cancel;
			errorNotify.OrderLocalID = atoi(pInputOrderAction->OrderLocalID);
			errorNotify.CancelLocalID = atoi(pInputOrderAction->OrderActionLocalID);
			errorNotify.OrderSysID = pInputOrderAction->OrderSysID;
			errorNotify.Error = from(pRspInfo);
			m_publisher.NotifyUI(errorNotify);
			m_publisher.PushResponse(errorNotify);

			auto msg = fmt::format("ZTP OrderAction Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}]"
				, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrderAction->InvestorID, pInputOrderAction->InstrumentID);
			Log::Info(msg);
		}
	}
}

void ZTPTraderHandler::OnRspQryInvestorAccount(CZtpApiRspAccountField*pTradingAccount, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("ZTPTraderHandler: QryTradingAccount Failed, ErrorCode[{}] ErrorMsg[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryAccount, from(pRspInfo));

	} else  if (pTradingAccount) {
		InvestorAccount account;
		account.Platform = TradePlatform::ZTP;
		account.InvestorID = pTradingAccount->InvestorID;
		account.Balance = pTradingAccount->Balance;
		account.Available = pTradingAccount->Availlable;
		account.Margin = pTradingAccount->Margin;
		account.Fee = pTradingAccount->Fee;
		account.CloseProfit = pTradingAccount->CloseProfit;
		account.FrozenFee = pTradingAccount->FrozenFee;
		account.FrozenMargin = pTradingAccount->FrozenMargin;
		account.PreBalance = pTradingAccount->PreBalance;
		m_publisher.NotifyUI(account);

		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryAccount);

		Log::Info(fmt::format("ZTPTraderHandler: QryTradingAccount Success, Capital[{:.2f}] Available[{:.2f}] Margin[{:.2f}] Profit[{:.2f}] Fee[{:.2f}]"
			,pTradingAccount->Balance, pTradingAccount->Availlable, pTradingAccount->Margin, pTradingAccount->CloseProfit, pTradingAccount->Fee));

//		m_publisher.PushResponse(CleanPositionNotification());
//		::Sleep(IntervalMS);
//		QueryPosition();
	}
}
// ZTP持仓里Position字段为同方向，总持仓YdPosition为此方向老仓
void ZTPTraderHandler::OnRspQryInvestorPosition(CZtpApiRspPositionField*pInvestorPosition, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pInvestorPosition) {
		Log::Info("ZTPTraderHandler: There is no position info be queried");
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryPosition);
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("ZTPTraderHandler: QueryPosition Failed, ErrorCode[{}] ErrorMsg[{}] UserID[{}]"
			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInvestorPosition->InvestorID);
		Log::Error(msg);

		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryPosition, SystemError);
		m_publisher.NotifyUI(msg);
	} else {
		InvestorPosition notification;
		notification.Platform = TradePlatform::ZTP;
		notification.InstID = ServiceSinglton::Services().Instrument->toID(type::InstrumentCode(pInvestorPosition->InstrumentID));
 		if (notification.InstID == INVALID_INSTRUMENT_ID) {
 			Log::Warn(fmt::format("ZTPTraderHandler: QueryPosition filter not support Instrument[{}]" ,pInvestorPosition->InstrumentID));
 			return;
 		}

		if (0 != pInvestorPosition->Position || 0 != pInvestorPosition->YdPosition) {
			FillingPosition(*pInvestorPosition, notification);
			m_publisher.NotifyUI(notification);

			char const* direction = (type::Direction::Buy == notification.Direction) ? "B" : "S";
			Log::Info(fmt::format("ZTPTraderHandler： OnRspQryInvestorPosition: UserID[{}] Instrument[{}] Dir[{}] Total[{}] Today[{}] History[{}] AvgPrice[{:.3f}]"
				,pInvestorPosition->InvestorID,pInvestorPosition->InstrumentID,direction,pInvestorPosition->Position,notification.Position,notification.YdPosition,notification.PositionCost));
		}

		if (bIsLast) {
			m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryPosition);
			Log::Info("ZTPTraderHandler: QueryPosition Success");
// 			::Sleep(IntervalMS);
		}
	}
}
void ZTPTraderHandler::OnRspQryInvestorOrder(CZtpApiOrderField *pOrder, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
		auto msg = fmt::format("ZTPTradeHandler: OnRspQryInvestorOrder Failed ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryOrder, SystemError);
		return;
	}
	if (pOrder == nullptr) {
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryOrder);
		return;
	}
	InvestorOrder order;
	order.Platform = TradePlatform::ZTP;
	order.InstrumentCode = type::InstrumentCode(pOrder->InstrumentID);
	order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
	order.Exchange = type::Exchange::SGE;
	bool isEmpty = (0 == strlen(pOrder->OrderSysID));
	if ((order.InstID == INVALID_INSTRUMENT_ID) || isEmpty) {
		Log::Warn(fmt::format("ZTPTraderHandler: OnRspQryInvestorOrder filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pOrder->InstrumentID, pOrder->InvestorID, pOrder->OrderSysID, pOrder->OrderLocalID));
		if (bIsLast) {
			m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryOrder);
		}
		return;
	}
	order.InvestorID = pOrder->InvestorID;
	order.Direction = ConvertDirectionFromZTP(pOrder->Direction);
	order.OffsetFlag = ConvertOperationFromZTP(pOrder->OffsetFlag);
	order.OrderPriceType = ConvertOrderTypeFromZTP(pOrder->OrderPriceType);
	order.InsertTime = pOrder->InsertTime;
	order.CancelTime = pOrder->CancelTime;
	order.LimitPrice = pOrder->OrderPrice;
	order.Volume = pOrder->OrderQuantity;
 	order.VolumeTrade = pOrder->TradeVolume;
 	order.VolumeRemain = order.Volume - order.VolumeTrade;

	order.OrderLocalID = atoi(pOrder->OrderLocalID);
	order.OrderSysID = pOrder->OrderSysID;
	order.OrderStatus = ConvertOrderStatus(pOrder->OrderStatus);
	order.UniqueID = pOrder->OrderLocalID;

	if (order.UniqueID.empty()) {
		order.UniqueID = pOrder->OrderSysID;
	}

	if (order.OrderStatus == type::OrderStatus::InsertFail && !order.OrderSysID.empty()) {
		order.OrderStatus = type::OrderStatus::CancelFail;
	}

	Log::Info(fmt::format("ZTP: Query Order [{}] Unique[{}] LocalID[{}] SysID[{}] [{}] [{}] [{}] [{}]", 
		order.InstrumentCode, order.UniqueID, order.OrderLocalID, order.OrderSysID, order.Direction, order.LimitPrice, order.Volume, order.VolumeTrade));

	m_publisher.NotifyUI(order);

	if (bIsLast) {
		Log::Info("ZTP: Query Order Finished");
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryOrder);
	}

}
void ZTPTraderHandler::OnRspQryInvestorTrade(CZtpApiTradeField *pTrade, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (pRspInfo != nullptr && pRspInfo->ErrorID != 0) {
		auto msg = fmt::format("ZTPTradeHandler: OnRspQryInvestorTrade Failed ErrorCode[{}] ErrorMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryTrade, SystemError);
		return;
	}
	if (pTrade == nullptr) {
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryTrade);
		return;
	}
	InvestorTrade trade;
	trade.Platform = TradePlatform::ZTP;
	trade.Exchange = type::Exchange::SGE;
	trade.InstrumentCode = type::InstrumentCode(pTrade->InstrumentID);
	trade.InstID = ServiceSinglton::Services().Instrument->toID(trade.InstrumentCode);
	bool isEmpty = (0 == strlen(pTrade->OrderSysID));
	if ((trade.InstID == INVALID_INSTRUMENT_ID) && (false == isEmpty)) {
		Log::Warn(fmt::format("ZTPTraderHandler: OnRspQryInvestorTrade filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pTrade->InstrumentID, pTrade->InvestorID, pTrade->OrderSysID, pTrade->OrderLocalID));
		return;
	}
	trade.InvestorID = pTrade->InvestorID;
	trade.TradeTime = pTrade->TradeTime;
	trade.Direction = ConvertDirectionFromZTP(pTrade->Direction);
	trade.OffsetFlag = ConvertOperationFromZTP(pTrade->OffsetFlag);
	trade.TradePrice = pTrade->TradePrice;
	trade.TradeVolume = pTrade->TradeVolume;
	trade.OrderLocalID = atoi(pTrade->OrderLocalID);
	trade.OrderSysID = pTrade->OrderSysID;
	trade.TradeID = pTrade->TradeID;
	m_publisher.NotifyUI(trade);
	Log::Info(fmt::format("ZTP: Query Trade [{}] [{}] [{}] [{}]",
		trade.InstrumentCode, trade.Direction, trade.TradePrice, trade.TradeVolume));

	if (bIsLast) {
		Log::Info("ZTP: Query Trade Finished");
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryTrade);
	}
}

void ZTPTraderHandler::OnRtnOrder(CZtpApiOrderField *pOrder) {
	if (NULL == pOrder)
		return;
	InvestorOrder order;
	order.Platform = TradePlatform::ZTP;
	order.InstrumentCode = type::InstrumentCode(pOrder->InstrumentID);
	order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
	bool isEmpty = (0 == strlen(pOrder->OrderSysID));
	if ((order.InstID == INVALID_INSTRUMENT_ID) && (false==isEmpty)) {
		Log::Warn(fmt::format("ZTPTraderHandler: OnRtnOrder filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			,pOrder->InstrumentID, pOrder->InvestorID, pOrder->OrderSysID, pOrder->OrderLocalID));
		return;
	}
	order.InvestorID = pOrder->InvestorID;
	order.InsertTime = pOrder->InsertTime;
	order.CancelTime = pOrder->CancelTime;
	order.Exchange = type::Exchange::SGE;		
	order.Direction = ConvertDirectionFromZTP(pOrder->Direction);
	order.OffsetFlag = ConvertOperationFromZTP(pOrder->OffsetFlag);
	order.OrderPriceType = ConvertOrderTypeFromZTP(pOrder->OrderPriceType);
	order.LimitPrice = pOrder->OrderPrice;
	order.Volume = pOrder->OrderQuantity;
 	order.VolumeTrade = pOrder->TradeVolume;
 	order.VolumeRemain = order.Volume - order.VolumeTrade;
 	order.VolumeRemain = pOrder->RemainQuantity;
	order.OrderLocalID = atoi(pOrder->OrderLocalID);
	order.OrderSysID =  pOrder->OrderSysID;
	order.OrderStatus = ConvertOrderStatus(pOrder->OrderStatus);
	order.UniqueID = pOrder->OrderLocalID;
	if (order.UniqueID.empty()) {
		order.UniqueID = pOrder->OrderSysID;
	}
	Log::Info(fmt::format("ZTP OnRtnOrder: Investor[{}] Inst[{}] Dir[{}] Opt[{:c}] PriceType[{}] Unique[{}] LocalID[{}] SysID[{}] Price[{:.3f}] Volume[{}] Traded[{}] Remain[{}] Status[{:c}] OrderType[{:c}]",
		order.InvestorID,order.InstrumentCode, order.Direction,order.OffsetFlag, order.OrderPriceType, order.UniqueID,
		order.OrderLocalID,order.OrderSysID, order.LimitPrice, order.Volume, order.VolumeTrade, order.VolumeRemain, order.OrderStatus, order.OrderPriceType));
	m_publisher.NotifyUI(OrderRtnNotification{ order });
	m_publisher.PushResponse(order);
}
void ZTPTraderHandler::OnRtnTrade(CZtpApiTradeField *pTrade) {
	if (NULL == pTrade)
		return;

	InvestorTrade trade;
	trade.Platform = TradePlatform::ZTP;
	trade.Exchange = type::Exchange::SGE;
	trade.InstrumentCode = type::InstrumentCode(pTrade->InstrumentID);
	trade.InstID = ServiceSinglton::Services().Instrument->toID(trade.InstrumentCode);
	if (trade.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("ZTPTraderHandler: OnRtnTrade filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, pTrade->InstrumentID ,pTrade->InvestorID, pTrade->OrderSysID, pTrade->OrderLocalID));
		return;
	}
	trade.InvestorID = pTrade->InvestorID;
	trade.TradeTime = pTrade->TradeTime;
	trade.Direction = ConvertDirectionFromZTP(pTrade->Direction);
	trade.OffsetFlag = ConvertOperationFromZTP(pTrade->OffsetFlag);
	trade.TradeVolume = pTrade->TradeVolume;
	trade.TradePrice = pTrade->TradePrice;
	trade.OrderLocalID = atoi(pTrade->OrderLocalID);
	trade.OrderSysID =  pTrade->OrderSysID;
	m_publisher.NotifyUI(TradeNotification{ trade });
	m_publisher.PushResponse(trade);

	char const* direction = IsBuy(trade.Direction) ? "B" : "S";
	Log::Info(fmt::format("ZTPTraderHandler OnRtnTrade: InvestorID[{}] Instrument[{}] Dir[{}] Opt[{:c}] Price[{:.3f}] Trade[{}] SysID[{}] LocalID[{}]"
		,pTrade->InvestorID, pTrade->InstrumentID, direction, pTrade->OffsetFlag, pTrade->TradePrice, pTrade->TradeVolume, pTrade->OrderSysID, pTrade->OrderLocalID));
}

//交易所错误委托回报
void ZTPTraderHandler::OnErrRtnOrderInsert(CZtpApiInputOrderField *pInputOrder, CZtpApiRspInfoField *pRspInfo) {
	if (NULL == pInputOrder)
		return;
	
	if (IsErrorRspInfo(pRspInfo)) {
		InvestorOrder order;
		order.InstrumentCode = type::InstrumentCode(pInputOrder->InstrumentID);
		order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
		if (INVALID_INSTRUMENT_ID == order.InstID) {
			Log::Warn(fmt::format("ZTPTraderHandler: OnErrRtnOrderInsert received not support Instrument[{}], userID[{}] localID[{}] "
				,pInputOrder->InstrumentID, pInputOrder->InvestorID, pInputOrder->OrderLocalID));
			return;
		}

		OrderErrorNotification noti;
		noti.Type = OrderErrorNotification::Insert;
		noti.Platform = TradePlatform::ZTP;
		noti.OrderLocalID = atoi(pInputOrder->OrderLocalID);
		noti.Error = from(pRspInfo);
		if (noti.OrderLocalID == 0) {
			Log::Warn(fmt::format("ZTPTraderHandler: OnErrRtnOrderInsert received other system response Instrument[{}], userID[{}] localID[{}] "
				, pInputOrder->InstrumentID, pInputOrder->InvestorID, pInputOrder->OrderLocalID));
			return;
			return;
		}
		m_publisher.NotifyUI(noti);
		m_publisher.PushResponse(noti);

		//返回内容和OnRspOrderInsert中返回的错误相同，注释之
 		char const* direction = ConvertDirectionFromZTP(pInputOrder->Direction) == type::Direction::Sell ? "S" : "B";
 		auto msg = fmt::format("Exchange OrderInsert Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] Dir[{}] Opt[{}] Info[{:.3f},{}] LocalID[{}]"
 			, pRspInfo->ErrorID, pRspInfo->ErrorMsg, pInputOrder->InvestorID, pInputOrder->InstrumentID, direction, pInputOrder->OffsetFlag, pInputOrder->OrderPrice, pInputOrder->OrderQuantity, pInputOrder->OrderLocalID);
 		Log::Error(msg);
	}
}
//交易所撤单错误回报
void ZTPTraderHandler::OnErrRtnOrderAction(CZtpApiInputOrderActionField*pOrderAction, CZtpApiRspInfoField *pRspInfo) {

	if (NULL == pOrderAction)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		InvestorOrder order;
		order.InstrumentCode = type::InstrumentCode(pOrderAction->InstrumentID);
		order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
		if (INVALID_INSTRUMENT_ID == order.InstID) {
			Log::Warn(fmt::format("ZTPTraderHandler OnErrRtnOrderAction received not exist Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}] ActionID[{}]",
				pOrderAction->InstrumentID, pOrderAction->InvestorID, pOrderAction->OrderSysID, pOrderAction->OrderLocalID, pOrderAction->OrderActionLocalID));
			return;
		}

		OrderErrorNotification errorNotify;
		errorNotify.Platform = TradePlatform::ZTP;
		errorNotify.Type = OrderErrorNotification::Cancel;
		errorNotify.OrderLocalID = atoi(pOrderAction->OrderLocalID);
		errorNotify.CancelLocalID = atoi(pOrderAction->OrderActionLocalID);
		errorNotify.OrderSysID = pOrderAction->OrderSysID;
		order.UniqueID = pOrderAction->OrderLocalID;
		if (order.UniqueID.empty()) {
			order.UniqueID = pOrderAction->OrderActionLocalID;
		}

		errorNotify.Error = from(pRspInfo);
		m_publisher.NotifyUI(errorNotify);
		m_publisher.PushResponse(errorNotify);
		auto msg = fmt::format("Exchange OrderAction Failed: ErrorCode[{}] ErrorMsg[{}] UserID[{}] Instrument[{}] SystemID[{}] LocalID[{}] "
			,pRspInfo->ErrorID,pRspInfo->ErrorMsg,pOrderAction->InvestorID,pOrderAction->InstrumentID,pOrderAction->OrderSysID,pOrderAction->OrderLocalID);
		Log::Error(msg);
	}
}

void ZTPTraderHandler::OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus) {
	if (NULL == pInstrumentStatus)
		return;

	bool notifyTag = true;
	ExchangeNotification notification;
	notification.Platform = TradePlatform::ZTP;
	notification.Exchange = type::Exchange::SGE;
	notification.Status = ConvertExchangeStatusFromZTP(*pInstrumentStatus);
	if (notifyTag) {
		m_publisher.NotifyUI(notification);
		m_publisher.PushResponse(notification);
		notifyTag = false;
		Log::Info(fmt::format("Exchange[{}] Status[{}]", GetExchangeDescribe(notification.Exchange), *pInstrumentStatus));
	}
}

void ZTPTraderHandler::OnRspQryInstrument(CZtpApiRspInstrumentField* pInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryInstrument, from(pRspInfo));
		return;
	}
	if (nullptr == pInstrument) {
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryInstrument);
		return;
	}

	Instrument inst;
	inst.Platform = TradePlatform::ZTP;
	inst.Exchange = type::Exchange::SGE;
	inst.Code = pInstrument->InstrumentID;
	inst.InstID = ServiceSinglton::Services().Instrument->toID(inst.Code);
	inst.LongPosLimit = pInstrument->LongPosLimit;
	inst.ShortPosLimit = pInstrument->ShortPosLimit;
	inst.MaxOrderVolume = pInstrument->MaxLimitOrderVolume;
	inst.MinOrderVolume = pInstrument->MinLimitOrderVolume;
	inst.Name = pInstrument->InstrumentName;
	inst.ProductClass = ConvertProductClassFromZTP(pInstrument->ProductClass);
	inst.UnitPrice = pInstrument->PriceTick;
	inst.VolumeMultiple = pInstrument->VolumeMultiple;

	m_publisher.NotifyUI(inst);

	ServiceSinglton::Services().Instrument->Add(inst);

	if (bIsLast) {
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryInstrument);
	}
}

void ZTPTraderHandler::OnRspQryInvestorMargin(CZtpApiRspMarginField* pMargin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pMargin == nullptr) {
		return;
	}
	if (IsErrorRspInfo(pRspInfo)) {
		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryMargin, from(pRspInfo));
		return;
	}
	InvestorMargin margin;
	margin.Platform = TradePlatform::ZTP;
	margin.Exchange = type::Exchange::SGE;
	margin.BrokerID = pMargin->BrokerID;
	margin.InvestorID = pMargin->InvestorID;
	margin.InstrumentCode = pMargin->InstrumentID;
	margin.InstID = ServiceSinglton::Services().Instrument->toID(margin.InstrumentCode);
	margin.LongMarginRate = pMargin->LongMarginRate;
	margin.LongMarginAmt = pMargin->LongMarginAmt;
	margin.ShortMarginRate = pMargin->ShortMarginRate;
	margin.ShortMarginAmt = pMargin->ShortMarginAmt;
	if (margin.InstID != INVALID_INSTRUMENT_ID) {
		Log::Info(fmt::format("ZTP Query Margin: [{}] [{}] [{}] [{}] [{}]",
			margin.InstrumentCode, margin.LongMarginRate, margin.LongMarginAmt, margin.ShortMarginRate, margin.ShortMarginAmt));
		m_publisher.NotifyUI(margin);
	}

	if (bIsLast) {
		Log::Info("ZTP Query Margin Finished");
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryMargin);
	}
}

void ZTPTraderHandler::OnRspQryInvestorFee(CZtpApiRspFeeField* pFee, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (pFee == nullptr) {
		return;
	}
	if (IsErrorRspInfo(pRspInfo)) {
		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QueryFee, from(pRspInfo));
		return;
	}
	InvestorFee fee;
	fee.Platform = TradePlatform::ZTP;
	fee.Exchange = type::Exchange::SGE;
	fee.BrokerID = pFee->BrokerID;
	fee.InvestorID = pFee->InvestorID;
	fee.InstrumentCode = pFee->InstrumentID;
	fee.InstID = ServiceSinglton::Services().Instrument->toID(fee.InstrumentCode);
	fee.OffsetFeeAmt = pFee->OffsetFeeAmt;
	fee.OffsetFeeRate = pFee->OffsetFeeRate;
	fee.OffsetTodayFeeAmt = pFee->OffsetTodayFeeAmt;
	fee.OffsetTodayFeeRate = pFee->OffsetTodayFeeRate;
	fee.OpenFeeAmt = pFee->OpenFeeAmt;
	fee.OpenFeeRate = pFee->OpenFeeRate;
	if (fee.InstID != INVALID_INSTRUMENT_ID) {
		Log::Info(fmt::format("ZTP Query Fee: [{}] [{}] [{}] [{}] [{}] [{}] [{}]",
			fee.InstrumentCode, fee.OpenFeeRate, fee.OpenFeeAmt, fee.OffsetFeeRate, fee.OffsetFeeAmt, fee.OffsetTodayFeeRate, fee.OffsetTodayFeeAmt));
		m_publisher.NotifyUI(fee);
	}
	if (bIsLast) {
		Log::Info("ZTP Query Fee Finished");
		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QueryFee);
	}
}


void ZTPTraderHandler::QueryTradingAccount() {
	CZtpApiQryAccountField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_investor.value());
	int rtnCode = m_pTraderApi->ReqQryInvestorAccount(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		auto msg = fmt::format("ZTPTraderHandler: QryTradingAccount() Failed, RtnCode[{}]", rtnCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void ZTPTraderHandler::QueryPosition() {
	CZtpApiQryPositionField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_investor.value());
	int rtnCode = m_pTraderApi->ReqQryInvestorPosition(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		Log::Error(fmt::format("ZTPTraderHandler: QueryPosition() Failed, RtnCode[{}]", rtnCode));
	}
}
void ZTPTraderHandler::QueryTrade() {
	CZtpApiQryTradeField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_investor.value());
	int rtnCode = m_pTraderApi->ReqQryInvestorTrade(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		Log::Error(fmt::format("ZTPTraderHandler: QueryTrade() Failed, RtnCode[{}]", rtnCode));
	}
}
void ZTPTraderHandler::QueryOrder() {
	CZtpApiQryOrderField queryReq = { 0 };
	strcpy_s(queryReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(queryReq.InvestorID, m_investor.value());
	int rtnCode = m_pTraderApi->ReqQryInvestorOrder(&queryReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
		Log::Error(fmt::format("ZTPTraderHandler: QueryTrade() Failed, RtnCode[{}]", rtnCode));
	}
}

void ZTPTraderHandler::FillingPosition(CZtpApiRspPositionField const& position, InvestorPosition& pos) {
	pos.Exchange = type::Exchange::SGE;
	pos.BrokerID = position.BrokerID;
	pos.InvestorID = position.InvestorID;
	pos.InstrumentCode = position.InstrumentID;
	pos.InstID = ServiceSinglton::Services().Instrument->toID(pos.InstrumentCode);
	pos.Direction = ConvertDirectionFromZTP(position.Direction);
	pos.Position = position.Position;
	pos.PositionCost = position.PositionCost;
	pos.ProductClass = ConvertProductClassFromZTP(position.ProductClass);
	pos.YdPosition = position.YdPosition;
	pos.YdPositionPrice = position.YdPositionPrice;
	
// 	double multiplier = m_instHelper.GetMultiplier(notification.InstID);
// 	notification.PositionCost = position.OpenCost / (position.Position * multiplier);	//开仓均价自己计算
}

bool ZTPTraderHandler::IsErrorRspInfo(CZtpApiRspInfoField* pRspInfo) {
	return ((NULL != pRspInfo) && (0 != pRspInfo->ErrorID));
}
