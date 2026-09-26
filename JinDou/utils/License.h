#pragma once

#include <tchar.h>
#include <string>
#include <optional>
#include "type/ZTPType.h"
using namespace ztp;
class License {
public:
	static License& Instance();
	bool Init(std::string const& license);
	bool IsClosingToExpire(type::TradingDay const& tradingDay);
	bool IsExpire(type::TradingDay const& tradingDay);
private:
	std::string id_;
	int days_ = 0;
};
