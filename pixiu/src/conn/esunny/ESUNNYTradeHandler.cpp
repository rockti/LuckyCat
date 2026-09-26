#include "pch.h"
#include "conn/esunny/ESUNNYTradeHandler.h"
#include "utils/Log.h"
#include "notify/ConnPublisher.h"
#include "config/Config.h"
#include "notify/Notify.h"
#include "conn/ExchangeConn.h"
#include "model/Instrument.h"
#include "model/dealing/ExchangeRequest.h"
#include "utils/TimeChecker.h"

inline Direction ConvertDirectionFromESUNNY(ITapTrade::TAPISideType direction) {
	return direction == ITapTrade::TAPI_SIDE_BUY ? Direction::BUY : Direction::SELL;
}

inline std::pair<bool,LocalOrderID> GetLocalIdByDirection(Direction direction, int refInt) {
	if (LocalIdACC::IsMarketing(refInt)) {
		return std::make_pair(true,direction == Direction::BUY ? refInt: refInt+1);
	} else {
		return std::make_pair(false,refInt);
	}
}

ESUNNYTradeHandler::ESUNNYTradeHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher)
:m_config(config),m_instHelper(instHelper),m_publisher(publisher),m_pTradeApi(NULL), m_sessionID(0), m_isApiReady(false) {
}

