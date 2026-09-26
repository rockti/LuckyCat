#pragma once

#include "model/dealing/Dealing.h"
#include "Marketing.h"
#include "SameStatus.h"
#include "service/PositionService.h"
#include "model/Instrument.h"
#include "strategy/hf_marketing/NeturalSpreadStrategy.h"
#include "utils/Log.h"

namespace hf::netural {
	class NeturalSpreadStrategy;
	enum class PositionType {
		Today,
		Total
	};

	struct Position {
		InstID InstID = INVALID_INSTRUMENT_ID;
		Direction Direction = Direction::BUY;
		PositionType PositionType = PositionType::Today;
		money OrderPrice = 0.0;
		int Ordered = 0;
		int Dealed = 0;
		int Cancelled = 0;
		inline int PendingQty() const { return Ordered - Dealed - Cancelled; }
		inline bool IsNoPending() const { return PendingQty() == 0; }
		inline bool IsPending() const { return PendingQty() > 0; }
		inline bool IsAllCancelled() const { return Cancelled == Ordered; }

		Position(Position const& other) = default;
		Position& operator=(Position const& other) = default;
	};

	//组合的Position
	struct HFPosition {
		Position Major;
		Position Reverse;

		HFPosition() = default;
		HFPosition(Position const& major, Position const& reverse) :
			Major(major), Reverse(reverse) {
		}
		HFPosition(HFSpread const& spread, Dealing const& majorDealing)
			:Major{ spread.Instruments.Major, spread.Orders.Major.Direction, PositionType::Today, majorDealing.GetOrderPrice(), majorDealing.GetOrderQuantity(), 0, 0 }
			,Reverse{ spread.Instruments.Major, spread.Orders.Reverse.Direction, PositionType::Today, 0.0, 0, 0, 0 }	{
		}
		HFPosition(HFPosition const& other) = default;
		HFPosition& operator=(HFPosition const& other) = default;
		bool IsBothSideNoPending() const {
			return Major.IsNoPending() && Reverse.IsNoPending();
		}
	};

	//价位上的组合Position
	class HFPricePosition {
	public:
		HFPricePosition(Instrument const& instrument, money majorPrice, int reverseTick, HFLines const& lines, HFSpread const& spread, HFPosition const& position)
			:m_majorPrice(majorPrice), m_reverseTick(reverseTick), m_lines(&lines), m_spread(spread), m_position(position) {
			m_perFreezeMargin = instrument.CalculateMargin(majorPrice, 1, spread.GetMajorOperation());
			m_perFreezeFee = instrument.CalculateFee(majorPrice, 1, spread.GetMajorOperation());
		}
		HFPricePosition() = default;
		HFPricePosition& operator=(HFPricePosition const& other) = default;

		HFLines const& GetLines() const { return *m_lines; }
		HFSpread const& Spread() const { return m_spread; }
		HFPosition const& Position() const { return m_position; }

		bool IsNeedBalance() const;
		bool IsMajorPending() const;
		bool IsReversePending() const;
		bool HasPending() const { return IsMajorPending() || IsReversePending(); }

		int GetGainTick() const { return m_reverseTick; }
		money GetLastPrice() const { return m_spread.GetLastPrice(); }		// 最新价
		money GetFirstReversePrice() const { return m_spread.GetFirstReverseQuote(); }	// 盘口价
		money GetWorstReversePrice() const { return m_spread.GetFirstMajorQuote(); }	// 止损价
		money GetReversePriceByQuote(int reverseTick) const { return m_spread.GetReversePriceByQuote(reverseTick); }

		void FreezeTotal() {
			money freezeMargin = m_perFreezeMargin * m_position.Major.Ordered;
			money freezeFee = m_perFreezeFee * m_position.Major.Ordered;
			auto& account = pos::GlobalPositionStore::Instance().Account();
			account.FreezeMargin(freezeMargin);
			account.FreezeFee(freezeFee);
			account.ReCalcAvailable();
		}
		void FreezeFee() {
			money freezeFee = m_perFreezeFee * m_position.Major.Ordered;
			auto& account = pos::GlobalPositionStore::Instance().Account();
			account.FreezeFee(freezeFee);
			account.ReCalcAvailable();
		}

