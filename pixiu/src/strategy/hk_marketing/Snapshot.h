#pragma once
#include "strategy/Business.h"


namespace hk {
	enum class BiasTick {
		BuyOneTick = -2,
		BuyRound = -1,
		Normal = 0,
		SellRound = 1,
		SellOneTick = 2,
	};

	struct Snapshot : public SnapshotBase {
		int Position;
		BiasTick BiasTick;
		CharArray<127> PosInfo;
		CharArray<127> HedgeInfo;
		int HedgePosition;
		inline bool IsChanged(Snapshot const& other) const {
			if (ID != other.ID) return false;
			return Status != other.Status ||
				Position != other.Position ||
				BiasTick != other.BiasTick ||
				HedgePosition != other.HedgePosition;
		}
	};

}