void ESUNNYTradeHandler::OnConnect() {
	Log::Info("ETH: FrontConnected");
}
void ESUNNYTradeHandler::OnRspLogin(ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPITradeLoginRspInfo* loginRspInfo) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ETH: UserLogin success, waiting for API initialize...");
	} else {
		auto msg = fmt::format("ETH: UserLogin Failed, {}", GetErrorDescription(errorCode));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void ESUNNYTradeHandler::OnExpriationDate(ITapTrade::TAPIDATE date, int days) {
	Log::Warn(fmt::format("ETH: API is still [{}] days to expire, {} ]", days));
}
void ESUNNYTradeHandler::OnAPIReady(ITapTrade::TAPIINT32 errorCode) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		m_isApiReady = true;
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::HKEX_ESUNNY;
		notification.Type = ExchangeNotification::TRADER_CONNECTED;
		m_publisher.NotifyUI(notification);
		Log::Info("ETH: API is ready");
		m_publisher.PushResponse(notification);
		QueryFund();
		QueryTradingDate();

	} else {
		auto msg = fmt::format("ETH: APIInit Failed, {}", GetErrorDescription(errorCode));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void ESUNNYTradeHandler::QueryTradingDate() {
	m_sessionID = RequestIdACC::Generate();
	auto nRtn = m_pTradeApi->QryTradingDate(&m_sessionID);
	if (ITapTrade::TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ESUNNYTradeHandler: QueryTradingDate failed, {}", GetErrorDescription(nRtn));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}

}
void ESUNNYTradeHandler::OnDisconnect(ITapTrade::TAPIINT32 reasonCode) {
	m_isApiReady = false;
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HKEX_ESUNNY;
	notification.Type = ExchangeNotification::TRADER_DISCONNECTED;
	m_publisher.NotifyUI(notification);

	auto msg = fmt::format("ETH:FrontDisconnected, {}", GetErrorDescription(reasonCode));
	m_publisher.NotifyUI(msg);
	Log::Warn(msg);

	m_publisher.PushResponse(notification);

}
//ESunny的委托错误应答在这里
void ESUNNYTradeHandler::OnRspOrderAction(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderActionRsp* info) {
	if (NULL == info)	return;

	if (ITapTrade::TAPIERROR_SUCCEED != errorCode) {
		auto msg = fmt::format("ETH:OnRspOrderAction, errorCode[{}] errorMsg[{}]", info->OrderInfo->ErrorCode, info->OrderInfo->ErrorText);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}

bool IsFakOrder(ITapTrade::TapAPIOrderInfo const& order) {
	return order.TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_FAK;
}

void ESUNNYTradeHandler::OnRtnOrder(const ITapTrade::TapAPIOrderInfoNotice* info) {
	if (NULL == info)	return;

	OrderNotification notification;
	char instrumentCode[20] = "";
	sprintf_s(instrumentCode, "%s%s", info->OrderInfo->CommodityNo, info->OrderInfo->ContractNo);
	notification.InstID = m_instHelper.GetInstID(instrumentCode);
	notification.Exchange = InstIdExchange(notification.InstID);

	if (notification.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("ETH:OnRtnOrder received not support Instrument[{}], AccountNo[{}] SystemID[{}] LocalID[{}] "
			,instrumentCode,info->OrderInfo->AccountNo,info->OrderInfo->OrderExchangeSystemNo,info->OrderInfo->RefInt));
		return;
	}
	notification.Direction = ConvertDirectionFromESUNNY(info->OrderInfo->OrderSide);
	notification.OrderPrice = info->OrderInfo->OrderPrice;
	notification.OrderQuantity = info->OrderInfo->OrderQty;
	notification.DealQuantity = info->OrderInfo->OrderMatchQty;	//全量成交
	auto [isMarketing, localId] = GetLocalIdByDirection(notification.Direction,info->OrderInfo->RefInt);
	notification.LocalId = localId;
	notification.IsMarketing = isMarketing;
	notification.Status = OrderNotification::E_UNKOWN;
	strcpy_s(notification.SystemNo, info->OrderInfo->OrderNo);

	int errorCode = (ITapTrade::TAPIERROR_SUCCEED == info->ErrorCode && ITapTrade::TAPIERROR_SUCCEED != info->OrderInfo->ErrorCode) ? info->OrderInfo->ErrorCode : info->ErrorCode;
	Log::Warn(fmt::format("ETH:OnRtnOrder, ErrorCode[{}] Inst[{}] Type[{}] Dir[{}] SysNo[{}] LocalId[{}] Deal[{:.2f},{}] OrderStatus[{}] ",
		errorCode, instrumentCode, info->OrderInfo->TimeInForce, info->OrderInfo->OrderSide, info->OrderInfo->OrderNo,localId, info->OrderInfo->OrderMatchPrice, info->OrderInfo->OrderMatchQty, info->OrderInfo->OrderState));

	//更新交易状态
	if (ITapTrade::TAPIERROR_SUCCEED != errorCode && ITapTrade::TAPIERROR_ORDERDELETE_NOT_STATE != errorCode) {
		notification.Status = OrderNotification::E_SENT_FAILED;
		auto msg = fmt::format("ETH:OrderSend failed {}, OrderStatus[{}]", GetErrorDescription(errorCode), info->OrderInfo->OrderState);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	} else if (IsPartial(*info->OrderInfo) || IsAllTraded(*info->OrderInfo)) {
		notification.Status = OrderNotification::E_QUANTITY_TRADE;
		notification.DealPrice = info->OrderInfo->OrderMatchPrice;
	} else if (IsCanceled(*info->OrderInfo)) {
		notification.Status = OrderNotification::E_CANCELED;

		if (IsFakOrder(*info->OrderInfo) && notification.DealQuantity > 0) {
			notification.DealPrice = info->OrderInfo->OrderMatchPrice;
			notification.Status = OrderNotification::E_FAK_PARTIAL_TRADE;
			Log::Info("ETH:OnRtnOrder FAK cancel, E_FAK_PARTIAL_TRADE");
		} else if (info->OrderInfo->OrderState == ITapTrade::TAPI_ORDER_STATE_LEFTDELETED) { //'A' 
			notification.DealPrice = info->OrderInfo->OrderMatchPrice;
			notification.Status = OrderNotification::E_FUCK_ESUNNY_A_CANCEL;
		}
	} else if (IsAccepted(*info->OrderInfo)) {
		notification.Status = OrderNotification::E_ACCEPT;
	} else if (IsOrdered(*info->OrderInfo)) {
		notification.Status = OrderNotification::E_ORDERED;
	}
	m_publisher.PushResponse(notification);
	if (info->OrderInfo != nullptr) {
// 		if (!CheckTime(info->OrderInfo->OrderUpdateTime)) {
// 			Log::Error(fmt::format("ETH:current time has more then 1 hour difference with server time, exit"));
// 			exit(0);
// 		}
	}
}


void ESUNNYTradeHandler::OnRtnFill(const ITapTrade::TapAPIFillInfo* info) {
//	return; // 全部使用OnRtnOrder，RtnFill里是增量成交
	if (NULL == info) return;

	char instrumentCode[20] = "";
	sprintf_s(instrumentCode, "%s%s", info->CommodityNo, info->ContractNo);

	Log::Warn(fmt::format("ETH:OnRtnFill, Inst[{}] Dir[{}] SysNo[{}] Deal[{:.2f},{}] ",
		instrumentCode, info->MatchSide, info->OrderNo, info->MatchPrice, info->MatchQty));

	return;

/*
	OrderNotification notification;
	char instrumentCode[20] = "";
	sprintf_s(instrumentCode, "%s%s", info->CommodityNo, info->ContractNo);
	notification.InstID = m_instHelper.GetInstID(instrumentCode);

// 	if (notification.InstID < 0) {
// 		Log::Warn(fmt::format("ESUNNYTradeHandler OnRtnTrade received not exist InstrumentID[{}], AccountNo[{}] SystemID[{}] LocalID[{}] "
// 			,instrumentCode, info->AccountNo, info->OrderSystemNo, m_orderRefMap[info->OrderNo]));
// 		return;
// 	}

// 	strcpy_s(notification.instrumentCode, instrumentCode);
// 	strcpy_s(notification.clientId, info->AccountNo);
	notification.Direction = ConvertDirectionFromESUNNY(info->MatchSide);
	 
	notification.DealQuantity = info->MatchQty;
	notification.DealPrice = info->MatchPrice;
//	notification.LocalId = m_orderRefMap[info->OrderNo];
	strcpy_s(notification.SystemNo, info->OrderSystemNo);
// 	strcpy_s(notification.tradeNo, info->MatchNo);
	notification.Status = OrderNotification::E_TRADE;
	m_publisher.PushResponse(notification);

// 	char const* direction = (Direction::SELL == notification.direction) ? "S" : "B";
*/
}

//做市商双边指令
void ESUNNYTradeHandler::OnRspHKMarketOrderInsert(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderMarketInsertRsp* info) {
	return;
}
void ESUNNYTradeHandler::OnRspHKMarketOrderDelete(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderMarketDeleteRsp* info) {
	if (NULL == info)	return;

	if (ITapTrade::TAPIERROR_SUCCEED != errorCode) {
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspHKMarketOrderDelete, {}", GetErrorDescription(errorCode));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}

//查询指令集合
void ESUNNYTradeHandler::OnRspQryTradingDate(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPITradingCalendarQryRsp* info) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode && NULL != info) {
		const char* MAX_DATE = "2022-1-10"; //Owen TODO removing in the future
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryTradingDate, {}", 
			info->CurrTradeDate);
		Log::Info(msg);
		if (strcmp(info->CurrTradeDate, MAX_DATE) > 0) {
			MessageBox(NULL, _T("软件已经过期!"), _T("错误"), MB_ICONSTOP);
			exit(-1);
		}
	} else {
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryTradingDate, Failed, Error{} {}", 
			errorCode, GetErrorDescription(errorCode));
		Log::Error(msg);
	}

}
void ESUNNYTradeHandler::OnRspQryAccount(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIUINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED==errorCode && NULL!= info) {
		Log::Info(fmt::format("ESUNNYTradeHandler: QryAccount success, accountNo[{}], accountType[{}]]", info->AccountNo, info->AccountType));
		QueryPosition();
	}
}
void ESUNNYTradeHandler::OnRspQryFund(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIFundData* info) {
	if (ITapTrade::TAPIERROR_SUCCEED==errorCode && NULL != info) {
		if (0==strcmp(info->CurrencyNo, "HKD")) {
			FundsNotification notification;
			notification.Platform = ConnPlatform::HKEX_ESUNNY;
			notification.Capital = info->Balance;
			notification.Available = info->Available;		
			notification.Margin = info->AccountMaintenanceMargin;	//占用保证金
			notification.Fee = info->FrozenFee;						//占用手续费
			notification.CloseProfit = info->CloseProfit;
			m_publisher.PushResponse(notification);
// 			m_publisher.NotifyUI(notification);		// 不给界面，界面资金由框架推送UpdateAvailable

			Log::Info(fmt::format("ETH: QryFund Success, Capital[{:.2f}] Available[{:.2f}] Margin[{:.2f}] Profit[{:.2f}] Fee[{:.2f}]"
				, info->Balance, info->Available, info->AccountMaintenanceMargin, info->CloseProfit, info->FrozenFee));
			m_publisher.PushResponse(CleanPositionNotification());
			QueryPosition();
		}
	} else {
		Log::Error(fmt::format("ETH: QueryFund Failed, {}", GetErrorDescription(errorCode)));
	}
}
void ESUNNYTradeHandler::OnRtnFund(const ITapTrade::TapAPIFundData* info) {
	return;
	//推送资金变化，暂时不用，打个日志看看推送频率
	Log::Warn(fmt::format("ETH: OnRtnFund, Account[{}] Profit[{}]", info->AccountNo, info->CloseProfit));
}
void ESUNNYTradeHandler::OnRspQryOrder(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIOrderInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ESUNNYTradeHandler: QryOrder success");
	} else {
		Log::Error(fmt::format("ETH: QryOrder Failed, {}", GetErrorDescription(errorCode)));
	}
}
void ESUNNYTradeHandler::OnRspQryFill(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIFillInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ESUNNYTradeHandler: QryFill success");
	} else {
		Log::Error(fmt::format("ETH: QryFill Failed, {}", GetErrorDescription(errorCode)));
	}
}
void ESUNNYTradeHandler::OnRspQryPosition(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIPositionInfo* info) {
	//暂时只打个日志，如果需要持仓明细再说
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ETH: QryPosition success");
	} else {
		Log::Error(fmt::format("ETH: QryPosition Failed, {}]", GetErrorDescription(errorCode)));
	}
}
void ESUNNYTradeHandler::OnRtnPosition(const ITapTrade::TapAPIPositionInfo* info) {
	return;
	//推送持仓明细，暂时不用，打个日志看看推送频率
	Log::Warn(fmt::format("ETH: OnRtnPosition, Account[{}] Instrument[{}{}] Direction[{}] Quantity[{}] AvgPrice[{}] MatchNo[{}]"
		,info->AccountNo, info->CommodityNo, info->ContractNo, info->MatchSide, info->PositionQty, info->PositionPrice, info->MatchNo));
}
void ESUNNYTradeHandler::OnRspQryPositionSummary(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIPositionSummary* info) {
	//查询持仓汇总
	if (ITapTrade::TAPIERROR_SUCCEED==errorCode && NULL != info) {
		char instrumentCode[20] = "";
		sprintf_s(instrumentCode, "%s%s", info->CommodityNo, info->ContractNo);
		PositionNotification notification;
		notification.InstID = m_instHelper.GetInstID(instrumentCode);
		if (IsInvalidInstID(notification.InstID)) {
			Log::Error(fmt::format("ETH: QueryPositionSummary received not exist Instrument[{}]", instrumentCode));
			return;
		}

		notification.Direction = (ITapTrade::TAPI_SIDE_BUY == info->MatchSide) ? Direction::BUY : Direction::SELL;
		notification.Quantity = info->PositionQty;
		notification.Price = info->PositionPrice;
		m_publisher.PushResponse(notification);
		m_publisher.NotifyUI(notification);
	} else {
		Log::Info("ETH: there is no position be queried");
	}
}
void ESUNNYTradeHandler::OnRtnPositionSummary(const ITapTrade::TapAPIPositionSummary* info) {
	return;
	//推送持仓汇总，暂时不用，打个日志看看推送频率
	Log::Info(fmt::format("ETH: OnRtnPositionSummary, Account[{}] Instrument[{}{}] Direction[{}] Quantity[{}] AvgPrice[{}]"
		, info->AccountNo, info->CommodityNo, info->ContractNo, info->MatchSide, info->PositionQty, info->PositionPrice));
}

