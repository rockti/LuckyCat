#pragma once

#pragma warning(push)
#pragma warning(disable:4100)
#include "CTP/ThostFtdcMdApi.h"
#pragma warning(pop)

#include "model/Quote.h"
#include "conn/ExchangeConn.h"

class QueueContext;
class ConnPublisher;
struct ConnConfig;
class CTPMarketHandler : public CThostFtdcMdSpi
{
public:
	CTPMarketHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher);

	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	void SetMdApi(CThostFtdcMdApi* pMdApi) { m_pMdApi = pMdApi; }
	void SubscribeMarket(char const* instrumentCode);			//只有CTP是需要根据合约来订阅行情的，其他交易所都是根据主题来订阅行情
	void DoSubscribe();

private:
	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	bool IsInvalidQuote(CThostFtdcDepthMarketDataField* pDepthMarketData);

	Quote CreateQuote(CThostFtdcDepthMarketDataField const& pDepthMarketData, InstID instID);

private:
	ConnConfig const& m_config;
	ExchangeConnInstrumentHelper const& m_instHelper;
	ConnPublisher& m_publisher;
	CThostFtdcMdApi* m_pMdApi;

	std::vector<std::string> m_subscribeList;		//订阅列表
};