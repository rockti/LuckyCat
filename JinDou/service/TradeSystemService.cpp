
#include "service/TradeSystemService.h"
#include "runner/Framework.h"
#include "notify/StrategyPublisher.h"
#include "notify/Notify.h"
#include "QElapsedTimer.h"
#include "service/InstrumentProgressManager.h"
#include "utils/Log.h"
//#include <date/date.h>
#include <queue>

//using namespace date;

const int CTP_RATE_LIMIT = 1000;
// CTP and ESunny has a limitation of query request. 
// You can only sent 1 query request per second. we need to manager it.
class QueryManager {
	enum class QueryType {
		Instrument,
		InstrumentMargin,
		InstrumentFee,
		Account,
		Position,
		Order,
		Trade,
	};

	struct QueryTask {
		QueryType Type = QueryType::Account;
		type::InstrumentCode InstCode;
	};
public:
	QueryManager(TradePlatform platform, int interval) :platform_(platform), interval_(interval) {

	}
	void QueryAccount() {
		auto type = QueryType::Account;
		auto iter = taskIndex_.find(toQueryKey(type));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type };
			queryTasks_.push(task);
		}
	}
	void QueryPosition() {
		auto type = QueryType::Position;
		auto iter = taskIndex_.find(toQueryKey(type));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type };
			queryTasks_.push(task);
		}
	}
	void QueryOrder() {
		auto type = QueryType::Order;
		auto iter = taskIndex_.find(toQueryKey(type));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type };
			queryTasks_.push(task);
		}
	}
	void QueryTrade() {
		auto type = QueryType::Trade;
		auto iter = taskIndex_.find(toQueryKey(type));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type };
			queryTasks_.push(task);
		}
	}
	void QueryInstrument(type::InstrumentCode const& instCode) {
		auto type = QueryType::Instrument;
		auto iter = taskIndex_.find(toQueryKey(type, instCode));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type, instCode };
			queryTasks_.push(task);
		}
	}
	void QueryMargin(type::InstrumentCode const& instCode) {
		auto type = QueryType::InstrumentMargin;
		auto iter = taskIndex_.find(toQueryKey(type,instCode));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type, instCode };
			queryTasks_.push(task);
		}
	}
	void QueryFee(type::InstrumentCode const& instCode) {
		auto type = QueryType::InstrumentFee;
		auto iter = taskIndex_.find(toQueryKey(type, instCode));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type, instCode };
			queryTasks_.push(task);
		}
	}

	void QueryInstruments() {
		auto type = QueryType::Instrument;
		auto iter = taskIndex_.find(toQueryKey(type));
		if (iter == taskIndex_.end()) {
			QueryTask task{ type };
			queryTasks_.push(task);
		}
	}

	void QueryFees() {

		if (platform_ == TradePlatform::CTP) {
			auto instService = ServiceSinglton::Services().Instrument;
			auto hotInsts = instService->GetHotInstruments(TradePlatform::CTP);
			for (auto& inst : hotInsts) {
				QueryFee(inst.Code);
			}
		} else {
			QueryFee(type::InstrumentCode{ "" });
		}
	}

	void QueryMargins() {
		if (platform_ == TradePlatform::CTP) {
			auto instService = ServiceSinglton::Services().Instrument;
			auto hotInsts = instService->GetHotInstruments(TradePlatform::CTP);
			for (auto& inst : hotInsts) {
				auto platform = ServiceSinglton::Services().Exchange->toPlatform(inst.Exchange);
				QueryMargin(inst.Code);
			}
		} else {
			QueryMargin(type::InstrumentCode{ "" });
		}
	}

	void RunOnce() {
		if (elapsedTimer_.hasExpired(interval_)) {
			if (!queryTasks_.empty()) {
				auto& task = queryTasks_.front();
				runTask(task);
				queryTasks_.pop();
				elapsedTimer_.start();
				taskIndex_.erase(toQueryKey(task.Type, task.InstCode));
			}
		}
	}

