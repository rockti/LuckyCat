#pragma once

#include "model/Constants.h"
#include "service/Service.h"

class AccountService : public ServiceBase<IAccountObserver,IAccountService> {
public:
	AccountService();
	virtual void Update(TradePlatform platform, InvestorAccount const& account) override;
	virtual AccountResult Get(TradePlatform platform) override;
private:
	void onChange(InvestorAccount const& account);
	InvestorAccount accounts_[10];
};