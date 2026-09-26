#pragma once
#include "model/Constants.h"

struct OrderCommand {
	Direction Direction;
	Operation Operation;
	OrderType Type;

	bool operator==(OrderCommand const& other) const {
		return other.Direction == Direction && other.Operation == Operation && other.Type == Type;
	}

	bool IsOpen() const { return Operation == Operation::OPEN; }
	bool IsBuy() const { return Direction == Direction::BUY; }
	bool IsSell() const { return Direction == Direction::SELL; }
};

const OrderCommand BuyOpenNormal{ Direction::BUY, Operation::OPEN, OrderType::NORMAL };
const OrderCommand BuyOpenFAK{ Direction::BUY, Operation::OPEN, OrderType::FAK };

const OrderCommand SellOpenNormal{ Direction::SELL, Operation::OPEN, OrderType::NORMAL };
const OrderCommand SellOpenFAk{ Direction::SELL, Operation::OPEN, OrderType::FAK };

const OrderCommand BuyOffsetNormal{ Direction::BUY, Operation::OFFSET, OrderType::NORMAL };
const OrderCommand BuyOffsetFAK{ Direction::BUY, Operation::OFFSET, OrderType::FAK };

const OrderCommand SellOffsetNormal{ Direction::SELL, Operation::OFFSET, OrderType::NORMAL };
const OrderCommand SellOffsetFAK{ Direction::SELL, Operation::OFFSET, OrderType::FAK };

const OrderCommand BuyOffsetTodayNormal{ Direction::BUY, Operation::OFFSET_TODAY, OrderType::NORMAL };
const OrderCommand BuyOffsetTodayFAK{ Direction::BUY, Operation::OFFSET_TODAY, OrderType::FAK };

const OrderCommand SellOffsetTodayNormal{ Direction::SELL, Operation::OFFSET_TODAY, OrderType::NORMAL };
const OrderCommand SellOffsetTodayFAK{ Direction::SELL, Operation::OFFSET_TODAY, OrderType::FAK };

//只看买卖，不管开平（用于非期货）
const OrderCommand BuyFak{ Direction::BUY, Operation::UNKNOWN, OrderType::FAK };
const OrderCommand SellFak{ Direction::SELL, Operation::UNKNOWN, OrderType::FAK };

const OrderCommand BuyNormal{ Direction::BUY, Operation::UNKNOWN, OrderType::NORMAL };
const OrderCommand SellNormal{ Direction::SELL, Operation::UNKNOWN, OrderType::NORMAL };
