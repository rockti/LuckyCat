#include "stdafx.h"
#include "Investor.h"
#include "Repository.h"
#include <fmt/format.h>
#include <array>
#include <algorithm>

#undef max

data::CInvestorPosition* Investor::OnPosition(data::CPositionChange const& positionChange) {
	//目前只支持spot notify
	if (positionChange.ChangeType != type::PositionChangeFlag::Spot) {
		return nullptr;
	}
	type::Direction posDir = positionChange.Direction;
	if (positionChange.ProductClass == type::ProductClass::Spot) {
		posDir = type::Direction::Buy;
	}

	data::CInvestorPosition* pPosition = m_investorRepo.GetPosition(positionChange.InvestorID, positionChange.InstrumentID, posDir);
	if (pPosition == nullptr) {
		pPosition = createEmptyPosition(positionChange);
	}

	if (type::IsOpen(positionChange)) {
		pPosition->YdPosition += positionChange.Volume;
	} else {
		//出库先减昨仓，如果昨仓不够减今仓
		int ydPos = pPosition->YdPosition;
		ydPos -= positionChange.Volume;
		pPosition->YdPosition = ydPos <= 0 ? 0 : ydPos;

		if (ydPos < 0) {
			pPosition->Position += ydPos;
			//今仓小于0就变为0
			if (pPosition->Position < 0) {
				pPosition->Position = 0;
			}
		}
		
	}
	return pPosition;
}

template<class T>
data::CInvestorPosition* Investor::createEmptyPosition(const T& v) {
	data::CInvestorPosition position;
	position.BrokerID = v.BrokerID;
	position.Direction = v.Direction;
	position.ExchangeID = v.ExchangeID;
	position.InstrumentID = v.InstrumentID;
	position.InvestorID = v.InvestorID;
	position.ProductClass = v.ProductClass;
	position.FrozenFee = 0.0;
	position.FrozenMargin = 0.0;
	position.FrozenPosition = 0;
	position.Position = 0;
	position.PositionCost = 0.0;
	position.YdPosition = 0;
	position.YdPositionPrice = 0.0;
	position.UsedMargin = 0.0;
	return m_investorRepo.InitAdd(position);
}

OrderResult Investor::CreateOrder(data::COrder const& inputOrder, bool checkUserLocalId) {

	if (checkUserLocalId) {
		if (auto order = FindOrder(inputOrder.UserOrderLocalID); order.IsSuccess()) {
			return DuplicateOrderError;
		}
	}
	data::COrder memOrder;
	CopyAPIInputOrderEntity(&memOrder, &inputOrder);
	memOrder.OrderStatus = type::OrderStatus::New;
	memOrder.OrderLocalID = inputOrder.OrderLocalID;

	auto error = link(inputOrder, memOrder);
	if (!error.IsSuccess()) {
		return error;
	}
	double margin = 0.0; 
	if (IsLong(&memOrder) || IsShort(&memOrder)) {
		margin = m_baseAlgorithm.CalculateMargin(inputOrder.Volume, inputOrder.LimitPrice, inputOrder.Direction, memOrder.getInstrument()->VolumeMultiple, memOrder.getInvestorMargin());
	}
	double fee = m_baseAlgorithm.CalculateFee(inputOrder.Volume, inputOrder.LimitPrice, inputOrder.OffsetFlag, memOrder.getInstrument()->VolumeMultiple, memOrder.getInvestorFee());
	memOrder.FrozenMargin = margin;
	memOrder.FrozenFee = fee;
	memOrder.VolumeRemain = memOrder.Volume;
	memOrder.VolumeTrade = 0;

	auto result = SaveOrder(memOrder);
	if (!result.first.IsSuccess()) {
		return result.first;
	}

	Order order(*this, result.second, memOrder.getInstrument());
	if (Error error = Freeze(order); !error.IsSuccess()) {
		order.OnInsertFailed(error, false); //这种不记录，也为了方便处理持仓不足时解冻持仓的问题
		return error;
	}
	return order;
}

