#pragma once

#include "service/Service.h"

class PositionService : public ServiceBase<IPositionObserver,IPositionService> {
public:
	virtual InvestorPosition const& InitAdd(InvestorPosition const& position) override;
	virtual PositionResult Get(type::InstID inst, type::Direction direction) const override;
	virtual bool Has(type::InstID inst, type::Direction direction) const override;
	virtual InstrumentPositionResult Get(type::InstID inst) override;
	virtual Positions All() const override;
	virtual void ClearAll() override;
	virtual void ClearPlatform(TradePlatform platform) override;

	virtual void OnQuote(Quote const& quote) override;

	virtual void OnTrade(InvestorTrade const& trade) override;
	virtual void OnOrderRtn(InvestorOrder const& order) override;
	virtual void OnInitOrder(InvestorOrder const& order) override;
	virtual void FreezePosition(InvestorOrder const& order) override;
	virtual void UnfreezePosition(InvestorOrder const& order) override;
private:
	void recalc(InstrumentPosition& pos, Quote const& quote);
	double sum_position_profit(TradePlatform platform);
	double calcProfit(type::Direction dir, InvestorPosition position, SimpleQuote const& quote);
	double calcMargin(InvestorPosition const& pos);
	InvestorPosition& fillByTrade(InstrumentPosition& pos, InvestorTrade const& trade);

	void calcMarginAndNotifyUpdate(InvestorPosition& pos);
	void notifyDelete(InvestorPosition const& pos);
	void notifyProfit(TradePlatform platform, double positionProfit);
	void notifyUpdate(InvestorPosition const& pos);

	void onOffsetTrade(InvestorTrade const& trade);
	void onNoOffsetTrade(InvestorTrade const& trade);
private:
	std::map<type::InstrumentCode, InstrumentPosition> positions_;
};


