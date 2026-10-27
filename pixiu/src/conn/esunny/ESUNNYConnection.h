#pragma once
#include "conn/ExchangeConn.h"
#include "conn/esunny/ESUNNYQuoteHandler.h"
#include "conn/esunny/ESUNNYTradeHandler.h"
#include "model/Instrument.h"
#include "config/Config.h"

class Dealing;
class ConnPublisher;
class ESUNNYConnection : public ExchangeConn
{
public:
	ESUNNYConnection(ConnConfig const& config, Instruments const& instruments, ConnPublisher& publisher);
	virtual void Init();
	virtual void Release();
	virtual void Login(char const* password);
	virtual void ResetOrderSet() {}
	virtual void InsertOrder(OrderInfo const& order);
	virtual void InsertExOrder(OrderInfo const& order) {}
	virtual int InsertMultiOrders(OrderInfo const* orders[]) { return 0; }
	virtual void CancelOrder(OrderCancel const& action);
	virtual void SubscribeQuote(char const* instrumentCode);
	virtual void UnSubscribeQuote(char const* instrumentCode);
	virtual void QueryMblMarket(char const* instrumentCode);
	virtual void QueryFund();
	virtual void QueryOrder();
	virtual void QueryTrade();
	virtual void QueryPosition();

	virtual void SetVertificateCode(char const* code);
	virtual void InsertMarketingOrder(MarketingOrderInfo const& order);
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel);

private:
	void InitQuoteApi();
	void InitTradeApi();
	std::string GetErrorDescription(TAPIINT32 errorCode);

	ConnConfig quoteConfig(ConnConfig const& config);
	ConnConfig tradeConfig(ConnConfig const& config);
	std::pair<std::string, int> SeparatePort(std::string const& hostUrl);
private:
	ConnConfig m_quoteConfig;
	ConnConfig m_tradeConfig;
	ExchangeConnInstrumentHelper m_instHelper;

	ESUNNYTradeHandler m_tradeHandler;
	ESUNNYQuoteHandler m_quoteHandler;
	ITapTrade::ITapTradeAPI* m_pTradeApi;
	ITapQuoteAPI* m_pQuoteApi;
	ConnPublisher& m_publisher;
};