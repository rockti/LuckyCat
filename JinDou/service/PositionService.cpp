#include "service/PositionService.h"
#include "container/Utils.h"
#include "utils/Log.h"

void initByPosition(InstrumentPosition& existedPos, InvestorPosition const& pos) {
	existedPos.BuyPosition.Exchange = pos.Exchange;
	existedPos.BuyPosition.BrokerID = pos.BrokerID;
	existedPos.BuyPosition.InvestorID = pos.InvestorID;
	existedPos.BuyPosition.InstrumentCode = pos.InstrumentCode;
	existedPos.BuyPosition.InstID = pos.InstID;
	existedPos.BuyPosition.Direction = type::Direction::Buy;
	existedPos.BuyPosition.Position = 0;
	existedPos.BuyPosition.PositionCost = 0;
	existedPos.BuyPosition.YdPosition = 0;
	existedPos.BuyPosition.YdPositionPrice = 0;

	existedPos.SellPosition.Exchange = pos.Exchange;
	existedPos.SellPosition.BrokerID = pos.BrokerID;
	existedPos.SellPosition.InvestorID = pos.InvestorID;
	existedPos.SellPosition.InstrumentCode = pos.InstrumentCode;
	existedPos.SellPosition.InstID = pos.InstID;
	existedPos.SellPosition.Direction = type::Direction::Sell;
	existedPos.SellPosition.Position = 0;
	existedPos.SellPosition.PositionCost = 0;
	existedPos.SellPosition.YdPosition = 0;
	existedPos.SellPosition.YdPositionPrice = 0;
}

InvestorPosition const& PositionService::InitAdd(InvestorPosition const& position) {
	InvestorPosition* pPos = nullptr;
	InstrumentPosition* existedPos = nullptr;
	auto iter = positions_.find(position.InstrumentCode);
	if (iter != positions_.end()) {
		existedPos = &iter->second;
	} else {
		existedPos = &positions_[position.InstrumentCode];
		initByPosition(*existedPos, position);
	}
	if (type::IsBuy(position)) {
		auto& buyPosition = existedPos->BuyPosition;
		pPos = &buyPosition;;
	} else {
		auto& sellPosition = existedPos->SellPosition;
		pPos = &sellPosition;
	}
	switch (position.PositionType) {
	case type::PositionType::All: {
		*pPos = position;
		break;
	}
	case type::PositionType::TodayOnly: {
		pPos->Position = position.Position;
		pPos->PositionCost = position.PositionCost;
		break;
	}
	case type::PositionType::HistoryOnly: {
		pPos->YdPosition = position.YdPosition;
		pPos->YdPositionPrice = position.YdPositionPrice;
		break;
	}
	}
	calcMarginAndNotifyUpdate(*pPos);
	return *pPos;
}

PositionResult PositionService::Get(type::InstID inst, type::Direction direction) const {
	type::InstrumentCode code = ServiceSinglton::Services().Instrument->toCode(inst);
	auto iter = positions_.find(code);
	if (iter == positions_.end()) {
		return { NotFoundError };
	}
	InvestorPosition const* pPos = nullptr;
	if (type::IsBuy(direction)) {
		pPos = &(iter->second.BuyPosition);
	} else {
		pPos = &(iter->second.SellPosition);
	}
	if (pPos->TotalVolume() > 0) {
		return *pPos;
	}
	else {
		return { NotFoundError };
	}
}

InstrumentPositionResult PositionService::Get(type::InstID instID) {
	auto code = ServiceSinglton::Services().Instrument->toCode(instID);
	if (code == "") {
		return { InvalidInstrumentError };
	}
	auto iter = positions_.find(code);
	if (iter == positions_.end()) {
		return { NotFoundError };
	} else {
		return { iter->second };
	}
}

bool PositionService::Has(type::InstID instID, type::Direction direction) const {
	auto code = ServiceSinglton::Services().Instrument->toCode(instID);
	if (code == "") {
		return false;
	}
	auto iter = positions_.find(code);
	if(iter == positions_.end()) {
		return false;
	}
	if (IsBuy(direction)) {
		return !iter->second.BuyPosition.IsEmpty();
	} else {
		return !iter->second.SellPosition.IsEmpty();
	}
}

IPositionService::Positions PositionService::All() const {
	Positions results;
	for (auto const& kv : positions_) {
		auto const& p = kv.second;
		if (!p.BuyPosition.IsEmpty()) {
			results.push_back(p.BuyPosition);
		}
		if (!p.SellPosition.IsEmpty()) {
			results.push_back(p.SellPosition);
		}
	}

	return results;
}

