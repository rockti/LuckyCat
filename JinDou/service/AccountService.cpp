
#include "service/AccountService.h"


AccountService::AccountService() {
}

void AccountService::Update(TradePlatform platform, InvestorAccount const& account) {
	accounts_[uint8_t(platform)] = account;
	for (auto& o : observers_) {
		o->OnChange(account);
	}
}

AccountResult AccountService::Get(TradePlatform platform) {
	return { accounts_[uint8_t(platform)] };
}
