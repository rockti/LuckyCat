#pragma once

#include "model/Constants.h"
#include "model/Timer.h"
#include "model/dealing/ExchangeRequest.h"
#include "model/dealing/MarketingDealing.h"

class Trade;
class Quote;
class OrderCancel;
struct PendingItem;
struct StrategyCommand;
struct OrderCommand;
struct TimerCommand;
class StrategyPublisher;
class Instrument;
class DealingRepository;
class Dealing;
class UIUpdater;
class TriggerTimer;


enum class BusinessStatus {
	New,
	Running,
	Pausing,
	Paused,
	STOP,
};

char const* Desc(BusinessStatus status);

class Business {
public:
	Business(UIUpdater* updater, StrategyPublisher& publisher, DealingRepository& dealings, InstID inst=INVALID_INSTRUMENT_ID, InstID exInst = INVALID_INSTRUMENT_ID)
		:m_updater(updater), m_publisher(publisher), m_dealings(dealings), m_grabInst(inst), m_exGrabInst(exInst) {
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
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
		case BusinessStatus::STOP:
		default:
			break;
		}
	}
	void OnQuote(Quote const& quote) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
			DoOnQuote(quote);
			break;
		case BusinessStatus::STOP:
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
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
		case BusinessStatus::STOP:
			DoOnOrdered(dealing);
			break;
		default:
			break;
		}
	}

	void OnCanceled(Dealing const& dealing) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
			DoOnCancelled(dealing);
			break;
		case BusinessStatus::STOP:
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
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
			DoOnQuantityTrade(dealing, quantity);
			break;
		case BusinessStatus::STOP:
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
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
			DoOnOther(dealing);
			break;
		case BusinessStatus::STOP:
			break;
		default:
			break;
		}
	}

	void OnTrade(Trade const& trade) {
		switch (m_status) {
		case BusinessStatus::New:
			break;
		case BusinessStatus::Running:
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
			DoOnTrade(trade);
			break;
		case BusinessStatus::STOP:
			break;
		default:
			break;
		}
	}

	void OnCommand(StrategyCommand const& command) {
		switch (m_status) {
		case BusinessStatus::New:
		case BusinessStatus::Running:
		case BusinessStatus::Pausing:
		case BusinessStatus::Paused:
			DoOnCommand(command);
			break;
		case BusinessStatus::STOP:
			break;
		default:
			break;
		}
	}

	void OnExchangeStatus(Exchange exchange, ExchangeStatus status) {
		DoOnExchangeStatus(exchange, status);
	}

	void OnConnectStatus(Exchange exchange, ConnectStatus connectStatus) {
		DoOnConnectStatus(exchange, connectStatus);
	}

	void OnStatusCommand(StrategyCommand const& command);

	OrderInfo CreateOrder(InstID instID, OrderCommand const& order, QtyT quantity, money price, TSGEProductClassType productClass = SGE_OC_Unknown);
	Dealing& PushOrder(OrderInfo const& orderInfo);
	void PushAction(OrderCancel const& action);
	void PushTimerOrder(TimerCommand const& timerOrder);

	Instrument const& GetInstrument(InstID instID) const;

	MarketingOrderInfo CreateMarketingOrder(InstID instID, int quantity, money buyPrice, money sellPrice);

	MarketingDealing PushMarketingOrder(MarketingOrderInfo const& orderInfo);
	void PushMarketingAction(MarketingOrderCancel const& action);

	inline StrategyPublisher& GetPublisher() const { return m_publisher; }
	inline InstID GetGrabInst() const { return m_grabInst; }
	inline InstID GetExGrabInst() const { return m_exGrabInst; }

	inline bool IsStop() const { return m_status == BusinessStatus::STOP; }
	inline bool IsRunning() const { return m_status == BusinessStatus::Running || m_status == BusinessStatus::Pausing; }
	inline bool IsPausing() const { return m_status == BusinessStatus::Pausing; }
	inline bool IsPaused() const { return m_status == BusinessStatus::Paused; }

	inline void SetStop() { m_status = BusinessStatus::STOP; }		// 不可再启动
	inline void SetPaused() { m_status = BusinessStatus::Paused; }	// 可重新启动

private:
	virtual void DoOnInit() = 0;
	virtual void DoOnQuote(Quote const& quote) = 0;
	virtual void DoOnCommand(StrategyCommand const& command) = 0;
	virtual void DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) = 0;

	virtual void DoOnOrdered(Dealing const& dealing) = 0;
	virtual void DoOnCancelled(Dealing const& dealing) = 0;
	virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) = 0;
	virtual void DoOnOther(Dealing const& dealing) = 0;
	virtual void DoOnTrade(Trade const& trade) = 0;
	virtual void DoOnConnectStatus(Exchange exchange, ConnectStatus connectStatus) = 0;

protected:
	int m_id;
	InstID m_grabInst;		// GrabBusiness专用，不可重复！
	InstID m_exGrabInst;	// GrabBusiness专用，不可重复！
	UIUpdater* m_updater;
	StrategyPublisher& m_publisher;
	DealingRepository& m_dealings;
	BusinessStatus m_status = BusinessStatus::New;
};

struct Data;
class UIUpdater {
public:
	virtual void Update(Data const& data) = 0;
};

struct SnapshotBase {
	UIUpdater* UIUpdaterPtr = nullptr;
	int ID;
	BusinessStatus Status = BusinessStatus::New;
};

template<class BS>
class BusinessUpdater {
public:
	BusinessUpdater(Business* business, BS const& bs) :m_business(business),m_bs(bs), m_oldBS(bs) {

	}
	~BusinessUpdater() {
		if (m_bs.IsChanged(m_oldBS)) {
			m_business->GetPublisher().NotifyUIBusinessSnapshot(m_bs);
		}
	}
private:
	Business* m_business;
	BS const& m_bs;
	BS m_oldBS;
};
