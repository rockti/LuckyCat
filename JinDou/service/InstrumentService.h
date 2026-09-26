#pragma once

#include "model/Constants.h"
#include "service/Service.h"

class InstrumentService : public ServiceBase<IInstrumentObserver,IInstrumentService> {
	struct InstStruct {
		Instrument Inst;
		InvestorFee Fee;
		InvestorMargin Margin;
	};
public:
	virtual Instrument const& Add(Instrument const& inst) override;
	virtual void AddFee(TradePlatform platform, InvestorFee const& fee) override;
	virtual void AddMargin(TradePlatform platform, InvestorMargin const& margin) override;
	virtual void AddVarietyFeeByPresentInstrument(TradePlatform platform, type::VarietyCode const& varityCode, InvestorFee const& fee) override;
	virtual void AddVarietyMarginByPresentInstrument(TradePlatform platform, type::VarietyCode const& varityCode, InvestorMargin const& margin) override;
	virtual void AddVarietyFee(TradePlatform platform, type::VarietyCode const& varityCode, InvestorFee const& fee) override;
	virtual void AddVarietyMargin(TradePlatform platform, type::VarietyCode const& varityCode, InvestorMargin const& margin) override;
	virtual type::InstID toID(type::InstrumentCode const& instCode) override;
	virtual type::InstrumentCode const& toCode(type::InstID instID) override;
	virtual InstrumentResult Get(type::InstID instID) override;
	virtual InstrumentResult Get(type::InstrumentCode instCode) override;
	virtual FeeResult GetFee(type::InstID instID) override;
	virtual FeeResult GetFee(type::InstrumentCode instCode) override;
	virtual MarginResult GetMargin(type::InstID instID) override;
	virtual MarginResult GetMargin(type::InstrumentCode instCode) override;

	virtual InstrumentResults GetAll(TradePlatform platform) override;
	virtual InstrumentResults GetAll() override;
	virtual InstrumentResults GetSortedAll() override;
	virtual InstrumentResults GetSortedInstrumentsExcept(type::InstrumentCode const& code) override;
	virtual InstrumentResults GetHotInstruments(TradePlatform platform) override;


	virtual InstrumentResults GetAll(type::VarietyCode const& varietyCode) override;

private:
	Deque<InstStruct> insts_;
	std::map<type::VarietyCode, InvestorFee> varietyFees_;
	std::map<type::VarietyCode, InvestorMargin> varietyMargins_;
};
