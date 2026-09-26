#include "runner/ConnRunner.h"
#include "conn/ExchangeConn.h"
#include "model/dealing/ExchangeRequest.h"
#include "notify/Notify.h"
#include "model/dealing/MarketingDealing.h"
#include "service/Service.h"

void ConnRunnerHandler::Handle(Data const& data) {
	switch (data.Type) {
	case DataType::OrderInsert: {
		OrderInsertRequest const& insertRequest = *(OrderInsertRequest*)data.Payload;
		auto platform = ServiceSinglton::Services().Exchange->toPlatform(insertRequest.Exchange);
		auto& conn = *m_conns[int(platform)];
		conn.InsertOrder(insertRequest);
		break;
	}
	case DataType::OrderCancel: {
		OrderCancelRequest const& action = *(OrderCancelRequest*)data.Payload;
		auto platform = ServiceSinglton::Services().Exchange->toPlatform(action.Exchange);
		auto& conn = *m_conns[int(platform)];
		conn.CancelOrder(action);
		break;
	}
	case DataType::APICommand: {
		ConnCommand const& command = *(ConnCommand*)data.Payload;
		onCommand(command);
		break;
	}
	default:
		throw ILLEGAL_MESSAGE_TYPE;
	}
}

void ConnRunnerHandler::onCommand(ConnCommand const& command) {
	switch (command.Type) {
	case ConnCommand::LOGIN: {
		LoginCommand& loginCmd = *(LoginCommand*)command.Data;
		m_conns[PlatformIndex(command.Platform)]->Login(loginCmd.TradeInvestor,loginCmd.TradePassword,loginCmd.QuoteInvestor,loginCmd.QuotePassword);
		break;
	}
	case ConnCommand::RELOGIN: {
		m_conns[PlatformIndex(command.Platform)]->ReLogin(command.SubSystem);
		break;
	}
	case ConnCommand::LOGOUT: {
		m_conns[PlatformIndex(command.Platform)]->Release();
		break;
	}
	case ConnCommand::CHANGE_PASSWORD: {
		ChangePasswordCommand& cmd = *(ChangePasswordCommand*)command.Data;
		m_conns[PlatformIndex(command.Platform)]->ChangePassword(cmd.OldPassword, cmd.NewPassword);
		break;
	}
	case ConnCommand::POSITION_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryPositions();
		break;
	}
	case ConnCommand::ORDER_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryOrders();
		break;
	}
	case ConnCommand::TRADE_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryTrades();
		break;
	}
	case ConnCommand::MARKET_SUBSCRIBE: {
		type::InstrumentCode instCode((char const*)(command.Data));
		m_conns[PlatformIndex(command.Platform)]->SubscribeQuote(instCode);
		break;
	}
	case ConnCommand::INSTRUMENT_QRY: {
		type::InstrumentCode inst{ command.Data };
		if (inst.empty()) {
			m_conns[PlatformIndex(command.Platform)]->QueryInstruments();
		} else {
			m_conns[PlatformIndex(command.Platform)]->QueryInstrument(inst);
		}
		break;
	}
	case ConnCommand::ACCOUNT_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryAccount();
		break;
	}
	case ConnCommand::FEE_QRY: {
		type::InstrumentCode inst{ command.Data };
		if (inst.empty()) {
			m_conns[PlatformIndex(command.Platform)]->QueryFees();
		} else {
			m_conns[PlatformIndex(command.Platform)]->QueryFee(inst);
		}
		break;
	}
	case ConnCommand::MARGIN_QRY: {
		type::InstrumentCode inst{ command.Data };
		if (inst.empty()) {
			m_conns[PlatformIndex(command.Platform)]->QueryMargins();
		} else {
			m_conns[PlatformIndex(command.Platform)]->QueryMargin(inst);
		}
		break;
	}
	default:
		throw ILLEGAL_MESSAGE_TYPE;
		break;
	}
}
