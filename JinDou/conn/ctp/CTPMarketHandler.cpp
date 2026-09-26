#include "conn/ctp/CTPMarketHandler.h"

#pragma warning(disable:4100)

#include "model/Constants.h"
#include "notify/Notify.h"
#include "utils/Log.h"
#include "conn/ExchangeConn.h"
#include "config/Config.h"
#include "notify/ConnPublisher.h"
#include "domain/Error.h"
#include "service/Service.h"
#include "service/InstrumentProgressManager.h"

CTPMarketHandler::CTPMarketHandler(TradeSystemConfig const& config, ConnPublisher& publisher)
	:m_config(config),m_publisher(publisher),m_pMdApi(0) {
}

void CTPMarketHandler::OnFrontConnected() {
	Log::Info("CTPMarketHandler: FrontConnected");
	TradePlatformNotification tn;
	tn.Platform = TradePlatform::CTP;
	tn.Type = TradePlatformNotification::CONNECTED;
	tn.SubSystem = SubSystem::Quote;
	m_publisher.NotifyUI(tn);
	m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QuoteConnect);
}
void CTPMarketHandler::OnFrontDisconnected(int nReason) {
	TradePlatformNotification tn;
	tn.Platform = TradePlatform::CTP;
	tn.Type = TradePlatformNotification::DISCONNECTED;
	tn.SubSystem = SubSystem::Quote;
	m_publisher.NotifyUI(tn);

 	auto msg = fmt::format("CTPMarketHandler: FrontDisconnected, Reason[{:x}]", nReason);
	Log::Warn(msg);
}
void CTPMarketHandler::OnHeartBeatWarning(int nTimeLapse) {
	Log::Warn(fmt::format("CTPMarketHandler: HeartBeatWarning TimeLapse[{}]s", nTimeLapse));
}

void CTPMarketHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
 		auto msg = fmt::format("CTPMarketHandler: OnRspError(), RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);

		ErrorNotification noti;
		noti.Platform = TradePlatform::CTP;
		noti.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(noti);
	}
}
void CTPMarketHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pRspUserLogin) {
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QuoteLogin, SystemError);
		return;
	}

	if (IsErrorRspInfo(pRspInfo)) {
		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGIN;
		tn.SubSystem = SubSystem::Quote;
		tn.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(tn);
		auto msg = fmt::format("CTPMarketHandler: UserLogin Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::CTP, InitProcess::QuoteLogin, tn.Error);
	} else {
 		Log::Info("CTPMarketHandler: UserLogin Success");

		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGIN;
		tn.SubSystem = SubSystem::Quote;
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::QuoteLogin);

		//TODO: 断线重连
	}
}
void CTPMarketHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pUserLogout)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		Log::Warn(fmt::format("CTPMarketHandler: UserLogout Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	} else {
		TradePlatformNotification tn;
		tn.Platform = TradePlatform::CTP;
		tn.Type = TradePlatformNotification::LOGOUT;
		tn.SubSystem = SubSystem::Trade;
		m_publisher.NotifyUI(tn);
 		Log::Info("CTPMarketHandler: UserLogout Success");
	}
}
void CTPMarketHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pSpecificInstrument)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		auto msg = fmt::format("CTPMarketHandler: SubMarketData[{}] Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]",
 			pSpecificInstrument->InstrumentID, nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	} else {
		InstrumentProgressManager::CTP().AddQuoteDone(type::InstrumentCode(pSpecificInstrument->InstrumentID));
	}
}

void CTPMarketHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) {
	if (NULL == pDepthMarketData)
		return;

	if (IsInvalidQuote(pDepthMarketData))
		return;

	auto instID = ServiceSinglton::Services().Instrument->toID(type::InstrumentCode(pDepthMarketData->InstrumentID));
	if (INVALID_INSTRUMENT_ID == instID)
		return;

	Quote quote(CreateQuote(*pDepthMarketData, instID));
	m_publisher.PushQuote(quote);
}

