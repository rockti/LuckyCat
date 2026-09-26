#include "InstrumentProgressManager.h"
#include "service/Service.h"

InstrumentProgressManager& InstrumentProgressManager::CTP() {
	static InstrumentProgressManager s_mgr;
	return s_mgr;

}

InstrumentProgressManager& InstrumentProgressManager::HKEX() {
	static InstrumentProgressManager s_mgr;
	return s_mgr;

}

void InstrumentProgressManager::InitInstruments(std::vector<type::InstrumentCode> const& insts) {
	progress_.Insts.insert(insts.begin(), insts.end());
}

void InstrumentProgressManager::AddInstDone(type::InstrumentCode const& inst) {
		progress_.InstDone.emplace(inst);
}

void InstrumentProgressManager::AddFeeDone(type::InstrumentCode const& inst) {
	progress_.FeeDone.emplace(inst);
}

void InstrumentProgressManager::AddMarginDone(type::InstrumentCode const& inst) {
	progress_.MarginDone.emplace(inst);
}

void InstrumentProgressManager::AddQuoteDone(type::InstrumentCode const& inst) {
	progress_.QuoteDone.emplace(inst);
}

void InstrumentProgressManager::AddVarietyFeeDone(type::VarietyCode const& varietyCode) {
	auto insts = ServiceSinglton::Services().Instrument->GetAll(varietyCode);
	for (auto const& inst : insts) {
		AddFeeDone(inst.Code);
	}
}

void InstrumentProgressManager::AddVarietyMarginDone(type::VarietyCode const& varietyCode) {
	auto insts = ServiceSinglton::Services().Instrument->GetAll(varietyCode);
	for (auto const& inst : insts) {
		AddMarginDone(inst.Code);
	}
}

bool InstrumentProgressManager::IsFinished(TradePlatform platform) const {
	return progress_.MinDone() == progress_.Insts.size();
}

