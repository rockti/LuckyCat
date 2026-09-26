#pragma once
#include "Service.h"

#include "PositionService.h"
#include "AccountService.h"
#include "TradeService.h"
#include "OrderService.h"
#include "ExchangeService.h"
#include "TradeSystemService.h"
#include "InstrumentService.h"
#include "QueryService.h"
#include "QuoteService.h"
#include "InitService.h"
#include "StrategyService.h"
static ServiceStore s_services;

void ServiceSinglton::InitAccountService(IAccountObserver* observer)  {
	if (s_services.Account == nullptr) {
		s_services.Account = new AccountService();
		s_services.Account->Register(observer);
	}
}

void ServiceSinglton::InitPositionService(IPositionObserver* observer) {
	if (s_services.Position == nullptr) {
		s_services.Position = new PositionService();
		s_services.Position->Register(observer);
	}
}

void ServiceSinglton::InitTradeService(ITradeObserver* observer) {
	if (s_services.Trade == nullptr) {
		s_services.Trade = new TradeService();
		s_services.Trade->Register(observer);
	}
}

void ServiceSinglton::InitOrderService(IOrderObserver* observer) {
	if (s_services.Order == nullptr) {
		s_services.Order = new OrderService();
		s_services.Order->Register(observer);
	}

}

void ServiceSinglton::InitTradeSystemService(IExchangeStatusObserver* exchangeObserver , ITradeSystemObserver* tradeSystemObserver) {
	if (s_services.TradeSystem == nullptr) {
		s_services.TradeSystem = new TradeSystemService();
		s_services.TradeSystem->Register(exchangeObserver);
		s_services.TradeSystem->Register(tradeSystemObserver);
	}
}

void ServiceSinglton::InitInstrumentService(IInstrumentObserver* instObserver) {
	if (s_services.Instrument == nullptr) {
		s_services.Instrument = new InstrumentService();
		s_services.Instrument->Register(instObserver);
	}

}
void ServiceSinglton::InitExchangeService() {
	if (s_services.Exchange == nullptr) {
		s_services.Exchange = new ExchangeService();
	}
}

void ServiceSinglton::InitQueryService() {
	if (s_services.Query == nullptr) {
		s_services.Query = new QueryService(*s_services.TradeSystem);
	}

}

void ServiceSinglton::InitInitService() {
	if (s_services.Init == nullptr) {
		s_services.Init = new InitService();
	}
}

void ServiceSinglton::InitStrategyService(IStrategyObserver* observer) {
	if (s_services.Strategy == nullptr) {
		s_services.Strategy = new StrategyService();
		s_services.Strategy->Register(observer);
	}
}

void ServiceSinglton::InitQuoteService(IQuoteObserver* observer) {
	if (s_services.Quote == nullptr) {
		s_services.Quote = new QuoteService();
		s_services.Quote->Register(observer);
	}
}

ServiceStore& ServiceSinglton::Services() {
	return s_services;
}

QString NameService::Name(type::OrderStatus orderStatus) {
	switch (orderStatus) {
	case ztp::type::OrderStatus::New:
		return QString::fromLocal8Bit("待发送");
	case ztp::type::OrderStatus::InsertQueueing:
		return QString::fromLocal8Bit("已发送");
	case ztp::type::OrderStatus::NoTradeQueueing:
		return QString::fromLocal8Bit("未成交");
	case ztp::type::OrderStatus::InsertFail:
		return QString::fromLocal8Bit("委托失败");
	case ztp::type::OrderStatus::AllTraded:
		return QString::fromLocal8Bit("完全成交");
	case ztp::type::OrderStatus::PartTradedQueueing:
		return QString::fromLocal8Bit("部分成交");
	case ztp::type::OrderStatus::CancelQueueing:
		return QString::fromLocal8Bit("撤单发送");
	case ztp::type::OrderStatus::CancelFail:
		return QString::fromLocal8Bit("撤单失败");
	case ztp::type::OrderStatus::Canceled:
		return QString::fromLocal8Bit("已撤销");
	case ztp::type::OrderStatus::NoTradeNotQueueing:
		return QString::fromLocal8Bit("已撤销");
	case ztp::type::OrderStatus::Unknown:
		return QString::fromLocal8Bit("未知");
	default:
		return QString::fromLocal8Bit("未知");
	}
}

