#pragma once

#include "ZTPType.h"

using namespace ztp;

class MaxOrderLocalIDSinglton {
public:
	static inline uint32_t MIN_LOCAL_ID = 10'000'000;
public:
	static void InitMinLocalID(type::OrderLocalID minLocalID) {
		if (minLocalID >= MIN_LOCAL_ID) {
			MIN_LOCAL_ID = minLocalID;
		}
		m_maxOrderLocalID = MIN_LOCAL_ID;
		printNlog("MaxID=[{}]", m_maxOrderLocalID);
	}
	static void InitMaxOrderLocalID(type::OrderLocalID const& maxLocalID) {
		if (!IsMine(maxLocalID)) return;

		if (maxLocalID < MIN_LOCAL_ID) {
			m_maxOrderLocalID = MIN_LOCAL_ID;
		} else {
			m_maxOrderLocalID = maxLocalID + 100; // add 100 for safe
		}
		printNlog("MaxID=[{}]", m_maxOrderLocalID);
	}
	static inline bool IsMine(type::OrderLocalID const& localID) {
		return localID >= MIN_LOCAL_ID && localID < MIN_LOCAL_ID + 1'000'0000;
	}
	static uint32_t Get() {
		return m_maxOrderLocalID;
	}
	static uint32_t GetNextOrderLocalID() {
		return ++m_maxOrderLocalID;
	}
private:
	static uint32_t m_maxOrderLocalID;
};

