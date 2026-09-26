#pragma once
#include "License.h"
#include "String.h"
#include "Encryption.h"
#include <date/date.h>

using namespace date;
using namespace ztp;
License& License::Instance() {
	static License s_license;
	return s_license;
}
char const* lovely = "Qian";

int toBirth() {
	int q = lovely[0];
	int i = lovely[1];
	int a = lovely[2];
	int n = lovely[3];

	int t = q * q * q * q;
	int year = t / 10000 - i * (i % 10) * (i % 10) + 336;
	int month = (a - t / 100 % 100) / 10;
	int day = (t % 100 + n) / 10 % 10;
	return year * 10000 + month * 100 + day;
}

bool License::Init(std::string const& license) {
	SecureFile sf;
	bool result = sf.DecryptLicense(license, id_, days_);
	return result;
}

local_days to_localdays(std::string const& date) {
	auto strs = split(date, "-");
	int year = stoi(strs[0]);
	int month = stoi(strs[1]);
	int day = stoi(strs[2]);
	return local_days(year_month_day{ date::year(year), date::month(month), date::day(day) });
}

local_days to_localdays(type::TradingDay const& tradingDay) {
	const char* pday = tradingDay.value();
	int year = stoi(std::string{ std::string_view{ pday,4 }});
	int month = stoi(std::string{ std::string_view{ pday + 4, 2 } });
	int day = stoi(std::string{ std::string_view{ pday + 6, 2 } });
	
	return local_days(year_month_day{ date::year(year), date::month(month), date::day(day) });
}


local_days birthday() {
	int birth = toBirth();
	year_month_day start(date::year(birth / 10000), date::month(birth / 100 % 100), date::day(birth % 100));
	return local_days(start);
}

local_days expire_days(int days) {
	auto start_days = birthday();
	year_month_day expire(start_days + date::days(days));
	return local_days(expire);
}

bool License::IsClosingToExpire(type::TradingDay const& tradingDay) {

	date::days days = expire_days(days_) - to_localdays(tradingDay);
	if (days.count() < 30) {
		return true;
	}
	return false;
}

bool License::IsExpire(type::TradingDay const& tradingDay) {
	date::days days = expire_days(days_) - to_localdays(tradingDay);
	if (days.count() < 0) {
		return true;
	}
	return false;
}
