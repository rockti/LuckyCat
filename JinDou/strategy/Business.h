#pragma once

#include "model/Constants.h"
#include "model/dealing/ExchangeRequest.h"
#include "model/dealing/MarketingDealing.h"
#include <map>

class Trade;
class Quote;
class OrderCancelRequest;
struct PendingItem;
struct StrategyCommand;
struct OrderCommand;
class StrategyPublisher;
class BusinessDealingStore;
class Dealing;
class UIUpdater;
class Business;

class BusinessDealingStore {
public:
	static BusinessDealingStore& Instance();
	Dealing& Add(Dealing const& dealing) {
		auto result = m_dealings.emplace(dealing.GetLocalId(), dealing);
		return result.first->second;
	}

	void Remove(type::OrderLocalID localID) {
		m_dealings.erase(m_dealings.find(localID));
	}

	Dealing* FindDealing(type::OrderLocalID localID) {
		auto iter = m_dealings.find(localID);
		if (iter != m_dealings.end()) {
			return &iter->second;
		}
		return nullptr;
	}
private:
	std::map<type::OrderLocalID, Dealing> m_dealings;
};


enum class BusinessStatus {
	New,
	Running,
	Paused,
	Stop,
};

class Business {
public:
	Business(StrategyPublisher& publisher, BusinessDealingStore& dealings)
		:m_publisher(publisher), m_dealings(dealings) {
		static int s_id = 0;
		m_id = s_id++;
	}
	virtual ~Business() = 0 {}

	int GetId() const { return m_id; }

	void OnInit() {
		switch (m_status) {
		case BusinessStatus::New:
			DoOnInit();
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
		case BusinessStatus::Stop:
		default:
			break;
		}
	}
	void OnQuote(Quote const& quote) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnQuote(quote);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnOrdered(Dealing const& dealing) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
		case BusinessStatus::Stop:
			DoOnOrdered(dealing);
			break;
		default:
			break;
		}
	}

	void  OnCanceled(Dealing const& dealing) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnCancelled(dealing);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnQuantityTrade(Dealing const& dealing, int quantity) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnQuantityTrade(dealing, quantity);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnInsertFail(Dealing const& dealing) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnInsertFail(dealing);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnCancelFail(Dealing const& dealing) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnCancelFail(dealing);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnOther(Dealing const& dealing) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnOther(dealing);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnTrade(InvestorTrade const& trade) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnTrade(trade);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnCommand(StrategyCommand const& command) {
		switch (m_status) {
		case BusinessStatus::New:
		case BusinessStatus::Running:
		case BusinessStatus::Paused:
			DoOnCommand(command);
			break;
		case BusinessStatus::Stop:
			break;
		default:
			break;
		}
	}

	void OnExchangeStatus(type::Exchange exchange, ExchangeStatus status) {
		DoOnExchangeStatus(exchange, status);
	}

	void OnConnectStatus(type::Exchange exchange, ConnectStatus connectStatus) {
		DoOnConnectStatus(exchange, connectStatus);
	}

	void OnPendingMarket(PendingItem const& pendingItem) {
		DoOnPendingMarket(pendingItem);
	}

	void UpdateSnapshot(bool deleted) {
		DoUpdateSnapshot(deleted);
	}
	void onStatusCommand(StrategyCommand const& command);

	OrderInsertRequest CreateOrder(type::InstID instID, OrderCommand const& order, int quantity, type::Money price);
	void PushOrder(OrderInsertRequest const& orderInfo);
	void PushAction(OrderCancelRequest const& action);

	inline StrategyPublisher& GetPublisher() const { return m_publisher; }

	inline bool IsRunning() const { return m_status == BusinessStatus::Running; }
	inline bool IsStopped() const { return m_status == BusinessStatus::Stop; }
	inline bool IsPaused() const { return m_status == BusinessStatus::Paused; }
	inline bool IsNew() const { return m_status == BusinessStatus::New; }

	// add 4 test
	void SetPause() { m_status = BusinessStatus::Paused; }
	void SetRunning() { m_status = BusinessStatus::Running; }

	BusinessDealingStore& GetDealingStore() {
		return m_dealings;
	}
private:
	virtual void DoOnInit() = 0;
	virtual void DoOnQuote(Quote const& quote) = 0;
	virtual void DoOnPendingMarket(PendingItem const& pendingItem) = 0;
	virtual void DoOnCommand(StrategyCommand const& command) = 0;
	virtual void DoOnExchangeStatus(type::Exchange exchange, ExchangeStatus status) = 0;

	virtual void DoOnOrdered(Dealing const& dealing) = 0;
	virtual void DoOnCancelled(Dealing const& dealing) = 0;
	virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) = 0;
	virtual void DoOnOther(Dealing const& dealing) = 0;
	virtual void DoOnInsertFail(Dealing const& dealing) = 0;
	virtual void DoOnCancelFail(Dealing const& dealing) = 0;
	virtual void DoOnTrade(InvestorTrade const& trade) = 0;
	virtual void DoOnConnectStatus(type::Exchange exchange, ConnectStatus connectStatus) = 0;
	virtual void DoUpdateSnapshot(bool deleted) = 0;

protected:
	int m_id;
	StrategyPublisher& m_publisher;
	BusinessDealingStore& m_dealings;
	BusinessStatus m_status = BusinessStatus::New;
};

struct Data;

struct SnapshotBase {
	int ID;
	BusinessStatus Status = BusinessStatus::New;
};



template<class BS>
class BusinessUpdater {
public:
	BusinessUpdater(Business* business, BS const& bs, bool isDeleted = false) :
		m_business(business),m_bs(bs), m_oldBS(bs), m_deleted(isDeleted) {

	}
	~BusinessUpdater() {
		m_business->UpdateSnapshot(m_deleted);
		if (m_bs.IsChanged(m_oldBS)) {
			m_business->GetPublisher().NotifyUIBusinessSnapshot(m_bs);
		}
	}
	void SetDeleted() {
		m_deleted = true;
	}
private:
	Business* m_business;
	BS const& m_bs;
	BS m_oldBS;
	bool m_deleted;
};
