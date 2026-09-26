#pragma once

#include "model/dealing/Dealing.h"
#include <optional>
#include "strategy/hk_marketing/Arguments.h"

namespace hk {
	//会累计对冲，如果一次打不中，不重试， 到下次主力成交时会一起累计处理
	class HedgeSender {
	public:
		virtual bool IsHedge() const = 0;
		virtual money PushHedgeOrder(Direction hedgeDirection, QtyT qty) = 0;
	};
	class Hedge {
	public:
		explicit Hedge(HedgeSender& sender) :m_hedgeSender(sender) {}
		void OnMajorTrade(Trade const& majorTrade);
		void OnHedgeTrade(Trade const& hedgeTrade);
		void OnHedgeCancel(Dealing const& dealing);
		int CalcHedgeQuantity();
		std::string GetHedgeInfo() const;
		inline int GetHedgePos() const { return m_hedgePos; }
		inline int GetMajorPos() const { return m_majorPos; }
		inline int GetHedgeFreezePos() const { return m_hedgeFreezePos; }
		inline int GetUnhedgePos() const { return m_majorPos + m_hedgePos * 3; }
	private:
		HedgeSender& m_hedgeSender;
		int m_majorPos = 0;
		int m_hedgeFreezePos = 0;
		int m_hedgePos = 0;

		using LocalID = int;
		using FreezeMajorPosition = int;

		std::map<LocalID, FreezeMajorPosition> m_hedgeFreezes;
	};
}