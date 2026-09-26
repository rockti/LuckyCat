#pragma once

#include "service/Service.h"
#include "QSettings.h"
#include "QCoreApplication.h"

struct SavedInvestor {
	TradePlatform Platform;
	bool Checked = false;
	type::InvestorID TradeInvestor;
	type::InvestorID QuoteInvestor;
};

struct ZTPInvestorTrader {
	type::InvestorID Investor;
	uint8_t TraderNubmer = 1;
};

class CustomizedPersistService {
	using SavedInvestors = std::array<SavedInvestor, 10>;
public:
	inline static CustomizedPersistService& Instance() {
		static CustomizedPersistService s_service;
		return s_service;
	}
	void Load() {
		loadSavedInvestors();
		loadZTPInvestorTrader();
	}
	SavedInvestor const& GetSavedInvestor(TradePlatform platform) {
		return savedInvestors_[int(platform)];
	}
	void SaveInvestor(SavedInvestor savedInvestor) {
		savedInvestors_[int(savedInvestor.Platform)] = savedInvestor;
		persist(savedInvestors_);
	}

	ZTPInvestorTrader GetTraderNumber() {
		return ztpInvestorTrader_;
	}

	void SaveTraderNumber(ZTPInvestorTrader const& ztpInvestorTrader) {
		persist(ztpInvestorTrader);
	}


private:
	const char* InvestorGroup = "Investors";
	void loadSavedInvestors() {
		QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
		settings.beginGroup(InvestorGroup);
		auto const& config = ConfigLoader::Get();
		for (auto const& ts : config.TradeSystemConfigs) {
			SavedInvestor savedInvestor;
			auto key = QString("%1_%2").arg(toString(ts.Platform).c_str()).arg("TradeInvestor");
			auto tradeInvestor = settings.value(key).toString();
			if (tradeInvestor.isEmpty()) {
				continue;
			}
			key = QString("%1_%2").arg(toString(ts.Platform).c_str()).arg("QuoteInvestor");
			auto quoteInvestor = settings.value(key).toString();
			key = QString("%1_%2").arg(toString(ts.Platform).c_str()).arg("Checked");
			bool checked = settings.value(key) == "false" ? false : true;
			savedInvestor.Checked = checked;
			savedInvestor.Platform = ts.Platform;
			savedInvestor.TradeInvestor = tradeInvestor.toStdString();
			savedInvestor.QuoteInvestor = quoteInvestor.toStdString();
			savedInvestors_[int(ts.Platform)] = savedInvestor;
		}
		settings.endGroup();
	}
	void persist(SavedInvestors const& savedInvestors) {
		QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
		settings.beginGroup(InvestorGroup);
		for (auto const& si : savedInvestors) {
			if (si.TradeInvestor.empty()) {
				continue;
			}
			auto key = QString("%1_%2").arg(toString(si.Platform).c_str()).arg("TradeInvestor");
			settings.setValue(key, si.TradeInvestor.value());
			key = QString("%1_%2").arg(toString(si.Platform).c_str()).arg("QuoteInvestor");
			settings.setValue(key, si.QuoteInvestor.value());
			key = QString("%1_%2").arg(toString(si.Platform).c_str()).arg("Checked");
			settings.setValue(key, si.Checked ? "true" : "false");

		}
		settings.endGroup();
	}

	const char* TraderGroup = "ZTPInvestorTrader";
	void loadZTPInvestorTrader() {
		QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
		settings.beginGroup(TraderGroup);
		auto const& config = ConfigLoader::Get();
		auto key = QString("Investor");
		auto ztpInvestor = settings.value(key).toString();
		if (ztpInvestor.isEmpty()) {
			return;
		}
		ztpInvestorTrader_.Investor = ztpInvestor.toStdString();

		key = "TraderNumber";
		ztpInvestorTrader_.TraderNubmer = settings.value(key).toString().toInt();
		settings.endGroup();
	}

	void persist(ZTPInvestorTrader const& investorTrader) {
		ztpInvestorTrader_ = investorTrader;
		QSettings settings(QSettings::NativeFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
		settings.beginGroup(TraderGroup);
		auto key = QString("Investor");
		settings.setValue(key, investorTrader.Investor.value());
		key = "TraderNumber";
		settings.setValue(key, investorTrader.TraderNubmer);
		settings.endGroup();
	}

	SavedInvestors savedInvestors_;
	ZTPInvestorTrader ztpInvestorTrader_;
};