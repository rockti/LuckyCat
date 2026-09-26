#include "conn/ztp/ZTPMarketHandler.h"

#pragma warning(disable:4100)

#include "model/Constants.h"
#include "notify/Notify.h"
#include "utils/Log.h"
#include "conn/ExchangeConn.h"
#include "config/Config.h"
#include "notify/ConnPublisher.h"
#include "domain/Error.h"
#include "service/Service.h"

ZTPMarketHandler::ZTPMarketHandler(TradeSystemConfig const& config, ConnPublisher& publisher)
	:m_config(config),m_publisher(publisher),m_pMdApi(0)
{
}

void ZTPMarketHandler::OnFrontConnected() {

	TradePlatformNotification en;
	en.Platform = TradePlatform::ZTP;
	en.Type = TradePlatformNotification::CONNECTED;
	en.SubSystem = SubSystem::Quote;
	m_publisher.NotifyUI(en);

	m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QuoteConnect);

	Log::Info("ZTPMarketHandler FrontConnected");
// 	CZtpApiReqLoginField loginReq = {0};
// 	strcpy_s(loginReq.BrokerID, m_config.Credential.BrokerID.c_str());
// 	strcpy_s(loginReq.AccountID, m_config.Credential.UserID.c_str());
// 	strcpy_s(loginReq.Password, m_config.Credential.Password.c_str());
//  	strcpy_s(loginReq.UserProductInfo, m_config.Credential.UserProductInfo.c_str());
// 	loginReq.UserType = ZTP_API_UT_Person;
// 	int rtnCode = m_pMdApi->ReqUserLogin(&loginReq, RequestIdACC::Generate());
// 	if (0 != rtnCode)
// 	{
//  		auto msg = fmt::format("ZTPMarketHandler ReqUserLogin() Failed, RtnCode[{}]", rtnCode);
//  		Log::Error(msg);
// 
// 		m_publisher.NotifyUI(msg);
// 	}
}
void ZTPMarketHandler::OnFrontDisconnected(int nReason) {

	TradePlatformNotification en;
	en.Platform = TradePlatform::ZTP;
	en.Type = TradePlatformNotification::DISCONNECTED;
	en.SubSystem = SubSystem::Quote;
	m_publisher.NotifyUI(en);

 	auto msg = fmt::format("ZTPMarketHandler FrontDisconnected, Reason[{:x}]", nReason);
	m_publisher.NotifyUI(msg);
	Log::Warn(msg);
}
void ZTPMarketHandler::OnRspError(CZtpApiRspInfoField*pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo))
	{
 		auto msg = fmt::format("ZTPMarketHandler OnRspError(), RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);


	}
}
void ZTPMarketHandler::OnRspUserLogin(CZtpApiRspLoginField *pRspUserLogin, CZtpApiRspInfoField*pRspInfo, int nRequestID, bool bIsLast)
{
	if (IsErrorRspInfo(pRspInfo)) {
		TradePlatformNotification en;
		en.Platform = TradePlatform::ZTP;
		en.Type = TradePlatformNotification::LOGIN_FAILED;
		en.SubSystem = SubSystem::Quote;
		en.Error = Error{ pRspInfo->ErrorID, pRspInfo->ErrorMsg };
		m_publisher.NotifyUI(en);

		m_publisher.NotifyFailToUI(TradePlatform::ZTP, InitProcess::QuoteLogin, AuthFailed);

		auto msg = fmt::format("ZTPMarketHandler UserLogin Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", 
			nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		m_publisher.NotifyUI(msg);
	}
	else
	{
 		Log::Info("ZTPMarketHandler UserLogin Success");
		TradePlatformNotification en;
		en.Platform = TradePlatform::ZTP;
		en.Type = TradePlatformNotification::LOGIN;
		en.SubSystem = SubSystem::Quote;
		m_publisher.NotifyUI(en);

		m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::QuoteLogin);


		if (!m_subscribeList.empty())		//如果断线重登录，自动订阅原来订阅过的行情
		{
			DoSubscribe();
		}
	}
}
void ZTPMarketHandler::OnRspUserLogout(CZtpApiRspLogoutField*pUserLogout, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pUserLogout)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		Log::Warn(fmt::format("ZTPMarketHandler UserLogout Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg));
	} else {
		TradePlatformNotification en;
		en.Platform = TradePlatform::ZTP;
		en.Type = TradePlatformNotification::LOGOUT;
		en.SubSystem = SubSystem::Quote;
		m_publisher.NotifyUI(en);

 		Log::Info("ZTPMarketHandler UserLogout Success");
	}
}
void ZTPMarketHandler::OnRspSubMarketData(CZtpApiSpecificInstrumentField*pSpecificInstrument, CZtpApiRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pSpecificInstrument)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
 		auto msg = fmt::format("ZTPMarketHandler SubMarketData[{}] Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]",
 			pSpecificInstrument->InstrumentID, nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
 		Log::Error(msg);
		return;
	}
	auto msg = fmt::format("ZTPMarketHandler SubMarketData[{}] Success", pSpecificInstrument->InstrumentID);
	Log::Info(msg);
}