//交易员补单指令
void ESUNNYTradeHandler::OnRspOrderLocalRemove(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalRemoveRsp* info) {

}
void ESUNNYTradeHandler::OnRspOrderLocalInput(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalInputRsp* info) {

}
void ESUNNYTradeHandler::OnRspOrderLocalModify(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalModifyRsp* info) {

}
void ESUNNYTradeHandler::OnRspFillLocalInput(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIFillLocalInputRsp* info) {

}
void ESUNNYTradeHandler::OnRspFillLocalRemove(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIFillLocalRemoveRsp* info) {

}
void ESUNNYTradeHandler::QueryFund() {
	ITapTrade::TapAPIFundReq qryFund = {0};
	strcpy_s(qryFund.AccountNo, m_config.Credential.UserID.c_str());

	m_sessionID = RequestIdACC::Generate();
	ITapTrade::TAPIINT32 nRtn = m_pTradeApi->QryFund(&m_sessionID, &qryFund);
	if (ITapTrade::TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ESUNNYTradeHandler: QryFund failed, {}", GetErrorDescription(nRtn));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}

}
void ESUNNYTradeHandler::QueryPosition() {
	ITapTrade::TapAPIPositionQryReq positionQry = { 0 };
	strcpy_s(positionQry.AccountNo, m_config.Credential.UserID.c_str());

	m_sessionID = RequestIdACC::Generate();
	ITapTrade::TAPIINT32 nRtn = m_pTradeApi->QryPositionSummary(&m_sessionID, &positionQry);	//查询持仓汇总还是明细要斟酌
	if (ITapTrade::TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ETH: QueryPosition failed, {}", GetErrorDescription(nRtn));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
	m_publisher.PushResponse(CleanPositionNotification());
}

void ESUNNYTradeHandler::OnRtnContactInfo(ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TAPISTR_40 ContactInfo) {
	ITapTrade::TAPIINT32 nRtn = m_pTradeApi->RequestVertificateCode(&m_sessionID, const_cast<char*>(ContactInfo));
	if (ITapTrade::TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ETH: RequestVertificateCode failed, {}", GetErrorDescription(nRtn));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	} else {
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::HKEX_ESUNNY;
		notification.Type = ExchangeNotification::CODE_LOGIN;		//二次认证登陆
		m_publisher.NotifyUI(notification);
	}
}
void ESUNNYTradeHandler::OnRspRequestVertificateCode(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIRequestVertificateCodeRsp* rsp) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ETH: VertificateCode success");
	} else {
		Log::Error(fmt::format("ETH: VertificateCode Failed, {}", GetErrorDescription(errorCode)));
	}
}
std::string ESUNNYTradeHandler::GetErrorDescription(ITapTrade::TAPIINT32 errorCode) {
	switch (errorCode) {
	/***************************************登录过程错误***************************************/
	case ITapTrade::TAPIERROR_LOGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录过程执行错误");
	case ITapTrade::TAPIERROR_LOGIN_USER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户不存在");
	case ITapTrade::TAPIERROR_LOGIN_DDA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要进行动态认证");
	case ITapTrade::TAPIERROR_LOGIN_LICENSE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户未授权");
	case ITapTrade::TAPIERROR_LOGIN_MODULE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录模块不正确");
	case ITapTrade::TAPIERROR_LOGIN_FORCE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要强制修改密码");
	case ITapTrade::TAPIERROR_LOGIN_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录状态禁止登陆");
	case ITapTrade::TAPIERROR_LOGIN_PASS:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录密码不正确");
	case ITapTrade::TAPIERROR_LOGIN_RIGHT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有该模块登录权限");
	case ITapTrade::TAPIERROR_LOGIN_COUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录数量超限");
	case ITapTrade::TAPIERROR_LOGIN_NOTIN_SERVERFLAGUSRES:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户不在服务器标识下可登录用户列表中");
	case ITapTrade::TAPIERROR_LOGIN_FREEZE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户已被冻结");
	case ITapTrade::TAPIERROR_LOGIN_TOFREEZE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "密码错误，用户冻结");
	case ITapTrade::TAPIERROR_LOGIN_ACCOUNTSTATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户状态不允许登录");
	case ITapTrade::TAPIERROR_LOGIN_SECCERTIFI:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要进行二次认证");
	case ITapTrade::TAPIERROR_LOGIN_NOSECONDSET:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未绑定二次认证信息");
	case ITapTrade::TAPIERROR_LOGIN_NOTURSTHOST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不受信任的计算机登录");
	case ITapTrade::TAPITAPIERROR_SECONDCERTIFICATION_FAIL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "二次验证失败");
	case ITapTrade::TAPITAPIERROR_SECONDCERTIFICATION_TIMEOVER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "二次验证超时");
	/***************************************交易服务错误***************************************/
	case ITapTrade::TAPIERROR_ORDERINSERT_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "资金账号不存在");
	case ITapTrade::TAPIERROR_ORDERINSERT_ACCOUNT_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "资金账号状态不正确");
	case ITapTrade::TAPIERROR_ORDERINSERT_TRADECENT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "资金账号交易中心不一致");
	case ITapTrade::TAPIERROR_ORDERINSERT_CONTRACT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单无效的合约");
	case ITapTrade::TAPIERROR_ORDERINSERT_LME_NOTREADY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "LME未准备就绪");
	case ITapTrade::TAPIERROR_ORDERINSERT_ERROR_ORDER_TYPE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不支持的下单类型");
	case ITapTrade::TAPIERROR_ORDERINSERT_ORDER_TYPE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不合法的委托类型");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户权限禁止交易");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_COM_GROUP:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户品种分组禁止交易");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_SYSTEM:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "系统权限禁止交易");
	case ITapTrade::TAPIERROR_ORDER_CLOSE_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户权限只可平仓");
	case ITapTrade::TAPIERROR_ORDER_CLOSE_SYSTEM:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "系统权限只可平仓");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_RISK:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户品种风控权限禁止交易");
	case ITapTrade::TAPIERROR_ORDER_CLOSE_RISK:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户品种风控权限只可平仓");
	case ITapTrade::TAPIERROR_ORDERINSERT_POSITIONMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "持仓量超过最大限制");
	case ITapTrade::TAPIERROR_ORDERINSERT_ONCEMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单超过单笔最大量");
	case ITapTrade::TAPIERROR_ORDERINSERT_TRADEROUTE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单合约无交易路由");
	case ITapTrade::TAPIERROR_ORDER_IN_MOD_PRICE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托价格超出偏离范围");
	case ITapTrade::TAPIERROR_ORDER_IN_GIVEUP_POS_MAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "超过GiveUp最大持仓量");
	case ITapTrade::TAPIERROR_UPPERCHANNEL_NOT_LOGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未登录网关");
	case ITapTrade::TAPIERROR_UPPERCHANNEL_NOT_FOUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未找到网关信息");
	case ITapTrade::TAPIERROR_ORDERINSERT_NOTENOUGHFUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单资金不足");
	case ITapTrade::TAPIERROR_ORDERINSERT_FEE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "手续费参数错误");
	case ITapTrade::TAPIERROR_ORDERINSERT_MARGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "保证金参数错误");
	case ITapTrade::TAPIERROR_ORDERINSERT_BASENOFUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "总基币资金不足");
	case ITapTrade::TAPIERROR_ORDERDELETE_NOT_SYSNO:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "撤单无此系统号");
	case ITapTrade::TAPIERROR_ORDERDELETE_NOT_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态不允许撤单");
	case ITapTrade::TAPIERROR_ORDERDELETE_NO_INPUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单不允许撤单");
	case ITapTrade::TAPIERROR_ORDERMODIFY_NOT_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态不允许改单");
	case ITapTrade::TAPIERROR_ORDERINPUT_REPEAT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单重复");
	case ITapTrade::TAPIERROR_UPPER_ONCEMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单超过上手单笔最大量");
	case ITapTrade::TAPIERROR_UPPER_POSITIONMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单超过上手最大持仓量");
	case ITapTrade::TAPIERROR_ORDERINSERT_CLOSEMODE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "开平方式错误");
	case ITapTrade::TAPIERROR_CLOSE_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托平仓持仓不足");
	case ITapTrade::TAPIERROR_CLOSE_MATCH:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交平仓失败");
	case ITapTrade::TAPIERROR_MOD_DEL_NO_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未找到本地委托");
	case ITapTrade::TAPIERROR_MOD_DEL_GATEWAY_DISCON:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "与网关断开连接");
	case ITapTrade::TAPIERROR_MATCHINPUT_REPEAT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交重复");
	case ITapTrade::TAPIERROR_MATCHINPUT_NO_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交未找到对应委托");
	case ITapTrade::TAPIERROR_MATCHINPUT_NO_CONTRACT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交合约不存在");
	case ITapTrade::TAPIERROR_MATCHINPUT_PARM_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交参数错误");
	case ITapTrade::TAPIERROR_MATCHINPUT_OSTATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交委托状态错误");
	case ITapTrade::TAPIERROR_MATCHREMOVE_NO_MATCH:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交删除未找到成交");
	case ITapTrade::TAPIERROR_MATCHREMOVE_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态成交不可删");
	case ITapTrade::TAPIERROR_ORDERINPUT_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不允许录入此状态订单");
	case ITapTrade::TAPIERROR_ORDERINPUT_MOD_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "错误的修改订单请求");
	case ITapTrade::TAPIERROR_ORDERREMOVE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "订单不可删，存在对应成交");
	case ITapTrade::TAPIERROR_ORDERINPUT_MOD_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不合法的委托状态");
	case ITapTrade::TAPIERROR_ORDEREXCHANGE_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态不允许订单转移");
	case ITapTrade::TAPIERROR_ORDERREMOVE_NOT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "订单不允许删除");
	case ITapTrade::TAPIERROR_ORDERMARKET_DELETE_NOTFOUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单未找到委托");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_ACCOUNT_NE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单客户不一致");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_COMMODITY_NE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单品种不一致");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_CONTRACT_NE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单合约不一致");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_SIDE_EQ:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单买卖方向相同");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_SIDE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单买卖方向错误");
	case ITapTrade::TAPIERROR_ORDERMARKET_OTHER_SIDE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商单边检查未通过");
	/***************************************网关错误代码***************************************/
	case ITapTrade::TAPIERROR_GW_NOT_READY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "网关未就绪，未连接上手");
	case ITapTrade::TAPIERROR_GW_INVALID_COMMODITY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "品种错误");
	case ITapTrade::TAPIERROR_GW_INVALID_CONTRACT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "合约错误");
	case ITapTrade::TAPIERROR_GW_INVALID_FIELD:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "报单字段有误");
	case ITapTrade::TAPIERROR_GW_INVALID_PRICE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "价格不合法");
	case ITapTrade::TAPIERROR_GW_INVALID_VOLUME:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "数量不合法");
	case ITapTrade::TAPIERROR_GW_INVALID_TYPE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "报单类型不合法");
	case ITapTrade::TAPIERROR_GW_INVALID_MODE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托模式不合法");
	case ITapTrade::TAPIERROR_GW_ORDER_NOT_EXIST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托不存在（改单、撤单)");
	case ITapTrade::TAPIERROR_GW_SEND_FAIL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "发送报单失败");
	/***************************************前置返回错误***************************************/
	case ITapTrade::TAPIERROR_TRADEFRONT_MODULETYPEERR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置不允许该模块登录");
	case ITapTrade::TAPIERROR_TRADEFRONT_TOOMANYDATA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "一次请求太多数据");
	case ITapTrade::TAPIERROR_TRADEFRONT_NODATA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置没有所要数据");
	case ITapTrade::TAPIERROT_TRADEFRONT_NOUSER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "所查询的操作员信息不存在");
	case ITapTrade::TAPIERROR_TRADEFRONT_DISCONNECT_TRADE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置与交易断开");
	case ITapTrade::TAPIERROR_TRADEFRONT_DISCONNECT_MANAGE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置与管理断开");
	case ITapTrade::TAPIERROR_TRADEFRONT_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下属资金账号不存在");
	case ITapTrade::TAPIERROR_TRADEFRONT_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "该操作员不允许交易");
	case ITapTrade::TAPIERROR_TRADEFRONT_FREQUENCY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "查询频率过快");
	case ITapTrade::TAPIERROR_TRADEFRONT_RUFUSE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "该授权不予许登录");
	case ITapTrade::TAPIERROR_TRADEFRONT_SELFMATCH:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "自成交验证不通过");
	/***************************************网络断开错误***************************************/
	case ITapTrade::TAPIERROR_ConnectFail:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "连接服务失败");
	case ITapTrade::TAPIERROR_LinkAuthFail:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "链路认证失败");
	case ITapTrade::TAPIERROR_HostUnavailable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "主机地址不可用");
	case ITapTrade::TAPIERROR_SendDataError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "发送数据错误");
	case ITapTrade::TAPIERROR_NOFrontAvailable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没用可用的接入前置");
	case ITapTrade::TAPIERROR_RepeatLogin:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "重复登录");
	case ITapTrade::TAPIERROR_InnerError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "内部错误");
	case ITapTrade::TAPIERROR_LastReqNotFinish:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "上一次请求还没有结束");
	case ITapTrade::TAPIERROR_InputValueError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "输入参数非法");
	case ITapTrade::TAPIERROR_AuthCode_Invalid:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码不合法");
	case ITapTrade::TAPIERROR_AuthCode_Expired:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码超期");
	case ITapTrade::TAPIERROR_AuthCode_TypeNotMatch:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码类型不匹配");
	case ITapTrade::TAPIERROR_API_NotReady:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "API还没有准备好");
	case ITapTrade::TAPIERROR_UDP_LISTEN_FAILED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "UDP端口监听失败");
	case ITapTrade::TAPIERROR_UDP_LISTENING:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "UDP正在监听");
	case ITapTrade::TAPIERROR_NotImplemented:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "接口未实现");
	case ITapTrade::TAPIERROR_CallOneTimeOnly:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "每次登陆只允许调用一次");
	case ITapTrade::TAPIERROR_ORDER_FREQUENCY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "超过下单频率");
	case ITapTrade::TAPIERROR_RENTQRY_TOOFAST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "查询频率太快");
	case ITapTrade::TAPIERROR_CALL_NOCONDITION:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不符合调用条件");
	case ITapTrade::TAPIERROR_ORDER_NOTFOUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "改单撤单时没有找到对应订单");
	case ITapTrade::TAPIERROR_RIGHT_ORDERINPUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有订单录入或者成交录入");
	case ITapTrade::TAPIERROR_RIGHT_LOCALOPERATION:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有订单修改和订单删除权限，成交删除权限");
	case ITapTrade::TAPIERROR_RIGHT_ORDERTRANSFER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有订单转移权限");
	case ITapTrade::TAPIERROR_FILLINPUT_SYSTEMNO:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交录入时系统号为空");
	case ITapTrade::TAPIERROR_FILLREMOVE_MATCHNO:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交删除时成交号为空");
	case ITapTrade::TAPIERROR_FILLREQMOVE_NOFUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交删除时没有找到对应的成交");
	/***************************************网络断开错误***************************************/
	case ITapTrade::TAPIERROR_DISCONNECT_CLOSE_INIT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "主动断开");
	case ITapTrade::TAPIERROR_DISCONNECT_CLOSE_PASS:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "被动断开");
	case ITapTrade::TAPIERROR_DISCONNECT_READ_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "读错误");
	case ITapTrade::TAPIERROR_DISCONNECT_WRITE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "写错误");
	case ITapTrade::TAPIERROR_DISCONNECT_BUF_FULL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "缓冲区满");
	case ITapTrade::TAPIERROR_DISCONNECT_IOCP_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "异步操作错误");
	case ITapTrade::TAPIERROR_DISCONNECT_PARSE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "解析数据错误");
	case ITapTrade::TAPIERROR_DISCONNECT_CONNECT_TIMEOUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "连接超时");
	case ITapTrade::TAPIERROR_DISCONNECT_INIT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "初始化失败");
	case ITapTrade::TAPIERROR_DISCONNECT_HAS_CONNECTED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "已经连接");
	case ITapTrade::TAPIERROR_DISCONNECT_HAS_EXIT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "工作线程已结束");
	case ITapTrade::TAPIERROR_DISCONNECT_TRY_LATER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "操作正在进行，请稍后重试");
	default:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未知错误");
	}
}

