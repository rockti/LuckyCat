#pragma once
#include "type/ZTPType.h"

#include "notify/Notify.h"
class Quote;
struct ConnPlatformExecutorQueues;
struct TradePlatformNotification;
struct InitStatusNotification;
struct FundsNotification;
struct CleanPositionNotification;
struct OrderErrorNotification;
struct PositionNotification;
struct Data;

class ConnPublisher {
public:
	ConnPublisher(ConnPlatformExecutorQueues* apiQueues);
	void PushQuote(Quote const& quote);

	void PushResponse(InvestorOrder const& notification);
	void PushResponse(OrderErrorNotification const& notification);
	void PushResponse(InvestorTrade const& notification);
	void PushResponse(TradePlatformNotification const& notification);
	void PushResponse(ExchangeNotification const& notification);
	void PushResponse(InvestorAccount const& notification);
	void PushResponse(CleanPositionNotification const& notification);
	void PushResponse(InvestorPosition const& notification);
	void PushResponse(InvestorMargin const& notification);
	void PushResponse(InvestorFee const& notification);
	void PushResponse(Instrument const& inst);

	void NotifyUI(TradePlatformNotification const& notification);
	void NotifyUI(ExchangeNotification const& notification);
	void NotifyUI(InitStatusNotification const& notification);
	void NotifyUI(InvestorAccount const& notification);
	void NotifyUI(InvestorPosition const& notification);
	void NotifyUI(Instrument const& notification);
	void NotifyUI(InvestorFee const& notification);
	void NotifyUI(InvestorMargin const& notification);
	void NotifyUI(InvestorOrder const& notification);
	void NotifyUI(OrderErrorNotification const& notification);
	void NotifyUI(OrderRtnNotification const& order);
	void NotifyUI(TradeNotification const& trade);
	void NotifyUI(ErrorNotification const& order);

	void NotifyUI(InvestorTrade const& notification);
	void NotifyUI(char const* msg);
	void NotifyUI(std::string const& msg);

	void NotifyProcessingToUI(TradePlatform platform, InitProcess process);
	void NotifyDoneToUI(TradePlatform platform, InitProcess process);
	void NotifyFailToUI(TradePlatform platform, InitProcess process, Error error);
private:
	bool notifyUI(Data const& data);
	bool pushResponse(Data const& data);
	ConnPlatformExecutorQueues& m_queues;
};