		void OnMajorTrade(int quantity) { 
			m_position.Major.Dealed += quantity;	// 更新数量

			{
				money freezeMargin = m_perFreezeMargin * quantity;
				money freezeFee = m_perFreezeFee * quantity;
				auto& account = pos::GlobalPositionStore::Instance().Account();
				account.UnfreezeMargin(freezeMargin);
				account.UnfreezeFee(freezeFee);
				account.ReCalcAvailable();
			}	// 释放冻结
		}
		void OnMajorCancel(int cancelQuantity) { 
			m_position.Major.Cancelled += cancelQuantity; 

			{
				money freezeMargin = m_perFreezeMargin * cancelQuantity;
				money freezeFee = m_perFreezeFee * cancelQuantity;
				auto& account = pos::GlobalPositionStore::Instance().Account();
				account.UnfreezeMargin(freezeMargin);
				account.UnfreezeFee(freezeFee);
				account.ReCalcAvailable();
			}	// 释放冻结
		}
		void OnReverseStop(money orderPrice, int quantity) {
			m_position.Reverse.OrderPrice = orderPrice;
			m_position.Reverse.Ordered = m_position.Reverse.Ordered - m_position.Reverse.Cancelled + quantity;
			m_position.Reverse.Cancelled = m_position.Reverse.Cancelled - quantity;
		}
		void OnReverseOrder(money orderPrice, int quantity) {
			m_position.Reverse.OrderPrice = orderPrice;
			m_position.Reverse.Ordered += quantity;
		}
		void OnReverseTrade(int quantity) { 
			m_position.Reverse.Dealed += quantity;
			{
				money freezeMargin = m_perFreezeMargin * quantity;
				money freezeFee = m_perFreezeFee * quantity;
				auto& account = pos::GlobalPositionStore::Instance().Account();
// 				account.UnfreezeMargin(freezeMargin);	Rvs考虑大单边不冻结保证金，这里也不需要解冻
				account.UnfreezeFee(freezeFee);
				account.ReCalcAvailable();
			}	// 释放冻结
		}
		void OnReverseCancel(int cancelQuantity) { 
			m_position.Reverse.Cancelled += cancelQuantity; 

			{
				money freezeMargin = m_perFreezeMargin * cancelQuantity;
				money freezeFee = m_perFreezeFee * cancelQuantity;
				auto& account = pos::GlobalPositionStore::Instance().Account();
// 				account.UnfreezeMargin(freezeMargin);	Rvs考虑大单边不冻结保证金，这里也不需要解冻
				account.UnfreezeFee(freezeFee);
				account.ReCalcAvailable();
			}	// 释放冻结
		}

	private:
		int m_reverseTick = 0;
		money m_majorPrice = 0;		//订单簿上的价位索引
		HFLines const* m_lines;
		HFSpread m_spread;			//一组委托组合
		HFPosition m_position;		//组合的平衡管理
		money m_perFreezeMargin = 0;
		money m_perFreezeFee = 0;
	};

	struct DealingStatus {
		LocalOrderID LocalId;
		SystemNoT SystemNo;
		bool IsFinished = false;
	};
	struct HFDealingStatus {
		void AddReverse(DealingStatus const& status) {
			reverses_.emplace(status.LocalId, status);
		}
		void OnDealing(Dealing const& dealing) {
			if (dealing.GetLocalId() == major_.LocalId) {
				major_.SystemNo = dealing.GetSystemNo();
				major_.IsFinished = dealing.IsFinished();
				return;
			} else {
				auto iter = reverses_.find(dealing.GetLocalId());
				if (iter != reverses_.end()) {
					iter->second.SystemNo = dealing.GetSystemNo();
					iter->second.IsFinished = dealing.IsFinished();
				}
			}
		}
		LocalOrderID MajorLocalId() const {
			return major_.LocalId;
		}
		void SetMajorLocalId(LocalOrderID localId) {
			major_.LocalId = localId;
		}
		DealingStatus const& MajorStatus() const {
			return major_;
		}