void PositionService::ClearAll() {
	positions_.clear();
}

void PositionService::ClearPlatform(TradePlatform platform) {
	map_erase_if(positions_, [&](auto const& v) { 
		auto const& pos = v.second;
		auto p = ServiceSinglton::Services().Exchange->toPlatform(pos.BuyPosition.Exchange);
		return p == platform;
		}, [&](auto& v) {
			if (v.BuyPosition.TotalVolume() > 0) {
				notifyDelete(v.BuyPosition);
			}
			if (v.SellPosition.TotalVolume() > 0) {
				notifyDelete(v.SellPosition);
			}
		});
}

void PositionService::OnQuote(Quote const& quote) {
	auto inst = ServiceSinglton::Services().Instrument->Get(quote.GetInstID());
	auto iter = positions_.find(inst->Code);
	if (iter != positions_.end()) {
		recalc(iter->second, quote);
		auto platform = ServiceSinglton::Services().Exchange->toPlatform(inst->Exchange);
		auto positionProfit = sum_position_profit(platform);
		notifyProfit(platform, positionProfit);
	}
}

void PositionService::recalc(InstrumentPosition& pos, Quote const& quote) {
	auto oldBuyProfit = pos.BuyPosition.PositionProfit;
	auto oldSellProfit = pos.SellPosition.PositionProfit;
	pos.BuyPosition.PositionProfit = calcProfit(type::Direction::Buy, pos.BuyPosition, quote.GetLastPrice());
	pos.SellPosition.PositionProfit = calcProfit(type::Direction::Sell, pos.SellPosition, quote.GetLastPrice());

	if (!pos.BuyPosition.IsEmpty() && oldBuyProfit != pos.BuyPosition.PositionProfit) {
		notifyUpdate(pos.BuyPosition);
	}
	if (!pos.SellPosition.IsEmpty() && oldSellProfit != pos.SellPosition.PositionProfit) {
		notifyUpdate(pos.SellPosition);
	}
}

double PositionService::sum_position_profit(TradePlatform platform) {
	double result = 0;
	for (auto& kv : positions_) {
		auto& pos = kv.second;
		auto posPlatform = ServiceSinglton::Services().Exchange->toPlatform(pos.BuyPosition.Exchange);
		if (posPlatform != platform) continue;
		if (!pos.BuyPosition.IsEmpty()) {
			result += pos.BuyPosition.PositionProfit;
		}
		if (!pos.SellPosition.IsEmpty()) {
			result += pos.SellPosition.PositionProfit;
		}
	}
	return result;
}

double PositionService::calcProfit(type::Direction dir, InvestorPosition position, SimpleQuote const& quote) {
	if (position.IsEmpty()) {
		return 0;
	}

	auto inst = ServiceSinglton::Services().Instrument->Get(position.InstID);
	if (inst->ProductClass == type::ProductClass::Spot) {
		return 0;	//现货不计算持仓盈亏
	}
	auto totalPosition = position.Position + position.YdPosition;
	auto quoteTotal = quote.GetPrice() * totalPosition * inst->VolumeMultiple;
	auto todayTotal = position.PositionCost * position.Position * inst->VolumeMultiple;
	auto ydTotal = position.YdPositionPrice * position.YdPosition * inst->VolumeMultiple;
	auto total = todayTotal + ydTotal;
	if (IsBuy(dir)) {
		return quoteTotal - total;
	} else {
		return total - quoteTotal;
	}
}

double PositionService::calcMargin(InvestorPosition const& pos) {
	auto platform = ServiceSinglton::Services().Exchange->toPlatform(pos.Exchange);
	auto inst = ServiceSinglton::Services().Instrument->Get(pos.InstID);
	if (!inst) return 0;
	auto margin = ServiceSinglton::Services().Instrument->GetMargin(pos.InstID);

	if (!margin) return 0;
	double rate = 0.0;
	double amt = 0;
	if (IsBuy(pos.Direction)) {
		rate = margin->LongMarginRate;
		amt = margin->LongMarginAmt;
	} else {
		rate = margin->ShortMarginRate;
		amt = margin->ShortMarginRate;
	}
	auto usedMargin = pos.Position* pos.PositionCost * inst->VolumeMultiple* rate + pos.Position * amt;
	usedMargin += pos.YdPosition * pos.YdPositionPrice * inst->VolumeMultiple * rate + pos.YdPosition * amt;
	return usedMargin;
}


