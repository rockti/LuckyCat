#pragma once
#include "strategy/Strategy.h"
#include "model/Constants.h"
#include "model/dealing/OrderCommand.h"

class OrderCancel;

namespace mct {
	struct MCTArguments {};
	struct MCTOrder {
		InstID InstID;
		OrderCommand OrderCommand;
		int Quantity;
		money Price;
	};
	struct MCTCancel {
		OrderCancel actionCommand;
	};
	struct MCTCommand {
		enum CommandType { Order, Cancel };
		union { MCTOrder mctOrder; MCTCancel mctCancel; };
		MCTCommand(CommandType type, MCTOrder const& order)
			:Type(type), mctOrder(order) {
		}
		MCTCommand(CommandType type, MCTCancel const& cancel)
			:Type(type), mctCancel(cancel) {
		}
		CommandType Type;
	};


class MCTStrategy : public Strategy<MCTArguments> {
public:
	MCTStrategy(UIUpdater* updater, StrategyPublisher& publisher, DealingRepository& dealings)
		:Strategy<MCTArguments>(updater, MCTArguments{}, publisher, dealings) {
	}
public:
	virtual void DoOnInit() override;
	virtual void DoOnQuote(Quote const& market) override;
	virtual void DoOnCommand(StrategyCommand const& command) override;

	virtual void DoOnOrdered(Dealing const& dealing) override;
	virtual void DoOnCancelled(Dealing const& dealing) override;
	virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) override;

	virtual void DoOnOther(Dealing const& dealing) override;

	virtual void DoOnTrade(Trade const& trade) override;
	virtual void DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) override;
	virtual void DoOnConnectStatus(Exchange exchange, ConnectStatus status) override;

private:
	void Order(InstID instID, OrderCommand const& order, int quantity, money price);
	void Action(OrderCancel const& action);
};
}
