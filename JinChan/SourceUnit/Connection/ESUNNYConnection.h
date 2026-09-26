#pragma once
#include "ESUNNYQuoteHandler.h"

class Dealing;
class ConnPublisher;
class ESUNNYConnection 
{
public:
	ESUNNYConnection(CMarketFlowManager* pFlowManager, int topicID);
	virtual void Init();
	virtual void Release();
	virtual void Login(char const* inputPwd = nullptr);
	virtual void SubscribeQuote(char const* instrumentCode);
	virtual void UnSubscribeQuote(char const* instrumentCode);
	virtual void QueryMblMarket(char const* instrumentCode);

private:
	void InitQuoteApi();
	std::string GetErrorDescription(TAPIINT32 errorCode);
private:
	ESUNNYQuoteHandler m_quoteHandler;
	ITapQuoteAPI* m_pQuoteApi;
};