#include "pch.h"

#include "strategy/Business.h"
#include "service/QuoteService.h"
#include "model/dealing/OrderCommand.h"
#include "notify/StrategyPublisher.h"
#include "service/DealingRepository.h"
#include "model/dealing/MarketingDealing.h"
#include "model/factory/InstrumentFactory.h"
#include "notify/Notify.h"


OrderInfo Business::CreateOrder(InstID instID, OrderCommand const& order, QtyT quantity, money price, TSGEProductClassType productClass) {
	auto& instrument = GetInstrument(instID);

	OrderInfo orderInfo;
	orderInfo.InstCode = instrument.GetInstCode();
	orderInfo.Exchange = instrument.GetExchange();
	orderInfo.InstID = instID;
	orderInfo.Direction = order.Direction;
	orderInfo.Operation = order.Operation;
	orderInfo.Quantity = quantity;
	orderInfo.Price = price;
	orderInfo.OrderType = order.Type;
	orderInfo.LocalId = LocalIdACC::NextNormal();
	orderInfo.ProductClass = productClass;

	return orderInfo;
}
Dealing& Business::PushOrder(OrderInfo const& orderInfo) {
	Dealing dealing{ orderInfo };
	dealing.SetBusiness(this);
	m_publisher.PushOrder(orderInfo);
	return m_dealings.Add(dealing);
}
void Business::PushAction(OrderCancel const& action) {
	m_publisher.PushAction(action);
}
void Business::PushTimerOrder(TimerCommand const& timerOrder) {
	m_publisher.PushTimerOrder(timerOrder);
}

Instrument const& Business::GetInstrument(InstID instrumentID) const {
	return InstrumentFactory::GetInstruments().Get(instrumentID);
}

MarketingOrderInfo Business::CreateMarketingOrder(InstID instID, int quantity, money buyPrice, money sellPrice) {
	auto& instrument = GetInstrument(instID);

	MarketingOrderInfo order;
	order.Exchange = instrument.GetExchange();
	order.InstID = instID;
	order.InstCode = instrument.GetInstCode();
	order.BuyLocalId = LocalIdACC::NextBuyMarketing();
	order.SellLocalId = LocalIdACC::SellMarketingID(order.BuyLocalId);
	order.Quantity = quantity;
	order.BuyPrice = buyPrice;
	order.SellPrice = sellPrice;
	return order;
}
MarketingDealing Business::PushMarketingOrder(MarketingOrderInfo const& orderInfo) {
	Dealing& buyDealing = m_dealings.Add(Dealing{orderInfo.GetBuyOrder(),this});
	Dealing& sellDealing = m_dealings.Add(Dealing{orderInfo.GetSellOrder(), this});
	m_publisher.PushOrder(orderInfo);
	return MarketingDealing{buyDealing,sellDealing};
}
void Business::PushMarketingAction(MarketingOrderCancel const& action) {
	m_publisher.PushAction(action);
}

void Business::OnStatusCommand(StrategyCommand const& command) {
	if (BusinessStatus::STOP == m_status) {
		return;
	}

	switch (command.Type) {
	case StrategyCommand::START: {
		if (BusinessStatus::New == m_status || BusinessStatus::Paused == m_status) {
			m_status = BusinessStatus::Running;
		}
		return;
	}
	case StrategyCommand::PAUSE: {
		if (BusinessStatus::New == m_status) {
			m_status = BusinessStatus::Paused;
		}
		else if (BusinessStatus::Running == m_status) {
			m_status = BusinessStatus::Pausing;
		}
		return;
	}
	case StrategyCommand::STOP: {
		m_status = BusinessStatus::STOP;
		return;
	}
	}
}

char const* Desc(BusinessStatus status) {
	switch (status) {
	case BusinessStatus::Running:
		return "Running";
	case BusinessStatus::Pausing:
		return "Pausing";
	case BusinessStatus::Paused:
		return "Paused";
	case BusinessStatus::STOP:
		return "Stop";
	default:
		return "New";
	}
}

