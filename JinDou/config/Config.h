#pragma once

#include "model/Constants.h"
#include <vector>
#include <string>
#include <istream>
struct Credential {
	std::string BrokerID;				//BrokerID
	std::string UserID;					//平台资金帐户号
	std::string InvestorID;				//投资者编号， CTP与UserID相同，FEMAS是UserID后加01|02
	std::string Password;				
	std::string UserProductInfo;
	std::string TradeAuthCode;
	std::string QuoteAuthCode;
	std::string AppID;
};

struct TradeSystemConfig {
	TradePlatform Platform;
	std::string TradeEndpoint;
	std::string QuoteEndpoint;
	std::string Varieties;
	Credential Credential;
};

enum class UserMode {
	Single,
	Multiple,
};

enum class UIMode {
	Normal, 
	HasStrategy
};
struct Config {
	std::string License;
	UserMode UserMode;
	UIMode UIMode;
	std::vector<TradeSystemConfig> TradeSystemConfigs;
};

class ConfigLoader {
public:
	static void Load(char const* file);
	static void Load(std::istream& is);
	static void Load(std::string const& content);
	static Config& Get();
	static void Save();
};
