#pragma once

#include"model/Constants.h"

typedef uint32_t FreezeID;

class Dealing;

class Account {
public:
	Account(ConnPlatform platform) {

	}

	void Clear() {

	}

	void InitAvailable(money available) {

	}

	void InitPositions() {

	}

	bool Freeze(FreezeID freezeID, money amount) { return false; }
	bool Unfreeze(FreezeID freezeID) { return true; }

	bool ApplyDealing(Dealing const& dealing) {
		return false;
	}
};