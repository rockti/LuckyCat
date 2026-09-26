#pragma once

#include <set>
#include "Arguments.h"

namespace hk {

	struct Position {
		Position(Arguments const& arg) :m_arg(arg), m_pos(arg.Position) {
		}

		inline int Buy(TickT price, QtyT buy) {
			m_pos += buy;
			return m_pos;
		}
		inline int Sell(TickT price, QtyT sell) {
			m_pos -= sell;
			return m_pos;
		}

		inline int Pos() const {
			return m_pos;
		}

		inline void ResetPos(int pos) {
			m_pos = pos;
		}

		inline bool IsBuyBias() const {
			return m_pos > 0;
		}

		inline bool IsSellBias() const {
			return m_pos < 0;
		}

		inline int BalanceBuy(TickT price, QtyT qty) {
			return Buy(price, qty);
		}

		inline int BalanceSell(TickT price, QtyT qty) {
			return Sell(price, qty);
		}

		inline int IsBuyOneTick() const {
			return m_pos >= m_arg.RiskQuantity;
		}

		inline int IsSellOneTick() const {
			return m_pos < 0 && abs(m_pos) >= m_arg.RiskQuantity;
		}

		inline bool IsOverMaxQuantity() const {
			return abs(m_pos) >= m_arg.MaxQuantity;
		}

		BiasTick GetBiasTick() const {
			BiasTick result = BiasTick::Normal;
			if (IsBuyBias()) {
				result = BiasTick::BuyRound;
				if (IsBuyOneTick()) {
					result = BiasTick::BuyOneTick;
				}
			} else if (IsSellBias()) {
				result = BiasTick::SellRound;
				if (IsSellOneTick()) {
					result = BiasTick::SellOneTick;
				}
			}
			return result;
		}

	private:
		Arguments const& m_arg;
		int m_pos = 0;
	};


}
