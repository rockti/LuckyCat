#pragma once
#include "conn/ExchangeConn.h"
#include "conn/ctp/CTPTraderHandler.h"
#include "conn/ctp/CTPMarketHandler.h"
#include "config/Config.h"


class CTPConnection : public ExchangeConn
{
public:
	CTPConnection(TradeSystemConfig& config, ConnPublisher& publisher);
	virtual void Init();
	virtual void Release();
	virtual void Login(type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
		type::InvestorID const& quoteInvestor, type::Password const& quotePassword);
	virtual void ReLogin(SubSystem subSystem) override;
	virtual void InsertOrder(OrderInsertRequest const& order);
	virtual void CancelOrder(OrderCancelRequest const& action);
	virtual void SubscribeQuote(type::InstrumentCode const& instCode);
	virtual void UnSubscribeQuote(type::InstrumentCode const& instCode);
	virtual void QueryAccount();
	virtual void QueryOrders();
	virtual void QueryTrades();
	virtual void QueryPositions();
	virtual void QueryInstruments();;
	virtual void QueryFees() {};
	virtual void QueryMargins() {};

	virtual void QueryFee(type::InstrumentCode const& instCode);
	virtual void QueryMargin(type::InstrumentCode const& instCode);
	virtual void QueryInstrument(type::InstrumentCode const& instCode);

	virtual void SetVertificateCode(char const* password) { throw std::logic_error("CTP can not set VeritificateCode"); }


	virtual void ChangePassword(type::Password const& oldPassword, type::Password const& newPassword) override;

private:
	void initTradeApi();
	void initQuoteApi();
	void loginQuote(const ztp::type::InvestorID& tradeInvestor, const ztp::type::Password& tradePassword);

private:
	TradeSystemConfig& m_config;
	std::string m_traderUrl;
	std::string m_marketUrl;

	CTPTraderHandler m_traderHandler;
	CTPMarketHandler m_marketHandler;

	CThostFtdcTraderApi* m_pTraderApi;
	CThostFtdcMdApi* m_pMdApi;
	type::InvestorID m_investor;
	type::Password m_password;

	ConnPublisher& m_publisher;
};
