#pragma once

#include <optional>
#include "EntityCopy.h"
#include "ErrorDefine.h"
#include "ZTPType.h"

using namespace ztp;

class Investor;
class Order;


class Trade {
public:
	Trade(Order& order, data::CTrade* pTrade);

	Order const& GetOrder() const{
		return m_order;
	}

	int GetVolume() const {
		return m_trade->TradeVolume;
	}

	double GetPrice() const {
		return m_trade->TradePrice;
	}

	int GetMultiple() const;

	double GetTradeFee() const {
		return m_trade->UsedFee;
	}

	double GetMargin() const {
		return m_trade->UsedMargin;
	}

	data::CTrade* GetOriginalTrade() const {
		return m_trade;
	}

	type::ProductClass GetProductClass() const {
		return m_trade->ProductClass;
//		return m_order.GetOriginalOrder()->getInstrument()->ProductClass;
	}

	bool IsSpot() const;
private:
	Order& m_order;
	data::CTrade* m_trade;
};

inline bool IsLong(data::COrder const* order) {
	return type::IsBuy(order)&& type::IsOpen(order);
}

inline bool IsShort(data::COrder const* order) {
	return type::IsSell(order) && type::IsOpen(order);
}


class Order {
public:
	Order(Investor& investor, data::COrder* order, data::CInstrument* instrument) :
		m_investor(&investor),m_order(order),m_instrument(instrument) {
	}
	Order() {
	}
	Order(Investor& investor, int insertError) :
		m_investor(&investor),m_insertErrorCode(insertError) {
	}

	int GetInsertError() const { return m_insertErrorCode; }
	Order(Order const& other) = default;
//	Order& operator=(Order other) = default;
	Trade CreateTrade(data::CTrade const& trade);
	void OnAccepted(char const* orderSysID);
	void OnInsertFailed(Error error, bool unfreeze = true);
	void OnCancelSending(type::UserOrderLocalID UserOrderActionLocalID, type::OrderLocalID ActionLocalID);
	void OnCancelFailed(Error error);
	void OnCancelled();

	void OnOrder(data::COrder& notifyOrder);
	void OnTrade(Trade const& trade);

	bool IsLong() const;
	bool IsShort() const;
	bool IsSpot() const;
	type::OrderLocalID GetOrderLocalID() const {
		return m_order->OrderLocalID;
	}

	type::OrderLocalID GetCancelLocalID() const {
		return m_order->ActionLocalID;
	}
	double GetFreezeMargin() const {
		return m_order->FrozenMargin;
	}
	double GetFreezeFee() const {
		return m_order->FrozenFee;
	}

	int GetVolume() const {
		return m_order->Volume;
	}
	double GetPrice() const {
		return m_order->LimitPrice;
	}

	auto GetDirection() const {
		return m_order->Direction;
	}

	auto GetOffsetFlag() const {
		return m_order->OffsetFlag;
	}

	auto GetStatus() const {
		return m_order->OrderStatus;
	}

	bool IsOffset() const {
		return m_order->OffsetFlag == type::OffsetFlag::Close;
	}

	data::CInstrument const& GetInstrument() const {
		return *m_instrument;
	}
	data::CInvestorMargin const& GetInvestorMargin() const {
		return *m_order->getInvestorMargin();
	}
	data::CInvestorFee const& GetInvestorFee() const {
		return *m_order->getInvestorFee();
	}

	data::CInvestorPosition* GetInvestorPosition() const {
		return m_order->getInvestorPosition();
	}

	bool IsNew() const {
		return m_order->OrderStatus == type::OrderStatus::New;
	}

	data::COrder* GetOriginalOrder() const {
		return m_order;
	}

private:
	Investor* m_investor = nullptr;
	data::COrder* m_order = nullptr;
	data::CInstrument* m_instrument = nullptr;
	int m_insertErrorCode = 0;
	int m_cancelErrorCode = 0;
};


