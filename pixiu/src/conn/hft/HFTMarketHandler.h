#pragma once

#include "EESQuoteApi.h"
#include <set>
#include "model/Quote.h"
#include "conn/ExchangeConn.h"
#include "EES/EESQuoteApiStruct.h"

enum QUOTE_TYPE {
	FAST = 0,
	DEPTH
};

class QueueContext;
class ConnPublisher;
struct ConnConfig;
class HFTMarketHandler : public EESQuoteSpi {
public:
	HFTMarketHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& m_instHelper, ConnPublisher& publisher);

	virtual void OnConnect();
	virtual void OnDisconnect();
	virtual void OnRtnDepthMarketData(int id, char const* buff, unsigned int len);

	void SetMdApi(EESQuoteApi* pMdApi) { m_pMdApi = pMdApi; }
	void SubscribeMarket(InstID instID);

private:
	Quote OnProcSimpleQuote(InstID instID, efh3_2_fut_lev1* pInfo);
	Quote OnProcDepthQuote(InstID instID, efh3_2_fut_lev2* pInfo);
private:
	ConnConfig const& m_config;
	ExchangeConnInstrumentHelper const& m_instHelper;
	ConnPublisher& m_publisher;
	EESQuoteApi* m_pMdApi;

	std::set <InstID> m_subscribeList;
};
