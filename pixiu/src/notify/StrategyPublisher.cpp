#include "pch.h"

#include "runner/Queues.h"
#include "notify/StrategyPublisher.h"
#include "model/dealing/ExchangeRequest.h"
#include "model/dealing/Dealing.h"
#include "model/factory/InstrumentFactory.h"
#include "service/PositionService.h"


void StrategyPublisher::PushTimerOrder(TimerCommand const& cmd) {
	Data data;	
	data.Type = DataType::TimerCommand;
	data.Size = sizeof(TimerCommand);
	memcpy(data.Data, &cmd, data.Size);
	m_queues.PushRequest(data);
}

void StrategyPublisher::PushOrder(OrderInfo const& order) {
	Data data;
	data.Type = DataType::Order;
	data.Size = sizeof(OrderInfo);
	memcpy(data.Data, &order, data.Size);
	m_queues.PushRequest(data);
}
void StrategyPublisher::PushAction(OrderCancel const& action) {
	Data data;
	data.Type = DataType::Cancel;
	data.Size = sizeof(OrderCancel);
	memcpy(data.Data, &action, data.Size);
	m_queues.PushRequest(data);
}

void StrategyPublisher::PushOrder(MarketingOrderInfo const& order) {
	Data data;
	data.Type = DataType::MarketingQuote;
	data.Size = sizeof(MarketingOrderInfo);
	memcpy(data.Data, &order, data.Size);
	m_queues.PushRequest(data);
}
void StrategyPublisher::PushAction(MarketingOrderCancel const& action) {
	Data data;
	data.Type = DataType::MarketingCancel;
	data.Size = sizeof(MarketingOrderCancel);
	memcpy(data.Data, &action, data.Size);
	m_queues.PushRequest(data);
}

void StrategyPublisher::QueryPosition(InstID instID) {
	Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);

	ConnCommand command;
	command.Type = ConnCommand::POSITION_QRY;
	command.Platform = Platform(instrument.GetExchange());

	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushRequest(data);
}
void StrategyPublisher::SubscribeQuote(InstID instID) {
	Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);

	ConnCommand command;
	command.Type = ConnCommand::MARKET_SUBSCRIBE;
	command.Platform = Platform(instrument.GetExchange());
	strcpy_s(command.Data, instrument.GetInstCode().Str());

	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushRequest(data);
}

void StrategyPublisher::NotifyUI(char const* msg) {
	Data data;
	memset(data.Data, 0, sizeof(data.Data));
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::min(sizeof(data.Data)-1, strlen(msg)));
	memcpy(data.Data, msg, data.Size);
	m_queues.NotifyUI(data);
}
void StrategyPublisher::NotifyUI(std::string const& msg) {
	Data data;
	memset(data.Data, 0, sizeof(data.Data));
	data.Type = DataType::UIMessage;
	data.Size = static_cast<uint16_t>(std::min(sizeof(data.Data)-1, msg.size()));
	memcpy(data.Data, msg.c_str(), data.Size);
	m_queues.NotifyUI(data);
}
void StrategyPublisher::NotifyUI(Dealing const& dealing) {
	Data data;
	data.Type = DataType::Dealing;
	data.Size = sizeof(Dealing);
	memcpy(data.Data, &dealing, data.Size);
	m_queues.NotifyUI(data);
}
void StrategyPublisher::NotifyUI(PlatformAccount const& account) {
	Data data;
	data.Type = DataType::UpdateAvailable;
	data.Size = sizeof(PlatformAccount);
	memcpy(data.Data, &account, data.Size);
	m_queues.NotifyUI(data);
}
void StrategyPublisher::NotifyUI(OrderNotification const& notification) {
	Data data;
	data.Type = DataType::Trade;
	data.Size = sizeof(OrderNotification);
	memcpy(data.Data, &notification, data.Size);
	m_queues.NotifyUI(data);
}
