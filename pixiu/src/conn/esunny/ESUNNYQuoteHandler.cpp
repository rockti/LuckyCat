#include "pch.h"
#include "conn/esunny/ESUNNYQuoteHandler.h"
#include "model/Constants.h"
#include "notify/Notify.h"
#include "utils/Log.h"
#include "notify/ConnPublisher.h"
#include "config/Config.h"
#include "utils/TimeChecker.h"
#include "utils/LicenseChecker.h"

#include <ctime>

ESUNNYQuoteHandler::ESUNNYQuoteHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher)
	:m_config(config), m_instHelper(instHelper), m_publisher(publisher), m_pQuoteApi(NULL), m_sessionID(0), m_isApiReady(false) {
}

void ESUNNYQuoteHandler::OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo* info) {
	if (TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ESUNNYQuoteHandler: UserLogin success, waiting for API initialize...");
	} else {
		auto msg = fmt::format("ESUNNYQuoteHandler: UserLogin Failed, {}", GetErrorDescription(errorCode));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void ESUNNYQuoteHandler::OnAPIReady() {
	m_isApiReady = true;

	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HKEX_ESUNNY;
	notification.Type = ExchangeNotification::QUOTE_CONNECTED;
	m_publisher.NotifyUI(notification);
	Log::Info("ESUNNYQuoteHandler: API is ready");

	TAPIINT32 nRtn = m_pQuoteApi->QryCommodity(&m_sessionID);
	if (TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ESUNNYQuoteHandler: QryCommodity failed, rtnCode[{}]", nRtn);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
	m_publisher.PushResponse(notification);
}
void ESUNNYQuoteHandler::OnDisconnect(TAPIINT32 reasonCode) {
	m_isApiReady = false;
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HKEX_ESUNNY;
	notification.Type = ExchangeNotification::QUOTE_DISCONNECTED;
	m_publisher.NotifyUI(notification);

	auto msg = fmt::format("ESUNNYQuoteHandler: FrontDisconnected, {}", GetErrorDescription(reasonCode));
	m_publisher.NotifyUI(msg);
	Log::Warn(msg);	

	m_publisher.PushResponse(notification);
}
void ESUNNYQuoteHandler::OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo* info) {
	if (TAPIERROR_SUCCEED==errorCode && NULL!=info) {
		//暂时只对GDR和GDU两个Commodity进行处理
		if (0==strcmp("GDR",info->Commodity.CommodityNo) || 0==strcmp("GDU",info->Commodity.CommodityNo)) {
			TapAPICommodity commodity = { 0 };
			memcpy(&commodity, &info->Commodity, sizeof(TapAPICommodity));
			TAPIINT32 nRtn = m_pQuoteApi->QryContract(&m_sessionID, &commodity);
			if (TAPIERROR_SUCCEED != nRtn) {
				auto msg = fmt::format("ESUNNYQuoteHandler: QryContract failed, {}", GetErrorDescription(nRtn));
				m_publisher.NotifyUI(msg);
				Log::Error(msg);
			}
		}
	} else {
		Log::Error(fmt::format("ESUNNYQuoteHandler: OnRspQryCommodity failed, {}", GetErrorDescription(errorCode)));
	}
}
void ESUNNYQuoteHandler::OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo* info)
{
	if (TAPIERROR_SUCCEED==errorCode && NULL!=info) {
		std::string exchange(info->Contract.Commodity.ExchangeNo);
		if (0==strcmp("GDR",info->Contract.Commodity.CommodityNo) || 0==strcmp("GDU",info->Contract.Commodity.CommodityNo)) {
			Log::Info(fmt::format("ESUNNYQuoteHandler: OnRspQryContract received Instrument[{}{}]", info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1));
		}
	} else {
		Log::Error(fmt::format("ESUNNYQuoteHandler: OnRspQryContract failed, {}", GetErrorDescription(errorCode)));
	}
}

void ESUNNYQuoteHandler::OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole* info) {
	if (TAPIERROR_SUCCEED==errorCode && NULL!=info) {
		auto msg = fmt::format("ESUNNYQuoteHandler: SubscribeQuote success, Instrument[{} {}]", info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1);
		Log::Info(msg.c_str());
		m_publisher.NotifyUI(msg);

// 		if (!CheckTime(info->DateTimeStamp)) {
// 			Log::Error(fmt::format("current time has more then 1 hour difference with server time, exit"));
// 			exit(0);
// 		}

// 		if (!LicenseChecker::Validate("pi.xiu")) {
// 			Log::Error("EQH: invalid software");
// 			exit(0);
// 		}
	} else {
		Log::Error(fmt::format("ESUNNYQuoteHandler: SubscribeQuote failed, errorMsg{}", GetErrorDescription(errorCode)));
	}
}
void ESUNNYQuoteHandler::OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract* info) {
	if (TAPIERROR_SUCCEED==errorCode && NULL!=info) {
		auto msg = fmt::format("ESUNNYQuoteHandler: UnSubscribeQuote success, Instrument[{}{}]", info->Commodity.CommodityNo, info->ContractNo1);
		Log::Info(msg.c_str());
		m_publisher.NotifyUI(msg);
	} else {
		Log::Warn(fmt::format("ESUNNYQuoteHandler: UnSubscribeQuote failed, errorMsg[{}]", GetErrorDescription(errorCode)));
	}
}
void ESUNNYQuoteHandler::OnRtnQuote(const TapAPIQuoteWhole* info) {
	if (NULL!= info && !IsInvalidQuote(info)) {	
		std::string instCode = info->Contract.Commodity.CommodityNo;
		instCode += info->Contract.ContractNo1;
		InstID instID = m_instHelper.GetInstID(instCode.c_str());
		if (INVALID_INSTRUMENT_ID == instID) {
			Log::Error(fmt::format("ESUNNYQuoteHandler: OnRtnQuote can not find instID by instCode[{}]", instCode));
			return;
		}

		Quote quote(CreateQuote(instID, info));
		m_publisher.PushQuote(quote);
	}
}
void ESUNNYQuoteHandler::SubscribeQuote(char const* instrumentCode) {
	auto iterator = find(m_subscribeList.begin(), m_subscribeList.end(), instrumentCode);
	if (iterator == m_subscribeList.end())
	{
		m_subscribeList.push_back(instrumentCode);	//加入本地订阅列表
		DoSubscribe(instrumentCode);				//订阅合约
		Log::Info(fmt::format("ESUNNYQuoteHandler: DoSubscribe[{}] market", instrumentCode));
	}
}
void ESUNNYQuoteHandler::UnsubscribeQuote(char const* instrumentCode) {
	auto iterator = find(m_subscribeList.begin(), m_subscribeList.end(), instrumentCode);
	if (iterator != m_subscribeList.end())
	{
		DoUnsubscribe(instrumentCode);				//退订合约
		m_subscribeList.erase(iterator);			//从本地订阅列表中删除
		Log::Info(fmt::format("ESUNNYQuoteHandler: DoUnsubscribe[{}] market", instrumentCode));
	}
}
void ESUNNYQuoteHandler::DoSubscribe(char const* instCode) {
	TapAPIContract contractInfo = { 0 };
	Instrument const* inst = m_instHelper.FindInstrument(instCode);
	if (inst == nullptr) {
		auto msg = fmt::format("can not subscribe not supported instrument[{}]", instCode);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		return;
	} else {
		FillingContractInfo(*inst, contractInfo);
		TAPIINT32 nRtn = m_pQuoteApi->SubscribeQuote(&m_sessionID, &contractInfo);
		if (TAPIERROR_SUCCEED != nRtn) {
			auto msg = fmt::format("ESUNNYQuoteHandler: SubscribeQuote failed, {}", GetErrorDescription(nRtn));
			Log::Error(msg);
			m_publisher.NotifyUI(msg);
		}
	}
}
void ESUNNYQuoteHandler::DoUnsubscribe(char const* instCode) {
	TapAPIContract contractInfo = { 0 };
	Instrument const* inst = m_instHelper.FindInstrument(instCode);
	if (inst == nullptr) return;

	FillingContractInfo(*inst, contractInfo);
	TAPIINT32 nRtn = m_pQuoteApi->UnSubscribeQuote(&m_sessionID, &contractInfo);
	if (TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ESUNNYQuoteHandler: UnsubscribeQuote failed, {}", GetErrorDescription(nRtn));
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
}

void ESUNNYQuoteHandler::FillingContractInfo(Instrument const& inst, TapAPIContract& contractInfo) {
	//这里其实要做转换HKEX，CME等品种对应的交易所，先写死港交所的期货
	strcpy_s(contractInfo.Commodity.ExchangeNo, "HKEX");
	if (inst.GetVariety().GetCode() == "GC") {
		strcpy_s(contractInfo.Commodity.ExchangeNo, "COMEX");
	}
	contractInfo.Commodity.CommodityType = TAPI_COMMODITY_TYPE_FUTURES;		//期货
	strcpy_s(contractInfo.Commodity.CommodityNo, inst.GetVariety().GetCode().c_str());	//品种
	strcpy_s(contractInfo.ContractNo1, inst.GetContractCode());		//合约月份						
	contractInfo.CallOrPutFlag1 = TAPI_CALLPUT_FLAG_NONE;
	contractInfo.CallOrPutFlag2 = TAPI_CALLPUT_FLAG_NONE;
}
bool ESUNNYQuoteHandler::IsInvalidQuote(const TapAPIQuoteWhole* pDepthMarketData) {
	return false;
}

template<class PA, class VA>
inline void fillArray(PA const& price, VA const& volume, SimpleQuoteArray& quoteArray) {
	quoteArray[0] = SimpleQuote(price[0], int(volume[0]));
	quoteArray[1] = SimpleQuote(price[1], int(volume[1]));
	quoteArray[2] = SimpleQuote(price[2], int(volume[2]));
	quoteArray[3] = SimpleQuote(price[3], int(volume[3]));
	quoteArray[4] = SimpleQuote(price[4], int(volume[4]));
}

Quote ESUNNYQuoteHandler::CreateQuote(InstID instID, const TapAPIQuoteWhole* pDepthMarketData) {
	SimpleQuoteArray buyPriceArray;
	fillArray(pDepthMarketData->QBidPrice, pDepthMarketData->QBidQty, buyPriceArray);
	SimpleQuoteArray sellPriceArray;
	fillArray(pDepthMarketData->QAskPrice, pDepthMarketData->QAskQty, sellPriceArray);

	money openPrice = pDepthMarketData->QOpeningPrice;
	money highestPrice = pDepthMarketData->QHighPrice;
	money lowestPrice = pDepthMarketData->QLowPrice;
	money upperLimit = pDepthMarketData->QLimitUpPrice;
	money lowerLimit = pDepthMarketData->QLimitDownPrice;
	int openInterest = int(pDepthMarketData->QPositionQty);
	money turnover = pDepthMarketData->QTotalTurnover;
	money lastPrice = pDepthMarketData->QLastPrice;
	money lastSettlement = pDepthMarketData->QSettlePrice;
	SimpleQuote hidenBuy(pDepthMarketData->QImpliedBidPrice, uint32_t(pDepthMarketData->QImpliedBidQty));
	SimpleQuote hidenSell(pDepthMarketData->QImpliedAskPrice, uint32_t(pDepthMarketData->QImpliedAskQty));

	Quote market(instID, &m_instHelper.GetInstrument(instID), buyPriceArray, sellPriceArray, openPrice, highestPrice, lowestPrice,
		upperLimit, lowerLimit, openInterest, lastPrice, lastSettlement, int(pDepthMarketData->QLastQty), turnover, false, 0, hidenBuy, hidenSell);
	return market;
}

std::string ESUNNYQuoteHandler::GetErrorDescription(TAPIINT32 errorCode) {
	switch (errorCode) {
		/***************************************TapAPI内部错误***************************************/
	case TAPIERROR_ConnectFail:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "连接服务失败");
	case TAPIERROR_LinkAuthFail:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "链路认证失败");
	case TAPIERROR_HostUnavailable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "主机地址不可用");
	case TAPIERROR_SendDataError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "发送数据错误");
	case TAPIERROR_TestIDError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "测试编号不合法");
	case TAPIERROR_NotReadyTestNetwork:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没准备好测试网络");
	case TAPIERROR_CurTestNotOver:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "当前网络测试还没结束");
	case TAPIERROR_NOFrontAvailable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没用可用的接入前置");
	case TAPIERROR_DataPathAvaiable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "数据路径不可用");
	case TAPIERROR_RepeatLogin:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "重复登录");
	case TAPIERROR_InnerError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "内部错误");
	case TAPIERROR_LastReqNotFinish:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "上一次请求还没有结束");
	case TAPIERROR_InputValueError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "输入参数非法");
	case TAPIERROR_AuthCode_Invalid:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码不合法");
	case TAPIERROR_AuthCode_Expired:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码超期");
	case TAPIERROR_AuthCode_TypeNotMatch:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码类型不匹配");
	case TAPIERROR_API_NotReady:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "API还没有准备好");
	case TAPIERROR_UDP_LISTEN_FAILED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "UDP端口监听失败");
	case TAPIERROR_UDP_LISTENING:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "UDP正在监听");
	case TAPIERROR_NotImplemented:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "接口未实现");
	case TAPIERROR_CallOneTimeOnly:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "每次登陆只允许调用一次");
	case TAPIERROR_Frequently:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "查询频率太高");
		/*************************************网络断开错误*************************************/
	case TAPIERROR_DISCONNECT_CLOSE_INIT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "主动断开");
	case TAPIERROR_DISCONNECT_CLOSE_PASS:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "被动断开");
	case TAPIERROR_DISCONNECT_READ_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "读错误");
	case TAPIERROR_DISCONNECT_WRITE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "写错误");
	case TAPIERROR_DISCONNECT_BUF_FULL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "缓冲区满");
	case TAPIERROR_DISCONNECT_IOCP_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "异步操作错误");
	case TAPIERROR_DISCONNECT_PARSE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "解析数据错误");
	case TAPIERROR_DISCONNECT_CONNECT_TIMEOUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "连接超时");
	case TAPIERROR_DISCONNECT_INIT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "初始化失败");
	case TAPIERROR_DISCONNECT_HAS_CONNECTED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "已经连接");
	case TAPIERROR_DISCONNECT_HAS_EXIT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "工作线程已结束");
	case TAPIERROR_DISCONNECT_TRY_LATER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "操作正在进行，请稍后重试");
	case TAPIERROR_DISCONNECT_HEARTBEAT_FAILED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "心跳检测失败");
		/***************************************登陆错误***************************************/
	case TAPIERROR_LOGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录过程执行错误");
	case TAPIERROR_LOGIN_USER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户不存在");
	case TAPIERROR_LOGIN_DDA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要进行动态认证");
	case TAPIERROR_LOGIN_LICENSE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户未授权");
	case TAPIERROR_LOGIN_MODULE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录模块不正确");
	case TAPIERROR_LOGIN_FORCE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要强制修改密码");
	case TAPIERROR_LOGIN_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录状态禁止登陆");
	case TAPIERROR_LOGIN_PASS:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录密码不正确");
	case TAPIERROR_LOGIN_RIGHT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有该模块登录权限");
	case TAPIERROR_LOGIN_COUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录数量超限");
	case TAPIERROR_LOGIN_NOTIN_SERVERFLAGUSRES:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户不在服务器标识下可登录用户列表中");
	case TAPIERROR_LOGIN_USER_EXPIRED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登陆用户不在有效期");
	case TAPIERROR_LOGIN_NO_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登陆用户无下属客户");
	case TAPIERROR_LOGIN_NO_JGT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户是子帐号且授权中未包含机构通标记");
	case TAPIERROR_LOGIN_ERROR_TIMES:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户密码错误次数超限，禁止登陆");
	case TAPIERROR_LOGIN_ERROR_AUTHTYPE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户授权类型错误");
	case TAPIERROR_LOGIN_ERROR_AUTHEXPIRED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户授权到期或到期日错误");
	case TAPIERROR_LOGIN_ERROR_PASSWDEXPIRED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户密码超过有效天数");
		/***************************************行情业务错误***************************************/
	case TAPIERROR_SUBSCRIBEQUOTE_MAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "超过行情最大总订阅数");
	case TAPIERROR_SUBSCRIBEQUOTE_EXCHANGE_MAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "超过该交易所行情最大订阅数");
	case TAPIERROR_SUBSCRIBEQUOTE_NO_RIGHT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有该行情的订阅权限");
	case TAPIERROR_SUBSCRIBEQUOTE_NO_EXCHANGE_RIGHT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有该交易所下行情的订阅权限");
	case TAPIERROR_SUBSCRIBEQUOTE_COMMODITY_NOT_EXIST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "品种不存在");
	case TAPIERROR_SUBSCRIBEQUOTE_CONTRACT_MAY_NOT_EXIST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "合约可能不存在");
	case TAPIERROR_QUOTEFRONT_UNKNOWN_PROTOCOL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不支持的行情协议");
	default:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未知错误");
	}

}