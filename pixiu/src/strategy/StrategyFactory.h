#pragma once
#include "model/Constants.h"

namespace hk {
	struct Arguments;
}

namespace hf::netural{
	struct Arguments;
}

namespace hf::spread {
	struct Arguments;
}

namespace tg::grab {
	struct Arguments;
}

class StrategyPublisher;
class Business;
class UIUpdater;
class StrategyFactory {
public:
	static std::shared_ptr<Business> NewHKMarketing(UIUpdater* updater, hk::Arguments const& arguments, StrategyPublisher& publisher);
	static std::shared_ptr<Business> NewHFMarketing(UIUpdater* updater, hf::netural::Arguments const& arguments, StrategyPublisher& publisher);
	static std::shared_ptr<Business> NewSpread(UIUpdater* updater, hf::spread::Arguments const& arguments, StrategyPublisher& publisher);
	static std::shared_ptr<Business> NewSuperGrab(UIUpdater* updater, tg::grab::Arguments const& arguments, StrategyPublisher& publisher);
	static std::shared_ptr<Business> NewMCT(UIUpdater* updater, StrategyPublisher& publisher);
};