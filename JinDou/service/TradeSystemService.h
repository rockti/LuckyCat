#pragma once

#include "model/Constants.h"
#include "service/Service.h"

class QueryManager;

struct TPStatus {
	bool QuoteConnected = false;
	bool QuoteLogined = false;
	bool TradeConnected = false;
	bool TradeLogined = false;

	inline bool IsConnected() const {
		return QuoteConnected && TradeConnected;
	}
};
class TradeSystemService : public ServiceBase2<IExchangeStatusObserver,ITradeSystemObserver,ITradeSystemService> {

public:
	TradeSystemService();
	virtual void Login(TradePlatform platform, type::InvestorID const& investor, type::Password const& password) override;
	virtual void Login(TradePlatform platform, type::InvestorID const& tradeInvestor, type::Password const& tradePassword, 
		type::InvestorID const& quoteInvestor, type::Password const& quotePassword) override;
	virtual void ReLogin(TradePlatform platform, SubSystem subSystem) override;
	virtual bool IsConnected(TradePlatform platform, SubSystem subSystem) const override;
	virtual bool IsLogined(TradePlatform platform, SubSystem subSystem) const override;
	virtual void ChangePassword(TradePlatform platform, type::Password const& oldPassword, type::Password const& newPassword) override;

	virtual void Insert(OrderInsertRequest const& order) override;
	virtual void Cancel(OrderCancelRequest const& action) override;
	virtual void SubscribeQuote(type::Exchange exchange, type::InstrumentCode const& code) override;
	virtual void RunQuery() override;

	virtual void QueryInstruments(TradePlatform platform) override;
	virtual void QueryAccount(TradePlatform platform) override;
	virtual void QueryPositions(TradePlatform platform) override;
	virtual void QueryOrders(TradePlatform platform) override;
	virtual void QueryTrades(TradePlatform platform) override;
	virtual void QueryFees(TradePlatform platform) override;
	virtual void QueryMargins(TradePlatform platform) override;	
	virtual void QueryFee(TradePlatform platform, type::InstrumentCode const& code) override;
	virtual void QueryMargin(TradePlatform platform, type::InstrumentCode const& code) override;


	virtual void OnConnected(TradePlatform platform, SubSystem subSystem) override;
	virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) override;
	virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) override;
	virtual void OnLogout(TradePlatform platform, SubSystem subSystem) override;
	virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) override;
	virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) override;

	virtual void OnStatus(type::Exchange, ExchangeStatus status) override;
	virtual void OnOrderError(OrderErrorNotification const& en) override;
	virtual void OnError(ErrorNotification const& en) override;

	virtual type::TradingDay const& GetTradingDay() const override;
	virtual void SetTradingDay(type::TradingDay const& tradingDay) override {
		tradingDay_ = tradingDay;
	}
private:
	QueryManager* getQueryManager(TradePlatform platform);
	std::array<TPStatus, 10> tpStatuses_ = {};
	std::map<type::Exchange, ExchangeStatus> exchangeStatuses_;

	std::shared_ptr<QueryManager> ctpQueryManager_;
	std::shared_ptr<QueryManager> esunnyQueryManager_;
	type::TradingDay tradingDay_;
};