#pragma once

#include <array>
#include <unordered_map>
#include "model/Constants.h"
#include "service/Service.h"

class Business;
class IQuoteHandler;
struct Data;
class Quote;
struct StrategyCommand;
struct TradePlatformNotification;
class StrategyPublisher;


class StrategyHandler {
public:
	StrategyHandler(StrategyPublisher& publisher) 
		: m_publisher(publisher) {}
	void Handle(Data const& data);
private:
	void onCommand(StrategyCommand const& command);
	void onQuote(Quote const& quote);
	void onOrderError(OrderErrorNotification const& orderError);
	void onTrade(InvestorTrade const& command);
	void onExchangeNotification(TradePlatformNotification const& exchange);

	void onOrder(InvestorOrder const& order);

	void addBusiness(Business* businessPtr);

	StrategyPublisher& m_publisher;
	std::unordered_map<int, std::shared_ptr<Business>> m_businesses;
};

