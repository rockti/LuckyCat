#include "strategy/Business.h"
#include "service/QuoteService.h"
#include "model/dealing/OrderCommand.h"
#include "notify/StrategyPublisher.h"
#include "model/dealing/MarketingDealing.h"
#include "model/factory/InstrumentFactory.h"
#include "notify/Notify.h"


OrderInsertRequest Business::CreateOrder(type::InstID instID, OrderCommand const& order, int quantity, type::Money price) {
	auto instrument = ServiceSinglton::Services().Instrument->Get(instID);

	OrderInsertRequest orderInfo;
	orderInfo.InstCode = instrument->Code;
	orderInfo.Exchange = instrument->Exchange;
	orderInfo.InstID = instID;
	orderInfo.Direction = order.Direction;
	orderInfo.OffsetFlag = order.Operation;
	orderInfo.Volume = quantity;
	orderInfo.Price = price;
	orderInfo.OrderPriceType = order.Type;
	orderInfo.OrderLocalID = LocalIdACC::NextNormal();

	return orderInfo;
}

void Business::PushOrder(OrderInsertRequest const& orderInfo) {
	m_publisher.PushOrder(orderInfo);
}

void Business::PushAction(OrderCancelRequest const& action) {
	m_publisher.PushAction(action);
}

void Business::onStatusCommand(StrategyCommand const& command) {
	switch (command.Type) {
	case StrategyCommand::START: {
		m_status = BusinessStatus::Running;
		return;
	}
	case StrategyCommand::PAUSE: {
		m_status = BusinessStatus::Paused;
		return;
	}
	case StrategyCommand::STOP: {
		m_status = BusinessStatus::Stop;
		return;
	}

	}
}

BusinessDealingStore& BusinessDealingStore::Instance() {
	static BusinessDealingStore store;
	return store;

}
