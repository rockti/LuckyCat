#include "notify/Notify.h"
#include "notify/ConnPublisher.h"
#include "runner/Queues.h"
#include "model/Quote.h"

ConnPublisher::ConnPublisher(ConnPlatformExecutorQueues* apiQueues) :m_queues(*apiQueues) {
}

bool ConnPublisher::notifyUI(Data const& data) {
	return m_queues.NotifyUI(data);
}

bool ConnPublisher::pushResponse(Data const& data) {
	return m_queues.PushResponse(data);
}

void ConnPublisher::PushResponse(InvestorOrder const& notification) {
	Data data;
	data.Type = DataType::Order;
	data.Size = sizeof(InvestorOrder);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(OrderErrorNotification const& notification) {
	Data data;
	data.Type = DataType::OrderErrorRtn;
	data.Size = sizeof(OrderErrorNotification);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(InvestorTrade const& notification) {
	Data data;
	data.Type = DataType::Trade;
	data.Size = sizeof(InvestorTrade);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(Instrument const& inst) {
	Data data;
	data.Type = DataType::Instrument;
	data.Size = sizeof(InvestorTrade);
	memcpy(data.Payload, &inst, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(TradePlatformNotification const& notification) {
	Data data;
	data.Type = DataType::TradePlatform;
	data.Size = sizeof(TradePlatformNotification);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(ExchangeNotification const& notification) {
	Data data;
	data.Type = DataType::Exchange;
	data.Size = sizeof(ExchangeNotification);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(InvestorAccount const& notification) {
	Data data;
	data.Type = DataType::Account;
	data.Size = sizeof(InvestorAccount);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(CleanPositionNotification const& notification) {
	Data data;
	data.Type = DataType::CleanPosition;
	data.Size = 0;
	pushResponse(data);
}

void ConnPublisher::PushResponse(InvestorPosition const& notification) {
	Data data;
	data.Type = DataType::Position;
	data.Size = sizeof(InvestorPosition);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(InvestorMargin const& notification) {
	Data data;
	data.Type = DataType::Margin;
	data.Size = sizeof(InvestorMargin);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::PushResponse(InvestorFee const& notification) {
	Data data;
	data.Type = DataType::Fee;
	data.Size = sizeof(InvestorFee);
	memcpy(data.Payload, &notification, data.Size);
	pushResponse(data);
}

void ConnPublisher::NotifyUI(TradePlatformNotification const& notification) {
	Data data;
	data.Type = DataType::TradePlatform;
	data.Size = sizeof(TradePlatformNotification);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(ExchangeNotification const& notification) {
	Data data;
	data.Type = DataType::Exchange;
	data.Size = sizeof(ExchangeNotification);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(InitStatusNotification const& notification) {
	Data data;
	data.Type = DataType::InitStatus;
	data.Size = sizeof(InitStatusNotification);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(InvestorAccount const& notification) {
	Data data;
	data.Type = DataType::Account;
	data.Size = sizeof(InvestorAccount);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(InvestorPosition const& notification) {
	Data data;
	data.Type = DataType::Position;
	data.Size = sizeof(InvestorPosition);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(Instrument const& inst) {
	Data data;
	data.Type = DataType::Instrument;
	data.Size = sizeof(Instrument);
	memcpy(data.Payload, &inst, data.Size);
	notifyUI(data);
}


void ConnPublisher::NotifyUI(InvestorFee const& fee) {
	Data data;
	data.Type = DataType::Fee;
	data.Size = sizeof(InvestorFee);
	memcpy(data.Payload, &fee, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(InvestorMargin const& margin) {
	Data data;
	data.Type = DataType::Margin;
	data.Size = sizeof(InvestorMargin);
	memcpy(data.Payload, &margin, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(InvestorOrder const& order) {
	Data data;
	data.Type = DataType::Order;
	data.Size = sizeof(InvestorOrder);
	memcpy(data.Payload, &order, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(OrderRtnNotification const& order) {
	Data data;
	data.Type = DataType::OrderRtn;
	data.Size = sizeof(OrderRtnNotification);
	memcpy(data.Payload, &order, data.Size);
	notifyUI(data);
}


void ConnPublisher::NotifyUI(OrderErrorNotification const& notification) {
	Data data;
	data.Type = DataType::OrderErrorRtn;
	data.Size = sizeof(OrderErrorNotification);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(ErrorNotification const& notification) {
	Data data;
	data.Type = DataType::Error;
	data.Size = sizeof(ErrorNotification);
	memcpy(data.Payload, &notification, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(TradeNotification const& trade) {
	Data data;
	data.Type = DataType::TradeRtn;
	data.Size = sizeof(TradeNotification);
	memcpy(data.Payload, &trade, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(InvestorTrade const& trade) {
	Data data;
	data.Type = DataType::Trade;
	data.Size = sizeof(InvestorTrade);
	memcpy(data.Payload, &trade, data.Size);
	notifyUI(data);
}


void ConnPublisher::NotifyUI(char const* msg) {
	Data data;
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::max(sizeof(data.Payload), strlen(msg)));
	memcpy(data.Payload, msg, data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyUI(std::string const& msg) {
	Data data;
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::max(sizeof(data.Payload), msg.size()));
	memcpy(data.Payload, msg.c_str(), data.Size);
	notifyUI(data);
}

void ConnPublisher::NotifyProcessingToUI(TradePlatform platform, InitProcess process) {
	InitStatusNotification ipn;
	ipn.Error = NoError;
	ipn.Platform = platform;
	ipn.Process = process;
	ipn.Status = InitStatus::Processing;
	NotifyUI(ipn);
}

void ConnPublisher::NotifyDoneToUI(TradePlatform platform, InitProcess process) {
	InitStatusNotification ipn;
	ipn.Error = NoError;
	ipn.Platform = platform;
	ipn.Process = process;
	ipn.Status = InitStatus::Done;
	NotifyUI(ipn);
}

void ConnPublisher::NotifyFailToUI(TradePlatform platform, InitProcess process, Error error) {
	InitStatusNotification ipn;
	ipn.Error = error;
	ipn.Platform = platform;
	ipn.Process = process;
	ipn.Status = InitStatus::Fail;
	NotifyUI(ipn);
}

void ConnPublisher::PushQuote(Quote const& quote) {
	Data data;
	data.Type = DataType::Quote;
	data.Size = sizeof(quote);
	memcpy(data.Payload, &quote, sizeof(quote));
	notifyUI(data);
	pushResponse(data);
}