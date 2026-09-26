#pragma once

#include "model/Constants.h"
#include "notify/Notify.h"
#include "model/dealing/ExchangeRequest.h"

// class OrderCancelRequest;
struct StrategyExecutorQueues;
// struct OrderInsertRequest;
// struct Data;
// class Dealing;

class StrategyPublisher {
public:
	StrategyPublisher(StrategyExecutorQueues* queues) :m_queues(*queues) {}

	void PushOrder(OrderInsertRequest const& order);
	void PushAction(OrderCancelRequest const& action);
	void PushToConn(Data const& data);

	void NotifyUI(char const* msg);
	void NotifyUI(std::string const& msg);
	void NotifyUI(InvestorOrder const& order);

	template<class BS>
	void NotifyUIBusinessSnapshot(BS const& snapshot) {};

private:
	StrategyExecutorQueues& m_queues;
};
