#pragma once
#include "strategy/PositionManager.h"
namespace hk {
	

// 	enum class Status {
// 		Start = 0,
// 		Sent,
// 		Quoting,
// 		Cancelling,
// 		TradeCancelling,
// 		End,
// 		Fail,
// 	};


	struct HedgePair {
		HedgePair(Direction direct, pos::TickPosition major):m_direct(direct),m_major(major) {
			assert(major.Quantity % 3 == 0);
			m_hedgeQty = major.Quantity / 3;
		}

		void OnHedgeTrade(QtyT qty) {
			if (IsHedging()) {
				m_hedged += qty;
			} else {
// 				Log::Error("Fuck ESunny - Hedge");
			}
		}
		
		void OnHedgeCancel() {

		}

		inline bool IsHedging() const {
			return RemainHedgeQty() != 0;
		}

		inline bool IsBalanced() const {
			return RemainHedgeQty() == 0;
		}
		inline QtyT RemainHedgeQty() const {
			return m_hedgeQty - m_hedged;
		}


	private:
		Direction m_direct = Direction::BUY;
		pos::TickPosition m_major;
		QtyT m_hedgeQty = 0;
		QtyT m_hedged = 0;
	};

	class HedgePositionManager {
	public:
		/*
		1.如果正在Hedge，记录的持仓
		2.如果Hedge结束，看是否应该正向或反向Hedge
		*/
		void OnMajorTrade(int quantity) {
			if (HasHedge()) {
			}
		}
		
		/*
		分为几种情况：
		1.Hedge部分成交且结束，根据Major持仓变化看是否继续Hedge，或是反向Hedge
		*/
		void OnHedgeTrade(int qty) {
			if (IsHedging()) {
				m_hedgePos += qty;
				m_needHedge -= qty;
			} else {
// 				Log::Error("Fuck ESunny - Hedge");
			}
// 			if (IsBalanced() && IsMajorChanged()) {
// 				if (isValidMajorQty(majorQuantity)) {
// 
// 				}
// 			}
		}
		void OnHedgeCancel() {

		}
		inline void Init(int majorQuantity) {
			if (isValidMajorQty(majorQuantity)) {
				m_initMajorPos = majorQuantity;
				m_hedgePos = 0;
				m_needHedge = -(majorQuantity / 3);
			}
		}
		void Reset() {
			m_initMajorPos = 0;
			m_hedgePos = 0;
			m_needHedge = 0;
		}

		inline bool IsHedging() const {
			return m_needHedge != 0;
		}

		inline bool IsBalanced() const { 
			return m_hedgePos * 3 == m_initMajorPos;
		}

		inline bool HasHedge() const {
			return m_hedgePos != 0;
		}
	private:

		inline bool isValidMajorQty(int majorQty) {
			return majorQty != 0 && majorQty % 3 == 0;
		}

		int m_initMajorPos = 0;
		int m_hedgePos = 0;
		int m_needHedge = 0;
	};

	class HedgeManager {
	public:
		HedgeManager(HKMarketingStrategy& parent) :
			m_parent(parent), m_arg(parent.GetArg()),
			m_position(parent.GetPosition()), m_detailedPositions(parent.GetDetailedPositions()) {
		}

		inline bool IsHedgeable() const {
			return m_detailedPositions.Size() >= 3;
		}

		inline int GetHedgeQuantity() const {
			//m_detailedPositions.GetQty(2);
		}

		inline void Hedge(Quote const& followQuote) {
			//m_parent.PushOrder();
		}

		void OnFollowTrade(Trade const& trade) {
			if (IsBuy(trade)) {
				m_positionManager.OnHedgeTrade(trade.GetDealQuantity());
			} else {
				m_positionManager.OnHedgeTrade(-trade.GetDealQuantity());
			}
		}

		void OnMajorTrade(Trade const& trade) {
			pos::DirectPosition p{ 
				trade.GetDealing().GetDirection(), 
				priceToTick(trade.GetDealPrice(), m_arg.UnitPrice), 
				QtyT(trade.GetDealQuantity()) };
			m_positionTrend.OnPosition(p);
			if (IsBuy(trade)) {
				m_positionManager.OnMajorTrade(trade.GetDealQuantity());
			} else {
				m_positionManager.OnMajorTrade(-trade.GetDealQuantity());
			}
		}

		void OnFollowCancel() {

		}
	private:

	private:
		HKMarketingStrategy& m_parent;
		Arguments const& m_arg;
		Position& m_position;
		pos::TickPositions<>& m_detailedPositions;

		pos::PositionTrend m_positionTrend;
		HedgePositionManager m_positionManager;
	};
}
