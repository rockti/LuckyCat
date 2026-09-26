#include "pch.h"

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
	memcpy(data.Data, &command, data.Size);
	m_queues.PushStrategyCommand(data);
}

void UIPublisher::AddBusiness(Business* business) {
	StrategyCommand command;
	command.Type = StrategyCommand::ADD_BUSINESS;
	command.Business = business;

	Data data;
	data.Type = DataType::StrategyCommand;
	data.Size = sizeof(StrategyCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushStrategyCommand(data);
}

void UIPublisher::Login(ConnPlatform connPlatform, char const* password) {
	ConnCommand command;
	command.Type = ConnCommand::LOGIN;
	command.Platform = connPlatform;
	strcpy_s(command.Data, password);

	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushConnCommand(data);
}

void UIPublisher::SetVertificateCode(ConnPlatform connPlatform, char const* code) {
	ConnCommand command;
	command.Type = ConnCommand::SET_CODE;
	command.Platform = connPlatform;
	strcpy_s(command.Data, code);

	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushConnCommand(data);
}

void UIPublisher::QryFund(ConnPlatform connPlatform) {
	ConnCommand command;
	command.Type = ConnCommand::FUND_QRY;
	command.Platform = connPlatform;

	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushConnCommand(data);
}

void UIPublisher::QryPosition(ConnPlatform connPlatform) {
	ConnCommand command;
	command.Type = ConnCommand::POSITION_QRY;
	command.Platform = connPlatform;

	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushConnCommand(data);
}

void UIPublisher::SubscribeQuote(InstID instID) {
	Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);

	ConnCommand command;
	command.Type = ConnCommand::MARKET_SUBSCRIBE;
	command.Platform = Platform(instrument.GetExchange());
	strcpy_s(command.Data, instrument.GetInstCode().Str());
	
	Data data;
	data.Type = DataType::ConnCommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Data, &command, data.Size);
	m_queues.PushConnCommand(data);
}
