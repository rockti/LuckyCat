#include "pch.h"
#include "model/Variety.h"

#include <chrono>



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
			bool overFriday = today > nth_friday(ym, nthFriday);
			return overFriday ? next_season(ym) : ym;
		} else {
			return next_season(ym);
		}
	}

	year_month first_index_month(year_month_day const& today) {
		year_month ym = year_month{ today.year(),today.month() };
		bool overFriday = today > nth_friday(ym, 3);
		return overFriday ? next(ym) : ym;
	}

	std::vector<year_month> futureMonths(year_month_day const& today) {
		vector<year_month> results;

		const year_month currentMonth{ today.year(), today.month() };
		for (int i = 0; i <= MONTH_COUNT; ++i) {				
			results.push_back(currentMonth + months(i));
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
		auto third = next_season(second);
		auto fourth = next_season(third);
		results.push_back(first);
		results.push_back(second);
		results.push_back(third);
		results.push_back(fourth);
		return results;
	}

	std::vector<year_month> varietyMonths(Variety const& variety, year_month_day const& today) {
		switch (variety.GetType()) {
		case Variety::Future:
			return futureMonths(today);
		case Variety::Bond:
			return bondMonths(today);
		case Variety::Index:
			return indexMonths(today);
		default:
			break;
		}
		return vector<year_month>();
	}
}

Variety::TYPE GetSHFEVarietyType(std::string const& m_code) {
	return Variety::Future;
}
Variety::TYPE  GetCFFEXVarietyType(std::string const& m_code) {
	if (m_code[0] == 'T') {
		return Variety::Bond;
	}
	else {
		return Variety::Index;
	}
}
Variety::TYPE GetDCEXVarietyType(std::string const& m_code) {
	return Variety::Future;
}
Variety::TYPE GetCZCEXVarietyType(std::string const& m_code) {
	return Variety::Future;
}
Variety::TYPE GetSGEVarietyType(std::string const& m_code) {
	if (m_code == "Au100g" || m_code =="Au99.99") {
		return Variety::Spot;
	}
	else {
		return Variety::Defer;
	}
}
Variety::TYPE GetHKEXVarietyType(std::string const& m_code) {
	return Variety::Future;
}

std::string Variety::GetExchangeName() const{
	switch (m_exchangeFlag) {
	case Exchange::SHFE:	return "SHFE";
	case Exchange::HKEX:	return "HKEX";
	case Exchange::CFFEX:	return "CFFEX";
	case Exchange::SGE:		return "SGE";
	case Exchange::DCE:		return "DCE";
	case Exchange::CZCE:	return "CZCE";
	default:	return "UNKNOWN";
	}
}
void Variety::initType() {
	switch (m_exchangeFlag) {
	case Exchange::CTP:	break;
	case Exchange::CFFEX:
		m_type = GetCFFEXVarietyType(m_code);
		break;
	case Exchange::CZCE:
		m_type = GetCZCEXVarietyType(m_code);
		break;
	case Exchange::DCE:
		m_type = GetDCEXVarietyType(m_code);
		break;
	case Exchange::SHFE:
		m_type = GetSHFEVarietyType(m_code);
		break;
	case Exchange::SGE:
		m_type = GetSGEVarietyType(m_code);
		break;
	case Exchange::HKEX:
		m_type = GetHKEXVarietyType(m_code);
		break;
	default:
		break;
	}
}

std::string Variety::instrumentCode(int year, unsigned month) const {
	switch (m_exchangeFlag) {
	case Exchange::CZCE:
		return fmt::format("{:d}{:02d}", year%10, month);	// 郑州年用1位表示
	case Exchange::CTP:
	case Exchange::SGE:
		return "";
	case Exchange::CFFEX:
	case Exchange::DCE:
	case Exchange::SHFE:
	default:
		return fmt::format("{:02d}{:02d}", year%100, month);
	}
}

void Variety::initInstrumentCodes(year_month_day const& today) {
	if (m_exchangeFlag == Exchange::SGE) {	
		m_instrumentCodes.push_back(m_code);
	} 	// 金交所合约不带月份，无需组合
	else {
		for each (auto ym in varietyMonths(*this,today)) {
			auto code = instrumentCode(int(ym.year()), unsigned(ym.month()));
			m_instrumentCodes.push_back(code);
		}
	}
}
