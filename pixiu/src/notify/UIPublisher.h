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
	void Login(ConnPlatform connPlatform, char const* password);
	void Logout(ConnPlatform connPlatform);
	void SetVertificateCode(ConnPlatform connPlatform, char const* code);
	void SubscribeQuote(InstID instID);
	void QryFund(ConnPlatform connPlatform);
	void QryPosition(ConnPlatform connPlatform);
private:
	UIExecutorQueues& m_queues;
};