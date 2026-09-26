#pragma once

#pragma warning(push)
#pragma warning(disable:4100)
#include "thirdapi/ZTP/ZTPUserMDApi.h"
#pragma warning(pop)

#include "model/Quote.h"
#include "conn/ExchangeConn.h"

class QueueContext;
class ConnPublisher;
struct TradeSystemConfig;
class ZTPMarketHandler : public CZtpFtdMdSpi
{
public:
	ZTPMarketHandler(TradeSystemConfig const& config, ConnPublisher& publisher);

	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	//错误应答
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//登录应答
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//录出应答
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//订阅行情应答，返回订阅的合约代码
	virtual void OnRspSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//退订行情应答，返回退订的合约代码
	virtual void OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//深度行情变化通知
	virtual void OnRtnDepthMarketData(CZtpApiDepthMarketDataField* pDepthMarketDataField);

	void SetMdApi(CZtpFtdMdApi* pMdApi) { m_pMdApi = pMdApi; }
	void SubscribeMarket(type::InstrumentCode const& instCode);			//只有CTP是需要根据合约来订阅行情的，其他交易所都是根据主题来订阅行情
	void DoSubscribe();

private:
	bool IsErrorRspInfo(CZtpApiRspInfoField* pRspInfo);
	bool IsInvalidQuote(CZtpApiDepthMarketDataField* pDepthMarketData);
	SimpleQuote LegalSimpleQuote(type::Price price, uint32_t quantity);

	Quote CreateQuote(CZtpApiDepthMarketDataField const& pDepthMarketData, type::InstID instID);

private:
	TradeSystemConfig const& m_config;
	ConnPublisher& m_publisher;
	CZtpFtdMdApi* m_pMdApi;

	std::vector<type::InstrumentCode> m_subscribeList;		//订阅列表
};