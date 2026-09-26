#pragma once
#include "type/ZTPType.h"
#include <array>
using namespace ztp;
struct CancelCount {
	const int CancelBuff = 10;
	type::TradingDay TradingDay;
	type::InstID InstID;
	uint32_t Max = 0;
	uint32_t Current = 0;

	bool IsOverLimit() const {
		return Current + CancelBuff > Max;
	}
	bool operator < (CancelCount const& other) {
		return this->InstID < other.InstID;
	}
};


class CancellationLimiter {
	static CancellationLimiter self_;
public:
	void Init(type::InstID instID, uint32_t maxCancelCount, uint32_t canceledCount) {
		auto& cancelCount = instCanceledCount_[int(instID)];
		cancelCount.InstID = instID;
		cancelCount.Max = maxCancelCount;
		cancelCount.Current = canceledCount;
	}

	CancelCount const& Add(type::InstID instID) {
		auto& cancelCount = instCanceledCount_[int(instID)];
		cancelCount.Current++;
		return cancelCount;
	}
	
	CancelCount const& Get(type::InstID instID) const {
		return instCanceledCount_[int(instID)];
	}
	static CancellationLimiter& Instance() {
		return self_;
	}

	static bool IsCountCancel(InvestorOrder const& order);
private:
	std::array<CancelCount,256> instCanceledCount_;
};

inline CancellationLimiter CancellationLimiter::self_;