Error Investor::isOverVolumeLimit(const data::COrder& inputOrder, data::CInstrument* pInst) {
	bool isWrongVolume = (0 != pInst->MinOrderVolume && inputOrder.Volume < pInst->MinOrderVolume) || (0 != pInst->MaxOrderVolume && inputOrder.Volume > pInst->MaxOrderVolume);
	if (isWrongVolume) {
		return InvalidOrderVolumeError;
	}
	return NoError;
}
Error Investor::link(const data::COrder& inputOrder, data::COrder& memOrder) {
	data::CInstrument* pInst = m_instRepo.Get(inputOrder.InstrumentID);
	if (pInst == nullptr) {
		return InvalidInstrumentError;
	}
	if (auto error = isOverVolumeLimit(inputOrder, pInst); !error.IsSuccess()) {
		return error;
	}

	if (pInst->ProductClass != inputOrder.ProductClass) {
		return InvalidProductClassError;
	}
	memOrder.linkInstrument(pInst);
	memOrder.linkInvestorAccount(m_account);
	auto error = linkMarginAndFee(inputOrder, memOrder);
	if (!error.IsSuccess()) {
		return error;
	}

	return linkPosition(inputOrder, memOrder);;
}
Error Investor::linkMarginAndFee(const data::COrder& inputOrder, data::COrder& memOrder) {
	data::CInvestorFee* pFee = m_investorRepo.GetFee(inputOrder.BrokerID,inputOrder.InvestorID,inputOrder.InstrumentID);
	type::InvestorID matchAll("*");
	if (pFee == nullptr) {
		pFee = m_investorRepo.GetFee(inputOrder.BrokerID, matchAll, inputOrder.InstrumentID);
	}
	data::CInvestorMargin* pMargin = m_investorRepo.GetMargin(inputOrder.BrokerID, inputOrder.InvestorID, inputOrder.InstrumentID);
	if (pMargin == nullptr) {
		pMargin = m_investorRepo.GetMargin(inputOrder.BrokerID, matchAll, inputOrder.InstrumentID);
	}
	if (pFee == nullptr || pMargin == nullptr) {
		return NoMarginOrFeeError;
	}
	memOrder.linkInvestorFee(pFee);
	memOrder.linkInvestorMargin(pMargin);
	return NoError;
}

Error Investor::linkPosition(const data::COrder& inputOrder, data::COrder& memOrder) {
	type::Direction posDirection = type::IsOpen(inputOrder) ? inputOrder.Direction : type::negetive(inputOrder.Direction);
	data::CInvestorPosition* pPosition = m_investorRepo.GetPosition(inputOrder.InvestorID,inputOrder.InstrumentID, posDirection);
	if (pPosition == nullptr) {
		pPosition = createEmptyPosition(inputOrder);
	}
	if (pPosition == nullptr) {
		return SystemError;
	}
	memOrder.linkInvestorPosition(pPosition);
	m_unilateralPositionStore->Set(*memOrder.getInstrument(), pPosition->Direction, pPosition);
	return NoError;
}

Trade Investor::CreateTrade(Order& order, data::CTrade const& inputTrade) {
	data::CTrade newTrade;
	CopyTradeEntity(&newTrade, &inputTrade);
	newTrade.UserOrderLocalID = order.GetOriginalOrder()->UserOrderLocalID;
	newTrade.OrderOperatorID = order.GetOriginalOrder()->OrderOperatorID;
	auto* pTrade = m_investorRepo.Add(newTrade);
		
	Trade trade{ order,(data::CTrade*)pTrade };
	return trade;
}

Result<Trade> Investor::CreateTrade(data::CTrade const& inputTrade) {
	auto result = getOrder(inputTrade.OrderSysID);
	if (!result) {
		return {result.Error};
	}
	return CreateTrade(*result.Value , inputTrade);
}

OrderResult Investor::FindOrder(type::OrderSysID const& orderSysID) {
	auto const* pOrder = m_investorRepo.GetOrder(m_info->InvestorID, orderSysID);
	if (pOrder == nullptr) {
		return OrderResult(OrderNotFoundError);
	}
	return OrderResult(from(pOrder));
}

OrderResult Investor::FindOrder(type::UserOrderLocalID const& localID) {
	auto const* pOrder = m_investorRepo.GetOrder(m_info->InvestorID, localID);
	if (pOrder == nullptr) {
		return OrderResult(OrderNotFoundError);
	}
	return OrderResult{ from(pOrder) };
}

OrderResult Investor::FindOrder(type::OrderLocalID const& localID) {
	auto const* pOrder = m_investorRepo.GetOrder(m_info->InvestorID,localID);
	if (pOrder == nullptr) {
		return OrderResult(OrderNotFoundError);
	}
	return OrderResult { from(pOrder) };
}

