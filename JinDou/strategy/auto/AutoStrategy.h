#pragma once
#include "strategy/Strategy.h"
#include "Context.h"

namespace automate {


	struct AutoSnapshot : public SnapshotBase {
		AutoArgument Argument;
		Position TotalMajor;
		Position TotalMinor;
		RunningMode Mode;
		RunningStatus RunningStatus;
		bool Deleted = false;

		bool IsChanged(AutoSnapshot const& old) const {
			return true;
		}
	};


	class AutoStrategy : public Strategy<AutoArgument> {
	public:
		AutoStrategy(AutoArgument const& arg, StrategyPublisher& publisher, BusinessDealingStore& dealingStore) :
 			Strategy<AutoArgument>(arg, publisher, dealingStore), context_(this, m_arg, majorQuote_, minorQuote_) {

		}
	private:
		virtual void DoOnInit() override;
		virtual void DoOnQuote(Quote const& quote) override;
		virtual void DoOnPendingMarket(PendingItem const& pendingItem) override {}
		virtual void DoOnCommand(StrategyCommand const& command) override;
		virtual void DoOnExchangeStatus(type::Exchange exchange, ExchangeStatus status) override;
		virtual void DoOnOrdered(Dealing const& dealing) override;
		virtual void DoOnCancelled(Dealing const& dealing) override;
		virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) override { }
		virtual void DoOnOther(Dealing const& dealing) override;
		virtual void DoOnInsertFail(Dealing const& dealing) override;
		virtual void DoOnCancelFail(Dealing const& dealing) override;
		virtual void DoOnTrade(InvestorTrade const& trade) override;
		virtual void DoOnConnectStatus(type::Exchange exchange, ConnectStatus connectStatus) override;

		bool isMajor(Dealing const& dealing) const;
		bool isMinor(Dealing const& dealing) const;
		
		bool isMajor(Quote const& quote) const;
		bool isMinor(Quote const& quote) const;

		bool isMajor(InvestorTrade const& trade) const;
		bool isMinor(InvestorTrade const& trade) const;
		void logArg();

		virtual void DoUpdateSnapshot(bool deleted) override;

		Context context_;
		Quote majorQuote_;
		Quote minorQuote_;
		AutoSnapshot snapshot_;
	};
}
