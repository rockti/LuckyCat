#pragma once

#include "model/Constants.h"

class OrderCancel;
struct StrategyExecutorQueues;
struct OrderInfo;
struct MarketingOrderInfo;
struct MarketingOrderCancel;
class Trade;
class Dealing;
struct PlatformAccount;
struct TimerCommand;
struct CleanUIPosition;
struct OrderNotification;

class StrategyPublisher {
public:
	StrategyPublisher(StrategyExecutorQueues* queues) :m_queues(*queues) {}

	void PushTimerOrder(TimerCommand const& cmd);

	void PushOrder(OrderInfo const& order);
	void PushOrder(MarketingOrderInfo const& order);

	void PushAction(OrderCancel const& action);
	void PushAction(MarketingOrderCancel const& action);

	void QueryPosition(InstID instID);		// handleOtherOrder
	void SubscribeQuote(InstID instID);

	void NotifyUI(char const* msg);
	void NotifyUI(std::string const& msg);	
	void NotifyUI(Dealing const& dealing);
	void NotifyUI(PlatformAccount const& account);			// 更新界面资金
	void NotifyUI(CleanUIPosition const& notification);		// 清空界面持仓
	void NotifyUI(OrderNotification const& notification);	// 更新界面持仓
	
	template<class BS>
	void NotifyUIBusinessSnapshot(BS const& snapshot) {};

private:
	StrategyExecutorQueues& m_queues;
};