Error Investor::Freeze(Order& order) {
	if (!FreezeMoney(order)) {
		return NotEnoughMoneyError;
	}

	if (!FreezePosition(order)) {
		UnfreezeOrderMoney(order);
		return NotEnoughPositoinError;
	}

	Log::Info(fmt::format("FreezeAccount: Balance[{:.3f}] Available[{:.3f}] Margin[{:.3f}] Fee[{:.3f}] CloseProfit[{:.3f}] PositionProfit[{:.3f}] FrozenMargin[{:.3f}] FrozenFee[{:.3f}] LongMargin[{:.3f},{:.3f}] ShortMargin[{:.3f},{:.3f}]"
		, m_account->Balance, m_account->Available, m_account->Margin, m_account->Fee, m_account->CloseProfit, m_account->PositionProfit
		, m_account->FrozenMargin, m_account->FrozenFee, m_account->LongMargin, m_account->LongFrozenMargin, m_account->ShortMargin, m_account->ShortFrozenMargin));
	return NoError;
}
// 
// std::vector<data::CInvestorPosition*> Investor::GetPositions() {
// 	return m_investorRepo.GetPositions(m_info->BrokerID,m_info->InvestorID);
// }


template<class T>
void calcMargin(T const& pos, double& longMargin, double& shortMargin) {
	if (type::IsBuy(pos)) {
		longMargin += pos.UsedMargin;
	} else {
		shortMargin += pos.UsedMargin;
	}
}

/*
需要根据持仓重新计算LongMargin和ShortMargin并正确初始化Margin，因为初始化表格中没有LongMargin和ShortMargin
如果不做初始化，保证金会不正确
*/

void Investor::InitAccountMarginByPositions() {
	double longMargin = 0;
	double shortMargin = 0;

	std::set<int> calcedUnilateralGroup;

	m_investorRepo.PositionForEach(m_info->InvestorID, [&](data::CInvestorPosition const& pos, bool isLast) {
		if (auto* pInst = m_instRepo.Get(pos.InstrumentID); pInst) {
			if (pInst->UnilateralGroup < 0) {
				calcMargin(pos, longMargin, shortMargin);
			} else if (calcedUnilateralGroup.find(pInst->UnilateralGroup) == calcedUnilateralGroup.end()) {
				if (auto* uPos = m_unilateralPositionStore->Get(pInst->UnilateralGroup); uPos) {
					calcMargin(uPos->GetUsedMargin(), longMargin, shortMargin);
					calcedUnilateralGroup.insert(pInst->UnilateralGroup);
				}
			}
		}
		});
	type::Money margin = longMargin + shortMargin;

	char marginStatus = 'E';
	auto originalMargin = m_account->Margin;
	if (m_account->Margin > margin) {
		/*
		如果从初始化表中初始化的保证金比我们根据持仓算出来的保证金多，
		为了不让客户有多的可用资金，将这部分放到Withdraw里
		*/
		m_account->Withdraw = m_account->Margin - margin;
		marginStatus = 'G';
	}
	if (m_account->Margin < margin) {
		marginStatus = 'L';
	}
	m_account->LongMargin = longMargin;
	m_account->ShortMargin = shortMargin;
	m_account->Margin = margin;
	UpdateBalanceAndAvailable(m_account);

	printNlog("Recalculate Investor Margin:InvestorID[{}] PreBalance[{:.2f}] Balance[{:.2f}] Available[{:.2f}] Withdraw[{:.2f}] OriginalMargin[{:.2f}] NewMargin[{:.2f}] LongMargin[{:.2f}] ShortMargin[{:.2f}], MarginStatus[{}]",
		m_account->InvestorID, m_account->PreBalance, m_account->Balance, m_account->Available, m_account->Withdraw,
		originalMargin, m_account->Margin,  m_account->LongMargin, m_account->ShortMargin,marginStatus);
}

bool Investor::FreezeMoney(Order const& order) {
	//可用资金大于预冻结保证金加手续费
	if (HasAvailable(order)) {
		if (order.IsLong()) {
			m_account->LongFrozenMargin += order.GetFreezeMargin();
		} else if (order.IsShort()) {
			m_account->ShortFrozenMargin += order.GetFreezeMargin();
		}
		m_account->FrozenMargin = m_account->LongFrozenMargin + m_account->ShortFrozenMargin;
		m_account->FrozenFee += order.GetFreezeFee();
		UpdateBalanceAndAvailable(m_account);			//更新账户权益
//		m_investorRepo.Update(*m_account);
		return true;
	} 
	return false;
}

