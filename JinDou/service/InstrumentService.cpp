
#include "InstrumentService.h"


namespace {
	using namespace date;
	using namespace std;
	const int MONTH_COUNT = 12;
#pragma warning(push)
#pragma warning(disable:4455)
	constexpr months operator"" m(unsigned long long val) { // return integral hours
		return months(val);
	}
#pragma warning(pop)


	year_month_day nth_friday(year_month const& ym, int nth) {
		return year_month_day{ Friday[nth] / ym.month() / ym.year() };
	}

	bool is_season(year_month const& ym) {
		return static_cast<unsigned>(ym.month()) % 3 == 0;
	}

	year_month next(year_month const& ym) {
		return ym + 1m;
	}

	year_month next_season(year_month const& ym) {
		if (is_season(ym)) {
			return ym + 3m;
		} else {
			auto nym = next(ym);
			for (; !is_season(nym); nym = next(nym)) {}
			return nym;
		}
	}

	year_month first_bond_month(year_month_day const& today, int nthFriday) {
		year_month ym = year_month{ today.year(),today.month() };
		if (is_season(ym)) {
			bool overThridFriday = today > nth_friday(ym, nthFriday);
			return overThridFriday ? next_season(ym) : ym;
		} else {
			return next_season(ym);
		}
	}

	year_month first_index_month(year_month_day const& today) {
		year_month ym = year_month{ today.year(),today.month() };
		bool overSecondFriday = today > nth_friday(ym, 3);
		if (is_season(ym) && overSecondFriday) {
			return next(ym);
		}
		return ym;
	}

	std::vector<year_month> futureMonths(year_month_day const& today) {
		vector<year_month> results;
		for (int i = 0; i < MONTH_COUNT; ++i) {
			auto ymd = today + months(i);
			year_month ym{ ymd.year(), ymd.month() };
			results.push_back(ym);
		}
		return results;
	}

	std::vector<year_month> bondMonths(year_month_day const& today) {
		vector<year_month> results;

		auto ym = first_bond_month(today, 2);
		results.push_back(ym);

		for (int i = 0; i < 2; ++i) {
			ym = next_season(ym);
			results.push_back(ym);
		}
		return results;
	}

	std::vector<year_month> indexMonths(year_month_day const& today) {
		vector<year_month> results;

		auto first = first_index_month(today);
		auto second = next(first);
		auto third = next_season(first);
		auto fourth = next_season(third);
		results.push_back(first);
		results.push_back(second);
		results.push_back(third);
		results.push_back(fourth);
		return results;
	}

	std::vector<year_month> varietyMonths(type::ProductClass productClass, year_month_day const& today) {
		switch (productClass) {
		case type::ProductClass::Future:
			return futureMonths(today);
		case type::ProductClass::Bond:
			return bondMonths(today);
		case type::ProductClass::Index:
			return indexMonths(today);
		default:
			break;
		}
		return vector<year_month>();
	}


	std::string instrumentCode(type::Exchange exchange, std::string const& varietyCode, int year, unsigned month) {
		switch (exchange) {

		case type::Exchange::CZCE:
			return fmt::format("{}{:d}{:02d}", varietyCode, year % 10, month);	//֣������1λ��ʾ
		case type::Exchange::SGE:
			return "";
		case type::Exchange::CFFEX:
		case type::Exchange::DCE:
		case type::Exchange::SHFE:
		case type::Exchange::HKEX:
		default:
			return fmt::format("{}{:02d}{:02d}", varietyCode,year % 100, month);
		}
	}

	std::vector<type::InstrumentCode> initInstrumentCodes(type::Exchange exchange, type::ProductClass productClass, 
		std::string const& varietyCode, year_month_day const& today) {

		std::vector<type::InstrumentCode> results;
		for each (auto ym in varietyMonths(productClass, today)) {
			auto code = instrumentCode(exchange, varietyCode,int(ym.year()), unsigned(ym.month()));
			results.push_back(type::InstrumentCode{ code.c_str() });
		}
		return results;
	}
}

Instrument const& InstrumentService::Add(Instrument const& inst) {
	auto instID = toID(inst.Code);
	if (instID == INVALID_INSTRUMENT_ID) {
		auto& v = insts_.emplace_back(InstStruct{ inst });
		v.Inst.InstID = insts_.size() - 1;
		for (auto& observer : observers_) {
			observer->OnAdd(inst);
		}
		return v.Inst;
	} else {
		return insts_[instID].Inst;
	}

}

