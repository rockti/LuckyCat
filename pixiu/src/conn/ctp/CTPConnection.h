#pragma once
#include "conn/ExchangeConn.h"
#include "conn/ctp/CTPTraderHandler.h"
#include "conn/ctp/CTPMarketHandler.h"
#include "config/Config.h"

class Instrument;

class CTPConnection : public ExchangeConn
{
public:
	CTPConnection(ConnConfig& config, Instruments const& instruments, ConnPublisher& publisher);
	virtual void Init();
	virtual void Release();
	virtual void Login(char const* password);
	virtual void ResetOrderSet() {}
	virtual void InsertOrder(OrderInfo const& order);
	virtual void InsertExOrder(OrderInfo const& order) {}
	virtual void InsertMultiOrders(unsigned int cnt, OrderInfo const* orders[]) {}
	virtual void CancelOrder(OrderCancel const& action);
	virtual void SubscribeQuote(char const* instrumentCode);
	virtual void UnSubscribeQuote(char const* instrumentCode);
	virtual void QueryMblMarket(char const* instrumentCode);
	virtual void QueryAccount();
	virtual void QueryOrder();
	virtual void QueryTrade();
	virtual void QueryPosition();
	virtual void QueryFund() {}

	virtual void SetVertificateCode(char const* password) { throw std::logic_error("CTP can not set VeritificateCode"); }
	virtual void InsertMarketingOrder(MarketingOrderInfo const& dealing) { throw std::logic_error("CTP not support marketing order"); }
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel) { throw std::logic_error("CTP not support marketing order"); }

private:
	void InitTradeApi();
	void InitQuoteApi();

private:
	ConnConfig& m_config;
	ExchangeConnInstrumentHelper m_instHelper;
	std::string m_traderUrl;
	std::string m_marketUrl;

	CTPTraderHandler m_traderHandler;
	CTPMarketHandler m_marketHandler;

	CThostFtdcTraderApi* m_pTraderApi;
	CThostFtdcMdApi* m_pMdApi;

	ConnPublisher& m_publisher;
};