inline std::tuple<int, int> getMatchPosition(InvestorPosition const& position, InvestorTrade const& trade) {
	int YdTradeMatchPosition = 0;
	int todayMatchPosition = trade.TradeVolume;
	if (position.YdPosition > 0) {
		int YdRemain = position.YdPosition - trade.TradeVolume;
		if (YdRemain < 0) {
			YdTradeMatchPosition = position.YdPosition;
			todayMatchPosition = abs(YdRemain);
		} else {
			YdTradeMatchPosition = trade.TradeVolume;
			todayMatchPosition = 0;
		}
	}
	return std::make_tuple(YdTradeMatchPosition, todayMatchPosition);
}

inline double CalcCloseProfit(type::Direction direct, int volume, double tradePrice, double posPrice, int multiple) {
	if (type::IsBuy(direct)) {
		return volume * (posPrice - tradePrice) * multiple;
	} else {
		return volume * (tradePrice - posPrice) * multiple;
	}
}

void fillBasicByTrade(InvestorPosition& pos, InvestorTrade const& trade) {
	pos.Exchange = trade.Exchange;
	pos.BrokerID = trade.BrokerID;
	pos.InvestorID = trade.InvestorID;
	pos.InstrumentCode = trade.InstrumentCode;
	pos.InstID = trade.InstID;
	pos.Direction = trade.Direction;
	pos.Position = 0;
	pos.PositionCost = 0;
	pos.YdPosition = 0;
	pos.YdPositionPrice = 0;
}

InvestorPosition& PositionService::fillByTrade(InstrumentPosition& pos, InvestorTrade const& trade) {
	fillBasicByTrade(pos.BuyPosition, trade);
	fillBasicByTrade(pos.SellPosition, trade);
	auto& dirPos = type::IsBuy(trade) ? pos.BuyPosition : pos.SellPosition;
	dirPos.Position = trade.TradeVolume;
	dirPos.PositionCost = trade.TradePrice;
	return dirPos;
}

inline void recalcTodayPositionAvgCost(InvestorPosition& pos, int todayVolume, double todayPrice) {
	int newPosition = pos.Position + todayVolume;
	pos.PositionCost = (pos.PositionCost * pos.Position + todayVolume * todayPrice) / newPosition;
	pos.Position = newPosition;
}

void PositionService::OnTrade(InvestorTrade const& trade) {
	auto inst = ServiceSinglton::Services().Instrument->Get(trade.InstID);

	if (PositionHasOffset(trade.InstID, trade.Exchange)) {
		onOffsetTrade(trade);
	} else {
		onNoOffsetTrade(trade);
	}


}

void PositionService::OnOrderRtn(InvestorOrder const& order) {
	if (!type::IsOpen(order) && (order.IsCancelled() || order.IsInsertFailed())) {
		UnfreezePosition(order);
	}
}

void PositionService::OnInitOrder(InvestorOrder const& order) {
	if (!type::IsOpen(order) && order.IsPending()) { // 平仓挂单
		FreezePosition(order);
	}
}

void PositionService::FreezePosition(InvestorOrder const& order) {
	if (!PositionHasOffset(order.InstID, order.Exchange)) {
		return;
	}

	if (type::IsOpen(order)) {
		return;
	}
	type::Direction posDirection = type::negetive(order.Direction);
	auto iter = positions_.find(order.InstrumentCode);
	if (iter == positions_.end()) {
		auto msg = fmt::format("Cann't find position for close order InstCode[{}] Dir[{}] Flag[{}] Volume[{}] Price[{}]",
			order.InstrumentCode, order.Direction, order.OffsetFlag, order.LimitPrice, order.Volume);
		Log::Error(msg);
		return;
	}

	auto& pos = type::IsBuy(posDirection) ? iter->second.BuyPosition : iter->second.SellPosition;
	//上期所区分平今平昨，需要特殊处理
	if (order.Exchange != type::Exchange::SHFE) {
		if (pos.TotalVolume() < order.VolumeRemain) {
			auto msg = fmt::format("Close Order over exist Position volume,InstCode[{}] Dir[{}] Flag[{}] Volume[{}] Price[{}], Pos[{}], YDPos[{}]",
				order.InstrumentCode, order.Direction, order.OffsetFlag, order.Volume, order.TradePrice, pos.Position, pos.YdPosition);
			Log::Error(msg);
			return;
		}
		pos.FrozenPosition += order.VolumeRemain;
		calcMarginAndNotifyUpdate(pos);

	} else {
		// close是平昨, closetoday是平今
		if (order.OffsetFlag == type::OffsetFlag::CloseToday) {
			pos.FrozenPosition += order.VolumeRemain;
			calcMarginAndNotifyUpdate(pos);
		} else if (order.OffsetFlag == type::OffsetFlag::Close) {
			pos.FrozenYDPosition += order.VolumeRemain;
			calcMarginAndNotifyUpdate(pos);
		}
	}
}