void InstrumentService::AddFee(TradePlatform platform, InvestorFee const& fee) {
	insts_[fee.InstID].Fee = fee;
	insts_[fee.InstID].Fee.isOriginal = true;
}

void InstrumentService::AddMargin(TradePlatform platform, InvestorMargin const& margin) {
	insts_[margin.InstID].Margin = margin;
	insts_[margin.InstID].Margin.isOriginal = true;
}

void InstrumentService::AddVarietyFeeByPresentInstrument(TradePlatform platform, type::VarietyCode const& varityCode, InvestorFee const& fee) {
	auto iter = varietyFees_.find(varityCode);
	if (iter != varietyFees_.end()) {
		if (iter->second.OpenFeeRate >= fee.OpenFeeRate) {
			return;
		}
	} else {
		varietyFees_[varityCode] = fee;
	}

	for (auto& inst : insts_) {
		InvestorFee newFee = fee;
		if (inst.Inst.Code == fee.InstrumentCode) {
			newFee.isOriginal = true;
		} else {
			newFee.isOriginal = false;
			newFee.InstrumentCode = inst.Inst.Code;
		}
		if (inst.Inst.VarityCode == varityCode) {
			inst.Fee = newFee;
		}
	}
}

void InstrumentService::AddVarietyMarginByPresentInstrument(TradePlatform platform, type::VarietyCode const& varityCode, InvestorMargin const& margin) {
	auto iter = varietyMargins_.find(varityCode);
	if (iter != varietyMargins_.end()) {
		if (iter->second.LongMarginRate >= margin.LongMarginRate) {
			return;
		}
	} else {
		varietyMargins_[varityCode] = margin;
	}

	for (auto& inst : insts_) {
		InvestorMargin newMargin = margin;
		if (inst.Inst.Code == margin.InstrumentCode) {
			newMargin.isOriginal = true;
		} else {
			newMargin.isOriginal = false;
			newMargin.InstrumentCode = inst.Inst.Code;
		}
		if (inst.Inst.VarityCode == varityCode) {
			inst.Margin = newMargin;
		}
	}
}


void InstrumentService::AddVarietyFee(TradePlatform platform, type::VarietyCode const& varityCode, InvestorFee const& fee) {
	auto iter = varietyFees_.find(varityCode);
	if (iter != varietyFees_.end()) {
		if (iter->second.OpenFeeRate >= fee.OpenFeeRate) {
			return;
		}
	} else {
		varietyFees_[varityCode] = fee;
	}

	for (auto& inst : insts_) {
		InvestorFee newFee = fee;
		newFee.InstrumentCode = inst.Inst.Code;
		newFee.InstID = inst.Inst.InstID;
		if (inst.Inst.VarityCode == varityCode) {
			inst.Fee = newFee;
		}
	}
}

void InstrumentService::AddVarietyMargin(TradePlatform platform, type::VarietyCode const& varityCode, InvestorMargin const& margin) {
	auto iter = varietyMargins_.find(varityCode);
	if (iter != varietyMargins_.end()) {
		if (iter->second.LongMarginRate >= margin.LongMarginRate) {
			return;
		}
	} else {
		varietyMargins_[varityCode] = margin;
	}

	for (auto& inst : insts_) {
		InvestorMargin newMargin = margin;
		newMargin.InstrumentCode = inst.Inst.Code;
		newMargin.InstID = inst.Inst.InstID;
		if (inst.Inst.VarityCode == varityCode) {
			inst.Margin = newMargin;
		}
	}
}

type::InstID InstrumentService::toID(type::InstrumentCode const& instCode) {
	for (auto const& inst : insts_) {
		if (inst.Inst.Code == instCode) {
			return inst.Inst.InstID;
		}
	}
	return INVALID_INSTRUMENT_ID;
}

type::InstrumentCode const& InstrumentService::toCode(type::InstID instID) {
	static type::InstrumentCode emptyCode{ "" };
	if (instID >= insts_.size()) {
		return emptyCode;
	}
	return insts_[instID].Inst.Code;
}

