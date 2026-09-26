#pragma once

#include <cassert>
#include "model/Constants.h"

enum class THPositionType {
	Today,
	History
};

enum class ExchangePositionType {
	TodayOrHistory,			//区分今仓昨仓
	Universal				//不区别今仓昨仓
};


struct SimplePosition {
	uint16_t Quantity = 0;
	money Price = 0.0;

	money GetTurnover() const {
		return Quantity * Price;
	}
};

inline SimplePosition AddMix(SimplePosition const& r, SimplePosition const& l) {

	if (l.Quantity == 0) {
		return r;
	}
	uint16_t newQuantity = r.Quantity + l.Quantity;
	money newPrice = (r.GetTurnover() + l.GetTurnover()) / newQuantity;
	return SimplePosition{ newQuantity, newPrice };
}

inline SimplePosition SubMix(SimplePosition const& r, SimplePosition const& l) {
//	assert(r.Quantity >= l.Quantity);

	if (l.Quantity == 0) {
		return r;
	}

	uint16_t newQuantity = r.Quantity - l.Quantity;
	money newPrice = (r.GetTurnover() - l.GetTurnover()) / newQuantity;
	return SimplePosition{ newQuantity, newPrice };
}



template<Exchange E>
struct ExchangePosition {};


template<ExchangePositionType Type, Direction D>
struct DirectionPosition;

template<Direction D>
struct DirectionPosition<ExchangePositionType::TodayOrHistory, D>{
	static const Direction Direction = D;
	SimplePosition TodayPosition;
	SimplePosition HistoryPosition;

	inline void Add(DirectionPosition const& position) {
		TodayPosition = AddMix(TodayPosition, position.TodayPosition);
		HistoryPosition = AddMix(HistoryPosition, position.HistoryPosition);
	}

	inline void Sub(DirectionPosition const& position) {
		TodayPosition = SubMix(TodayPosition, position.TodayPosition);
		HistoryPosition = SubMix(HistoryPosition, position.HistoryPosition);
	}
};

template<Direction D>
struct DirectionPosition<ExchangePositionType::Universal, D> {
	static const Direction Direction = D;
	SimplePosition Position;

	inline void Add(DirectionPosition const& position) {
		Position = AddMix(Position,position.Position);
	}

	inline void Sub(DirectionPosition const& position) {
		Position = SubMix(Position,position.Position);
	}
};

template<ExchangePositionType T>
struct TypePosition {
	const static ExchangePositionType Type = T;
	using BP = DirectionPosition<T, Direction::BUY>;
	using SP = DirectionPosition<T, Direction::SELL>;

	TypePosition() = default;
	inline TypePosition(InstID instID, BP buyPosition, SP sellPosition) :
		InstID(instID), BuyPosition(buyPosition), SellPosition(sellPosition) {
	}


	inline void Add(BP const& buyPosition) {
		this->BuyPosition.Add(buyPosition);
	}

	inline void Add(SP const& sellPosition) {
		this->SellPosition.Add(sellPosition);
	}

	inline void Sub(BP const& buyPosition) {
		this->BuyPosition.Sub(buyPosition);
	}

	inline void Sub(SP const& sellPosition) {
		this->SellPosition.Add(sellPosition);
	}

	inline void Add(TypePosition const& position) {
		Add(position.BuyPosition);
		Add(position.SellPosition);
	}

	inline void Sub(TypePosition const& position) {
		Sub(position.BuyPosition);
		Sub(position.SellPosition);
	}

	InstID InstID;
	BP BuyPosition;
	SP SellPosition;
};

template<ExchangePositionType T>
class ExchangePositions {
	using Position = TypePosition<T>;
	using BP = typename TypePosition<T>::BP;
	using SP = typename TypePosition<T>::SP;
public:
	inline void Add(InstID instID, BP const& position) {
		m_positions[instID].Add(position);
	}

	inline void Add(InstID instID, SP const& position) {
		m_positions[instID].Add(position);
	}

	inline void Sub(InstID instID, BP const& position) {
		m_positions[instID].Sub(position);
	}

	inline void Sub(InstID instID, SP const& position) {
		m_positions[instID].Add(position);
	}

	inline void Add(Position const& position) {
		m_positions[position.InstID].Add(position);
	}
	inline void Sub(Position const& position) {
		m_positions[position.InstID].Sub(position);
	}
	inline BP const& GetBuyPosition(InstID instID) {
		return m_positions[instID].BuyPosition;
	}
	inline SP const& GetSellPosition(InstID instID) {
		return m_positions[instID].SellPosition;
	}

	inline TypePosition<T> const& GetPosition(InstID instID) {
		return m_positions[instID];
	}
		 
	void Clear() {
		m_positions.clear();
	}

private:
	std::map<InstID, TypePosition<T>> m_positions;
};

