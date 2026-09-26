#pragma once

#include "service/Service.h"

class StrategyService : public ServiceBase<IStrategyObserver,IStrategyService> {
public:
	virtual void OnUpdate(int id, automate::AutoSnapshot const& snapshot) override;

private:
};