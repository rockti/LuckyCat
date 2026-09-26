#include "pch.h"

#include "strategy/StrategyFactory.h"
#include "strategy/Strategy.h"
#include "strategy/hk_marketing/HKMarketingStrategy.h"
#include "strategy/hf_marketing/NeturalSpreadStrategy.h"
#include "strategy/grab/GrabStrategy.h"
#include "strategy/spread/SpreadStrategy.h"
#include "strategy/mct/MCTStrategy.h"
#include "service/DealingRepository.h"
#include "service/PositionService.h"

using namespace hk;
using namespace hf::netural;
using namespace hf::spread;
using namespace tg::grab;
using namespace mct;

std::shared_ptr<Business> StrategyFactory::NewHKMarketing(UIUpdater* updater, hk::Arguments const& arguments, StrategyPublisher& publisher) {
	return std::shared_ptr<Business>(new hk::HKMarketingStrategy(updater, arguments, publisher, DealingRepository::Instance()));
}
std::shared_ptr<Business> StrategyFactory::NewHFMarketing(UIUpdater* updater, hf::netural::Arguments const& arguments, StrategyPublisher& publisher) {
	return std::shared_ptr<Business>(new NeturalSpreadStrategy(updater, arguments, publisher, DealingRepository::Instance()));
}
std::shared_ptr<Business> StrategyFactory::NewSpread(UIUpdater* updater, hf::spread::Arguments const& arguments, StrategyPublisher& publisher) {
	return std::shared_ptr<Business>(new SpreadStrategy(updater, arguments, publisher, DealingRepository::Instance()));
}
std::shared_ptr<Business> StrategyFactory::NewSuperGrab(UIUpdater* updater, tg::grab::Arguments const& arguments, StrategyPublisher& publisher) {
	return std::shared_ptr<Business>(new GrabStrategy(updater, arguments, publisher, DealingRepository::Instance()));
}
std::shared_ptr<Business> StrategyFactory::NewMCT(UIUpdater* updater, StrategyPublisher& publisher) {
	return std::shared_ptr<Business>(new MCTStrategy(updater, publisher, DealingRepository::Instance()));
}
