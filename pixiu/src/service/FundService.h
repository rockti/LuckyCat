#pragma once

#include "model/Constants.h"
#include "account/Funds.h"

class QuoteService;
class Dealing;
class Trade;
// 
// class FundsService
// {
// public:
// 	static FundsService& Instance();
// 	void Freeze(Exchange exchange, LocalOrderID localID, money freeze);
// 	money FreezeDealingFunds(Dealing const& dealing);
// 	void OnDealing(Dealing const& dealing);
// 	void OnTrade(Trade const& trade);
// 
// 	bool Enough(Exchange exchange, money m);
// 
// 	void SetAvailable(ConnPlatform platform, money available) {
// 		Funds& funds = m_funds[PlatformIndex(platform)];
// 		funds.SetAvailable(available);
// 	}
// 	inline Funds* GetFunds(ConnPlatform platform) {
// 		return &m_funds[PlatformIndex(platform)];
// 	}
// 
// private:
// 	Operation SubdivideOffsetOperation(Trade const& trade);		//细分平仓和平今
// 	void DealWithOpenTrade(Trade const& trade);
// 	void DealWithOffsetTrade(Trade const& trade);
// 
// 	inline Funds& GetFunds(Exchange exchange) {
// 		return m_funds[ConnIndex(exchange)];
// 	}
// 	Funds m_funds[3];
// };