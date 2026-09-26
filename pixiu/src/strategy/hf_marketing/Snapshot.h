#pragma once
#include "model/Constants.h"
#include "strategy/Business.h"
#include "service/PositionService.h"

namespace hf::netural {
	struct NeturalPosition {
		pos::PairPositionVolume* MajorVolume = nullptr;
		pos::PairPositionVolume* HedgeVolume = nullptr;
		pos::InstrumentDynamicData* MajorFlags = nullptr;
		pos::InstrumentDynamicData* HedgeFlags = nullptr;
	};

	class PriceBusinessManager;
	struct MarketingSnapshot : public SnapshotBase {
		NeturalPosition GlobalPosition;
		PriceBusinessManager* businessManagerPtr = nullptr;
		int PFactor = 0;		// 持仓偏向
		int SMajorCnt = 0;
		int BMajorCnt = 0;
		int SRvsCnt = 0;
		int BRvsCnt = 0;

		int GainAmt = 0;		// 盈利数
		int LossAmt = 0;		// 亏损数
		int HedgeAmt = 0;		// 对冲数
		int CancelAmt = 0;		// 撤单数

		money macroDBLStdev = 0.0;
		money macroAvgPrice = 0.0;
		inline bool IsChanged(MarketingSnapshot const& other) const {
			if (ID != other.ID) return false;
			return Status != other.Status || PFactor != other.PFactor || SMajorCnt != other.SMajorCnt 
				|| BMajorCnt != other.BMajorCnt || SRvsCnt != other.SRvsCnt || BRvsCnt != other.BRvsCnt
				|| GainAmt != other.GainAmt || LossAmt != other.LossAmt || HedgeAmt != other.HedgeAmt || CancelAmt != other.CancelAmt
				|| macroDBLStdev != other.macroDBLStdev || macroAvgPrice != other.macroAvgPrice;
		}
	};
}
