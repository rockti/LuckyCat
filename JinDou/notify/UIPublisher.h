#pragma
#include "model/Constants.h"

struct StrategyCommand;

class Business;
class ExchangeConn;
struct UIExecutorQueues;
struct Data;
class UIPublisher {
public:
	UIPublisher(UIExecutorQueues* queues) :m_queues(*queues) {}
	bool Pop(Data& data);
	void Push(StrategyCommand const& command);
	void AddBusiness(Business* business);
	void Login(TradePlatform connPlatform, char const* password);
	void SubscribeQuote(type::InstID instID);
private:
	UIExecutorQueues& m_queues;
};