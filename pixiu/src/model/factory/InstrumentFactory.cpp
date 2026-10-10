#include "pch.h"
#include "model/factory/InstrumentFactory.h"
#include <chrono>
#include "service/PositionService.h"
#include "config/Config.h"

//	{code, minor, multiple, unitprice, lowerMargin, higherMargin, ..., exchange }
static VarietyConfigs s_varietyConfigs = {
// 	{"Ag(T+D)", "", 1, 1, 0.13, 0.13, 0.13, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},				// 白银T+D（1手=1000克，最大10000手）
	{"ag", "2701", 15, 1.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 白银（报价15千克/手）
// 	{"al", "2701", 5, 5.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 铝		
// 	{"Au(T+D)", "", 1000, 0.01, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},			// 黄金T+D（1手=1000克，最大200手）
	{"au", "2701", 1000, 0.02, 0.09, 0.09, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 黄金（报价1000克/手）
// 	{"Au(T+N1)", "", 100, 0.05, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},			// 黄金T+D（1手=100克，最大2000手）
// 	{"Au(T+N2)", "", 100, 0.05, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},			// 黄金T+D（1手=100克，最大2000手）
// 	{"Au99.99", "", 10, 0.01, 1, 1, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},					// 黄金99.99（1手=10克/手，最大50000手）
// 	{"bu", "2701", 10, 2.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 沥青	
	{"cu", "2701", 5, 10.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 铜
// 	{"hc", "2701", 10, 1.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 热卷
// 	{"mAu(T+D)", "", 100, 0.01, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},			// 迷你黄金T+D（1手=100克，最大2000手）
// 	{"NYAuTN06", "", 100, 0.05, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},			// 沪纽金06（1手=100克，最大2000手）
// 	{"NYAuTN12", "", 100, 0.05, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SGE},			// 沪纽金12（1手=100克，最大2000手）
// 	{"ni", "2701", 1, 10.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 镍
// 	{"nr", "2701", 10, 5.00, 0.10, 0.08, 0.00, 0.00, 0.00, 3.01, 3.01, 0.00, Exchange::INE},			// 20号胶
// 	{"pb", "2701", 5, 5.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 铅
// 	{"rb", "2701", 10, 1.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 螺纹钢
// 	{"ru", "2701", 10, 5.00, 0.08, 0.08, 0.00, 0.00, 0.00, 3.01, 3.01, 0.00, Exchange::SHFE},			// 天然胶
// 	{"sp", "2701", 10, 2.00, 0.11, 0.11, 0.000051, 0.000051, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},	// 纸浆
// 	{"sn", "2701", 1, 10.00, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 锡
// 	{"ss", "2701", 5, 5.00, 0.11, 0.11, 0.00, 0.00, 0.00, 2.01, 2.01, 0.00, Exchange::SHFE},			// 不锈钢
// 	{"zn", "2701", 5, 5.00, 0.09, 0.09, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::SHFE},			// 锌

// 	{"IC", "2701", 200, 0.2, 0.15, 0.15, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},			// 中证500
// 	{"IF", "2701", 300, 0.2, 0.13, 0.13, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},			// 沪深300	
// 	{"IH", "2701", 300, 0.2, 0.13, 0.13, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},			// 上证50	
// 	{"IM", "2701", 200, 0.2, 0.13, 0.13, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},			// 中证1000	
// 	{"T", "2701", 10000, 0.005, 0.03, 0.03, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},		// 10年国债
// 	{"TF", "2701", 10000, 0.005, 0.012, 0.012, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},	// 5年国债
// 	{"TL", "2701", 10000, 0.01, 0.035, 0.035, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX},		// 30年国债
// 	{"TS", "2701", 20000, 0.002, 0.01, 0.01, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::CFFEX}		// 2年国债

// 	{"CUS", "1701", 100000, 0.0001, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::HKEX},	//人民币美元汇率
// 	{"GC", "1701", 3000, 0.1, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::HKEX},			//COMEX美元黄金
// 	{"GDR", "1701", 1000, 0.05, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::HKEX},		//人民币黄金
// 	{"GDU", "1701", 1000, 0.01, 0.10, 0.10, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, Exchange::HKEX},		//美元黄金
};


void fillMarginAndFee(Instrument& instrument, VarietyConfig const& config) {
	instrument.SetMargin(config.higherMargin);

	Charge fixedOpen(Charge::FIXED, config.fixedOpenCharge);
	Charge percentOpen(Charge::PERCENT, config.percentOpenCharge / 100);
	ChargeCombination openCombination;
	openCombination.push_back(fixedOpen);
	openCombination.push_back(percentOpen);
	instrument.SetOperationFee(Operation::OPEN, openCombination);

	Charge fixedOffset(Charge::FIXED, config.fixedOffsetCharge);
	Charge percentOffset(Charge::PERCENT, config.percentOffsetCharge / 100);
	ChargeCombination offsetCombination;
	offsetCombination.push_back(fixedOffset);
	offsetCombination.push_back(percentOffset);
	instrument.SetOperationFee(Operation::OFFSET, offsetCombination);

	Charge fixedOffsetToday(Charge::FIXED, config.fixedOffsetTodayCharge);
	Charge percentOffsetToday(Charge::PERCENT, config.percentOffsetTodayCharge / 100);
	ChargeCombination offsetTodayCombination;
	offsetTodayCombination.push_back(fixedOffsetToday);
	offsetTodayCombination.push_back(percentOffsetToday);
	instrument.SetOperationFee(Operation::OFFSET_TODAY, offsetTodayCombination);
}

using namespace date;
void CreateInstruments(VarietyConfig const& config, Instruments& instruments) {
	auto today = year_month_day(floor<days>(std::chrono::system_clock::now()));
	Variety variety(config.exchangeFlag, config.code, config.unitPrice, config.multiple, today);

	const auto shouldEnableTodayOffsetPenalty = ConfigLoader::Get().Rule.OffsetPunishmentVarieties.count(config.code) > 0;
	for (const auto& code : variety.GetInstrumentCodes()) {
		const auto instID = InstIDGen(variety.GetExchange(), static_cast<int>(instruments.Count()));

		auto& positionStore = pos::GlobalPositionStore::Instance();
		positionStore.InitInstrument(instID);

		if (shouldEnableTodayOffsetPenalty) {
			positionStore.EnableTodayOffsetPenalty(instID);
		}

		Instrument instrument(instID, variety, code);
		fillMarginAndFee(instrument, config);
		instruments.Add(instrument);
	}
}	// Optimized

static Instruments s_instruments;
static Varieties s_varieties;

Instruments const& InstrumentFactory::InitInstruments(VarietyConfigs const& VarityConfigs, Exchanges const& availableExchanges) {
	for each (VarietyConfig varietyConfig in VarityConfigs)	{
		if (availableExchanges.find(varietyConfig.exchangeFlag) != availableExchanges.end()) {
			CreateInstruments(varietyConfig, s_instruments);
		}
	}
	return s_instruments;
}

Instruments const& InstrumentFactory::GetInstruments() {
	return s_instruments;
}

void InstrumentFactory::Add(Variety const& variety, std::string const& yearMonth) {
	InstID instID = InstIDGen(variety.GetExchange(), (int)s_instruments.Count());
	Instrument instrument(instID, variety, yearMonth);
	s_instruments.Add(instrument);
}

#include "service/QuoteService.h"
void InstrumentFactory::Init() {
	Exchanges exchanges;
	exchanges.insert(Exchange::CFFEX);
	exchanges.insert(Exchange::SHFE);
	exchanges.insert(Exchange::SGE);

	Instruments const& instruments = InitInstruments(s_varietyConfigs, exchanges);
	for each (auto instrument in instruments.GetAll()) {
		InstID instID = instrument.GetID();
		Quote quote(instID, &instruments.Get(instID), SimpleQuoteArray(), SimpleQuoteArray(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, SimpleQuote(), SimpleQuote());
		WritableQuoteService().Initialize(quote);
	}
}
