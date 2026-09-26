#pragma once

#include "model/Constants.h"
#include "strategy/Business.h"
#include "service/QuoteService.h"
#include "notify/StrategyPublisher.h"
#include "model/dealing/Dealing.h"

class OrderCancelRequest;
class UIUpdater;


template<class Arg>
class Strategy : public Business {
public:
	Strategy(Arg const& arg, StrategyPublisher& publisher, BusinessDealingStore& dealings)
		:Business(publisher,dealings),m_arg(arg) {
	}
	virtual ~Strategy() = 0 {}

	Arg const& GetArg() const { return m_arg; }

protected:
	Arg m_arg;
};