#pragma once

#pragma warning(push)
#pragma warning(disable:4100)
#include "thirdapi/CTP/ThostFtdcMdApi.h"
#pragma warning(pop)

#include "model/Quote.h"
#include "conn/ExchangeConn.h"

class QueueContext;
class ConnPublisher;
struct TradeSystemConfig;
class CTPMarketHandler : public CThostFtdcMdSpi
{
public:
	CTPMarketHandler(TradeSystemConfig const& config, ConnPublisher& publisher);

	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void SetMdApi(CThostFtdcMdApi* pMdApi) { m_pMdApi = pMdApi; }
	void SubscribeMarket(type::InstrumentCode const& instCode);			//只有CTP是需要根据合约来订阅行情的，其他交易所都是根据主题来订阅行情
	void DoSubscribe();
private:
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	bool IsInvalidQuote(CThostFtdcDepthMarketDataField* pDepthMarketData);
	SimpleQuote LegalSimpleQuote(type::Price price, uint32_t quantity);

	Quote CreateQuote(CThostFtdcDepthMarketDataField const& pDepthMarketData, type::InstID instID);

private:
	TradeSystemConfig const& m_config;
	ConnPublisher& m_publisher;
	CThostFtdcMdApi* m_pMdApi;
	std::vector<type::InstrumentCode> m_subscribeList;		//订阅列表
};