bool ESUNNYTradeHandler::IsOrdered(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为已排队；有系统号；成交量为0
	return (ITapTrade::TAPI_ORDER_STATE_QUEUED ==order.OrderState) && (0==order.OrderMatchQty);
}
bool ESUNNYTradeHandler::IsCanceled(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为完全撤单、已撤余单；有系统号；同时委托量大于成交量。PS：FAK当作撤单处理
	return (ITapTrade::TAPI_ORDER_STATE_CANCELED==order.OrderState || ITapTrade::TAPI_ORDER_STATE_LEFTDELETED==order.OrderState) && (order.OrderQty>order.OrderMatchQty);
}
bool ESUNNYTradeHandler::IsPartial(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为部分成交；有系统号；同时委托量大于成交量
	return (ITapTrade::TAPI_ORDER_STATE_PARTFINISHED==order.OrderState) && (order.OrderQty>order.OrderMatchQty);
}
bool ESUNNYTradeHandler::IsAllTraded(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为完全成交；成交量等于委托量
	return (ITapTrade::TAPI_ORDER_STATE_FINISHED==order.OrderState) && (order.OrderMatchQty==order.OrderQty);
}
bool ESUNNYTradeHandler::IsAccepted(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为已受理；没有系统号
	return (ITapTrade::TAPI_ORDER_STATE_ACCEPT==order.OrderState);
}
bool ESUNNYTradeHandler::IsSendFailed(ITapTrade::TapAPIOrderInfo const& order) const {
	//居然是通过ErrorCode来判断的，奇葩的选择
	return (ITapTrade::TAPIERROR_SUCCEED!=order.ErrorCode);
}
