#pragma once

#include "model/Constants.h"
#include <vector>
#include <string>
#include <set>

struct Rule {
	std::set<std::string> ValidPanel;					// 逗号分隔的字符串: HFT,GRAB,SPREAD
	std::set<std::string> OffsetPunishmentVarieties;	// 逗号分隔的字符串: ag,au
};

struct Credential {
	std::string BrokerID;					// BrokerID；SGE为席位号
	std::string UserID;						// 平台资金帐户号；SGE为交易员号
	std::string InvestorID;					// 投资者编号， CTP与UserID相同，FEMAS是UserID后加01|02；SGE为交易编码
	std::string Password;					// SGE为会员密码
	std::string TradeAuthCode;
	std::string QuoteAuthCode;
	std::string AppID;
};	// 与ConnConfig对应

struct ConnConfig {
	ConnPlatform Platform;
	std::string TradeEndpoint;
	std::string QuoteEndpoint;
	std::string LocalEndpoint;				// 组播行情地址
	Credential Credential;
};

struct HKMMVarieties {
	std::string Variety;
	std::string Months;
	std::string Follows;
	std::string Hedge;
};

struct Config {
	std::vector<ConnConfig> ConnConfigs;	// 可多个
	HKMMVarieties MMVarities;				// 香港
	Rule Rule;
};

class ConfigLoader {
public:
	static void Load(char const* file);
	static Config& Get();
};