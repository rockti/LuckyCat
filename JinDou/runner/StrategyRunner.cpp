
#include "runner/StrategyRunner.h"
#include "strategy/Business.h"
#include "service/Service.h"
#include "notify/Notify.h"
#include "notify/StrategyPublisher.h"
#include "utils/Log.h"
#include "model/factory/InstrumentFactory.h"
#include <fmt/format.h>

void StrategyHandler::Handle(Data const& data) {
	switch (data.Type) {
	case DataType::StrategyCommand: {
		StrategyCommand const& command = *(StrategyCommand*)data.Payload;
		onCommand(command);
	}
	break;
	case DataType::Quote: {
		Quote const& quote = *(Quote*)data.Payload;
		onQuote(quote);
	}
	break;
	case DataType::Order: {
		InvestorOrder const& order = *(InvestorOrder*)data.Payload;
		onOrder(order);
	}
	break;
	case DataType::Trade: {
		InvestorTrade const& trade = *(InvestorTrade*)data.Payload;
		onTrade(trade);
	}
	break;
	case DataType::OrderErrorRtn: {
		OrderErrorNotification const& orderError = *(OrderErrorNotification*)data.Payload;
		onOrderError(orderError);
	}
	break;
	case DataType::Exchange: {
		TradePlatformNotification const& exchangeNotif = *(TradePlatformNotification*)data.Payload;
		onExchangeNotification(exchangeNotif);
	}
	break;
	default:
		break;
	}
}

void StrategyHandler::onCommand(StrategyCommand const& command) {
	switch (command.Type) {
	case StrategyCommand::ADD_BUSINESS:
		addBusiness(command.Business);
		break;
	default:
		auto iter = m_businesses.find(command.BusinessID);
		if (iter != m_businesses.end()) {
			iter->second->OnCommand(command);
			if (command.Type == StrategyCommand::DELETE_BUSINESS) {
				m_businesses.erase(iter);
			}
		}
		break;
	}
}

void StrategyHandler::onQuote(Quote const& quote) {
//	Quote const& exited = ServiceSinglton::Services().Quote->Update(quote);

	for (auto& kv: m_businesses) {
		kv.second->OnQuote(quote);
	}
}

void StrategyHandler::onOrderError(OrderErrorNotification const& orderError) {
	Dealing* dealing = BusinessDealingStore::Instance().FindDealing(orderError.OrderLocalID);
	if (dealing) {
		auto* business = dealing->GetBusiness();
		if (orderError.Type == OrderErrorNotification::Insert) {
			dealing->OnInsertFail(orderError.Error);
			business->OnInsertFail(*dealing);
		} else if (orderError.Type == OrderErrorNotification::Cancel){
			dealing->OnCancelFail(orderError.Error);
			business->OnCancelFail(*dealing);
		}
	}
}

void StrategyHandler::onExchangeNotification(TradePlatformNotification const& exchange) {
// 	ConnectStatus connectStatus = ConnectStatus::None;
// 	switch (exchange.Type) {
// 	case ExchangeNotification::QUOTE_CONNECTED:
// 		connectStatus = ConnectStatus::QuoteConnected;
// 		break;
// 	case ExchangeNotification::QUOTE_DISCONNECTED:
// 		connectStatus = ConnectStatus::QuoteDisconnected;
// 		break;
// 	case ExchangeNotification::TRADER_CONNECTED:
// 		connectStatus = ConnectStatus::TradeConnected;
// 		break;
// 	case ExchangeNotification::TRADER_DISCONNECTED:
// 		connectStatus = ConnectStatus::TradeDisconnected;
// 		break;
// 	default:
// 		break;
// 	}
// 
// 	for (auto business : m_businesses) {
// 		business->OnConnectStatus(exchange.Exchange, connectStatus);
// 	}

	//todo
}

void StrategyHandler::onOrder(InvestorOrder const& order) {
	Dealing* dealing = BusinessDealingStore::Instance().FindDealing(order.OrderLocalID);
	if (dealing) {
		dealing->OnRtnOrder(order);
		auto* business = dealing->GetBusiness();
		if (order.IsCancelled()) {
			business->OnCanceled(*dealing);
		} else if (!order.OrderSysID.empty() && order.OrderStatus == type::OrderStatus::NoTradeQueueing) {
			business->OnOrdered(*dealing);
		} else {
			business->OnOther(*dealing);
		}
	}
}

void StrategyHandler::onTrade(InvestorTrade const& trade) {
// 	Dealing* dealing = nullptr;
// 	auto order = ServiceSinglton::Services().Order->GetOrder(trade.OrderSysID);
// 	if (order) {
// 		dealing = BusinessDealingStore::Instance().FindDealing(order->OrderLocalID);
// 	}
	Dealing* dealing = BusinessDealingStore::Instance().FindDealing(trade.OrderLocalID);
	if (dealing) {
//		dealing->OnRtnOrder(trade.TradePrice,trade.TradeVolume);
		dealing->GetBusiness()->OnTrade(trade);
	}

}

void StrategyHandler::addBusiness(Business* business) {
	business->OnInit();
	m_businesses.emplace(business->GetId(), business);
}
