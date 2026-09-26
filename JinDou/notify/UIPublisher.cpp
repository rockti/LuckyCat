
#include "notify/UIPublisher.h"
#include "notify/Notify.h"
#include "runner/Queues.h"
#include "model/factory/InstrumentFactory.h"

bool UIPublisher::Pop(Data& data) {
	return m_queues.Pop(data);
}

void UIPublisher::Push(StrategyCommand const& command) {
	Data data;
	data.Type = DataType::StrategyCommand;
	data.Size = sizeof(StrategyCommand);
	memcpy(data.Payload, &command, data.Size);
	m_queues.PushStrategyCommand(data);
}

void UIPublisher::AddBusiness(Business* business) {
	StrategyCommand command;
	command.Type = StrategyCommand::ADD_BUSINESS;
	command.Business = business;
	Data data;
	data.Type = DataType::StrategyCommand;
	data.Size = sizeof(StrategyCommand);
	memcpy(data.Payload, &command, data.Size);
	m_queues.PushStrategyCommand(data);
}

void UIPublisher::Login(TradePlatform connPlatform, char const* password) {
	ConnCommand command;
	command.Type = ConnCommand::LOGIN;
	command.Platform = connPlatform;
	strcpy_s(command.Data, password);

	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);
	m_queues.PushConnCommand(data);
}

void UIPublisher::SubscribeQuote(type::InstID instID) {
	Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);

	ConnCommand command;
	command.Type = ConnCommand::MARKET_SUBSCRIBE;
	command.Platform = Platform(instrument.Exchange);
	strcpy_s(command.Data, instrument.Code.value());
	
	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);
	m_queues.PushConnCommand(data);
}