void ZTPMarketHandler::OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL == pSpecificInstrument)
		return;

	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("ZTPMarketHandler UnSubMarketData[{}] Failed, RequestId[{}] ErrorCode[{}] ErrorMsg[{}]",
			pSpecificInstrument->InstrumentID, nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);

	}
}

void ZTPMarketHandler::OnRtnDepthMarketData(CZtpApiDepthMarketDataField *pDepthMarketData) {
	if (NULL == pDepthMarketData)
		return;

	if (IsInvalidQuote(pDepthMarketData))
		return;

	auto instID = ServiceSinglton::Services().Instrument->toID(type::InstrumentCode(pDepthMarketData->InstrumentID));
	if (INVALID_INSTRUMENT_ID == instID)
		return;

	Log::Debug(fmt::format("JinDou Market {}[{:.2f},{:.2f},{:.2f} {:.2f},{:.2f},{:.2f}]", pDepthMarketData->InstrumentID,
		pDepthMarketData->AskPrice3, pDepthMarketData->AskPrice2, pDepthMarketData->AskPrice1,
		pDepthMarketData->BidPrice1, pDepthMarketData->BidPrice2, pDepthMarketData->BidPrice3));

	Quote quote(CreateQuote(*pDepthMarketData, instID));

	m_publisher.PushQuote(quote);
}

void ZTPMarketHandler::SubscribeMarket(type::InstrumentCode const& instCode) {

	m_publisher.NotifyDoneToUI(TradePlatform::ZTP, InitProcess::SubscribeQuote);

	auto iterator = find(m_subscribeList.begin(), m_subscribeList.end(), instCode);
	if (iterator == m_subscribeList.end()) {
		m_subscribeList.push_back(instCode);
		CZtpApiSpecificInstrumentField field;
		strcpy_s(field.InstrumentID, instCode.value());

		int rtnCode = m_pMdApi->ReqSubMarketData(&field, RequestIdACC::Generate());
		if (0 != rtnCode) {
			auto msg = fmt::format("ZTPMarketHandler SubscribeMarketData() Failed, RtnCode[{}]", rtnCode);
			Log::Error(msg);
			m_publisher.NotifyUI(msg);
			return;
		}
		Log::Info(fmt::format("ZTPMarketHandler DoSubscribe[{}] market", instCode));
	}
}
void ZTPMarketHandler::DoSubscribe()
{
	char* instruments[MAX_INSTRUMENT_COUNT] = {0};
	for (size_t i=0; i<m_subscribeList.size(); ++i) {
		instruments[i] = m_subscribeList[i].data();

		CZtpApiSpecificInstrumentField field;
		strcpy_s(field.InstrumentID, m_subscribeList[i].data());
		
		int rtnCode = m_pMdApi->ReqSubMarketData(&field, RequestIdACC::Generate());
		if (0 != rtnCode) {
			auto msg = fmt::format("ZTPMarketHandler SubscribeMarketData() Failed, RtnCode[{}]", rtnCode);
			Log::Error(msg);
			m_publisher.NotifyUI(msg);
			return;
		}
	}
}

bool ZTPMarketHandler::IsErrorRspInfo(CZtpApiRspInfoField* pRspInfo)
{
	return ((NULL!=pRspInfo) && (0!=pRspInfo->ErrorID));
}
bool ZTPMarketHandler::IsInvalidQuote(CZtpApiDepthMarketDataField* pDepthMarketData) {
	return  (0 == strlen(pDepthMarketData->InstrumentID)
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->BidPrice1) || DoubleComparer::equal(0.0, pDepthMarketData->BidPrice1) || pDepthMarketData->BidVolume1==0
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->AskPrice1) || DoubleComparer::equal(0.0, pDepthMarketData->AskPrice1) || pDepthMarketData->AskPrice1==0);
}
SimpleQuote ZTPMarketHandler::LegalSimpleQuote(type::Price price, uint32_t quantity) {
	if (DoubleComparer::equal(DBL_MAX, price) || 0 == quantity) {
		return SimpleQuote();
	}
	return SimpleQuote(price, quantity);
}

Quote ZTPMarketHandler::CreateQuote(CZtpApiDepthMarketDataField const& pDepthMarketData, type::InstID instID) {
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

	Quote quote(instID, buyPriceArray, sellPriceArray, openPrice, highestPrice, lowestPrice, upperLimit, lowerLimit, 
		openInterest, lastPrice, lastSettlement, pDepthMarketData.TradeVolume, false, SimpleQuote(), SimpleQuote());
	return quote;
}

