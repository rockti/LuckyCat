#pragma once

class Quote;
struct ConnPlatformExecutorQueues;
struct OrderNotification;
struct ExchangeNotification;
struct FundsNotification;
struct CleanPositionNotification;
struct PositionNotification;
struct Data;

class ConnPublisher {
public:
	ConnPublisher(ConnPlatformExecutorQueues* apiQueues);
	void PushQuote(Quote const& quote);
	void PushPrice(Quote const& quote);

	void PushResponse(OrderNotification const& notification);
	void PushResponse(ExchangeNotification const& notification);
	void PushResponse(CleanPositionNotification const& notification);
	void PushResponse(PositionNotification const& notification);
	void PushResponse(FundsNotification const& notification);

	void NotifyUI(ExchangeNotification const& notification);
	void NotifyUI(PositionNotification const& notification);
	void NotifyUI(std::string const& msg);
	void NotifyUI(char const* msg);

private:
	bool notifyUI(Data const& data);
	bool pushResponse(Data const& data);

	ConnPlatformExecutorQueues& m_queues;
};