void PositionService::UnfreezePosition(InvestorOrder const& order) {
	if (!PositionHasOffset(order.InstID, order.Exchange)) {
		return;
	}
	if (type::IsOpen(order)) {
		return;
	}
	type::Direction posDirection = type::negetive(order.Direction);
	auto iter = positions_.find(order.InstrumentCode);
	if (iter == positions_.end()) {
		auto msg = fmt::format("Cann't find position for close order InstCode[{}] Dir[{}] Flag[{}] Volume[{}] Price[{}]",
			order.InstrumentCode, order.Direction, order.OffsetFlag, order.LimitPrice, order.Volume);
		Log::Error(msg);
		return;
	}

	auto& pos = type::IsBuy(posDirection) ? iter->second.BuyPosition : iter->second.SellPosition;
	if (pos.FrozenPosition < order.VolumeRemain) {
		auto msg = fmt::format("no more frozenPosition for Unfrezee,InstCode[{}] Dir[{}] Flag[{}] Volume[{}] Price[{}], Pos[{}], YDPos[{}], FPos[{}]",
			order.InstrumentCode, order.Direction, order.OffsetFlag, order.Volume, order.TradePrice, pos.Position, pos.YdPosition,pos.FrozenPosition);
		Log::Error(msg);
		return;
	}
	if (order.Exchange != type::Exchange::SHFE) {
		pos.FrozenPosition -= order.VolumeRemain;
		calcMarginAndNotifyUpdate(pos);
	} else {
		// close是平昨, closetoday是平今
		if (order.OffsetFlag == type::OffsetFlag::CloseToday) {
			pos.FrozenPosition -= order.VolumeRemain;
			calcMarginAndNotifyUpdate(pos);
		} else if (order.OffsetFlag == type::OffsetFlag::Close) {
			pos.FrozenYDPosition -= order.VolumeRemain;
			calcMarginAndNotifyUpdate(pos);
		}
	}
}

void PositionService::calcMarginAndNotifyUpdate(InvestorPosition& pos) {
	pos.UsedMargin = calcMargin(pos);
	Log::Info(fmt::format("Position: Inst[{}] Dir[{}] Pos[{}|{}] YD[{}|{}] FZ[{}] YDFZ[{}] Margin[{}] Profit[{}]",
		pos.InstrumentCode, pos.Direction, pos.Position, pos.PositionCost,
		pos.YdPosition, pos.YdPositionPrice, pos.FrozenPosition, pos.FrozenYDPosition,
		pos.UsedMargin, pos.PositionProfit));
	notifyUpdate(pos);
}

void PositionService::notifyDelete(InvestorPosition const& pos) {
	for (auto* observer : observers_) {
		observer->OnDelete(pos);
	}
}
void PositionService::notifyProfit(TradePlatform platform, double positionProfit) {
	for (auto* observer : observers_) {
		observer->OnPositionProfit(platform, positionProfit);
	}
}

void PositionService::notifyUpdate(InvestorPosition const& pos) {
	for (auto* observer : observers_) {
		observer->OnChange(pos);
	}
}

