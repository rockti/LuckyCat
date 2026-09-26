#include "model/dealing/Dealing.h"

/*
Dealing::Dealing(OrderInfo const& orderInfo)
	: m_orderInfo(orderInfo), m_dealQuantity(0), m_remainQuantity(orderInfo.Quantity), m_cancelQuantity(0)
	, m_dealPrice(0.00), m_dealingStatus(D_SEND), m_isFree(false)

{
	memset(m_systemNo, 0, sizeof(m_systemNo));
	m_tradeVector.clear();
}

// char const* Dealing::GetOrderStatusName(OrderNotification::ORDER_STATUS status) const
// {
// 	switch (status)
// 	{
// 	case OrderNotification::E_ACCEPT:
// 		return "ctp-accept";
// 	case OrderNotification::E_ORDERED:
// 		return "ordered";
// 	case OrderNotification::E_SENT_FAILED:
// 		return "send failed";
// 	case OrderNotification::E_CANCELED:
// 		return "canceled";
// 	case OrderNotification::E_CANCEL_FAILED:
// 		return "cancel failed";
// 	case OrderNotification::E_PARTIAL:
// 		return "partial";
// 	case OrderNotification::E_TRADE:
// 		return "trade";
// 	default:
// 		return "unknown";
// 	}
// }



char const* Dealing::GetDealingStatusName() const
{
	switch (m_dealingStatus)
	{
	case D_SEND:
		return "send";
	case D_ACCEPTED:
		return "accepted";
	case D_SEND_FAILED:
		return "send failed";
	case D_ORDERED:
		return "ordered";
	case D_PARTIAL:
		return "partial";
	case D_TRADE:
		return "trade";
	case D_CANCEL_FAILED:
		return "cancel failed";
	case D_CANCELED:
		return "canceled";
	default:
		return "unknown";
	}
}

bool Dealing::OnAccepted()
{
	if (IsSend() || (IsRetry() || IsSendFailed()))		//CTP抢单用到后两种情况
	{
		m_dealingStatus = D_ACCEPTED;
		return true;
	}
	return false;
}

bool Dealing::OnFlowControl()
{
	if (IsSend() || IsAccepted() || IsOrdered() || IsRetry())			//如果是SEND_FAILED状态不再到Retry状态,撤单时是Ordered状态
	{
		m_dealingStatus = D_RETRY;
		return true;
	}
	return false;
}

bool Dealing::OnSendFailed()
{
	//IsAccepted():errCode41:交易所每秒发送请求数超过许可数
	if (IsSend() || IsAccepted() || IsRetry() || IsSendFailed())
	{
		m_dealingStatus = D_SEND_FAILED;
		return true;
	}
	return false;
}

bool Dealing::OnOrdered(char const* systemNo)
{
	assert(NULL != systemNo);
	if (IsSend() || IsAccepted() || IsSendFailed() || IsRetry())		//本地委托超时、断线都是SendFailed，然后收到交易所应答
	{
		strcpy_s(m_systemNo, systemNo);
		m_dealingStatus = D_ORDERED;
		return true;
	}
	return false;
}

bool Dealing::OnCanceled()
{
	//IsAccepted():报单尚在CTP服务器，如果要取消CTP的Accepted状态要将IsAccepted()改成IsSend()
	if (IsAccepted() || IsOrdered() || IsPartial() || IsCancelFailed() || IsRetry())
	{
		m_remainQuantity = 0;
		m_cancelQuantity = m_orderInfo.Quantity - m_dealQuantity;
		m_dealingStatus = D_CANCELED;
		return true;
	}
	return false;
}

bool Dealing::OnCancelFailed()
{
	//IsAccepted():报单尚在CTP服务器，如果要取消CTP的Accepted状态要将IsAccepted()改成IsSend()
	if (IsAccepted() || IsOrdered() || IsPartial() || IsCancelFailed() || IsRetry())
	{
		m_dealingStatus = D_CANCEL_FAILED;
		return true;
	}
	return false;
}

bool Dealing::OnTrade(Trade const& trade)
{
	//如果先收到撤单回报，则自行模拟成交回报，不再收取成交回报
	if (IsOrdered() || IsPartial())
	{
		int remainQuantity = m_orderInfo.Quantity - m_dealQuantity - trade.GetDealQuantity();
		if (remainQuantity >= 0)
		{
			m_dealQuantity += trade.GetDealQuantity();
			m_remainQuantity = m_orderInfo.Quantity - m_dealQuantity;
			m_tradeVector.push_back(trade);
			m_dealPrice = CalculateAveragePrice();
			m_dealingStatus = (m_remainQuantity == 0) ? D_TRADE : D_PARTIAL;
			return true;
		}
		//Log::Error(FT("Dealing::OnTrade() OrderQuantity[%d] < (DealQuantity[%d]+TradeQuantity[%d])", m_orderInfo.quantity, m_dealQuantity, trade.GetDealQuantity()));
	}
	return false;
}

money Dealing::CalculateAveragePrice()
{
	money sum = 0.00;
	for (size_t index = 0; index < m_tradeVector.size(); ++index)
	{
		Trade const& trade = m_tradeVector[index];
		sum += trade.GetTurnover();
	}
	return sum / m_dealQuantity;
}


InstID Trade::GetInstID() const
{
	return m_dealing->GetInstID();
}

*/

type::InstrumentCode const& Trade::GetInstCode() const {
	return DealingPtr->GetInstCode();
}

type::InstID Trade::GetInstID() const {
	return DealingPtr->GetInstID();
}
