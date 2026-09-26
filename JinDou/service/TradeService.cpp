
#include "TradeService.h"

InvestorTrade const& TradeService::Add(InvestorTrade const& trade) {
	auto& t = trades_.emplace_back(trade);
	calcMarginAndFee(t);

	sysIdIndex_.emplace(trade.OrderSysID, &t);
	localIdIndex_.emplace(trade.OrderLocalID, &t);
	tradeIdIndex_.emplace(std::make_pair(trade.OrderSysID, trade.TradeID), &t);

	notifyAdd(t);

	return t;
}

void TradeService::calcMarginAndFee(InvestorTrade& trade) {
	auto inst = ServiceSinglton::Services().Instrument->Get(trade.InstrumentCode);
	auto fee = ServiceSinglton::Services().Instrument->GetFee(trade.InstrumentCode);
	auto margin = ServiceSinglton::Services().Instrument->GetMargin(trade.InstrumentCode);
	auto amountFee = fee->OffsetFeeAmt;
	auto rateFee = fee->OffsetFeeRate;
	auto amtMargin = margin->ShortMarginAmt;
	auto rateMargin = margin->ShortMarginRate;
	if (type::IsOpen(trade)) {
		amountFee = fee->OpenFeeAmt;
		rateFee = fee->OpenFeeRate;
		amtMargin = margin->LongMarginAmt;
		rateMargin = margin->LongMarginRate;
	} else if (trade.Exchange == type::Exchange::SHFE && trade.OffsetFlag == type::OffsetFlag::CloseToday) {
		amountFee = fee->OffsetTodayFeeAmt;
		rateFee = fee->OffsetTodayFeeRate;
	}
	auto f = trade.TradePrice * trade.TradeVolume * inst->VolumeMultiple * rateFee;
	f += trade.TradeVolume * amountFee;
	trade.UsedFee = f;

	auto m = trade.TradePrice * trade.TradeVolume * inst->VolumeMultiple * rateMargin;
	m += trade.TradeVolume * amtMargin;
	trade.UsedMargin = m;
}

template<class Index, class Key>
Trades getTrades(Index const& index, Key const& key) {
	Trades results;
	auto [begin, end] = index.equal_range(key);
	Index::mapped_type const pre = nullptr;
	IndexForEach(index, key, [&](Index::mapped_type const& value) {
		results.push_back(*value);
		});
	return results;
}

Trades TradeService::GetTrades(type::OrderLocalID localID) const {
	return getTrades(localIdIndex_,localID);
}

Trades TradeService::GetTrades(type::OrderSysID const& sysNo) const {
	return getTrades(sysIdIndex_, sysNo);
}

TradeResult TradeService::GetTrade(type::OrderSysID const& sysNo, type::TradeID const& tradeID) const {
	auto iter = tradeIdIndex_.find(std::make_pair(sysNo, tradeID));
	if (iter != tradeIdIndex_.end()) {
		return { *iter->second };
	}
	return { SystemError };
}

void TradeService::notifyAdd(InvestorTrade trade)  {
	for (auto& observer : observers_) {
		observer->OnAdd(trade);
	}
}
