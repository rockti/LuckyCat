#pragma once

#include "model/Constants.h"
#include <vector>
#include "model/dealing/ExchangeRequest.h"
#include "type/ZTPType.h"
using namespace ztp;
class Business;

class Dealing;

struct Trade {
	Dealing* DealingPtr;
	type::Price Price;
	type::Volume Volume;
	type::OrderLocalID OrderLocalID;
	type::OrderSysID OrderSysID;
	type::TradeID TradeID;

	type::InstrumentCode const& GetInstCode() const;
	type::InstID GetInstID() const;
};

class Dealing {
public:
	enum class Status {
		New = 0,
		FlowControl,
		SendFailed,
		CancelFailed,
		Accepted,
		Ordered,
		Partial,
		Cancelled,
		Traded,
	};
public:
	Dealing(Business* business, void* hook, OrderInsertRequest const& orderRequest):
		m_business(business),m_hookPtr(hook){
		m_order.OrderStatus = type::OrderStatus::New;
		m_order.Exchange = orderRequest.Exchange;
		m_order.InstID = orderRequest.InstID;
		m_order.InstrumentCode = orderRequest.InstCode;
		m_order.Direction = orderRequest.Direction;
		m_order.OffsetFlag = orderRequest.OffsetFlag;
		m_order.OrderPriceType = orderRequest.OrderPriceType;
		m_order.LimitPrice = orderRequest.Price;
		m_order.Volume = orderRequest.Volume;
		m_order.OrderLocalID = orderRequest.OrderLocalID;
	}

	Dealing() {

	}

	inline void* GetHookPtr() const { return m_hookPtr; }
	inline Business* GetBusiness() const { return m_business; }
	inline void SetHookPtr(void* hookPtr) { m_hookPtr = hookPtr; }
	inline void SetBusiness(Business* business) { m_business = business; }

	inline type::OrderLocalID GetLocalId() const { return m_order.OrderLocalID; }
	inline auto GetDirection() const { return m_order.Direction; }
	inline auto GetOffsetFlag() const { return m_order.OffsetFlag; }
	inline auto GetExchange() const { return m_order.Exchange; }
	inline auto GetInstID() const { return m_order.InstID; }
	inline int GetOrderQuantity() const { return m_order.Volume; }
	inline type::Price GetOrderPrice() const { return m_order.LimitPrice; }
	inline auto GetOrderType() const { return m_order.OrderPriceType; }
	inline auto const& GetInstCode() const { return m_order.InstrumentCode; }
	inline int GetDealQuantity() const { return m_order.VolumeTrade; }
	inline int GetRemainQuantity() const { return m_order.VolumeRemain; }
	inline auto GetDealPrice() const { return m_order.TradePrice; }
	inline auto const& GetSystemNo() const { return m_order.OrderSysID; }

	inline bool IsOpenOrder() const { return m_order.OffsetFlag == type::OffsetFlag::Open; }

	inline bool IsCancelled() const { return m_order.IsCancelled(); }

	inline bool IsInsertFailed() const { return m_order.IsInsertFailed(); }

	inline void OnRtnOrder(InvestorOrder const& order) {
		m_order = order;
	}
	
	inline void OnInsertFail(Error const& error) {
		m_order.InsertError = error;
		m_order.OrderStatus = type::OrderStatus::InsertFail;
	}

	inline void OnCancelFail(Error const& error) {
		m_order.CancelError = error;
		if (!IsFinished()) {
			m_isCancelSent = false;
		}
	}

	inline void OnTrade(int quantity, type::Price price) {

	}

	inline bool IsCancelSent() const {
		return m_isCancelSent;
	}

	inline void SendCancel() {
		m_isCancelSent = true;
	}

	bool IsAccepted() const { return m_order.OrderStatus == type::OrderStatus::InsertQueueing || m_order.OrderStatus == type::OrderStatus::NoTradeQueueing; }
	bool IsOrdered() const { return m_order.OrderStatus == type::OrderStatus::NoTradeQueueing; }
	bool IsPartial() const { return m_order.OrderStatus == type::OrderStatus::PartTradedQueueing; }					//部分成交
	bool IsAllTrade() const { return m_order.IsAllTraded(); }						//全部成交
	bool IsCancelFailed() const { return m_order.OrderStatus == type::OrderStatus::CancelFail; }		//撤单失败（系统）
	bool IsCanceled() const { return m_order.IsCancelled();  }

	bool IsDealed() const { return IsPartial() || IsAllTrade(); }
	bool IsAllCanceled() const { return IsCanceled() && GetDealQuantity() == 0; }		//全部撤单，没有成交
	bool IsFinished() const { return IsCanceled() || IsAllTrade(); }					//交易行为结束
	bool IsDealAndFinished() const { return IsFinished() && GetDealQuantity() > 0; }		//交易行为结束，有成交

	bool IsPending() const {
		return IsAccepted() || IsOrdered() || IsPartial();
	}
private:
	InvestorOrder m_order;
	Business* m_business = nullptr;
	void* m_hookPtr = nullptr;
	bool m_isCancelSent = false;
};

