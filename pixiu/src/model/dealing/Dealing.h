#pragma once

#include "model/Constants.h"
#include <vector>
#include "model/dealing/ExchangeRequest.h"
#include "utils/Log.h"

class Business;

class Dealing;
class Trade {
public:
	Trade(Dealing& dealing, money dealPrice, int dealQuantity, char const* systemNo)
		:m_dealing(&dealing), m_dealPrice(dealPrice), m_dealQuantity(dealQuantity) {
		strcpy_s(m_systemNo, systemNo);
	}

	Dealing const& GetDealing() const { return *m_dealing; }
	char const* GetSystemNo() { return m_systemNo; }
	money GetDealPrice() const { return m_dealPrice; }
	int GetDealQuantity() const { return m_dealQuantity; }
	money GetTurnover() const { return m_dealPrice * m_dealQuantity; }
	InstID GetInstID() const;
	LocalOrderID GetLocalId() const;
private:
	Dealing* m_dealing;
	money m_dealPrice;
	int m_dealQuantity;
	char m_systemNo[21];
};

class Dealing {
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
	explicit Dealing(OrderInfo const& order, Business* business = nullptr)
		:m_order(order), m_business(business), m_dealQuantity(0),
		m_cancelQuantity(0), m_remainQuantity(order.Quantity),
		m_quantityTraded(0) {

	}
	Dealing() {}

	inline void* GetHookPtr() const { return m_hookPtr; }
	inline Business* GetBusiness() const { return m_business; }
	inline void SetHookPtr(void* hookPtr) { m_hookPtr = hookPtr; }
	inline void SetBusiness(Business* business) { m_business = business; }

	inline Status GetStatus() const { return m_status; }
	inline LocalOrderID GetLocalId() const { return m_order.LocalId; }
	inline Direction GetDirection() const { return m_order.Direction; }
	inline Operation GetOperation() const { return m_order.Operation; }
	inline Exchange GetExchange() const { return m_order.Exchange; }
	inline InstID GetInstID() const { return m_order.InstID; }
	inline int GetOrderQuantity() const { return m_order.Quantity; }
	inline char GetProductClass() const { return m_order.ProductClass; }
	inline money GetOrderPrice() const { return m_order.Price; }
	inline OrderType GetOrderType() const { return m_order.OrderType; }
	inline InstCodeT const& GetInstCode() const { return m_order.InstCode; }
	inline int GetDealQuantity() const { return m_dealQuantity; }		// 根据成交回报算的成交量
	inline int GetQuantityTraded() const { return m_quantityTraded; }	// 根据委托回报算的成交量
	inline int GetRemainQuantity() const { return m_remainQuantity; }
	inline int GetCancelQuantity() const { return m_cancelQuantity; }
	inline money GetDealPrice() const { return m_dealPrice; }
	inline SystemNoT const& GetSystemNo() const { return m_systemNo; }

	inline bool IsOpenOrder() const { return m_order.Operation == Operation::OPEN; }
	inline bool IsCancelled() const { return m_status == Status::Cancelled; }
	inline bool IsFailed() const { return m_status == Status::CancelFailed || m_status == Status::SendFailed || m_status == Status::FlowControl; }

	inline void OnAccepted() { m_status = Status::Accepted; }	
	inline void OnFlowControl() { m_status = Status::FlowControl; }
	inline void OnSendFailed() { m_status = Status::SendFailed; }
	inline void OnCancelFailed() { m_status = Status::CancelFailed; }
	
	inline void OnQuantityTrade(int traded/*成交总量*/) {
		m_hasQuantityTrade = true;
		m_quantityTraded = traded;
		m_remainQuantity = m_order.Quantity - m_quantityTraded;
		m_status = (m_remainQuantity > 0) ? Status::Partial : Status::Traded;
	}	// OnRtnOrder维护m_quantityTraded
	inline void OnOrdered(char const* systemNo) {
		m_systemNo = systemNo;
		if (m_status < Status::Ordered) {
			m_status = Status::Ordered;	// Order 状态不能覆盖后续状态
		}
	}
	inline void OnCanceled() {
		m_remainQuantity = 0;
		m_cancelQuantity = m_order.Quantity - std::max(m_quantityTraded, m_dealQuantity);
		m_status = Status::Cancelled;
	}
	inline void OnTradeDetail(int quantity/*成交明细*/, money price) {
		m_dealPrice = price;
		m_dealQuantity += quantity;
		m_remainQuantity = m_order.Quantity - m_dealQuantity;
		m_status = (m_remainQuantity > 0) ? Status::Partial : Status::Traded;
	}	// OnRtnTrade维护m_dealQuantity

	inline bool IsCancelSent() const {
		return m_isCancelSent;
	}
	inline void SendCancel() {
		m_isCancelSent = true;
	}

	bool IsNew() const { return m_status == Status::New; }
	bool IsAccepted() const { return m_status == Status::Accepted; }
	bool IsFlowControl() const { return m_status == Status::FlowControl; }
	bool IsSendFailed() const { return m_status == Status::SendFailed; }			// 发送失败（系统）
	bool IsOrdered() const { return m_status == Status::Ordered; }
	bool IsPartial() const { return m_status == Status::Partial; }					// 部分成交
	bool IsAllTrade() const { return m_status == Status::Traded; }					// 全部成交
	bool IsCancelFailed() const { return m_status == Status::CancelFailed; }		// 撤单失败（系统）
	bool IsCanceled() const { return m_status == Status::Cancelled; }

	bool IsAllCanceled() const { return IsCanceled() && m_quantityTraded == 0 && m_dealQuantity == 0; }
	bool IsFinished() const { return (m_hasQuantityTrade && m_quantityTraded == m_order.Quantity)/*全部成交*/ || IsCanceled() || IsAllTrade(); }		//交易行为结束

private:
	Status m_status = Status::New;					// 内部状态
	int m_quantityTraded = 0;						// 委托回报成交总量
	int m_dealQuantity = 0;							// 成交回报分笔成交量
	int m_cancelQuantity = 0;						// 撤单量
	int m_remainQuantity = 0;						// 剩余量
	money m_dealPrice = 0.0;						// 成交均价
	SystemNoT m_systemNo;							// 系统号
	Business* m_business = nullptr;
	void* m_hookPtr = nullptr;
	bool m_hasQuantityTrade = false;
	bool m_isCancelSent = false;
	OrderInfo m_order;
};

inline InstID Trade::GetInstID() const { return m_dealing->GetInstID(); }

inline LocalOrderID Trade::GetLocalId() const { return m_dealing->GetLocalId(); }

inline bool IsBuy(Dealing const& dealing) {
	return IsBuy(dealing.GetDirection());
}
inline bool IsBuy(Trade const& trade) {
	return IsBuy(trade.GetDealing());
}