QString NameService::Name(ExchangeStatus exchangestatus) {
	switch (exchangestatus) {
	case ExchangeStatus::BEFORE_TRADING:
		return QString::fromLocal8Bit("开盘前");
	case ExchangeStatus::NO_TRADING:
		return QString::fromLocal8Bit("非交易");
	case ExchangeStatus::CONTINOUS:
		return QString::fromLocal8Bit("连续交易");
	case ExchangeStatus::AUCTION_ORDERING:
		return QString::fromLocal8Bit("竞价报单");
	case ExchangeStatus::AUCTION_BALANCE:
		return QString::fromLocal8Bit("竞价平衡");
	case ExchangeStatus::AUCTION_MATCH:
		return QString::fromLocal8Bit("竞价撮合");
	case ExchangeStatus::CLOSED:
		return QString::fromLocal8Bit("收盘");
	default:
		return QString::fromLocal8Bit("未知");
	}
	return "";
}

QString NameService::Name(type::Direction direction) {
	switch (direction) {
	case ztp::type::Direction::Buy:
		return QString::fromLocal8Bit("买");
	case ztp::type::Direction::Sell:
		return QString::fromLocal8Bit("卖");
	default:
		return "";
	}
	return "";
}
QString NameService::Name(type::OrderPriceType type) {
	switch (type) {
	case ztp::type::OrderPriceType::LimitPrice:
		return QString::fromLocal8Bit("Normal");
	case ztp::type::OrderPriceType::FAK:
		return QString::fromLocal8Bit("FAK");
	case ztp::type::OrderPriceType::FOK:
		return QString::fromLocal8Bit("FOK");
	default:
		return "";
	}
	return "";
}
QString NameService::Name(type::OffsetFlag operation) {
	switch (operation) {
	case ztp::type::OffsetFlag::Open:
		return QString::fromLocal8Bit("开仓");	
	case ztp::type::OffsetFlag::Close:
		return QString::fromLocal8Bit("平仓");
	case ztp::type::OffsetFlag::CloseToday:
		return QString::fromLocal8Bit("平今");
	default:
		return "";
	}
	return "";
}

QString NameService::Name(automate::PriceMode priceMode) {
	switch (priceMode) {
	case automate::PriceMode::Pending:
		return QString::fromLocal8Bit("挂单模式");
	case automate::PriceMode::FAK:
		return QString::fromLocal8Bit("对价模式");
	default:
		return "";
	}
	return "";
}

QString NameService::Name(automate::StrategyMode strategyMode) {
	switch (strategyMode) {
	case automate::StrategyMode::TodayOpenClose:
		return QString::fromLocal8Bit("开平今仓");
	case automate::StrategyMode::YesterdayClose:
		return QString::fromLocal8Bit("只平老仓");
	case automate::StrategyMode::OpenClose:
		return QString::fromLocal8Bit("开平");
	default:
		return "";
	}
	return "";
}

QString NameService::Name(automate::RunningStatus status) {
	switch (status) {
	case automate::RunningStatus::Start:
		return QString::fromLocal8Bit("Start");
	case automate::RunningStatus::MinorSent:
		return QString::fromLocal8Bit("MinorSent");
	case automate::RunningStatus::Balancing:
		return QString::fromLocal8Bit("Balancing");
	case automate::RunningStatus::Pausing:
		return QString::fromLocal8Bit("Pausing");
	case automate::RunningStatus::Paused:
		return QString::fromLocal8Bit("Paused");
	case automate::RunningStatus::Fail:
		return QString::fromLocal8Bit("Fail");
	default:
		return "";
	}
	return "";
}
