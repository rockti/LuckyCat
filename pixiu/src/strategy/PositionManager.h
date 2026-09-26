#pragma once
#include "model/Constants.h"
#include <sstream>

namespace pos {
	struct DirectPosition {
		Direction Direct = Direction::BUY;
		TickT Price = 0;
		QtyT Quantity = 0;
	};

	struct TickPosition {
		TickT Price;
		QtyT Quantity;
	};

	struct greater_is_better {
		bool operator() (TickPosition const& l, TickPosition const& r) const {
			return l.Price > r.Price;
		}
	};


	struct less_is_better {
		bool operator() (TickPosition const& l, TickPosition const& r) const {
			return l.Price < r.Price;
		}
	};

	template<class Comp>
	class TickPositionSet {
		using Set = std::set<TickPosition, Comp>;
	public:
		using const_iterator = typename Set::const_iterator;
		using const_reference = typename Set::const_reference;
		using reference = typename Set::reference;

		const_iterator begin() const { return m_poses.begin(); }
		const_iterator end() const { return m_poses.end(); }

		const_reference top() const { return *m_poses.begin(); }
		reference top() { return  const_cast<TickPosition&>(*m_poses.begin()); }

		size_t size() const { return m_poses.size(); }

		void clear() { m_poses.clear(); }

		void push(TickPosition const& p) {

			auto posIter = m_poses.find(p);
			if (posIter != m_poses.end()) {
				TickPosition& pos = const_cast<TickPosition&>(*posIter);
				pos.Quantity += p.Quantity;
			} else {
				m_poses.insert(p);
			}
		}

		void pop() {
			m_poses.erase(m_poses.begin());
		}

	private:
		Set m_poses;
	};

	template<class buy_comp = less_is_better, class sell_comp = greater_is_better>
	struct TickPositions {

		using BuyPositions = TickPositionSet<buy_comp>;
		using SellPositions = TickPositionSet<sell_comp>;

		template<class MajorQueue, class ReverseQueue>
		void TradeMatch(TickPosition pos, MajorQueue& majorQueue, ReverseQueue& reverseQueue) {
			if (pos.Quantity == 0) return;
			if (majorQueue.size() > 0) {
				TickPosition& topPos = majorQueue.top();
				int remainQty = pos.Quantity - topPos.Quantity;
				if (remainQty >= 0) {
					pos.Quantity = remainQty;
					majorQueue.pop();
					TradeMatch(pos, majorQueue, reverseQueue);
				} else {
					topPos.Quantity = -remainQty;
				}
			} else {
				reverseQueue.push(pos);
			}
		}

		template<class Queue>
		inline void toString(Queue const& queue, std::stringstream& ss) const {
			for (TickPosition const& pos : queue) {
				ss << pos.Price << ":" << pos.Quantity << "|";
			}
		}

		const std::string toString()
		{
			std::stringstream ss;
			if (m_buyPositions.size() > 0) {
				ss << "B:";
				toString(m_buyPositions, ss);
				return ss.str();
			}
			if (m_sellPositions.size() > 0) {
				ss << "S:";
				toString(m_sellPositions, ss);
				return ss.str();
			}
			return "";
		}

		inline std::string SellTradeMatch(TickPosition pos) {
			TradeMatch(pos, m_buyPositions, m_sellPositions);
			return toString();
		}

		inline std::string BuyTradeMatch(TickPosition pos) {
			TradeMatch(pos, m_sellPositions, m_buyPositions);
			return toString();
		}

		inline void Reset() {
			m_buyPositions.clear();
			m_sellPositions.clear();
		}

		template<class Queue, class Comp>
		TickPosition Chance(TickPosition const& quote, Queue& queue) {
			Comp comp;
			QtyT balanceQty = 0;
			QtyT remainQty = quote.Quantity;
			for (auto const& tickPos : queue) {
				if (comp(quote.Price, tickPos.Price)) {
					if (remainQty <= tickPos.Quantity) {
						balanceQty += remainQty;
						break;
					} else {
						remainQty -= tickPos.Quantity;
						balanceQty += tickPos.Quantity;
					}
				}
			}
			return TickPosition{ quote.Price, balanceQty };
		}

		TickPosition BuyChance(TickPosition const& buyQuote) {
			return Chance<BuyPositions, std::greater_equal<TickT>>(buyQuote, m_buyPositions);
		}

		TickPosition SellChance(TickPosition const& sellQuote) {
			return Chance<SellPositions, std::less_equal<TickT>>(sellQuote, m_sellPositions);
		}
		inline size_t BuySize() const { return m_buyPositions.size(); }
		inline size_t SellSize() const { return m_sellPositions.size(); }

		inline size_t Size() const {
			return BuySize() > 0 ? BuySize() : SellSize();
		}

		TickPosition const& BuyTop() const { return m_buyPositions.top(); }
		TickPosition const& SellTop() const { return m_sellPositions.top(); }

		int GetDirection() const {
			if (BuySize() > 0) {
				return 1;
			}
			if (SellSize() > 0) {
				return -1;
			}
			return 0;
		}

	private:
		BuyPositions m_buyPositions;
		SellPositions m_sellPositions;
	};

	struct RecentPosition {
		Direction Direct = Direction::BUY;
		TickT First = 0;
		TickT Second = 0;
		inline void Add(Direction direct, TickT tick) {
			if (direct != Direct || First == 0) {
				Direct = direct;
				First = tick;
				Second = 0;
				return;
			} 
			if (Second == 0) {
				Second = tick;
			} else {
				First = Second;
				Second = tick;
			}
		}

		inline bool IsReady() const {
			return First != 0 && Second != 0;
		}

		inline bool IsBigJump(TickT bigJump) const {
			if (!IsReady()) return false;
			if (IsBuy(Direct)) {
				return Second - First > bigJump;
			} else {
				return First - Second > bigJump;
			}
		}

		inline void Reset() {
			Direct = Direction::BUY;
			First = 0;
			Second = 0;
		}
	};

	class PositionTrend {
	public:
		inline void OnPosition(DirectPosition const& p) {
			m_recentPosition.Add(p.Direct, p.Price);
		}

		inline std::pair<bool, Direction> IsBig(TickT bigJump) const {
			return std::make_pair(m_recentPosition.IsBigJump(bigJump), m_recentPosition.Direct);
		}

	private:
		RecentPosition m_recentPosition;
	};
}