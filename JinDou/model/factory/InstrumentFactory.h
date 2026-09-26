#pragma once

 #include "model/Instrument.h"
#include <set>
#include "type/ZTPType.h"

using namespace ztp;
// 
// struct VarietyConfig
// {
// 	char code[8];
// 	char minorCode[8];
// 	int multiple;
// 	type::Money unitPrice;
// 	type::Money lowerMargin;
// 	type::Money higherMargin;
// 	type::Money percentOpenCharge;
// 	type::Money percentOffsetCharge;
// 	type::Money percentOffsetTodayCharge;
// 	type::Money fixedOpenCharge;
// 	type::Money fixedOffsetCharge;
// 	type::Money fixedOffsetTodayCharge;
// 	Exchange exchangeFlag;
// };
// 
// typedef std::vector<VarietyConfig> VarietyConfigs;
// typedef std::set<Exchange> Exchanges;
// 
// 
struct InstrumentFactory {

// 	static Instruments const& InitInstruments(VarietyConfigs const& VarityConfigs, Exchanges const& availableExchanges);
	static Instruments const& GetInstruments();
	static void Add(Instrument const& inst);

};