		std::map<LocalOrderID, DealingStatus> const& ReverseStatuses() const {
			return reverses_;
		}
	private:
		DealingStatus major_;
		std::map<LocalOrderID, DealingStatus> reverses_;
	};
	
	class PriceBusiness {
	public:
		PriceBusiness(NeturalSpreadStrategy& parent, Dealing& majorDealing, HFPricePosition const& position, QuoteBias bias, uint32_t selfQty);
		void OnQuote();
		void OnDealing(Dealing const& dealing) { m_DealingStatus.OnDealing(dealing); }
		void OnMajorTrade(money orderPrice, int quantity);
		void OnMajorCancel(money orderPrice, int quantity);
		void OnReverseTrade(money reversePrice, int quantity);
		void OnReverseCancel(money orderPrice, int quantity);

		inline HFPricePosition const& GetHFPricePosition() const { return m_pricePosition; }
		inline Direction GetMajorDirection() const { return m_pricePosition.Spread().Orders.Major.Direction; }
		inline Direction GetReverseDirection() const { return m_pricePosition.Spread().Orders.Reverse.Direction; }
		inline Operation GetMajorOperation() const { return m_pricePosition.Spread().Orders.Major.Operation; }
		inline Operation GetReverseOperation() const { return m_pricePosition.Spread().Orders.Reverse.Operation; }
		inline money GetMajorOrderedPrice() const { return m_pricePosition.Position().Major.OrderPrice; }
		inline money GetReverseOrderedPrice() const { return m_pricePosition.Position().Reverse.OrderPrice; }
		inline TickT GetCostTick() const { return m_parent.GetArg().CostTick; }
		inline TickT GetPendingRvsTick() const { return priceToTick(m_pricePosition.Position().Reverse.OrderPrice, m_unitPrice); }	// 界面展示
		inline int GetPendingRvsQty() const { return m_pricePosition.Position().Reverse.PendingQty(); }								// 界面展示
		inline int GetGainTick() const { return m_pricePosition.GetGainTick(); }

		inline bool IsBalance() const { return !m_pricePosition.IsNeedBalance(); }
		inline bool IsMajorPending() const { return m_pricePosition.IsMajorPending(); }
		inline bool IsReversePending() const { return m_pricePosition.IsReversePending(); }
		inline bool IsRemoveable() const { return m_removeable && !m_pricePosition.HasPending(); }
		inline bool IsMajorDealing(Dealing const& dealing) const { return dealing.GetLocalId() == m_DealingStatus.MajorLocalId(); }
		inline bool IsReverseDealing(Dealing const& dealing) const { return dealing.GetInstID() == m_pricePosition.Spread().Instruments.Major && dealing.GetLocalId() != m_DealingStatus.MajorLocalId(); }

		int GetReverseTick(Direction majorDir);
		bool IsMajorMissChance();
		bool IsReverseNeedRetry();

	public:
		void pushCancel(DealingStatus const& dealingStatus);
		void cancelMajor();
		void sendReverse(money price, int qty);
		void cancelReverse();
		void SetRetryTag() { m_retryTag = true; }
		void Pause();
		void logCurrentPosition(char const* prefix);

		BasePhase* GetPhase(Phase index) { return m_machine.GetPhase(index); }
		void SetRemoveable() { m_removeable = true; }

		NeturalSpreadStrategy& m_parent;
		HFPricePosition m_pricePosition;
		HFDealingStatus m_DealingStatus;

		BasePhase* m_currentPhase = nullptr;
		Machine m_machine;
		money m_unitPrice = 0.0;
		QuoteBias m_bias = QuoteBias::FBD;

		int m_selfQty = 0;			// 报价时盘口量
		int m_maxFailedCnt = 3;		// 最大失败次数

		bool m_escapeTag = false;	// 逃离标记
		bool m_stopTag = false;		// 止损标记（预留，暂时无用）
		bool m_retryTag = false;	// 重发标记
		bool m_isOrderQuote = true;	// 报价行情作废

		bool m_removeable = false;
	};
}
