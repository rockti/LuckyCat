#pragma once
#include "service/PositionService.h"
#include "model/dealing/Dealing.h"
#include "strategy/Strategy.h"
#include "SpreadContext.h"

namespace hf::spread {
	struct SpreadSnapshot : public SnapshotBase {
		int MajorPos = 0;
		int MinorPos = 0;
		int CancelAmt = 0;
		money Spread = 0.0;

		bool IsChanged(SpreadSnapshot const& other) const {
			if (ID != other.ID) return false;
			return Status != other.Status || MajorPos != other.MajorPos || MinorPos != other.MinorPos || CancelAmt != other.CancelAmt || Spread != other.Spread;
		}
	};

	enum SpreadType {
		Spread = 0,
		Ratio
	};
	struct Arguments {
		SPInstruments Instruments;
		SpreadType Type;		// 价差模式（差价；比价）
		Direction Dir;			// 价差方向（和远月同向）

		QtyT MinQty = 0;		// 挂单量(PendingQty)
		QtyT MaxQty = 2;		// 对价量(FAKQty)
		QtyT Multiple = 1;		// Minor/Major
		QtyT MaxPosition = 1;	// 最大持仓

		money OfrOpen = 999'999;
		money BidOffset = 888'888;
		money OfrOffset = -888'888;
		money BidOpen = -999'999;
		int OfrPosition = 0;	// 待平持仓（负数表示上期所老仓）
	};

	class SpreadBusiness;
	class SpreadStrategy : public Strategy<Arguments> {
	public:
		friend class SpreadBusiness;
		SpreadStrategy(UIUpdater* updater, Arguments const& arguments, StrategyPublisher& publisher, DealingRepository& dealings);

	private:
		virtual void DoOnInit() override;
		virtual void DoOnQuote(Quote const& market) override;
		virtual void DoOnCommand(StrategyCommand const& command) override;

		virtual void DoOnOrdered(Dealing const& dealing) override;
		virtual void DoOnCancelled(Dealing const& dealing) override;
		virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) override;

		virtual void DoOnOther(Dealing const& dealing) override;
		virtual void DoOnTrade(Trade const& trade) override;
		virtual void DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) override {};
		virtual void DoOnConnectStatus(Exchange exchange, ConnectStatus connectStatus) override {};

		Dealing* PushOrderWithHook(OrderInfo const& orderInfo);

		bool isMinor(InstID instID) const;
		bool isMajor(InstID instID) const;
		void logArg();

	private:		
		SpreadSnapshot m_snapshot;
		SpreadBusiness* m_pBusiness;
		bool m_isQuoteReady = false;
	};
}
