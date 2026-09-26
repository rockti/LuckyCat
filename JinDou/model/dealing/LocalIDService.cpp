#pragma once
#include "LocalIDService.h"
#include <fstream>
LocalIDFileSaver::LocalIDFileSaver() {
	if (!fs_.is_open()) {
		fs_.open("localid.db", std::ios_base::in  | std::ios_base::binary);
	}
}

LocalIDFileSaver::~LocalIDFileSaver() {
	if (fs_.is_open()) {
		fs_.close();
	}
}

LocalOrderID LocalIDFileSaver::Load(type::TradingDay const& tradingDay) {
	fs_.seekg(0);
	fs_.read((char*)&store_, sizeof(store_));
	if (fs_.good()) {
		if (tradingDay == store_.TradingDay) {
			return atoi(store_.LastLocalID.value());
		}
	}
	store_.TradingDay = tradingDay;
	fs_.close();
	return 0;
}

void LocalIDFileSaver::SaveToFile(LocalOrderID id) {

	if (id > lastSaved_ && !store_.TradingDay.empty()) {
		if (!write_opened_) {
			fs_.close();
			fs_.open("localid.db", std::ios_base::out | std::ios_base::binary);
			write_opened_ = true;
		}
		std::string strLocalID = fmt::format("{:015d}", id);
		store_.LastLocalID = strLocalID;
		fs_.seekp(0);
		fs_.write((char*)&store_, sizeof(store_));
		fs_.flush();
		lastSaved_ = id;
	}
}
