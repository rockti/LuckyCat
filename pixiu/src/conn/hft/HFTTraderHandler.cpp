#include "pch.h"
#include "HFTTraderHandler.h"

#include "YD/ydError.h"
#include "config/Config.h"
#include "notify/ConnPublisher.h"
#include "notify/Notify.h"
#include "utils/Log.h"
#include "model/dealing/ExchangeRequest.h"

namespace {
	inline static Direction ConvertDirectionFromYD(char direction) {
		return direction == YD_D_Buy ? Direction::BUY : Direction::SELL;
	}
	inline static Operation ConvertOperationFromYD(char operation) {
		switch (operation) {
		case YD_OF_Open:
			return Operation::OPEN;
		case YD_OF_Close:
		case YD_OF_ForceClose:
		case YD_OF_CloseYesterday:
			return Operation::OFFSET;
		case YD_OF_CloseToday:
			return Operation::OFFSET_TODAY;
		default:
			return Operation::UNKNOWN;
		}
	}
	inline char const* GetOrderStatus(char status) {
		switch (status) {
		case YD_OS_Accepted:	return "Accepted";
		case YD_OS_Queuing:		return "Queuing";
		case YD_OS_Canceled:	return "Canceled";
		case YD_OS_AllTraded:	return "AllTraded";
		case YD_OS_Rejected:	return "Rejected";
		default: return "Unknown";
		}
	}
	inline char const* GetErrDesc(int errorCode) {
		switch (errorCode) {
		case YD_ERROR_NoError:	return "正确";
		case YD_ERROR_NoPositionToClose:	return "无仓位可平";
		case YD_ERROR_NoMoneyToOpen:	return "无足够资金开仓";
		case YD_ERROR_SystemNotReady:	return "ydServer尚未就绪";
		case YD_ERROR_InvalidInstrument:	return "无此合约";
		case YD_ERROR_InvalidAccount:	return "无此账户";
		case YD_ERROR_OrderFieldError:	return "报单请求字段填写错误";
		case YD_ERROR_MemoryExceed:	return "ydServer内存耗尽";
		case YD_ERROR_NoTradingCodeInExchange:	return "无此账户对应的交易所客户编码";
		case YD_ERROR_CanNotSendToExchange:	return "没有可用席位";
		case YD_ERROR_NoTradingRight:	return "无交易权限";
		case YD_ERROR_InvalidOrderVolume:	return "报单数量错误";
		case YD_ERROR_InvalidClientAPP:	return "AppID或AppAuthCode错误";
		case YD_ERROR_PositionLimitExceed:	return "超过持仓限量";
		case YD_ERROR_TradeVolumeExceed:	return "超过成交限量";
		case YD_ERROR_OrderCancelLimitExceed:	return "超过撤单限量";
		case YD_ERROR_OrderOpenLimitExceed:	return "超过开仓限量";
		case YD_ERROR_InvalidConnectionID:	return "无效的交易所席位连接编号";
		case YD_ERROR_AlreadyLogined:	return "已经完成登录";
		case YD_ERROR_PasswordError:	return "密码错误";
		case YD_ERROR_TooManyRequests:	return "登录请求数量超过限制";
		case YD_ERROR_InvalidUsername:	return "无此用户";
		case YD_ERROR_UsernameMismatch:	return "不可以修改其他客户的密码";
		case YD_ERROR_OldPasswordMismatch:	return "修改密码时提供了无效的旧密码";
		case YD_ERROR_AccountRequestTooFast:	return "报单请求超过流控限制";
		case YD_ERROR_PossibleSelfTrade:	return "未通过自成交校验";
		case YD_ERROR_NoAdminRight:	return "缺少管理权限";
		case YD_ERROR_InvalidAddress:	return "错误的登录IP地址";
		case YD_ERROR_OrderTypeNotSupported:	return "报单类型错误";
		case YD_ERROR_CancelOrderFieldError:	return "撤单请求中字段填写错误";
		case YD_ERROR_InvalidExchange:	return "无此交易所";
		case YD_ERROR_OrderNotFound:	return "无此报单";
		case YD_ERROR_OrderNotBelongToAccount:	return "非客户自己报单";
		case YD_ERROR_OrderFinished:	return "报单已经撤销或成交";
		case YD_ERROR_OnlyLimitOrderCanBeCanceled:	return "仅限价报单方可撤销";
		case YD_ERROR_ClientReportError:	return "客户报告错误";
		case YD_ERROR_TooManyOrders:	return "超过账户最大报单量限制";
		case YD_ERROR_InstrumentCanNotTrade:	return "合约不能交易";
		case YD_ERROR_YDOrderFlagNotSupported:	return "不合法的YDOrderFlag";
		case YD_ERROR_NotOptionInstrument:	return "非期权合约";
		case YD_ERROR_PriceOutOfLimit:	return "价格超过涨跌停板";
		case YD_ERROR_CrossPriceInQuote:	return "报价中价格交叉";
		case YD_ERROR_QuoteFieldError:	return "报价字段错误";
		case YD_ERROR_QuoteVolumeError:	return "报价数量错误";
		case YD_ERROR_QuoteNotFound:	return "报价没有找到";
		case YD_ERROR_CancelQuoteFieldError:	return "取消报价字段错误";
		case YD_ERROR_QuoteNotBelongToAccount:	return "报价不属于该账户";
		case YD_ERROR_QuoteFinished:	return "报价已完成";
		case YD_ERROR_QuoteNotSupported:	return "不支持的报价";
		case YD_ERROR_CannotCancelQuoteDerivedOrder:	return "报价衍生的委托不可撤销";
		case YD_ERROR_TooManyLogines:	return "登录连接超过最大限制";
		case YD_ERROR_NoEnoughPositiontoMakeCombPosition:	return "持仓不足无法组合持仓";
		case YD_ERROR_NoEnoughCombPosition:	return "组合持仓数量不足";
		case YD_ERROR_NoMoneyForSplitCombPosition:	return "资金不足无法拆分组合持仓";
		case YD_ERROR_InvalidCombPosition:	return "非法的组合持仓";
		case YD_ERROR_CannotSelectConnection:	return "无法发送席位优选结果";
		case YD_ERROR_SelectConnectionTooFrequently:	return "席位优选上报过于频繁";
		case YD_ERROR_InvalidSelectConnection:	return "非法的席位优选结果";
		case YD_ERROR_TooLowApiVersion:	return "API版本低于柜台最低要求";
		case YD_ERROR_InvalidTradingRight:	return "非法的交易权限";
		case YD_ERROR_InvalidProduct:	return "非法的产品";
		case YD_ERROR_InvalidAlterMoneyField:	return "非法的出入金字段值";
		case YD_ERROR_TooHighApiVersion:	return "Api版本高于ydServer";
		case YD_ERROR_MoneyUsageTooLow:	return "资金使用比例过低";
		case YD_ERROR_InvalidInstrumentPairToExecuteTogether:	return "合并执行合约对错误";
		case YD_ERROR_NotOnExpireDay:	return "不是到期日";
		case YD_ERROR_NotProperTime:	return "不是适当的业务时间";
		case YD_ERROR_OptonLongPositionCostLimitExceed:	return "超出买入额度";
		case YD_ERROR_PriceToTriggerFuse:	return "价格异常";
		case YD_ERROR_ExchangeDoesNotSupport:	return "交易所不支持";
		case YD_ERROR_InvalidOrderRef:	return "OrderRef不正确";
		case YD_ERROR_CannotSend:	return "无法发送报单到柜台";
		case YD_ERROR_TooManyInMultiOrders:	return "批量报单报单数超过16个";
		case YD_ERROR_CanNotSendToExchangeForFlowControl:	return "没有可用席位，且有席位被流控";
		case YD_ERROR_ExchangeConnectionSendError:	return "交易所API发送错误";
		case YD_ERROR_InternalRejected:	return "内部拒单";
		case YD_ERROR_TooManyCancels:	return "撤单太多";
		case 1026:	return "当前状态禁止此项操作";
		case 1028:	return "报单已成交";
		case 1029:	return "报单已撤销";
		case 1031:	return "可平仓位不足";
		case 1036:	return "资金不足";
		case 1037:	return "不合法的数量";
		case 1097:	return "重复的操作";
		default:	return "其他错误";
		}
	}
	inline static ExchangeStatus ConvertExchangeStatusFromYD(int segmentTime) {
		switch (segmentTime) {
		case 62100:	// 10:15
		case 66600:	// 11:30
			return ExchangeStatus::NO_TRADING;
		case 63000:	// 10:30
		case 72000:	// 13:00
		case 73800:	// 13:30
			return ExchangeStatus::CONTINOUS;
		default:
			return ExchangeStatus::UNKNOW;
		}
	}
}
HFTTraderHandler::HFTTraderHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher)
	:m_config(config), m_instHelper(instHelper), m_publisher(publisher), m_isForbidden(false), m_hasCaughtup(false) {
}

