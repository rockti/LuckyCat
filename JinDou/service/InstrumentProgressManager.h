#pragma once

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "type/ZTPType.h"
#include "model/Constants.h"

class InstrumentProgressManager {
	struct Progress {
		std::set<type::InstrumentCode> Insts;
		std::set<type::InstrumentCode> InstDone;
		std::set<type::InstrumentCode> FeeDone;
		std::set<type::InstrumentCode> MarginDone;
		std::set<type::InstrumentCode> QuoteDone;

		size_t MinDone() const {
			return std::min({ InstDone.size(), FeeDone.size(), MarginDone.size(), QuoteDone.size() });
		}
	};

public:
	static InstrumentProgressManager& CTP();
	static InstrumentProgressManager& HKEX();

	void InitInstruments(std::vector<type::InstrumentCode> const& insts);
	void AddInstDone(type::InstrumentCode const& inst);
	void AddFeeDone(type::InstrumentCode const& inst);
	void AddMarginDone(type::InstrumentCode const& inst);
	void AddQuoteDone(type::InstrumentCode const& inst);
	void AddVarietyFeeDone(type::VarietyCode const& varietyCode);
	void AddVarietyMarginDone(type::VarietyCode const& varietyCode);
	bool IsFinished(TradePlatform platform) const;
	bool IsFeeDone() const {
		return progress_.FeeDone.size() == progress_.Insts.size();
	}
	bool IsMarginDone() const {
		return progress_.MarginDone.size() == progress_.Insts.size();
	}
public:
	Progress progress_;
};
