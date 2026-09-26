#pragma once
#include "model/Constants.h"
#include "type/ZTPType.h"
#include "LocalIDService.h"
using namespace ztp;


struct OrderInsertRequest {
	type::Exchange Exchange = type::Exchange::SHFE;
	type::InstID InstID = INVALID_INSTRUMENT_ID;
	type::Direction Direction = type::Direction::Buy;
	type::OffsetFlag OffsetFlag = type::OffsetFlag::Unknown;
	type::OrderPriceType OrderPriceType = type::OrderPriceType::LimitPrice;
	type::InstrumentCode InstCode;
	type::OrderLocalID OrderLocalID = INVALID_LOCAL_ID;
	int Volume = 0;
	type::Money Price = 0.0;
};


struct OrderCancelRequest {
	type::Exchange Exchange;
	type::OrderSysID OrderSysID;
	type::InstrumentCode InstrumentCode;
	type::InstID InstID;
	type::OrderLocalID OrderLocalID;
	type::OrderLocalID CancelLocalID;
};