void HFTTraderHandler::notifyEvent(int apiEvent) {
	switch (apiEvent) {
	case YD_AE_TCPTradeConnected: {
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::HFT;
		notification.Type = ExchangeNotification::TRADER_CONNECTED;
		m_publisher.NotifyUI(notification);
		Log::Info("HFTTraderHandler: FrontConnected");
	}
		break;
	case YD_AE_TCPTradeDisconnected: {
		Log::Info("HFTTraderHandler: FrontDisConnected");
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::HFT;
		notification.Type = ExchangeNotification::TRADER_DISCONNECTED;
		m_publisher.NotifyUI(notification);
	}
		break;
	case YD_AE_TCPMarketDataConnected:
	case YD_AE_TCPMarketDataDisconnected:
		break;
	case YD_AE_ServerRestarted:
		Log::Info("HFTTraderHandler: ServerRestarted");
		break;
	case YD_AE_ServerSwitched:
		Log::Info("HFTTraderHandler: ServerSwitched");
		break;
	default:
		Log::Error(fmt::format("HFTTraderHandler: Received Unknown Event[{}]", apiEvent));
	}
}
void HFTTraderHandler::notifyReadyForLogin(bool hasLoginFailed) {
	// 当API准备登录时，发出此消息，用户需要在此时发出登录指令
	// 如果发生了断线重连，也会发出此消息，让用户重新发出登录指令，但是此时不允许换成另外的用户登录
	m_hasCaughtup = false;

	char const* userName = m_config.Credential.UserID.c_str();
	char const* password = m_config.Credential.Password.c_str();
	char const* appID = m_config.Credential.AppID.c_str();
	char const* authCode = m_config.Credential.TradeAuthCode.c_str();
	if (hasLoginFailed && m_isForbidden) {	//自动登录失败后禁止重复登录
		auto msg = fmt::format("HFTTraderHandler: last login is failed[{}-{}|{}-{}], forbidden login again!", userName, password, appID, authCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		m_isForbidden = true;
	} else {
		if (!m_pTraderApi->login(userName, password, appID, authCode)) {
			auto msg = "YDApi: send login() Failed";
			m_publisher.NotifyUI(msg);
			Log::Error(msg);
		}
	}
// 	Log::Info(fmt::format("HFTTraderHandler: Login request, User[{}] App[{}], AuthCode[{}]", userName, appID, authCode));
}
void HFTTraderHandler::notifyLogin(int errorNo, int maxOrderRef, bool isMonitor) {
	// 每次登录响应，都会获得此消息，用户应当根据errorNo来判断是否登录成功
	char const* password = m_config.Credential.Password.c_str();
	char const* appID = m_config.Credential.AppID.c_str();
	char const* authCode = m_config.Credential.TradeAuthCode.c_str();
 	char const* userName = m_config.Credential.UserID.c_str();
	if (YD_ERROR_NoError != errorNo) {
		auto msg = fmt::format("HFTTraderHandler: UserLogin[{},{},{},{}] Failed, ErrCode[{}] ErrMsg[{}]", userName, password, appID, authCode, errorNo, GetErrDesc(errorNo));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
	else {
		 LocalIdACC::Init(maxOrderRef);

		 ExchangeNotification notification;
		 notification.Platform = ConnPlatform::HFT;
		 notification.Type = ExchangeNotification::TRADER_LOGINED;
		 m_publisher.NotifyUI(notification);

		 auto msg = fmt::format("HFTTraderHandler: UserLogin[{}] Success, [{}, {}] MaxOrderRef[{}]", userName, appID, authCode, maxOrderRef);
		 m_publisher.NotifyUI(msg);
		 Log::Info(msg);
	}
}
void HFTTraderHandler::notifyFinishInit(void) {
// notifyFinishInit是在第一次登录成功后一小段时间发出的消息，表示API已经收到了今天的所有初始化信息，
// 包括所有的产品合约信息，昨行情和今日的固定行情（例如涨跌停板）信息，账号的日初信息，保证金率信息，
// 手续费率信息，昨持仓信息，但是还没有获得登录前已经发生的报单和成交信息，日内的出入金信息
// 这个时候，用户程序已经可以安全地访问所有API管理的数据结构了
// 用户程序获得所有YDSystemParam，YDExchange，YDProduct，YDInstrument，YDCombPositionDef，YDAccount，
// YDPrePosition，YDMarginRate，YDCommissionRate，YDAccountExchangeInfo，YDAccountProductInfo，
// YDAccountInstrumentInfo和YDMarketData的指针，都可以在未来长期安全使用，API不会改变其地址
// 但是API消息中的YDOrder、YDTrade、YDInputOrder、YDQuote、YDInputQuote和YDCombPosition的地址都是
// 临时的，在该消息处理完成后将不再有效

	char const* userName = m_config.Credential.UserID.c_str();
// 	const YDAccount* pAccount = m_pTraderApi->getAccountByID(userName);
	const YDAccount* pAccount = m_pTraderApi->getMyAccount();	// 新版本益达查账户资金
	if (nullptr != pAccount) {
		FundsNotification notification;
		notification.Platform = ConnPlatform::HFT;
		notification.Capital = pAccount->PreBalance;
		notification.Available = pAccount->PreBalance * pAccount->MaxMoneyUsage;
		m_publisher.PushResponse(notification);

		m_publisher.PushResponse(CleanPositionNotification());
		Log::Info(fmt::format("HFTTraderHandler: TradingAccount[{}], Capital[{:.2f}] Available[{:.2f}] Useage[{:.2f}]"
			,pAccount->AccountID, pAccount->PreBalance, notification.Available, pAccount->UserFloat));
	}
	
	for (int cnt = 0; cnt < m_pTraderApi->getPrePositionCount(); ++cnt) {
		PositionNotification notification;

		const YDPrePosition* pPrePosition = m_pTraderApi->getPrePosition(cnt);
		notification.InstID = m_instHelper.GetInstID(pPrePosition->m_pInstrument->InstrumentID);
		if (notification.InstID == INVALID_INSTRUMENT_ID) {
			Log::Warn(fmt::format("HFTTraderHandler: QueryPosition filter not support Instrument[{}]", pPrePosition->m_pInstrument->InstrumentID));
			continue;
		}
		FillingPositionNotification(*pPrePosition, notification);
		m_publisher.PushResponse(notification);
		m_publisher.NotifyUI(notification);

		char const* direction = (Direction::BUY == notification.Direction) ? "B" : "S";
		Log::Info(fmt::format("HFTTraderHandler: OnRspQryInvestorPosition, UserID[{}] Instr[{}] Dir[{}] Total[{}] AvgPrice[{:.3f}]",pAccount->AccountID, 
			pPrePosition->m_pInstrument->InstrumentID, direction, notification.Quantity, notification.Price.value()));
	}
}
void HFTTraderHandler::notifyCaughtUp(void) {
	m_hasCaughtup = true;

	// 如果资金收不到，启用这段代码再试试
// 	char const* userName = m_config.Credential.UserID.c_str();
// 	const YDAccount* pAccount = m_pTraderApi->getMyAccount();	// 新版本益达查账户资金
// 	if (nullptr != pAccount) {
// 		FundsNotification notification;
// 		notification.Platform = ConnPlatform::HFT;
// 		notification.Capital = pAccount->PreBalance;
// 		notification.Available = pAccount->PreBalance * pAccount->MaxMoneyUsage;
// 		m_publisher.PushResponse(notification);
// 
// 		m_publisher.PushResponse(CleanPositionNotification());
// 		Log::Info(fmt::format("HFTTraderHandler: TradingAccount[{}], Capital[{:.2f}] Available[{:.2f}] Useage[{:.2f}]"
// 			, pAccount->AccountID, pAccount->PreBalance, notification.Available, pAccount->UserFloat));
// 	}

	// 当日数据流水补齐后才能设置资金开始交易
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HFT;
	notification.Type = ExchangeNotification::CODE_LOGIN;
	m_publisher.NotifyUI(notification);
}

void HFTTraderHandler::notifyTradingSegment(const YDExchange* pExchange, int segmentTime) {
	Log::Info(fmt::format("HFTTraderHandler: Exchange[{}] stamp[{}]", pExchange->ExchangeID, segmentTime));

	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HFT;
	notification.Type = ExchangeNotification::EXCHANGE_STATUS;
	notification.Status = ConvertExchangeStatusFromYD(segmentTime);
	if (pExchange->ExchangeID[0] == 'S') {
		notification.Exchange = Exchange::SHFE;
		m_publisher.PushResponse(notification);
	}
	else if (pExchange->ExchangeID[0] == 'C') {
		notification.Exchange = Exchange::CFFEX;
		m_publisher.PushResponse(notification);
	}
	notification.Exchange = Exchange::CTP;	// 柜台状态
	m_publisher.NotifyUI(notification);
}

// YD系统校验失败
void HFTTraderHandler::notifyFailedOrder(const YDOrder* pFailedOrder, const YDInstrument* pInstrument, const YDAccount* pAccount) {
		OrderNotification notification;
		notification.InstID = m_instHelper.GetInstID(pInstrument->InstrumentID);
		notification.Direction = ConvertDirectionFromYD(pFailedOrder->Direction);
		notification.Operation = ConvertOperationFromYD(pFailedOrder->OffsetFlag);
		notification.LocalId = pFailedOrder->OrderRef;
		notification.OrderPrice = pFailedOrder->Price;
		notification.OrderQuantity = pFailedOrder->OrderVolume;
		notification.Status = OrderNotification::E_SENT_FAILED;
		notification.IsQueryOrder = m_hasCaughtup ? false : true;	//报单流水号没追上前的都是查询
		m_publisher.PushResponse(notification);
		if (m_hasCaughtup) {
			auto msg = fmt::format("HFT OrderInsert Failed: ErrCode[{}] ErrMsg[{}] Inst[{}] [{} {}：{:.3f},{}] LocalID[{}]", pFailedOrder->ErrorNo, GetErrDesc(pFailedOrder->ErrorNo),
				pInstrument->InstrumentID, Desc(notification.Direction), Desc(notification.Operation), pFailedOrder->Price, pFailedOrder->OrderVolume, notification.LocalId);
			m_publisher.NotifyUI(msg);
			Log::Error(msg);
		}
}
// YD系统撤单失败
void HFTTraderHandler::notifyFailedCancelOrder(const YDFailedCancelOrder* pFailedCancelOrder, const YDExchange* pExchange, const YDAccount* pAccount) {
	//易达系统错误撤单回报中没有LocalID，无法进行Dealing匹配，只能记录日志观察
	if (pFailedCancelOrder->ErrorNo != YD_ERROR_OrderFinished/*报单已全成交或已撤销*/&& pFailedCancelOrder->ErrorNo != 1028 && m_hasCaughtup) {
		auto msg = fmt::format("HFT OrderAction Failed: ErrCode[{}] ErrMsg[{}] SystemID[{}]", pFailedCancelOrder->ErrorNo, GetErrDesc(pFailedCancelOrder->ErrorNo), pFailedCancelOrder->OrderSysID);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}

// 交易所所有订单返回信息（应答、错误、回报）
void HFTTraderHandler::notifyOrder(const YDOrder* pOrder, const YDInstrument* pInstrument, const YDAccount* pAccount) {
	OrderNotification notification;
	notification.InstID = m_instHelper.GetInstID(pInstrument->InstrumentID);
	if (notification.InstID == INVALID_INSTRUMENT_ID) {
		Log::Info(fmt::format("HFTTraderHandler: OnRtnOrder filter not support Instrument[{}], SystemID[{}] OrderRef[{}]",
			pInstrument->InstrumentID, pOrder->OrderSysID, pOrder->OrderRef));
		return;
	}
	notification.Exchange = InstIdExchange(notification.InstID);
	notification.Direction = ConvertDirectionFromYD(pOrder->Direction);
	notification.Operation = ConvertOperationFromYD(pOrder->OffsetFlag);
	notification.LocalId = pOrder->OrderRef;
	notification.OrderPrice = pOrder->Price;
	notification.DealQuantity = pOrder->TradeVolume;			// 这里的成交量并非增量而是总量！
	notification.OrderQuantity = pOrder->OrderVolume;
	notification.IsQueryOrder = m_hasCaughtup ? false : true;	// 报单流水号没追上前的都是查询
	if (YD_OS_Rejected == pOrder->OrderStatus || YD_ERROR_NoError != pOrder->ErrorNo) {
		notification.Status = OrderNotification::E_SENT_FAILED;
		m_publisher.PushResponse(notification);
		if (m_hasCaughtup) {
			auto msg = fmt::format("HFT OrderInsert Failed: ErrCode[{}] ErrMsg[{}] Inst[{}] [{} {}：{:.3f},{}] LocalID[{}]", pOrder->ErrorNo, GetErrDesc(pOrder->ErrorNo),
				pInstrument->InstrumentID, Desc(notification.Direction), Desc(notification.Operation), pOrder->Price, pOrder->OrderVolume, pOrder->OrderRef);
			if (1026 != pOrder->ErrorNo && YD_ERROR_NoMoneyToOpen != pOrder->ErrorNo) {
				m_publisher.NotifyUI(msg);		// 废单报错不通知界面
			}
			Log::Error(msg);
		}
		return;
	}
	else {
		sprintf_s(notification.SystemNo, "%d", pOrder->OrderSysID);
	}

	// Test 日志
// 	Log::Info(fmt::format("HFTTraderHandler OnRtnOrder: LocalID[{}] Inst[{}] [{} {}：{:.3f}] Info[O:{} D:{}] SystemID[{}] OrderStatus[{}]",
// 		pOrder->OrderRef, pInstrument->InstrumentID, Desc(notification.Direction), Desc(notification.Operation), pOrder->Price,
// 		pOrder->OrderVolume, pOrder->TradeVolume, notification.SystemNo, GetOrderStatus(pOrder->OrderStatus)));

	// YD_OS_AllTraded 和 YD_OS_Canceled是互斥的
	std::string notifyStatus = "E_ACCEPT";
	if (YD_OS_AllTraded == pOrder->OrderStatus) {	// 成交
		notifyStatus = "E_QUANTITY_TRADE";
		notification.Status = OrderNotification::E_QUANTITY_TRADE;
		m_publisher.PushResponse(notification);
		return;
	}

	if (YD_OS_Canceled == pOrder->OrderStatus) {	// 撤单
		if (0 == pOrder->OrderSysID) {
			notifyStatus = "E_SENT_FAILED";
			notification.Status = OrderNotification::E_SENT_FAILED;
			auto msg = fmt::format("HFTTraderHandler: LocalID[{}] Cancelled by System! Inst[{}]", pOrder->OrderRef, pInstrument->InstrumentID);
			m_publisher.NotifyUI(msg);
			Log::Error(msg);
			return;
		}
		notifyStatus = "E_CANCELED";
		notification.Status = OrderNotification::E_CANCELED;
		if (YD_ODT_FAK == pOrder->OrderType && notification.DealQuantity > 0) {
			notification.DealPrice = notification.OrderPrice;
			notifyStatus = "E_FAK_PARTIAL_TRADE";
			notification.Status = OrderNotification::E_FAK_PARTIAL_TRADE;
		}	// FAK部成部撤通知在撤单里给
		m_publisher.PushResponse(notification);

		Log::Info(fmt::format("HFTTraderHandler OnRtnOrder: LocalID[{}] Inst[{}] [{} {}：{:.3f}] Info[O:{} D:{}] SystemID[{}] OrderStatus[{}]",
			pOrder->OrderRef, pInstrument->InstrumentID, Desc(notification.Direction), Desc(notification.Operation), pOrder->Price,
			pOrder->OrderVolume, pOrder->TradeVolume, pOrder->OrderSysID, GetOrderStatus(pOrder->OrderStatus)));
		return;
	}

	if (YD_OS_Queuing == pOrder->OrderStatus) {		// 挂单
		notifyStatus = "E_ORDERED";
		notification.Status = OrderNotification::E_ORDERED;
		if (pOrder->TradeVolume > 0) {
			notifyStatus = "E_QUANTITY_TRADE";
			notification.Status = OrderNotification::E_QUANTITY_TRADE;
		}
		m_publisher.PushResponse(notification);
		return;
	}
}
void HFTTraderHandler::notifyTrade(const YDTrade* pTrade, const YDInstrument* pInstrument, const YDAccount* pAccount) {
	OrderNotification notification;
	notification.InstID = m_instHelper.GetInstID(pInstrument->InstrumentID);
	if (notification.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("HFTTraderHandler: OnRtnTrade filter not support Instrument[{}], SystemID[{}] LocalID[{}] YDLocalID[{}]",
			pInstrument->InstrumentID, notification.SystemNo, pTrade->OrderRef, pTrade->OrderLocalID));
		return;
	}
	notification.Exchange = InstIdExchange(notification.InstID);
	notification.Direction = ConvertDirectionFromYD(pTrade->Direction);
	notification.Operation = ConvertOperationFromYD(pTrade->OffsetFlag);
	notification.DealQuantity = pTrade->Volume;		// OnRtnTrade里的成交量是增量
	notification.DealPrice = pTrade->Price;
	notification.LocalId = pTrade->OrderRef;
	notification.IsQueryTrade = m_hasCaughtup ? false : true;
	sprintf_s(notification.SystemNo, "%d", pTrade->OrderSysID);
	notification.Status = OrderNotification::E_TRADE;
	m_publisher.PushResponse(notification);

	char const* direction = (Direction::SELL == notification.Direction) ? "S" : "B";
	Log::Info(fmt::format("HFTTraderHandler OnRtnTrade: Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Qty[{}] SystemID[{}] LocalID[{}]",
		pInstrument->InstrumentID, direction, Desc(notification.Operation), pTrade->Price, pTrade->Volume, pTrade->OrderSysID, pTrade->OrderRef));
}

void HFTTraderHandler::notifyAccount(const YDAccount* pAccount) {
	Log::Info(fmt::format("HFTTraderHandler: notifyAccount, Account[{}] Balance[{:.2f}] usage[{:.2f}]", 
		pAccount->AccountID, pAccount->PreBalance, pAccount->MaxMoneyUsage));
}
void HFTTraderHandler::notifyMarketData(const YDMarketData* pMarketData) {
	InstID instID = m_instHelper.GetInstID(pMarketData->m_pInstrument->InstrumentID);
	if (m_subscribeList[instID] == false) {
		Quote quote(CreateQuote(*pMarketData, instID));
		if (quote.GetLastPrice() > 0) {
			m_publisher.PushQuote(quote);
// #ifdef _DEBUG
// 			m_subscribeList[instID] = false;
// #else
// 			m_subscribeList[instID] = true;		//给一次全档行情
// 			Log::Info(fmt::format("HFTTraderHandler: notifyMarketData, Inst[{}] Limit[{:.2f}, {:.2f}] Open[{:.2f}] Last[{:.2f},{}] Time[{}]", pMarketData->m_pInstrument->InstrumentID,
// 				pMarketData->UpperLimitPrice, pMarketData->LowerLimitPrice, pMarketData->PreClosePrice, pMarketData->LastPrice, pMarketData->Volume, pMarketData->TimeStamp));
// #endif
		}
	}
}

void HFTTraderHandler::notifyResponse(int errorNo, int requestType) {
	auto msg = fmt::format("HFTTraderHandler: notifyResponse, ErrNo[{}] ErrMsg[{}] RequestType[{}]", errorNo, GetErrDesc(errorNo), requestType);
	m_publisher.NotifyUI(msg);
	Log::Error(msg);
}

void HFTTraderHandler::SubscribeMarket(InstID instID) {
	m_subscribeList[instID] = false;
	InstCodeT instCode = m_instHelper.GetInstrument(instID).GetInstCode();
	const YDInstrument* pInstrument = m_pTraderApi->getInstrumentByID(instCode.Str());
	if (m_pTraderApi->subscribe(pInstrument)) {
// 		Log::Info(fmt::format("HFTTraderHandler: SubscribeQuote[{}] ", pInstrument->InstrumentID));
	}
}

void HFTTraderHandler::FillingPositionNotification(YDPrePosition const& position, PositionNotification& notification) {
	notification.Quantity = position.PrePosition;
	notification.TodayQty = 0;
	notification.Price = position.PreSettlementPrice;

	if (YD_HF_Arbitrage == position.HedgeFlag) {
		notification.HedgeType = HedgeType::ARBITRAGE;
	}
	else{
		notification.HedgeType = HedgeType::SPECULATE;
	}

	if (YD_PD_Long == position.PositionDirection) {
		notification.Direction = Direction::BUY;
	}
	else if (YD_PD_Short == position.PositionDirection) {
		notification.Direction = Direction::SELL;
	}
}

Quote HFTTraderHandler::CreateQuote(YDMarketData const& pDepthMarketData, InstID instID) {
	SimpleQuoteArray buyPriceArray;
	buyPriceArray[0] = SimpleQuote(pDepthMarketData.BidPrice, pDepthMarketData.BidVolume);
	SimpleQuoteArray sellPriceArray;
	sellPriceArray[0] = SimpleQuote(pDepthMarketData.AskPrice, pDepthMarketData.AskVolume);

	double openPrice = pDepthMarketData.PreClosePrice;		//用昨收盘代替开盘
	double highestPrice = pDepthMarketData.UpperLimitPrice;	//涨停代替最高
	double lowestPrice = pDepthMarketData.LowerLimitPrice;	//跌停代替最低
	double upperLimit = pDepthMarketData.UpperLimitPrice;
	double lowerLimit = pDepthMarketData.LowerLimitPrice;
	int openInterest = int(pDepthMarketData.OpenInterest);
	double lastPrice = pDepthMarketData.LastPrice;
	double lastSettlement = pDepthMarketData.PreSettlementPrice;
	double turnover = pDepthMarketData.Turnover;

	Quote quote(instID, &m_instHelper.GetInstrument(instID), buyPriceArray, sellPriceArray, openPrice,
		highestPrice, lowestPrice, upperLimit, lowerLimit, openInterest, lastPrice, lastSettlement,
		pDepthMarketData.Volume, turnover, true, 0, SimpleQuote(), SimpleQuote());
	return quote;
}
