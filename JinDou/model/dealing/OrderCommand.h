#pragma once
#include "model/Constants.h"

struct OrderCommand {
	type::Direction Direction;
	type::OffsetFlag Operation;
	type::OrderPriceType Type;

	bool operator==(OrderCommand const& other) const {
		return other.Direction == Direction &&
			other.Operation == Operation &&
			other.Type == Type;
	}
};

const OrderCommand BuyNormalOpen{ type::Direction::Buy, type::OffsetFlag::Open, type::OrderPriceType::LimitPrice };
const OrderCommand BuyFakOpen{ type::Direction::Buy, type::OffsetFlag::Open, type::OrderPriceType::FAK };

const OrderCommand SellNormalOpen{ type::Direction::Sell, type::OffsetFlag::Open, type::OrderPriceType::LimitPrice };
const OrderCommand SellFakOpen{ type::Direction::Sell, type::OffsetFlag::Open, type::OrderPriceType::FAK };

const OrderCommand BuyNormalOffset{ type::Direction::Buy, type::OffsetFlag::Close, type::OrderPriceType::LimitPrice };
const OrderCommand BuyFaklOffset{ type::Direction::Buy, type::OffsetFlag::Close, type::OrderPriceType::FAK };

const OrderCommand SellNormalOffset{ type::Direction::Sell, type::OffsetFlag::Close, type::OrderPriceType::LimitPrice };
const OrderCommand SellFakOffset{ type::Direction::Sell, type::OffsetFlag::Close, type::OrderPriceType::FAK };

const OrderCommand BuyNormalOffsetToday{ type::Direction::Buy, type::OffsetFlag::CloseToday, type::OrderPriceType::LimitPrice };
const OrderCommand BuyFakOffsetToday{ type::Direction::Buy, type::OffsetFlag::CloseToday, type::OrderPriceType::FAK };

const OrderCommand SellNormalOffsetToday{ type::Direction::Sell, type::OffsetFlag::CloseToday, type::OrderPriceType::LimitPrice };
const OrderCommand SellFakOffsetToday{ type::Direction::Sell, type::OffsetFlag::CloseToday, type::OrderPriceType::FAK };

const OrderCommand BuyFakUnknown{ type::Direction::Buy, type::OffsetFlag::Unknown, type::OrderPriceType::FAK };
const OrderCommand SellFakUnknown{ type::Direction::Sell, type::OffsetFlag::Unknown, type::OrderPriceType::FAK };

const OrderCommand BuyNormalUnknown{ type::Direction::Buy, type::OffsetFlag::Unknown, type::OrderPriceType::LimitPrice };
const OrderCommand SellNormalUnknown{ type::Direction::Sell, type::OffsetFlag::Unknown, type::OrderPriceType::LimitPrice };
