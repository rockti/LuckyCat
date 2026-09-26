#pragma once
#include "config/Config.h"
#include "conn/ExchangeConn.h"
#include "conn/hft/HFTMarketHandler.h"
#include "conn/hft/HFTTraderHandler.h"


//高频率交易接口
//行情使用盛立组播
//交易使用易达Api

class ConnPublisher;
class HFTConnection : public ExchangeConn {
public:
	HFTConnection(ConnConfig& config, Instruments const& instruments, ConnPublisher& publisher, bool isMulticast=true/*默认使用组播*/);
	virtual void Init();
	virtual void Release();
	virtual void Login(char const* password) {}
	virtual void ResetOrderSet();
	virtual void InsertOrder(OrderInfo const& order);
	virtual void InsertExOrder(OrderInfo const& order) {}
	virtual void InsertMultiOrders(unsigned int cnt, OrderInfo const* orders[]);
	virtual void CancelOrder(OrderCancel const& action);
	virtual void SubscribeQuote(char const* instrumentCode);
	virtual void UnSubscribeQuote(char const* instrumentCode) {}
	virtual void QueryMblMarket(char const* instrumentCode) {}
	virtual void QueryAccount() {}
	virtual void QueryOrder() {}
	virtual void QueryTrade() {}
	virtual void QueryPosition() {}
	virtual void QueryFund() {}

	virtual void SetVertificateCode(char const* password) { throw std::logic_error("HFT can not set VeritificateCode"); }
	virtual void InsertMarketingOrder(MarketingOrderInfo const& dealing) { throw std::logic_error("HFT not support marketing order"); }
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel) { throw std::logic_error("HFT not support marketing order"); }

private:
	void InitTradeApi();		//易达交易
	void InitQuoteApi();		//盛立TCP行情
	void InitMulticast();		//盛立组播行情

private:
	ConnConfig& m_config;
	ExchangeConnInstrumentHelper m_instHelper;
	ConnPublisher& m_publisher;
	bool m_isMulticast = true;

	HFTTraderHandler m_traderHandler;
	HFTMarketHandler m_marketHandler;

	YDApi* m_pTraderApi;
	EESQuoteApi* m_pMdApi;

	std::string m_apiVersion;
	YDExchange const* m_pExchange;

	// 以下变量用于InsertMultiorders
	bool m_isOrderReady = false;
	YDInputOrder m_orderSet[16] = {};
	const YDInstrument* m_instruments[16] = {};
};