#pragma once

#include "service/Service.h"

class TradeService : public ServiceBase<ITradeObserver,ITradeService> {
public:
	virtual InvestorTrade const& Add(InvestorTrade const& trade) override;

	void calcMarginAndFee(InvestorTrade& trade);

	virtual Trades GetTrades(type::OrderLocalID localID) const override;
	virtual Trades GetTrades(type::OrderSysID const& sysNo) const override;
	virtual TradeResult GetTrade(type::OrderSysID const& sysNo, type::TradeID const& tradeID) const override;
private:
	void notifyAdd(InvestorTrade trade);
	Deque<InvestorTrade> trades_;

	using TradeKey = std::pair<type::OrderSysID, type::TradeID>;

	using OrderSysIdIndex = MultipleIndex<type::OrderSysID, InvestorTrade>;
	using OrderLocalIdIndex = MultipleIndex<type::OrderLocalID, InvestorTrade>;
	using TradeIdIndex = SingleIndex<TradeKey, InvestorTrade>;
	OrderSysIdIndex sysIdIndex_;
	OrderLocalIdIndex localIdIndex_;
	TradeIdIndex tradeIdIndex_;

};