void Investor::UpdateOrder(Order const& order) {
	m_investorRepo.Update(*order.GetOriginalOrder());
}

void Investor::UpdateTrade(Trade const& trade) {
	m_investorRepo.Update(*trade.GetOriginalTrade());
}

void Investor::Unfreeze(Order const& order) {
	UnfreezeOrderMoney(order);
	UnfreezeOrderPosition(order);
}

void Investor::AddSysIDIndex(Order const& order) {
	m_investorRepo.AddOrderSysNoIndex(*order.GetOriginalOrder());
}

void Investor::UnfreezeOrderMoney(Order const& order) {
	if (order.IsLong()) {
		m_account->LongFrozenMargin -= order.GetFreezeMargin();
	} else if (order.IsShort()) {
		m_account->ShortFrozenMargin -= order.GetFreezeMargin();
	}
	m_account->FrozenMargin = m_account->LongFrozenMargin + m_account->ShortFrozenMargin;
	m_account->FrozenFee -= order.GetFreezeFee();
	UpdateBalanceAndAvailable(m_account);			//更新账户权益
//	m_investorRepo.Update(*m_account);
}

bool Investor::FreezePosition(Order const& order) {
	data::CInvestorPosition* pPosition = (data::CInvestorPosition*)order.GetInvestorPosition();
	if (order.IsOffset()) {
		if (GetAvailablePosition(order) < order.GetVolume()) {
			return false;
		}
		pPosition->FrozenPosition += order.GetVolume();
	}
	return true;
}


inline double CalcCloseProfit(type::Direction direct, int volume, double tradePrice, double posPrice, int multiple) {
	if (type::IsBuy(direct)) {
		return volume * (posPrice - tradePrice) * multiple;
	} else {
		return volume * (tradePrice - posPrice) * multiple;
	}
}
void Investor::OnTrade(Trade const& trade, double margin, double fee) {
	if (trade.GetOrder().IsOffset()) {
		onOffsetTrade(trade, margin, fee);
	} else {
		onOpenTrade(trade, margin, fee);
	}
}

std::tuple<int,int> GetMatchPosition(data::CInvestorPosition const& position, Trade const& trade) {
	int YdTradeMatchPosition = 0;
	int todayMatchPosition = trade.GetVolume();
	if (position.YdPosition > 0) {
		int YdRemain = position.YdPosition - trade.GetVolume();
		if (YdRemain < 0) {
			YdTradeMatchPosition = position.YdPosition;
			todayMatchPosition = abs(YdRemain);
		} else {
			YdTradeMatchPosition = trade.GetVolume();
			todayMatchPosition = 0;
		}
	}
	return make_tuple(YdTradeMatchPosition, todayMatchPosition);
}

void Investor::onOffsetTrade(Trade const& trade, double unfreezeMargin, double unfreezeFee) {
	data::CInvestorPosition& position = *trade.GetOrder().GetInvestorPosition();
	
	//需要根据昨仓的情况，正进行仓位的减少
	auto [YdTradeMatchPosition, todayMatchPosition] = GetMatchPosition(position, trade);
	position.Position -= todayMatchPosition;
	position.YdPosition -= YdTradeMatchPosition;
	position.FrozenPosition -= trade.GetVolume();

	double YdCloseProfit = 0.0;
	// 计算延期的昨仓CloseProfit，现货没有，
	type::Direction direct = trade.GetOriginalTrade()->Direction;
	if (type::IsDefer(trade.GetProductClass())) {
		YdCloseProfit = CalcCloseProfit(direct, YdTradeMatchPosition, trade.GetPrice(), position.YdPositionPrice, trade.GetMultiple());
	}
	double todayCloseProfit = CalcCloseProfit(direct, todayMatchPosition, trade.GetPrice(), position.PositionCost, trade.GetMultiple());
	m_account->CloseProfit += YdCloseProfit + todayCloseProfit;


	//如果有昨仓的平仓,需要根据昨结算来返回UsedMargin
	auto [ydFreeMargin, todayFreeMargin] = GetFreeMargin(trade, YdTradeMatchPosition, todayMatchPosition);
	double freeMargin = ydFreeMargin + todayFreeMargin;
	UnilateralPosition* uPos = m_unilateralPositionStore->Get(trade.GetOrder().GetInstrument());
	position.UsedMargin -= freeMargin;
	if (uPos != nullptr) {
		auto curUsedMargin = uPos->GetUsedMargin();
		auto newUsedMargin = uPos->CalcNewUsedMargin();
		UnilateralMarginChange marginChange{ curUsedMargin, newUsedMargin };
		if (marginChange.IsLessEqual()) {
			if (marginChange.IsChanged()) {
				uPos->SetUsedMargin(newUsedMargin);
			}
		} else {
			// 不可能，记录一下
			auto msg = fmt::format("unilateral free margin is negetive for offset trade");
			Log::Error(msg);
		}

		if (marginChange.IsChanged()) {
			recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, marginChange);
		} else {
			recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, 0.0);
		}
	} else {
		recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, freeMargin);
	}

	m_investorRepo.Update(*m_account);
	m_investorRepo.Update(position);
}