void PositionService::onOffsetTrade(InvestorTrade const& trade) {
	type::Direction posDirection = type::IsOpen(trade) ? trade.Direction : type::negetive(trade.Direction);
	auto iter = positions_.find(trade.InstrumentCode);
	if (iter == positions_.end()) {
		if (type::IsOpen(trade)) {
			InstrumentPosition instPos;
			auto& pos = fillByTrade(instPos, trade);
			positions_.emplace(trade.InstrumentCode, instPos);
			calcMarginAndNotifyUpdate(pos);
		} else {
			auto msg = fmt::format("Cann't find position for close trade InstCode[{}] Dir[{}] Flag[{}] Volume[{}] Price[{}]",
				trade.InstrumentCode, trade.Direction, trade.OffsetFlag, trade.TradeVolume, trade.TradePrice);
			Log::Error(msg);
		}
		return;
	}
	auto& instPos = iter->second;
	auto& pos = type::IsBuy(posDirection) ? instPos.BuyPosition : instPos.SellPosition;
	if (type::IsOpen(trade)) {
		recalcTodayPositionAvgCost(pos, trade.TradeVolume, trade.TradePrice);
		calcMarginAndNotifyUpdate(pos);
		return;
	}
	//平仓
	if (trade.TradeVolume > pos.TotalVolume()) {
		auto msg = fmt::format("Close Trade over exist Position volume,InstCode[{}] Dir[{}] Flag[{}] Volume[{}] Price[{}], Pos[{}], YDPos[{}]",
			trade.InstrumentCode, trade.Direction, trade.OffsetFlag, trade.TradeVolume, trade.TradePrice, pos.Position, pos.YdPosition);
		Log::Error(msg);
		return;
	}
	//上期所区别平今和平昨，需要分开计算
	if (trade.Exchange == type::Exchange::SHFE) {
		if (trade.OffsetFlag == type::OffsetFlag::CloseToday) {
			pos.Position -= trade.TradeVolume;;
			pos.FrozenPosition -= trade.TradeVolume;
		} else {
			pos.YdPosition -= trade.TradeVolume;;
			pos.FrozenYDPosition -= trade.TradeVolume;
		}
	} else {
		auto [YdTradeMatchPosition, todayMatchPosition] = getMatchPosition(pos, trade);
		pos.Position -= todayMatchPosition;
		pos.YdPosition -= YdTradeMatchPosition;
		pos.FrozenPosition -= trade.TradeVolume;
	}
	if (pos.IsEmpty()) {
		notifyDelete(pos);
	} else {
		calcMarginAndNotifyUpdate(pos);
	}
}

void PositionService::onNoOffsetTrade(InvestorTrade const& trade) {
	auto iter = positions_.find(trade.InstrumentCode);
	if (iter == positions_.end()) {
		InstrumentPosition instPos;
		auto& pos = fillByTrade(instPos, trade);
		positions_.emplace(trade.InstrumentCode, instPos);
		calcMarginAndNotifyUpdate(pos);
		return;
	}
	auto& instPos = iter->second;
	auto& myPos = type::IsBuy(trade.Direction) ? instPos.BuyPosition : instPos.SellPosition;
	if (instPos.BuyPosition.TotalVolume() == 0 && instPos.SellPosition.TotalVolume() == 0) {
		recalcTodayPositionAvgCost(myPos, trade.TradeVolume, trade.TradePrice);
		calcMarginAndNotifyUpdate(myPos);
		return;
	}
	auto& currentPos = instPos.BuyPosition.TotalVolume() > 0 ? instPos.BuyPosition : instPos.SellPosition;
	if (trade.Direction == currentPos.Direction) {
		recalcTodayPositionAvgCost(currentPos, trade.TradeVolume, trade.TradePrice);
		calcMarginAndNotifyUpdate(currentPos);
		return;
	}
	auto [YdTradeMatchPosition, todayMatchPosition] = getMatchPosition(currentPos, trade);
	if (currentPos.TotalVolume() >= trade.TradeVolume) {
		currentPos.Position -= todayMatchPosition;
		currentPos.YdPosition -= YdTradeMatchPosition;
		if (currentPos.YdPosition == 0) {
			currentPos.YdPositionPrice = 0;
		}
		if (currentPos.Position == 0) {
			currentPos.PositionCost = 0;
		}
		if (currentPos.IsEmpty()) {
			notifyDelete(currentPos);
		} else {
			calcMarginAndNotifyUpdate(currentPos);
		}
	} else {
		myPos.Position += trade.TradeVolume - currentPos.TotalVolume();
		myPos.PositionCost = trade.TradePrice;
		currentPos.Position = 0;
		currentPos.PositionCost = 0;
		currentPos.YdPosition  = 0;
		currentPos.YdPositionPrice = 0;
		
		notifyDelete(currentPos);
		calcMarginAndNotifyUpdate(myPos);
	}
}

bool PositionHasOffset(type::InstID instID, type::Exchange exchange) {
	return exchange != type::Exchange::CME && exchange != type::Exchange::HKEX;
}
