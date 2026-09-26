#pragma once

#include <map>
#include "model/dealing/MarketingDealing.h"

class DealingRepository {

public:
	static DealingRepository& Instance();

	Dealing& Add(Dealing const& dealing);
	Dealing* Get(LocalOrderID localID);

	void Remove(LocalOrderID localID);
private:
	std::map<LocalOrderID, Dealing> m_dealings; // key is localID;
};
