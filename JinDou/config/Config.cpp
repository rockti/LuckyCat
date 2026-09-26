#include "Config.h"

#include "cpptoml.h"
using namespace std;

static Config s_config;

inline TradePlatform platform(string const& p) {
	if (p == "CTP") {
		return TradePlatform::CTP;
	}
	if (p == "HKEX_ESUNNY") {
		return TradePlatform::HKEX_ESUNNY;
	}
	if (p == "ZTP") {
		return TradePlatform::ZTP;
	}
	return TradePlatform::CTP;
}

std::string s_configFile;

void ConfigLoader::Load(std::istream& is) {
	auto config = cpptoml::parse_file(is);
	auto license = config->get_qualified_as<string>("License");
	s_config.License = *license;

	s_config.UserMode = UserMode::Single;
	auto userMode = config->get_qualified_as<string>("UserMode");
	if (userMode) {
		if (*userMode == "Multiple") {
			s_config.UserMode = UserMode::Multiple;
		}
	}

	s_config.UIMode = UIMode::Normal;
	auto uiMode = config->get_qualified_as<string>("UIMode");
	if (uiMode) {
		if (*uiMode == "HasStrategy") {
			s_config.UIMode= UIMode::HasStrategy;
		}
	}

	auto connConfigs = config->get_table_array("TradeSystemConfigs");
	for (const auto& table : *connConfigs) {
		TradeSystemConfig tpConfig;
		tpConfig.Platform = platform(*table->get_qualified_as<string>("Platform"));
		tpConfig.Varieties = *table->get_qualified_as<string>("Varieties");
		tpConfig.TradeEndpoint = *table->get_qualified_as<string>("TradeEndpoint");
		tpConfig.QuoteEndpoint = *table->get_qualified_as<string>("QuoteEndpoint");
		tpConfig.Credential.AppID = *table->get_qualified_as<string>("Credential.AppID");
		tpConfig.Credential.BrokerID = *table->get_qualified_as<string>("Credential.BrokerID");
		tpConfig.Credential.InvestorID = *table->get_qualified_as<string>("Credential.InvestorID");
		tpConfig.Credential.QuoteAuthCode = *table->get_qualified_as<string>("Credential.QuoteAuthCode");
		tpConfig.Credential.TradeAuthCode = *table->get_qualified_as<string>("Credential.TradeAuthCode");
		tpConfig.Credential.UserID = *table->get_qualified_as<string>("Credential.UserID");
		tpConfig.Credential.UserProductInfo = *table->get_qualified_as<string>("Credential.UserProductInfo");

		s_config.TradeSystemConfigs.push_back(tpConfig);
	}
}
void ConfigLoader::Load(char const* file) {
	s_configFile = file;
	ifstream ifs{ file };
	Load(ifs);
}

void ConfigLoader::Load(std::string const& content) {
	std::stringstream ss;
	ss << content;
	Load(ss);
}
Config& ConfigLoader::Get() {
	return s_config;
}

void ConfigLoader::Save() {
	auto root = cpptoml::make_table();
	auto table_array = cpptoml::make_table_array();
	for (auto const& v : s_config.TradeSystemConfigs) {
		auto nested_table = cpptoml::make_table();
		nested_table->insert("Platform", toString(v.Platform));
		nested_table->insert("Varieties", v.Varieties);
		nested_table->insert("TradeEndpoint", v.TradeEndpoint);
		nested_table->insert("QuoteEndpoint", v.QuoteEndpoint);
		auto credential_table = cpptoml::make_table();
		credential_table->insert("AppID", v.Credential.AppID);
		credential_table->insert("BrokerID", v.Credential.BrokerID);
		credential_table->insert("InvestorID", v.Credential.InvestorID);
		credential_table->insert("QuoteAuthCode", v.Credential.QuoteAuthCode);
		credential_table->insert("TradeAuthCode", v.Credential.TradeAuthCode);
		credential_table->insert("UserID", v.Credential.UserID);
		credential_table->insert("UserProductInfo", v.Credential.UserProductInfo);
		nested_table->insert("Credential", credential_table);
		table_array->push_back(nested_table);
	}
	root->insert("TradeSystemConfigs",table_array);

	ofstream ofs(s_configFile, std::ios::trunc | std::ios::out);
	ofs << (*root);
	ofs.close();
}