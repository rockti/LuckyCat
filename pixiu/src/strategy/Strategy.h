#pragma once

#include "model/Constants.h"
#include "strategy/Business.h"
#include "service/QuoteService.h"
#include "notify/StrategyPublisher.h"
#include "model/dealing/Dealing.h"

class OrderCancel;
class UIUpdater;


template<class Arg>
class Strategy : public Business {
public:
	Strategy(UIUpdater* updater, Arg const& arg, StrategyPublisher& publisher, DealingRepository& dealings, InstID inst = INVALID_INSTRUMENT_ID, InstID exInst = INVALID_INSTRUMENT_ID)
		:Business(updater, publisher, dealings, inst, exInst), m_arg(arg) {

	}
	
	virtual ~Strategy() = 0 {}

	Arg const& GetArg() const { return m_arg; }

protected:
	Arg m_arg;
};