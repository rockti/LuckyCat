#include "pch.h"
#include "Config.h"
#include "utils/String.h"
#include "cpptoml.h"
using namespace std;

static Config s_config;

inline ConnPlatform platform(string const& p) {
	if (p == "CTP") {
		return ConnPlatform::CTP;
	}
	if (p == "SGE") {
		return ConnPlatform::SGE;
	}
	if (p == "HFT") {
		return ConnPlatform::HFT;
	}
	return ConnPlatform::CTP;
}

void ConfigLoader::Load(char const* file) {
	auto config = cpptoml::parse_file(file);
	auto connConfigs = config->get_table_array("ConnConfigs");
	for (const auto& table : *connConfigs) {
		ConnConfig connConfig;
		connConfig.Platform = platform(*table->get_qualified_as<string>("Platform"));
		connConfig.TradeEndpoint = *table->get_qualified_as<string>("TradeEndpoint");
		connConfig.QuoteEndpoint = *table->get_qualified_as<string>("QuoteEndpoint");						// SGE是加密机地址
		connConfig.LocalEndpoint = *table->get_qualified_as<string>("LocalEndpoint");						// 盛利是组播地址

		connConfig.Credential.BrokerID = *table->get_qualified_as<string>("Credential.BrokerID");			// SGE是会员号
		connConfig.Credential.UserID = *table->get_qualified_as<string>("Credential.UserID");				// SGE是交易员账号
		connConfig.Credential.InvestorID = *table->get_qualified_as<string>("Credential.InvestorID");		// SGE是交易编码
		connConfig.Credential.Password = *table->get_qualified_as<string>("Credential.Password");			// SGE是交易员密码
		connConfig.Credential.AppID = *table->get_qualified_as<string>("Credential.AppID");
		connConfig.Credential.TradeAuthCode = *table->get_qualified_as<string>("Credential.TradeAuthCode");	// SGE是ExchPubKey
		s_config.ConnConfigs.push_back(connConfig);
	}

	auto rules = config->get_table_array("Rules");
	for (const auto& table : *rules) {
		auto panelStr = *table->get_qualified_as<string>("Panels");
		if (!panelStr.empty()){
			auto panelVec = split(panelStr, ",");
			for (auto const& panel : panelVec) {
				s_config.Rule.ValidPanel.insert(panel);
			}
		}
		auto punishmentStr = *table->get_qualified_as<string>("PunishmentVarieties");
		if (!punishmentStr.empty()){
			auto punishmentVec = split(punishmentStr, ",");
			for (auto const& panel : punishmentVec) {
				s_config.Rule.OffsetPunishmentVarieties.insert(panel);
			}
		}
	}
// 	if (config->contains("HFF")) {
// 		auto table = config->get_table("HFN");
// 		if (!table->empty()) {
// 			auto varieties = *table->get_qualified_as<string>("PenaltyVarieties");
// 			auto vec = split(varieties, ",");
// 			for (auto const& variety : vec) {
// 				s_config.OffsetPenaltyVarieties.insert(variety);
// 			}
// 		}
// 	}		//for HFNetural

// 	if (config->contains("HKMM")) {
// 		auto tradeVariety = config->get_table("HKMM");
// 		if (!tradeVariety->empty()) {
// 			s_config.MMVarities.Variety = *tradeVariety->get_qualified_as<string>("Variety");
// 			s_config.MMVarities.Months = *tradeVariety->get_qualified_as<string>("Months");
// 			s_config.MMVarities.Follows = *tradeVariety->get_qualified_as<string>("Follows");
// 			s_config.MMVarities.Hedge = *tradeVariety->get_qualified_as<string>("Hedge");
// 		}
// 	}		//for HKMarketing
}
Config& ConfigLoader::Get() {
	return s_config;
}