private:
	QElapsedTimer elapsedTimer_;
	void query(ConnCommand::TYPE type, type::InstrumentCode const& instCode) {
		auto& publisher = Framework::Instance().GetStrategyPublisher();
		ConnCommand command;
		command.Type = type;
		command.Platform = platform_;
		if (!instCode.empty()) {
			strcpy_s(command.Data, instCode.value());
		}

		Data data;
		data.Type = DataType::APICommand;
		data.Size = sizeof(ConnCommand);
		memcpy(data.Payload, &command, data.Size);
		publisher.PushToConn(data);
	}
	void runTask(QueryTask& task) {
		switch (task.Type) {
		case QueryType::Instrument:
			query(ConnCommand::INSTRUMENT_QRY, task.InstCode);
			break;
		case QueryType::InstrumentMargin:
			query(ConnCommand::MARGIN_QRY, task.InstCode);
			break;
		case QueryType::InstrumentFee:
			query(ConnCommand::FEE_QRY, task.InstCode);
			break;
		case QueryType::Account:
			query(ConnCommand::ACCOUNT_QRY, task.InstCode);
			break;
		case QueryType::Position:
			query(ConnCommand::POSITION_QRY, task.InstCode);
			break;
		case QueryType::Order:
			query(ConnCommand::ORDER_QRY, task.InstCode);
			break;
		case QueryType::Trade:
			query(ConnCommand::TRADE_QRY, task.InstCode);
			break;
		default:
			break;
		}
	}
	std::string toQueryKey(QueryType type, type::InstrumentCode const& instCode = type::InstrumentCode("")) {
		switch (type) {
		case QueryType::Instrument:
			if (instCode.empty()) {
				return "Instrument";
			} else {
				return fmt::format("I:{}", instCode);
			}
		case QueryType::InstrumentMargin:
			if (instCode.empty()) {
				return "Margin";
			} else {
				return fmt::format("M:{}", instCode);
			}
		case QueryType::InstrumentFee:
			if (instCode.empty()) {
				return "Fee";
			} else {
				return fmt::format("F:{}", instCode);
			}
		case QueryType::Account:
			return "Account";
		case QueryType::Position:
			return "Position";
		case QueryType::Order:
			return "Order";
		case QueryType::Trade:
			return "Trade";
		default:
			return "Account";
		}
	}
	TradePlatform platform_;
	int interval_;
	std::queue<QueryTask> queryTasks_;
	std::set<std::string> taskIndex_;

};


void query(QueryManager* queryManager, StrategyPublisher& publisher, TradePlatform platform, ConnCommand::TYPE type, type::InstrumentCode const& code = type::InstrumentCode("")) {

	if (queryManager) {
		switch (type) {
		case ConnCommand::ORDER_QRY:
			queryManager->QueryOrder();
			break;
		case ConnCommand::TRADE_QRY:
			queryManager->QueryTrade();
			break;
		case ConnCommand::POSITION_QRY:
			queryManager->QueryPosition();
			break;
		case ConnCommand::INSTRUMENT_QRY:
			queryManager->QueryInstruments();
			break;
		case ConnCommand::FEE_QRY:
			if (code != "") {
				queryManager->QueryFee(code);
			} else {
				queryManager->QueryFees();
			}
			break;
		case ConnCommand::MARGIN_QRY:
			if (code != "") {
				queryManager->QueryMargin(code);
			}
			else {
				queryManager->QueryMargins();
			}
			break;
		case ConnCommand::ACCOUNT_QRY:
			queryManager->QueryAccount();
			break;
		default:
			break;
		}
		return;
	}

	ConnCommand command;
	command.Type = type;
	command.Platform = platform;

	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);
	publisher.PushToConn(data);
}

TradeSystemService::TradeSystemService() {
	ctpQueryManager_.reset(new QueryManager(TradePlatform::CTP, CTP_RATE_LIMIT));
	esunnyQueryManager_.reset(new QueryManager(TradePlatform::HKEX_ESUNNY, CTP_RATE_LIMIT));
}

void TradeSystemService::Login(TradePlatform platform, type::InvestorID const& investor, type::Password const& password) {
	auto& publisher= Framework::Instance().GetStrategyPublisher();
	ConnCommand command;
	command.Type = ConnCommand::LOGIN;
	command.Platform = platform;

	LoginCommand loginCmd;
	loginCmd.TradeInvestor = investor;
	loginCmd.TradePassword = password;
	loginCmd.QuoteInvestor = investor;
	loginCmd.QuotePassword = password;
	memcpy(command.Data, &loginCmd, sizeof(LoginCommand));

	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);
	publisher.PushToConn(data);
}

void TradeSystemService::Login(TradePlatform platform, type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
	type::InvestorID const& quoteInvestor, type::Password const& quotePassword) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	ConnCommand command;
	command.Type = ConnCommand::LOGIN;
	command.Platform = platform;

	LoginCommand loginCmd;
	loginCmd.TradeInvestor = tradeInvestor;
	loginCmd.TradePassword = tradePassword;
	loginCmd.QuoteInvestor = quoteInvestor;
	loginCmd.QuotePassword = quotePassword;
	memcpy(command.Data, &loginCmd, sizeof(LoginCommand));

	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);
	publisher.PushToConn(data);
}

void TradeSystemService::ReLogin(TradePlatform platform, SubSystem subSystem) {
	ConnCommand command;
	command.Type = ConnCommand::RELOGIN;
	command.Platform = platform;
	command.SubSystem = subSystem;

	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);

	auto& publisher = Framework::Instance().GetStrategyPublisher();
	publisher.PushToConn(data);
}

bool TradeSystemService::IsConnected(TradePlatform platform, SubSystem subSystem) const {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		return tpStatus.QuoteConnected;
	} else {
		return tpStatus.TradeConnected;
	}
}

bool TradeSystemService::IsLogined(TradePlatform platform, SubSystem subSystem) const {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		return tpStatus.QuoteLogined;
	} else {
		return tpStatus.TradeLogined;
	}
}

