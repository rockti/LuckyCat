#pragma once

#include "model/Constants.h"

namespace metrics {

	struct TradePair {
		QtyT Quantity = 0;
		QtyT OffsetQuantity = 0;
		int Profit = 0;

		void AddBuy(TickT price, QtyT qty, bool isOpen) {
			if (isOpen) {
				Quantity += qty;
			} else {
				OffsetQuantity += qty;
			}
			m_buy += int(price) * qty;
		}
		void AddSell(TickT price, QtyT qty, bool isOpen) {
			if (isOpen) {
				Quantity += qty;
			} else {
				OffsetQuantity += qty;
			}
			m_sell += int(price) * qty;
		}
		void Finish() {
			Profit = m_sell - m_buy;
		}
		void Reset() {
			m_buy = 0;
			m_sell = 0;
			Quantity = 0;
			OffsetQuantity = 0;
			Profit = 0;
		}

		inline bool IsBalanced() const {
			return Quantity <= OffsetQuantity;
		}

		inline QtyT GetPosition() const {
			return Quantity - OffsetQuantity;
		}
		int m_buy = 0;
		int m_sell = 0;
	};

	class ShortProfit {
	public:
		inline void Add(TradePair const& tradePair) {
			m_quantity += tradePair.Quantity;
			m_ticks += tradePair.Profit;
		}
		inline money AverageTicks() const {
			return m_ticks / m_quantity;
		} 

		inline QtyT Quantity() const {
			return m_quantity;
		}

		void Reset() {
			m_quantity = 0;
			m_ticks = 0;
		}
	private:
		QtyT m_quantity = 0;
		int m_ticks = 0;
	};

	class MovingProfit {
	public:
		inline void Buy(TickT price, QtyT qty) {
			m_ticks += price * uint64_t(qty);
		}
		inline void Sell(TickT price, QtyT qty) {
			m_ticks -= price * uint64_t(qty);
		}
		inline money SimulateBuy(TickT price, QtyT qty) {
			money profitTicks;

			return profitTicks;
		}

		inline money SimulateSell(TickT price, QtyT qty) {
			money profitTicks;

			return profitTicks;
		}

		inline void Reset() {

		}
		inline money ProfitTicks() const {
			return m_profitTicks;
		}
	private:
		uint64_t m_ticks = 0;
		QtyT m_qty = 0;
		money m_profitTicks = 0;
	};
}
