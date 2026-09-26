#pragma once

#include "model/Constants.h"
#include "model/dealing/Dealing.h"
#include "model/Quote.h"

class Business;
class IQuoteHandler;
class DealingRepository;
struct Data;
class Quote;
struct StrategyCommand;
struct OrderNotification;
struct FundsNotification;
struct ExchangeNotification;
class Dealing;
class DealingRepository;
class StrategyPublisher;

class DealingCircleStore {
	static const size_t SIZE = 20;
public:
	Dealing* Get(LocalOrderID localId) {
		for (Dealing& dealing : m_dealings) {
			if (dealing.GetLocalId() == localId) {
				return &dealing;
			}
		}
		return nullptr;
	}
	void Add(Dealing const& dealing) {
		m_dealings[m_currentPos % SIZE] = dealing;
		++m_currentPos;
	}

private:
	size_t m_currentPos = 0;
	std::array<Dealing, SIZE> m_dealings;
};

class StrategyHandler {
public:
	StrategyHandler(StrategyPublisher& publisher, DealingRepository& dealingRepository) 
		: m_publisher(publisher), m_dealingRepository(dealingRepository) {}
	void Handle(Data const& data);

private:
	void onCommand(StrategyCommand const& command);
	void onQuote(Quote const& quote);
	void onPrice(InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, money lastPrice, int volume, int interest, int id);
	void onFundsNotification(FundsNotification const& funds);
	void onOrderNotification(OrderNotification const& command);
	void onExchangeNotification(ExchangeNotification const& exchange);

	void onDealing(OrderNotification const& order);
	void handleDealing(const OrderNotification& order, Dealing& dealing);
	void handleGrabOrder(OrderNotification const& order);
	void handleOtherOrder(OrderNotification const& order);
	void removeFinishedDealing(Dealing const& dealing);
	void removeDealing(Dealing const& dealing);
	void removeMarketingDealings(Dealing const& dealing);
	void onTradeDetail(OrderNotification const& order, Dealing& dealing);
	void onFAKPartial(OrderNotification const& order, Dealing& dealing);
	void onQuantityTrade(Dealing& dealing, const OrderNotification& order);
	void onFuckESunnyA(OrderNotification const& order, Dealing& dealing);

	void addBusiness(Business* businessPtr);
	Dealing* findDealing(LocalOrderID localId);

	StrategyPublisher& m_publisher;
	DealingRepository& m_dealingRepository;

	std::vector<InstID> m_instIDs;
	std::vector<Business*> m_businesses;
	DealingCircleStore m_deletedDealings;
};