const double SPOT_MARGIN_PERCENT = 0.9;

std::tuple<double,double> Investor::GetFreeMargin(Trade const& trade, int YdTradeMatchPosition, int todayMatchPosition) {
	double ydFreeMargin = 0;
	double todayFreeMargin = 0;
	double ydPrice = trade.GetOrder().GetInvestorPosition()->YdPositionPrice;
	if (YdTradeMatchPosition > 0) {
		if (trade.IsSpot()) {
			ydPrice = trade.GetPrice();
		}
		ydFreeMargin = m_baseAlgorithm.CalculateMargin(YdTradeMatchPosition,
			ydPrice,
			trade.GetOrder().GetInvestorPosition()->Direction,
			trade.GetMultiple(),
			&trade.GetOrder().GetInvestorMargin());
	}
	if (todayMatchPosition > 0) {
		todayFreeMargin = m_baseAlgorithm.CalculateMargin(todayMatchPosition,
			trade.GetOrder().GetInvestorPosition()->PositionCost,
			trade.GetOrder().GetInvestorPosition()->Direction,
			trade.GetMultiple(),
			&trade.GetOrder().GetInvestorMargin());
	}
	if (trade.IsSpot()) {
		return make_tuple(ydFreeMargin * SPOT_MARGIN_PERCENT, todayFreeMargin * SPOT_MARGIN_PERCENT);
	} else {
		return make_tuple(ydFreeMargin, todayFreeMargin);
	}
}

void Investor::recalcMarginAndFee(const Trade& trade, double unfreezeMargin, double unfreezeFee, double margin) {
	if (trade.GetOrder().IsOffset()) {
		reduceMargin(trade, unfreezeMargin, margin);
	} else {
		increaseMargin(trade, unfreezeMargin, margin);
	}
	m_account->FrozenFee -= unfreezeFee;
	m_account->Fee += trade.GetTradeFee();
	UpdateBalanceAndAvailable(m_account);
	m_investorRepo.Update(*m_account);
}

void Investor::recalcMarginAndFee(const Trade& trade, double unfreezeMargin, double unfreezeFee, UnilateralMarginChange marginChange) {

	if (marginChange.IsSameDirection()) {
		recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, marginChange.ChangedMargin());

	} else {
		bool isLong = trade.GetOrder().IsLong() || (trade.GetOrder().IsOffset() && type::IsBuy(marginChange.New));
		bool isShort = trade.GetOrder().IsShort() || (trade.GetOrder().IsOffset() && type::IsSell(marginChange.New));
		if (isLong) {
			m_account->LongMargin += marginChange.NewMargin();
			m_account->ShortMargin -= marginChange.ExistedMargin();
			m_account->LongFrozenMargin -= unfreezeMargin;
		} else if (isShort) {
			m_account->ShortMargin += marginChange.NewMargin();
			m_account->LongMargin -= marginChange.ExistedMargin();
			m_account->ShortFrozenMargin -= unfreezeMargin;
		}

		recalcMargin();
		recalcFrozenMargin();
		m_account->FrozenFee -= unfreezeFee;
		m_account->Fee += trade.GetTradeFee();
		UpdateBalanceAndAvailable(m_account);
		m_investorRepo.Update(*m_account);
	}

}

void Investor::recalcFrozenMargin() {
	m_account->FrozenMargin = m_account->LongFrozenMargin + m_account->ShortFrozenMargin;
}