void TradeSystemService::ChangePassword(TradePlatform platform, type::Password const& oldPassword, type::Password const& newPassword) {
	ConnCommand command;
	command.Type = ConnCommand::CHANGE_PASSWORD;
	command.Platform = platform;
	ChangePasswordCommand cmd;
	cmd.OldPassword = oldPassword;
	cmd.NewPassword = newPassword;
	memcpy(command.Data, &cmd, sizeof(cmd));

	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);

	auto& publisher = Framework::Instance().GetStrategyPublisher();
	publisher.PushToConn(data);
}

void TradeSystemService::Insert(OrderInsertRequest const& order) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	publisher.PushOrder(order);
}

void TradeSystemService::Cancel(OrderCancelRequest const& action) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	publisher.PushAction(action);
}

void TradeSystemService::SubscribeQuote(type::Exchange exchange, type::InstrumentCode const& code) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	ConnCommand command;
	command.Type = ConnCommand::MARKET_SUBSCRIBE;
	command.Platform = ServiceSinglton::Services().Exchange->toPlatform(exchange);
	strcpy_s(command.Data, code.value());
	Data data;
	data.Type = DataType::APICommand;
	data.Size = sizeof(ConnCommand);
	memcpy(data.Payload, &command, data.Size);
	publisher.PushToConn(data);

}


void TradeSystemService::RunQuery() {
	ctpQueryManager_->RunOnce();
	esunnyQueryManager_->RunOnce();
}

void TradeSystemService::QueryInstruments(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::INSTRUMENT_QRY);
}

void TradeSystemService::QueryAccount(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::ACCOUNT_QRY);
}

void TradeSystemService::QueryPositions(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::POSITION_QRY);
}

void TradeSystemService::QueryOrders(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::ORDER_QRY);
}

void TradeSystemService::QueryTrades(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::TRADE_QRY);
}


void TradeSystemService::QueryFees(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::FEE_QRY);
}

void TradeSystemService::QueryMargins(TradePlatform platform) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::MARGIN_QRY);
}


void TradeSystemService::QueryFee(TradePlatform platform, type::InstrumentCode const& code) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::FEE_QRY, code);
}

void TradeSystemService::QueryMargin(TradePlatform platform, type::InstrumentCode const& code) {
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	query(getQueryManager(platform), publisher, platform, ConnCommand::MARGIN_QRY, code);
}



void TradeSystemService::OnConnected(TradePlatform platform, SubSystem subSystem) {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		if (!tpStatus.QuoteConnected && ServiceSinglton::Services().Init->GetPercent(platform) == 100) {
			//relogin if quote connection is disconnected
			ReLogin(platform, subSystem);
		}
		tpStatus.QuoteConnected = true;
	} else {
		tpStatus.TradeConnected = true;;
	}
	for (auto& observer : observers2_) {
		observer->OnConnected(platform, subSystem);
	}
}

void TradeSystemService::OnDisconnected(TradePlatform platform, SubSystem subSystem) {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		tpStatus.QuoteConnected = false;
	} else {
		tpStatus.TradeConnected = false;;
	}
	for (auto& observer : observers2_) {
		observer->OnDisconnected(platform, subSystem);
	}
}

void TradeSystemService::OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		tpStatus.QuoteLogined = true;
	} else {
		tpStatus.TradeLogined = true;
	}
	for (auto& observer : observers2_) {
		observer->OnLogin(platform, subSystem, tradingDay);
	}
}

void TradeSystemService::OnLogout(TradePlatform platform, SubSystem subSystem) {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		tpStatus.QuoteLogined = false;
	} else {
		tpStatus.TradeLogined = false;
	}
}
void TradeSystemService::OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) {
	auto& tpStatus = tpStatuses_[int(platform)];
	if (subSystem == SubSystem::Quote) {
		tpStatus.QuoteLogined = false;
	}
	else {
		tpStatus.TradeLogined = false;
	}
	for (auto& observer : observers2_) {
		observer->OnLoginFailed(platform, subSystem, error);
	}
}

void TradeSystemService::OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) {
	for (auto& observer : observers2_) {
		observer->OnPasswordChange(platform, SubSystem::Trade, error);
	}
}

void TradeSystemService::OnStatus(type::Exchange exchange, ExchangeStatus status) {
	exchangeStatuses_[exchange] = status;
	for (auto& observer : observers_) {
		observer->OnExchangeStatus(exchange, status);
	}
}

void TradeSystemService::OnOrderError(OrderErrorNotification const& en) {
	for (auto& observer : observers2_) {
		observer->OnOrderError(en);
	}
}

void TradeSystemService::OnError(ErrorNotification const& en) {
	for (auto& observer : observers2_) {
		observer->OnError(en);
	}
}

type::TradingDay const& TradeSystemService::GetTradingDay() const {
	return tradingDay_;
}

QueryManager* TradeSystemService::getQueryManager(TradePlatform platform)
{
	QueryManager* queryManager = nullptr;
	if (platform == TradePlatform::CTP) {
		queryManager = ctpQueryManager_.get();
	} else if (platform == TradePlatform::HKEX_ESUNNY) {
		queryManager = esunnyQueryManager_.get();
	}
	return queryManager;
}