void CTPMarketHandler::SubscribeMarket(type::InstrumentCode const& instCode) {
	m_publisher.NotifyDoneToUI(TradePlatform::CTP, InitProcess::SubscribeQuote);

	auto iterator = find(m_subscribeList.begin(), m_subscribeList.end(), instCode);
	if (iterator == m_subscribeList.end()) {		//只有新增行情合约的时候才进行订阅
		m_subscribeList.push_back(instCode);
		DoSubscribe();
		Log::Info(fmt::format("CTPMarketHandler: DoSubscribe[{}] market", instCode));
	}
}
void CTPMarketHandler::DoSubscribe() {
	char* instruments[MAX_INSTRUMENT_COUNT] = {0};
	for (size_t i=0; i<m_subscribeList.size(); ++i) {
		instruments[i] = m_subscribeList[i].data();
	}

	int rtnCode = m_pMdApi->SubscribeMarketData(instruments, int(m_subscribeList.size()));
	if (0 != rtnCode) {
 		auto msg = fmt::format("CTPMarketHandler: ReqSubscribeMarketData() Failed, RtnCode[{}]", rtnCode);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
}

bool CTPMarketHandler::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) {
	return ((NULL!=pRspInfo) && (0!=pRspInfo->ErrorID));
}
bool CTPMarketHandler::IsInvalidQuote(CThostFtdcDepthMarketDataField* pDepthMarketData) {
	return  (0 == strlen(pDepthMarketData->InstrumentID) || 0==pDepthMarketData->Volume
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->LastPrice)
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->BidPrice1)
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->AskPrice1));
}
SimpleQuote CTPMarketHandler::LegalSimpleQuote(type::Price price, uint32_t quantity) {
	if (DoubleComparer::equal(DBL_MAX, price) || 0 == quantity) {
		return SimpleQuote();
	}
	return SimpleQuote(price, quantity);
}

Quote CTPMarketHandler::CreateQuote(CThostFtdcDepthMarketDataField const& pDepthMarketData, type::InstID instID) {
	SimpleQuoteArray buyPriceArray;
	buyPriceArray[0] = SimpleQuote(pDepthMarketData.BidPrice1, pDepthMarketData.BidVolume1);
	buyPriceArray[1] = LegalSimpleQuote(pDepthMarketData.BidPrice2, pDepthMarketData.BidVolume2);
	buyPriceArray[2] = LegalSimpleQuote(pDepthMarketData.BidPrice3, pDepthMarketData.BidVolume3);
	buyPriceArray[3] = LegalSimpleQuote(pDepthMarketData.BidPrice4, pDepthMarketData.BidVolume4);
	buyPriceArray[4] = LegalSimpleQuote(pDepthMarketData.BidPrice5, pDepthMarketData.BidVolume5);
	SimpleQuoteArray sellPriceArray;
	sellPriceArray[0] = SimpleQuote(pDepthMarketData.AskPrice1, pDepthMarketData.AskVolume1);
	sellPriceArray[1] = LegalSimpleQuote(pDepthMarketData.AskPrice2, pDepthMarketData.AskVolume2);
	sellPriceArray[2] = LegalSimpleQuote(pDepthMarketData.AskPrice3, pDepthMarketData.AskVolume3);
	sellPriceArray[3] = LegalSimpleQuote(pDepthMarketData.AskPrice4, pDepthMarketData.AskVolume4);
	sellPriceArray[4] = LegalSimpleQuote(pDepthMarketData.AskPrice5, pDepthMarketData.AskVolume5);

	double openPrice = pDepthMarketData.OpenPrice;
	double highestPrice = pDepthMarketData.HighestPrice;
	double lowestPrice = pDepthMarketData.LowestPrice;
	double upperLimit = pDepthMarketData.UpperLimitPrice;
	double lowerLimit = pDepthMarketData.LowerLimitPrice;	
	int openInterest = int(pDepthMarketData.OpenInterest);
	double lastPrice = pDepthMarketData.LastPrice;
	double lastSettlement = pDepthMarketData.PreSettlementPrice;

	Quote quote(instID, buyPriceArray,sellPriceArray, openPrice, highestPrice, lowestPrice, upperLimit, lowerLimit, 
		openInterest, lastPrice, lastSettlement, pDepthMarketData.Volume, false, SimpleQuote(), SimpleQuote());
	return quote;
}