void Investor::reduceMargin(const Trade& trade, double unfreezeMargin, double margin) {
	if (type::IsBuy(trade.GetOrder().GetInvestorPosition())) {
		m_account->LongMargin -= margin;
	} else {
		m_account->ShortMargin -= margin;
	}
	recalcMargin();
	recalcFrozenMargin();
}

void Investor::recalcMargin() {
	m_account->Margin = m_account->ShortMargin + m_account->LongMargin;
}

void Investor::increaseMargin(const Trade& trade, double unfreezeMargin, double margin) {
	if (trade.GetOrder().IsLong()) {
		m_account->LongFrozenMargin -= unfreezeMargin;
		m_account->LongMargin += margin;
	} else if (trade.GetOrder().IsShort()) {
		m_account->ShortFrozenMargin -= unfreezeMargin;
		m_account->ShortMargin += margin;
	}
	recalcMargin();
	recalcFrozenMargin();
}

void Investor::onOpenTrade(Trade const& trade, double unfreezeMargin, double unfreezeFee) {
	data::CInvestorPosition* pPosition = trade.GetOrder().GetInvestorPosition();

	pPosition->Position = recalcTodayPositionAvgCost(*pPosition, trade.GetVolume(), trade.GetPrice());;
	UnilateralPosition* uPos = m_unilateralPositionStore->Get(trade.GetOrder().GetInstrument());
	double unilateralMargin = trade.GetMargin();

	pPosition->UsedMargin += trade.GetMargin();
	if (uPos != nullptr) {
		auto curUsedMargin = uPos->GetUsedMargin();
		auto newUsedMargin = uPos->CalcNewUsedMargin();
		UnilateralMarginChange marginChange{ curUsedMargin, newUsedMargin };
		if (marginChange.IsGreatEqual()) {
			if (marginChange.IsChanged()) {
				uPos->SetUsedMargin(newUsedMargin);
			}
		} else {
			// 不可能，记录一下
			auto msg = fmt::format("unilateral free margin is negetive for offset trade");
			Log::Error(msg);
		}
		if (marginChange.IsChanged()) {
			recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, marginChange);
		} else {
			recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, 0.0);
		}
	} else {
		recalcMarginAndFee(trade, unfreezeMargin, unfreezeFee, trade.GetMargin());
	}

}

int Investor::recalcTodayPositionAvgCost(data::CInvestorPosition& pos, int todayVolume, double todayPrice) {
	int newPosition = pos.Position + todayVolume;
	pos.PositionCost = (pos.PositionCost * pos.Position + todayVolume * todayPrice) / newPosition;
	return newPosition;
}

void Investor::UnfreezeOrderPosition(Order const& order) {
	if (order.IsOffset()) {
		data::CInvestorPosition* pPosition = order.GetInvestorPosition();
		pPosition->FrozenPosition -= order.GetVolume();
		m_investorRepo.Update(*pPosition);
	}
}


void Investor::UpdateBalanceAndAvailable(data::CInvestorAccount* account) {

	//委托产生需要冻结资金和持仓，FrozenMargin FrozeFee
	//当成交时，需要解冻资金和持仓，并更新资金和持仓，同时需要更新Margin和Fee，以及平仓单需要更新CloseProfit
	//平仓更新CloseProfit时，先平昨仓，再平今仓，周时昨仓需要使用昨结算价计算CloseProfit，平仓时也需要减少position上的UsedMargin
	//

	//当前权益=昨权益+入金-出金+平仓盈亏-占用手续费-其他费用（浮盈不可开新仓所以不计算持仓盈亏）
	type::Money currentBalance = account->PreBalance + account->Deposit - account->Withdraw 
		+ account->CloseProfit - account->Fee - account->OtherFee;
	account->Balance = currentBalance;

	//当前可用=当前权益-占用保证金-冻结保证金-冻结手续费-其他费用
	type::Money currentAvailable = account->Balance - account->Margin 
		- account->FrozenMargin - account->FrozenFee;
	account->Available = currentAvailable;
}

OrderResult Investor::getOrder(type::OrderSysID const& orderSysId) {
	auto const* pOrder = m_investorRepo.GetOrder(m_info->InvestorID, orderSysId);
	if (pOrder == nullptr) {
		return OrderResult(OrderNotFoundError);;
	}

	return OrderResult{ from(pOrder) };
}
