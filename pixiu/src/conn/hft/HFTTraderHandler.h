#pragma once
#include "YD/ydApi.h"
#include "conn/ExchangeConn.h"
#include "model/Quote.h"

struct ConnConfig;
class ConnPublisher;
class FrontScorer;
struct PositionNotification;
class HFTTraderHandler : public YDListener {
public:
	HFTTraderHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher, FrontScorer& scorer);

	virtual void notifyEvent(int apiEvent);
	virtual void notifyReadyForLogin(bool hasLoginFailed);	
	virtual void notifyLogin(int errorNo, int maxOrderRef, bool isMonitor);
	virtual void notifyFinishInit(void);
	virtual void notifyCaughtUp(void);
	virtual void notifyTradingSegment(const YDExchange* pExchange, int segmentTime);

	virtual void notifyFailedOrder(const YDOrder* pFailedOrder, const YDInstrument* pInstrument, const YDAccount* pAccount);
	virtual void notifyFailedCancelOrder(const YDFailedCancelOrder* pFailedCancelOrder, const YDExchange* pExchange, const YDAccount* pAccount);
	virtual void notifyOrder(const YDOrder* pOrder, const YDInstrument* pInstrument, const YDAccount* pAccount);
	virtual void notifyTrade(const YDTrade* pTrade, const YDInstrument* pInstrument, const YDAccount* pAccount);

	virtual void notifyAccount(const YDAccount* pAccount);
	virtual void notifyMarketData(const YDMarketData* pMarketData);


	// 不知道这个函数是干嘛用的
	virtual void notifyResponse(int errorNo, int requestType);
public:
	void SetTraderApi(YDApi* pTraderApi) { m_pTraderApi = pTraderApi; }
	void SubscribeMarket(InstID instID);

private:
	Quote CreateQuote(YDMarketData const& pDepthMarketData, InstID instID);
	void FillingPositionNotification(YDPrePosition const& position, PositionNotification& notification);
private:
	ConnConfig const& m_config;
	ExchangeConnInstrumentHelper const& m_instHelper;
	ConnPublisher& m_publisher;
	FrontScorer& m_scorer;

	bool m_isForbidden;
	bool m_hasCaughtup;
	YDApi* m_pTraderApi;
	YDExchange const* m_pExchange;
	std::map<Exchange, ExchangeStatus> m_exchangeStatusMap;
	std::map<InstID, bool> m_subscribeList;
};
