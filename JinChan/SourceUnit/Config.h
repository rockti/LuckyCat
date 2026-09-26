#pragma once
#include <string>

struct DatabaseConfig {
	std::string Endpoint;
	std::string User;
	std::string Password;
};

struct FrontConfig {
	std::string TradeUrl;
	std::string MarketUrl;
};

struct SGEConfig {
	std::string TopicId;
	std::string SGEAddress;
	int SGEPort;
	std::string SafeAddress;
	int SafePort;
	std::string ExchPubKey;

};

struct BrokerConfig {
	std::string BrokerID;
};

struct Config {
	std::string FlowPath;
	std::string CSVPath;
	FrontConfig Front;
	DatabaseConfig DB;
	SGEConfig SGE;
	BrokerConfig Broker;
};

struct ConfigSingleton {
	inline static Config s_config;
	static Config& GetConfig() {
		return s_config;
	}
};