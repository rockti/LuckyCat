#include "pch.h"
#include "conn/ctp/CTPMarketHandler.h"

#pragma warning(disable:4100)

#include "model/Constants.h"
#include "notify/Notify.h"
#include "utils/Log.h"
#include "conn/ExchangeConn.h"
#include "config/Config.h"
#include "notify/ConnPublisher.h"

CTPMarketHandler::CTPMarketHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher)
	:m_config(config),m_instHelper(instHelper),m_publisher(publisher),m_pMdApi(0) {
}

void CTPMarketHandler::OnFrontConnected() {
	Log::Info("CTPMarketHandler FrontConnected");
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::CTP;
	notification.Type = ExchangeNotification::QUOTE_CONNECTED;
	m_publisher.NotifyUI(notification);

	CThostFtdcReqUserLoginField loginReq = {0};
	strcpy_s(loginReq.BrokerID, m_config.Credential.BrokerID.c_str());
	strcpy_s(loginReq.UserID, m_config.Credential.UserID.c_str());
	strcpy_s(loginReq.Password, m_config.Credential.Password.c_str());
 	int rtnCode = m_pMdApi->ReqUserLogin(&loginReq, RequestIdACC::Generate());
	if (0 != rtnCode) {
 		auto msg = fmt::format("CTPMarketHandler ReqUserLogin() Failed, RtnCode[{}]", rtnCode);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
}
void CTPMarketHandler::OnFrontDisconnected(int nReason) {
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::CTP;
	notification.Type = ExchangeNotification::QUOTE_DISCONNECTED;
	m_publisher.NotifyUI(notification);

 	auto msg = fmt::format("CTPMarketHandler FrontDisconnected, Reason[{:x}]", nReason);
	m_publisher.NotifyUI(msg);
	Log::Warn(msg);
}
void CTPMarketHandler::OnHeartBeatWarning(int nTimeLapse) {
	Log::Warn(fmt::format("CTPMarketHandler HeartBeatWarning TimeLapse[{}]s", nTimeLapse));
}

void CTPMarketHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
 		auto msg = fmt::format("CTPMarketHandler OnRspError(), RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
}
void CTPMarketHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pRspUserLogin)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		auto msg = fmt::format("CTPMarketHandler UserLogin Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
	else {
 		Log::Info("CTPMarketHandler UserLogin Success");

		ExchangeNotification notification;
		notification.Platform = ConnPlatform::CTP;
		notification.Type = ExchangeNotification::QUOTE_LOGINED;
		m_publisher.NotifyUI(notification);

		//如果断线重登录，自动订阅原来订阅过的行情
		if (!m_subscribeList.empty()) {
			DoSubscribe();
		}
	}
}
void CTPMarketHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pUserLogout)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		Log::Warn(fmt::format("CTPMarketHandler UserLogout Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	}
	else {
 		Log::Info("CTPMarketHandler UserLogout Success");
	}
}
void CTPMarketHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pSpecificInstrument)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		auto msg = fmt::format("CTPMarketHandler SubMarketData[{}] Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]",
 			pSpecificInstrument->InstrumentID, nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
		
	}
}

void CTPMarketHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	if (NULL == pDepthMarketData)
		return;

	if (IsInvalidQuote(pDepthMarketData))
		return;

	InstID instID = m_instHelper.GetInstID(pDepthMarketData->InstrumentID);
	if (INVALID_INSTRUMENT_ID == instID)
		return;

	Quote quote(CreateQuote(*pDepthMarketData, instID));

	m_publisher.PushQuote(quote);
}

void CTPMarketHandler::SubscribeMarket(char const* instrumentCode) {
	auto iterator = find(m_subscribeList.begin(), m_subscribeList.end(), instrumentCode);
	// 只有新增行情合约的时候才进行订阅
	if (iterator == m_subscribeList.end()) {
		Log::Info(fmt::format("CTPMarketHandler DoSubscribe[{}] market", instrumentCode));
		m_subscribeList.push_back(instrumentCode);
		DoSubscribe();
	}
}
void CTPMarketHandler::DoSubscribe() {
	char* instruments[MAX_INSTRUMENT_COUNT] = {0};
	for (size_t i=0; i<m_subscribeList.size(); ++i) {
		instruments[i] = (char*)m_subscribeList[i].c_str();
	}

	int rtnCode = m_pMdApi->SubscribeMarketData(instruments, int(m_subscribeList.size()));
	if (0 != rtnCode) {
 		auto msg = fmt::format("CTPMarketHandler SubscribeMarketData() Failed, RtnCode[{}]", rtnCode);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
}

bool CTPMarketHandler::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {
	return ((NULL!=pRspInfo) && (0!=pRspInfo->ErrorID));
}
bool CTPMarketHandler::IsInvalidQuote(CThostFtdcDepthMarketDataField* pDepthMarketData) {
	return  (0 == strlen(pDepthMarketData->InstrumentID) || DoubleComparer::equal(DBL_MAX, pDepthMarketData->LastPrice)
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->BidPrice1) || DoubleComparer::equal(DBL_MAX, pDepthMarketData->AskPrice1));
}

Quote CTPMarketHandler::CreateQuote(CThostFtdcDepthMarketDataField const& pDepthMarketData, InstID instID) {
	SimpleQuoteArray buyPriceArray;
	buyPriceArray[0] = SimpleQuote(pDepthMarketData.BidPrice1, pDepthMarketData.BidVolume1);
	SimpleQuoteArray sellPriceArray;
	sellPriceArray[0] = SimpleQuote(pDepthMarketData.AskPrice1, pDepthMarketData.AskVolume1);

	double openPrice = pDepthMarketData.OpenPrice;
	double highestPrice = pDepthMarketData.HighestPrice;
	double lowestPrice = pDepthMarketData.LowestPrice;
	double upperLimit = pDepthMarketData.UpperLimitPrice;
	double lowerLimit = pDepthMarketData.LowerLimitPrice;	
	int openInterest = int(pDepthMarketData.OpenInterest);
	double lastPrice = pDepthMarketData.LastPrice;
	double lastSettlement = pDepthMarketData.PreSettlementPrice;
	double turnOver = pDepthMarketData.Turnover;
	char minute[3] = { pDepthMarketData.UpdateTime[3], pDepthMarketData.UpdateTime[4] };

	Quote quote(instID, &m_instHelper.GetInstrument(instID), buyPriceArray, sellPriceArray, openPrice, highestPrice, lowestPrice,
		upperLimit, lowerLimit, openInterest, lastPrice, lastSettlement, pDepthMarketData.Volume, turnOver, true, atoi(minute), SimpleQuote(), SimpleQuote());
	return quote;
}

