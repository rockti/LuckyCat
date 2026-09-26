#pragma once

 #include "model/Instrument.h"
#include <set>

struct VarietyConfig {
	char code[10];
	char minorCode[8];
	int multiple;
	money unitPrice;
	money lowerMargin;
	money higherMargin;
	money percentOpenCharge;
	money percentOffsetCharge;
	money percentOffsetTodayCharge;
	money fixedOpenCharge;
	money fixedOffsetCharge;
	money fixedOffsetTodayCharge;
	Exchange exchangeFlag;
};

typedef std::vector<VarietyConfig> VarietyConfigs;
typedef std::set<Exchange> Exchanges;


struct InstrumentFactory {
	static Instruments const& InitInstruments(VarietyConfigs const& VarityConfigs, Exchanges const& availableExchanges);
	static Instruments const& GetInstruments();
	static void Add(Variety const& variety, std::string const& yearMonth);
	static void Init();
};