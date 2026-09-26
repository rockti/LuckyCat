#pragma once
#include "FTDPackage.h"
#include "MarketRepository.h"
#include <bitset>
#include <functional>
#include "FtdDataStruct.h"
#include <array>
static const int MaxInstrumentCount = 128;
using InstIndexBits = bitset<128>;
struct InstIndex {
	static const int MaxInstIndex = 126;
	static const int InvalidInstIndex = 127;

	static InstIndex Of(uint8_t index) {
		return InstIndex(index);
	}

	static InstIndex InvalidIndex() {
		return InstIndex(InvalidInstIndex);
	}

	inline bool IsValid() {
		return index_ != InvalidInstIndex;
	}
	InstIndexBits toBit() {
		InstIndexBits bits;
		bits.set(index_);
		return bits;
	}
	uint8_t Index() const { return index_; }
private:
	InstIndex(uint8_t index) :index_(index) {
	}
	uint8_t index_;
};


struct InstIndexStore {
	InstIndexBits Bits;
	inline bool Add(InstIndex index) {
		Bits.set(index.Index());
		return true;
	}

	inline bool Remove(InstIndex index) {
		Bits.set(index.Index(), false);
		return true;
	}

	inline bool Has(InstIndex index) const {
		return Bits.test(index.Index());
	}
	inline bool Has(uint32_t index) {
		return Bits.test(index);
	}
	inline bool IsEmpty() const {
		return Bits.none();
	}
};

template<class InstID>
class InstrumentManager {
public:
	InstrumentManager() {
		ids_.reserve(InstIndex::MaxInstIndex);
	}
	inline InstIndex Add(InstID const& inst) {
		InstIndex index = InstIndex::Of(uint8_t(ids_.size()));
		ids_.push_back(inst);
		indexMap_.emplace(inst, index);
		bits_.Add(index);
		return index;
	}

	inline bool Has(InstID const& inst) {
		return indexMap_.contains(inst);
	}

	inline bool Has(InstIndex const& index) {
		return bits_.Has(index);
	}

	inline InstIndex GetIndex(InstID const& inst) {
		auto iter = indexMap_.find(inst);
		if (iter != indexMap_.end()) {
			return iter->second;
		}
		return InstIndex::InvalidIndex();
	}

	inline InstID const& GetID(InstIndex index) {
		return ids_[index.Index()];
	}
	using InstIDForEach = std::function<void(InstID const& inst)>;
	void ForEachByBits(InstIndexStore const& indexes, InstIDForEach func) {
		for (auto& kv : indexMap_) {
			if (indexes.Has(kv.second)) {
				func(kv.first);
			}
		}
	}

	InstIndex UpdateSnapshot(CFTDDepthMarketDataField const& market) {
		auto index = GetIndex(market.InstrumentID);
		if (index.IsValid()) {
			snapshots_[index.Index()] = market;
		}
		return index;
	}

	CFTDDepthMarketDataField* GetSnapshot(InstIndex index) {
		if (bits_.Has(index)) {
			auto* result = &snapshots_[index.Index()];
			if (result->InstrumentID != "") {
				return result;
			}
		}
		return nullptr;
	}
private:
	std::map<InstID, InstIndex> indexMap_;
	std::vector<InstID> ids_;
	InstIndexStore bits_;
	std::array<CFTDDepthMarketDataField,256> snapshots_;
};

InstrumentManager<CInstrumentIDType>& FTDInstance();