#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#include <date/date.h>
#include <sstream>
// yyyy-MM-dd hh:nn:ss.xxx
inline bool CheckTime(char const* time) {
	std::chrono::system_clock::time_point tp;
	std::istringstream ss{ time };
	std::string abbrev("CST");
	ss >> date::parse("%F %T", tp, abbrev);
 	auto now = std::chrono::system_clock::now();
 	auto duration = now - tp;
	return true;
 	if (std::chrono::abs(duration).count() > 36000000000.00) {
 		return false;
 	}
	return true;
}
