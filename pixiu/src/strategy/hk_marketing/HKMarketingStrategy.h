#pragma once

#include "strategy/Strategy.h"
#include "QuotingBusiness.h"
#include "Snapshot.h"
#include "Position.h"
#include "strategy/PositionManager.h"
#include "Hedge.h"
namespace hk {

	class QuoteChangeApplier;
	class BalanceHandler;

	struct TickQuote {
		Direction Direction;
		TickT Price = 0;
		QtyT Quantity = 0;
	};

	using TickQuoteArray =std::array<TickQuote, 5>;


	class HKMarketingStrategy : public Strategy<Arguments>, public HedgeSender {
	public:
		HKMarketingStrategy(UIUpdater* updater, Arguments const& arguments, StrategyPublisher& publisher, DealingRepository& dealings);

		void initSnapshot();

		void initDetailedPosition(const hk::Arguments& arguments);

		virtual void DoOnInit() override;
		virtual void DoOnQuote(Quote const& market) override;
		virtual void DoOnCommand(StrategyCommand const& command) override;
		virtual void DoOnOrdered(Dealing const& dealing) override;
		virtual void DoOnCancelled(Dealing const& dealing) override;
		virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) override;
		virtual void DoOnOther(Dealing const& dealing) override;
		virtual void DoOnTrade(Trade const& trade) override;

		virtual void DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) override;
		virtual void DoOnConnectStatus(Exchange exchange, ConnectStatus connectStatus) override;


		void Pause(std::string const& reason);
		void Stop();
		void Start();
		void Reset();

		void NotifyUI(std::string const& msg) { m_publisher.NotifyUI(msg); }


		Status GetMachineStatus() const { return m_business.GetStatus(); }

		Position& GetPosition() { return m_position; }
		QuotingBusiness& GetQuotingBusiness() { return m_business; }
		pos::TickPositions<>& GetDetailedPositions() { return m_detailedPositions; }


		Arguments const& GetArguments() const { return m_arg; }
		virtual bool IsHedge() const override { return m_arg.IsHedge; }
		virtual money PushHedgeOrder(Direction direct, QtyT qty) override;
	private:

		void onMajorOrder(Dealing const& dealing);
		void onMajorTrade(Trade const& trade);
		void updatePosition(const Trade& trade);
		void onMajorCancel(Dealing const& dealing);

		bool isQuotingDealing(const Dealing& dealing);
		void logDetailedPostion();

		void updateSnapshot();

		OrderInfo createHedgeOrder(Direction direct, QtyT qty);
		money getHedgePrice(Direction direct, Instrument const& instrument);

	private:
		QuotingBusiness m_business;
		Snapshot m_snapshot;
		Position m_position;
		pos::TickPositions<> m_detailedPositions;
		BiasTick m_biasTick = BiasTick::Normal;
		std::shared_ptr<QuoteChangeApplier> m_changeApplier;

		TickQuoteArray m_buyQuotes;
		TickQuoteArray m_sellQuotes;

		std::string m_posInfo;
		Quote const* m_lastFollowQuote;
		Quote const* m_lastHedgeQuote;

		std::string m_hedgeInfo;
		Hedge m_hedge;	
	};
	bool isEnd(const Dealing& dealing);
}
