
#include "notify/StrategyPublisher.h"
#include "model/dealing/ExchangeRequest.h"
#include "model/dealing/Dealing.h"
#include "runner/Queues.h"


void StrategyPublisher::PushOrder(OrderInsertRequest const& order) {
	Data data;
	data.Type = DataType::OrderInsert;
	data.Size = sizeof(OrderInsertRequest);
	memcpy(data.Payload, &order, data.Size);
	m_queues.Push(data);
}
void StrategyPublisher::PushAction(OrderCancelRequest const& action) {
	Data data;
	data.Type = DataType::OrderCancel;
	data.Size = sizeof(OrderCancelRequest);
	memcpy(data.Payload, &action, data.Size);
	m_queues.Push(data);
}

void StrategyPublisher::PushToConn(Data const& data) {
	m_queues.Push(data);
}



void StrategyPublisher::NotifyUI(char const* msg) {
	Data data;
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::max(sizeof(data.Payload), strlen(msg)));
	memcpy(data.Payload, msg, data.Size);
	m_queues.NotifyUI(data);
}

void StrategyPublisher::NotifyUI(std::string const& msg) {
	Data data;
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::max(sizeof(data.Payload), msg.size()));
	memcpy(data.Payload, msg.c_str(), data.Size);
	m_queues.NotifyUI(data);
}
void StrategyPublisher::NotifyUI(InvestorOrder const& order) {
	Data data;
	data.Type = DataType::Order;
	data.Size = sizeof(InvestorOrder);
	memcpy(data.Payload, &order, data.Size);
	m_queues.NotifyUI(data);
}



