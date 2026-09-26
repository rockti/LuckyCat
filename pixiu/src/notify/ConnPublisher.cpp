#include "pch.h"
#include "notify/Notify.h"
#include "notify/ConnPublisher.h"
#include "runner/Queues.h"
#include "model/Quote.h"

ConnPublisher::ConnPublisher(ConnPlatformExecutorQueues* apiQueues) :m_queues(*apiQueues) {
}

void ConnPublisher::PushQuote(Quote const& quote) {
	Data data;
	data.Type = DataType::Quote;
	data.Size = sizeof(quote);
	memcpy(data.Data, &quote, sizeof(quote));
	pushResponse(data);
}
void ConnPublisher::PushPrice(Quote const& quote) {
	Data data;
	data.Type = DataType::Price;
	data.Size = sizeof(quote);
	memcpy(data.Data, &quote, sizeof(quote));
	pushResponse(data);
}

void ConnPublisher::PushResponse(OrderNotification const& notification) {
	Data data;
	data.Type = DataType::Dealing;
	data.Size = sizeof(OrderNotification);
	memcpy(data.Data, &notification, data.Size);
	pushResponse(data);
}
void ConnPublisher::PushResponse(ExchangeNotification const& notification) {
	Data data;
	data.Type = DataType::Exchange;
	data.Size = sizeof(OrderNotification);
	memcpy(data.Data, &notification, data.Size);
	pushResponse(data);
}
void ConnPublisher::PushResponse(CleanPositionNotification const& notification) {
	Data data;
	data.Type = DataType::CleanPosition;
	data.Size = 0;
	pushResponse(data);
}
void ConnPublisher::PushResponse(PositionNotification const& notification) {
	Data data;
	data.Type = DataType::QryPosition;
	data.Size = sizeof(PositionNotification);
	memcpy(data.Data, &notification, data.Size);
	pushResponse(data);
}
void ConnPublisher::PushResponse(FundsNotification const& notification) {
	Data data;
	data.Type = DataType::QryFund;
	data.Size = sizeof(FundsNotification);
	memcpy(data.Data, &notification, data.Size);
	pushResponse(data);
}
bool ConnPublisher::pushResponse(Data const& data) {
	return m_queues.PushResponse(data);
}

void ConnPublisher::NotifyUI(ExchangeNotification const& notification) {
	Data data;
	data.Type = DataType::Exchange;
	data.Size = sizeof(ExchangeNotification);
	memcpy(data.Data, &notification, data.Size);
	notifyUI(data);
}
void ConnPublisher::NotifyUI(PositionNotification const& notification) {
	Data data;
	data.Type = DataType::QryPosition;
	data.Size = sizeof(PositionNotification);
	memcpy(data.Data, &notification, data.Size);
	notifyUI(data);
}
void ConnPublisher::NotifyUI(std::string const& msg) {
	Data data;
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::max(sizeof(data.Data), msg.size()));
	memcpy(data.Data, msg.c_str(), data.Size);
	notifyUI(data);
}
void ConnPublisher::NotifyUI(char const* msg) {
	Data data;
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::max(sizeof(data.Data), strlen(msg)));
	memcpy(data.Data, msg, data.Size);
	notifyUI(data);
}
bool ConnPublisher::notifyUI(Data const& data) {
	return m_queues.NotifyUI(data);
}