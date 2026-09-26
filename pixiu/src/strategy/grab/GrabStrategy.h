#pragma once

#include "model/dealing/Dealing.h"
#include "model/dealing/OrderCommand.h"
#include "strategy/Strategy.h"
#include "GrabBusiness.h"

namespace tg::grab {
	struct BusinessSnapshot : public SnapshotBase {
		QtyT Position1 = 0;
		QtyT Position2 = 0;
		inline bool IsChanged(BusinessSnapshot const& other) const {
			if (ID != other.ID)		
				return false;
			return Status != other.Status || Position1 != other.Position1 || Position2 != other.Position2;
		}
	};

	struct GBOrder {
		InstID Inst = INVALID_INSTRUMENT_ID;
		money OpenPrice = 0.0;
		money OffsetPrice = 0.0;
		money StopPrice = 0.0;
		QtyT OpenQty = 0;
		QtyT HoldQty = 0;			// 保留持仓(默认全部平仓)
		TSGEProductClassType ProductClass = SGE_OC_Unknown;
	};
	struct Arguments {
		GBOrder Order;
		GBOrder ExOrder;
		SYSTEMTIME StartTime;
		uint32_t Period;			// 委托间隔
		uint32_t ExPeriod;			// 唤醒间隔
		int AwakeCnt;				// 唤醒次数
		int TotalCnt;				// 委托次数
	};

	class GrabBusiness;
	class GrabStrategy : public Strategy<Arguments> {
	public:
		friend class GrabBusiness;
		GrabStrategy(UIUpdater* updater, Arguments const& arguments, StrategyPublisher& publisher, DealingRepository& dealings);
		~GrabStrategy();

	private:
		virtual void DoOnInit() override;
		virtual void DoOnQuote(Quote const& quote) override;
		virtual void DoOnCommand(StrategyCommand const& command) override;

		virtual void DoOnOrdered(Dealing const& dealing) override;
		virtual void DoOnCancelled(Dealing const& dealing) override;
		virtual void DoOnQuantityTrade(Dealing const& dealing, int quantity) override;

		virtual void DoOnTrade(Trade const& trade) override;
		virtual void DoOnOther(Dealing const& dealing) override;
		virtual void DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) override;
		virtual void DoOnConnectStatus(Exchange exchange, ConnectStatus connectStatus) override {};

		Dealing& PushPassiveOrderWithHook(OrderInfo const& orderInfo, GrabBusiness* grabBusiness);
		OrderInfo CreateOrderInfo(GBOrder const& order);

		void Pause();
		void Start();
		void MatchStart();
		void CheckStrategyStatus();

	private:
		BusinessSnapshot m_snapshot;
		GrabBusiness* m_pBusiness = nullptr;
		GrabBusiness* m_pExBusiness = nullptr;
		OrderInfo m_orderInfo;
		OrderInfo m_exOrderInfo;
		std::string m_lastSystemID;
		int m_maxCount = 0;
		int m_neturalBase = 0;
		bool m_isNeturalTag = false;	// 中立仓标志
		bool m_isFirstCloseTag = false;	// 首次收盘信号
	};
}
