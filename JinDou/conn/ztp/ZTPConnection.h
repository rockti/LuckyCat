#pragma once
#include "conn/ExchangeConn.h"
#include "conn/ztp/ZTPTraderHandler.h"
#include "conn/ztp/ZTPMarketHandler.h"
#include "config/Config.h"


class ZTPConnection : public ExchangeConn
{
public:
	ZTPConnection(TradeSystemConfig& config, ConnPublisher& publisher);
	virtual void Init() override;
	virtual void Release() override;
	virtual void Login(type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
		type::InvestorID const& quoteInvestor, type::Password const& quotePassword) override;
	virtual void ReLogin(SubSystem subSystem) override;
	virtual void InsertOrder(OrderInsertRequest const& order) override;
	virtual void CancelOrder(OrderCancelRequest const& action) override;
	virtual void SubscribeQuote(type::InstrumentCode const& instCode) override;
	virtual void UnSubscribeQuote(type::InstrumentCode const& instCode) override;
	virtual void QueryInstruments() override;
	virtual void QueryAccount() override;
	virtual void QueryOrders() override;
	virtual void QueryTrades() override;
	virtual void QueryPositions() override;
	virtual void QueryFees() override;
	virtual void QueryMargins() override;

	virtual void QueryInstrument(type::InstrumentCode const& instCode) override;
	virtual void QueryFee(type::InstrumentCode const& instCode) override;
	virtual void QueryMargin(type::InstrumentCode const& instCode) override;


	virtual void SetVertificateCode(char const* password) { throw std::logic_error("CTP can not set VeritificateCode"); }
	virtual void InsertMarketingOrder(MarketingOrderInfo const& dealing) { throw std::logic_error("CTP not support marketing order"); }
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel) { throw std::logic_error("CTP not support marketing order"); }


	virtual void ChangePassword(type::Password const& oldPassword, type::Password const& newPassword) override;

private:
	void initTradeApi();
	void initQuoteApi();
	void loginQuote();

private:
	TradeSystemConfig& m_config;
	std::string m_traderUrl;
	std::string m_marketUrl;

	ZTPTraderHandler m_traderHandler;
	ZTPMarketHandler m_marketHandler;

	CZtpFtdTradeApi* m_pTraderApi;
	CZtpFtdMdApi* m_pMdApi;

	ConnPublisher& m_publisher;
	type::InvestorID m_investor;
	type::Password m_password;
};