InstrumentResult InstrumentService::Get(type::InstID instID) {
	return { insts_[instID].Inst };
}

InstrumentResult InstrumentService::Get(type::InstrumentCode instCode) {
	auto id = toID(instCode);
	if (id == INVALID_INSTRUMENT_ID) {
		return { InvalidInstrumentError };
	}
	return Get(id);
}

FeeResult InstrumentService::GetFee(type::InstID instID) {
	if (instID >= insts_.size()) {
		return { InvalidInstrumentError };
	}
	return insts_[instID].Fee;
}

FeeResult InstrumentService::GetFee(type::InstrumentCode instCode) {
	auto id = toID(instCode);
	if (id == INVALID_INSTRUMENT_ID) {
		return { InvalidInstrumentError };
	}
	return GetFee(id);
}

MarginResult InstrumentService::GetMargin(type::InstID instID) {
	if (instID >= insts_.size()) {
		return { InvalidInstrumentError };
	}
	return insts_[instID].Margin;
}

MarginResult InstrumentService::GetMargin(type::InstrumentCode instCode) {
	auto id = toID(instCode);
	if (id == INVALID_INSTRUMENT_ID) {
		return { InvalidInstrumentError };
	}
	return GetMargin(id);
}


InstrumentResults InstrumentService::GetAll(TradePlatform platform) {
	InstrumentResults results;
	auto& exchangeService = ServiceSinglton::Services().Exchange;
	for (auto const& inst : insts_) {
		if (exchangeService->toPlatform(inst.Inst.Exchange) == platform) {
			results.push_back(inst.Inst);
		}
	}
	return results;
}

InstrumentResults InstrumentService::GetAll() {
	InstrumentResults results;
	auto& exchangeService = ServiceSinglton::Services().Exchange;
	for (auto const& inst : insts_) {
		results.push_back(inst.Inst);
	}
	return results;
}

inline bool exchangeCompare(type::Exchange a, type::Exchange b) {
	static int cnt = 0;
	static std::map<type::Exchange, int> exchangeSequence = {
		{ type::Exchange::SGE, cnt++ },
		{ type::Exchange::SHFE, cnt++ },
		{ type::Exchange::MT5, cnt++ },
		{ type::Exchange::CME, cnt++ },
		{ type::Exchange::HKEX, cnt++ },
		{ type::Exchange::CFFEX, cnt++ },
		{ type::Exchange::CZCE, cnt++ },
		{ type::Exchange::DCE, cnt++ },
	};

	return exchangeSequence[a] < exchangeSequence[b];
}

InstrumentResults InstrumentService::GetSortedAll() {
	auto insts = GetAll();
	sort(insts.begin(), insts.end(), [](Instrument const& a, Instrument const& b) {
		if (a.Exchange == b.Exchange) {
			return a.Code < b.Code;
		} else {
			return exchangeCompare(a.Exchange, b.Exchange);
		}
	});
	return insts;
}

InstrumentResults InstrumentService::GetSortedInstrumentsExcept(type::InstrumentCode const& code) {
	auto insts = GetAll();

	sort(insts.begin(), insts.end(), [](Instrument const& a, Instrument const& b) {
		if (a.Exchange == b.Exchange) {
			return a.Code < b.Code;
		} else {
			return exchangeCompare(a.Exchange, b.Exchange);
		}
	});
	insts.erase(std::remove_if(insts.begin(), insts.end(), [&](Instrument const& a) {
		return a.Code == code;
		}));
	return insts;
}

inline bool end_with(std::string const& fullString, std::string const& ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

InstrumentResults InstrumentService::GetHotInstruments(TradePlatform platform) {
	InstrumentResults results;
	if (platform == TradePlatform::CTP) {
		auto insts = GetAll(platform);
		for (auto& inst : insts) {
			if (end_with(inst.Code.value(), "12") || end_with(inst.Code.value(), "06")) {
				results.push_back(inst);
			}
		}
	}
	return results;
}

InstrumentResults InstrumentService::GetAll(type::VarietyCode const& varietyCode) {
	InstrumentResults results;
	for (auto const& inst : insts_) {
		if (inst.Inst.VarityCode == varietyCode) {
			results.push_back(inst.Inst);
		}
	}
	return results;
}
