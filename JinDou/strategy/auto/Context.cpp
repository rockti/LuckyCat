#include "Context.h"
#include "model/dealing/Dealing.h"
#include "model/Quote.h"
#include "service/Service.h"
#include "AutoStrategy.h"
#include "utils/Log.h"
#include "service/CancellationLimiter.h"

namespace automate {

	bool Running::IsMinorFinished() const {
		if (MinorDealing) {
			return MinorDealing->IsFinished();
		}
		return true;
	}

	bool Running::IsAllBalanceFinished() const {
		return UnfinishedBalanceDealings.empty();
	}

	bool Running::IsBalanced() const {
		return ContextPtr->VolumeBalancer().IsBalanced(this->Position);
	}

	AvailablePrice Running::getMajorPrice(type::Volume volume) const {
		switch (this->Mode) {
		case RunningMode::BuySpreadOpen: {
			return BuySelector->GetMajorOpenPrice(volume, this->Arg->MajorLostTick);
			break;
		}
		case RunningMode::BuySpreadClose: {
			return BuySelector->GetMajorClosePrice(volume, this->Arg->MajorLostTick);
			break;
		}
		case RunningMode::SellSpreadOpen: {
			return SellSelector->GetMajorOpenPrice(volume, this->Arg->MajorLostTick);
			break;
		}
		case RunningMode::SellSpreadClose: {
			return SellSelector->GetMajorClosePrice(volume, this->Arg->MajorLostTick);
			break;
		}
		default:
			break;
		}
		return AvailablePrice{ type::Direction::Buy, type::OffsetFlag::Unknown };
	}

	AvailablePrice Running::getOppsoiteMinorPrice(type::Volume volume) const {
		if (type::IsBuy(MinorDealing->GetDirection())) {
			auto const& sell1 = BuySelector->GetMinorQuote().GetSell();
			return AvailablePrice{ type::Direction::Sell, negetive(MinorDealing->GetOffsetFlag()), sell1.GetPrice(), std::min(volume.value(), sell1.GetQuantity()) };
		} else {
			auto const& buy1 = BuySelector->GetMinorQuote().GetBuy();
			return AvailablePrice{ type::Direction::Sell, negetive(MinorDealing->GetOffsetFlag()), buy1.GetPrice(), std::min(volume.value(), buy1.GetQuantity()) };
		}
	}

	OrderInsertRequest Running::CreateMajorInsertRequest(type::Volume volume) {
		auto instrument = ServiceSinglton::Services().Instrument->Get(Arg->Major);

		AvailablePrice price = getMajorPrice(volume);
		OrderInsertRequest request;
		request.Price = price.Price;
		request.Volume = price.Volume;
		request.InstID = Arg->Major;
		request.Exchange = instrument->Exchange;
		request.InstCode = instrument->Code;
		request.Direction = type::negetive(MinorDealing->GetDirection());
		request.OffsetFlag = MinorDealing->GetOffsetFlag();
		if (instrument->ProductClass == type::ProductClass::Spot) {
			if (request.Direction == type::Direction::Buy) {
				request.OffsetFlag = type::OffsetFlag::Open;
			} else {
				request.OffsetFlag = type::OffsetFlag::Close;
			}
		}
		request.OrderPriceType = type::OrderPriceType::FAK;
		request.OrderLocalID = LocalIdACC::NextNormal();
		UnfinishedBalanceDealings.emplace(request.OrderLocalID);
		this->Position.MajorFrozen += volume;

		return request;
	}

	OrderInsertRequest Running::CreateMinorOppositeInsertRequest(type::Volume volume) {
		auto instrument = ServiceSinglton::Services().Instrument->Get(Arg->Minor);

		AvailablePrice price = getOppsoiteMinorPrice(volume);
		OrderInsertRequest request;
		request.Price = price.Price;
		request.Volume = price.Volume;
		request.InstID = Arg->Minor;
		request.Exchange = instrument->Exchange;
		request.InstCode = instrument->Code;
		request.Direction = type::negetive(MinorDealing->GetDirection());
		request.OffsetFlag = negetive(MinorDealing->GetOffsetFlag());
		request.OrderPriceType = type::OrderPriceType::FAK;
		request.OrderLocalID = LocalIdACC::NextNormal();
		UnfinishedBalanceDealings.emplace(request.OrderLocalID);
		this->Position.OppositeMinorFrozen += volume;
		return request;
	}

	void Running::MinorTrade(type::Volume tradeVolume) {
		this->Position.Minor += tradeVolume;
	}

	void Running::FinishMajor(type::OrderLocalID localID, type::Volume orderVolume, type::Volume tradeVolume) {
		UnfinishedBalanceDealings.erase(localID);
		this->Position.Major += tradeVolume;
		this->Position.MajorFrozen -= orderVolume;
	}

	void Running::FinishMinorOpposite(type::OrderLocalID localID, type::Volume orderVolume, type::Volume tradeVolume) {
		UnfinishedBalanceDealings.erase(localID);
		this->Position.OppositeMinor += tradeVolume;
		this->Position.OppositeMinorFrozen -= orderVolume;
	}

	bool Running::isPendingOrderOverTolerance() const {
		auto minorUnitPrice = ContextPtr->GetMinorInst().UnitPrice;
		bool result = false;
		auto tolerance = Arg->Tolerance * minorUnitPrice;
		auto const& minorQuote = BuySelector->GetMinorQuote();
		double diff = 0.00;
		if (type::IsBuy(MinorDealing->GetDirection())) {
			diff = minorQuote.GetBuy().GetPrice() - MinorDealing->GetOrderPrice();
		} else {
			diff = MinorDealing->GetOrderPrice() - minorQuote.GetSell().GetPrice();
		}
		return diff > tolerance;
	}

	bool Running::isSpreadOverTolerance() const {
		auto minorUnitPrice = ContextPtr->GetMinorInst().UnitPrice;
		auto orderPrice = MinorDealing->GetOrderPrice();
		switch (this->Mode) {
		case RunningMode::BuySpreadOpen: {
			return BuySelector->IsOpenLost(*Arg, orderPrice, minorUnitPrice);
			break;
		}
		case RunningMode::BuySpreadClose: {
			return BuySelector->IsCloseLost(*Arg, orderPrice, minorUnitPrice);
			break;
		}
		case RunningMode::SellSpreadOpen: {
			return SellSelector->IsOpenLost(*Arg, orderPrice, minorUnitPrice);
			break;
		}
		case RunningMode::SellSpreadClose: {
			return SellSelector->IsCloseLost(*Arg, orderPrice, minorUnitPrice);
			break;
		}
		default:
			return false;
			break;
		}
	}

	bool Running::LostOpportunity() const {
		//TODO
		// 1.挂单价格与现在的buy1或sell1的差异大于等于 arg.Tolerance
		// 2.挂单价格与major buy1或sell1的价差已不满足价差条件,需要增加arg.Tolerance
		// 3.要配平的major数量大于盘口major的量, minor 挂单也要撤单, 先不做 TODO

		return isPendingOrderOverTolerance() || isSpreadOverTolerance();
	}

	type::OffsetFlag Running::negetive(type::OffsetFlag minorOffsetFlag) const {
		if (minorOffsetFlag == type::OffsetFlag::Open) {
			return type::OffsetFlag::Close;
		} else {
			return type::OffsetFlag::Open;
		}
	}

	OrderInsertRequest Running::CreateMinorOrderRequest() const {
		auto instrument = ServiceSinglton::Services().Instrument->Get(Arg->Minor);
		OrderInsertRequest request;
		request.Exchange = instrument->Exchange;
		request.InstID = Arg->Minor;
		request.InstCode = instrument->Code;
		request.Direction = this->MinorPrice.Directin;
		request.OffsetFlag = this->MinorPrice.OffsetFlag;
		if (instrument->ProductClass == type::ProductClass::Spot) {
			if (request.Direction == type::Direction::Buy) {
				request.OffsetFlag = type::OffsetFlag::Open;
			} else {
				request.OffsetFlag = type::OffsetFlag::Close;
			}
		}
		request.OrderPriceType = type::OrderPriceType::LimitPrice;
		if (Arg->PriceMode == PriceMode::FAK) {
			request.OrderPriceType = type::OrderPriceType::FAK;
		}
		request.OrderLocalID = LocalIdACC::NextNormal();
		request.Price = this->MinorPrice.Price;
		request.Volume = this->MinorPrice.Volume;
		return request;

	}

	OrderCancelRequest Running::CreateMinorCancelRequest() const {
		OrderCancelRequest request;
		request.OrderLocalID = this->MinorDealing->GetLocalId(); 
		request.CancelLocalID = LocalIdACC::NextNormal();
		request.Exchange = this->MinorDealing->GetExchange();
		request.InstID = this->MinorDealing->GetInstID();
		request.InstrumentCode = this->MinorDealing->GetInstCode();
		request.OrderSysID = this->MinorDealing->GetSystemNo();
		return request;
	}

	BalanceVolume Running::GetBalanceVolume() const {
		return ContextPtr->VolumeBalancer().GetBalancingVolume(*MinorDealing, this->Position);
	}

	Context::Context(Business* business, AutoArgument const& arg, Quote const& majorQuote, Quote const& minorQuote) :
		arg_(arg), business_(business),running_(Running::None(arg_)), buySelector_(majorQuote, minorQuote), sellSelector_(majorQuote, minorQuote),
		fakBuySelector_(majorQuote,minorQuote),fakSellSelector_(majorQuote,minorQuote),
		balancer_(VolumeRatio{ arg.VolumeMultiple }, majorQuote, minorQuote) {
		Init(this);
		auto& instService = ServiceSinglton::Services().Instrument;
		majorInst_ = instService->Get(arg_.Major).Value.value();
		minorInst_ = instService->Get(arg_.Minor).Value.value();
		toleranceUnitPrice_ = minorInst_.UnitPrice;
		if (arg.ExistedVolume.Volume > 0) {
			minorPosition_.Direction = arg.ExistedVolume.OpenDirection;
			minorPosition_.Volume = arg.ExistedVolume.Volume;
			majorPosition_.Direction = negetive(minorPosition_.Direction);
			majorPosition_.Volume = minorPosition_.Volume * arg.VolumeMultiple;
		}
	}

	Running Context::ReadyToOpen() {
		if (arg_.Mode == StrategyMode::YesterdayClose) {
			return Running::None(arg_);
		}

		if (minorPosition_.Volume >= arg_.MaxVolume) {
			return Running::None(arg_);
		}
		IPriceSelector* buySelector = &buySelector_;
		IPriceSelector* sellSelector = &sellSelector_;

		if (arg_.PriceMode == PriceMode::FAK) {
			buySelector = &fakBuySelector_;
			sellSelector = &fakSellSelector_;
		}


		if (buySelector->IsOpenReady(arg_)) {
			type::Volume maxOpenableQuantity = arg_.MaxVolume - minorPosition_.Volume;
			auto maxOpenVolume = std::min(maxOpenableQuantity, arg_.Quantity);
			auto openParam = buySelector->GetMinorOpenPrice(maxOpenVolume);

			return Running{ this, RunningMode::BuySpreadOpen, &arg_, openParam, buySelector, sellSelector };
		}

		if (sellSelector->IsOpenReady(arg_)) {
			type::Volume maxOpenableQuantity = arg_.MaxVolume - minorPosition_.Volume;
			auto maxOpenVolume = std::min(maxOpenableQuantity, arg_.Quantity);
			auto openParam = sellSelector->GetMinorOpenPrice(maxOpenVolume);

			return Running{ this, RunningMode::SellSpreadOpen, &arg_, openParam, buySelector, sellSelector };
		}
		return Running::None(arg_);
	}

	Running Context::ReadyToClose() {
		if (minorPosition_.Volume == 0) {
			return Running::None(arg_);
		}

		IPriceSelector* buySelector = &buySelector_;
		IPriceSelector* sellSelector = &sellSelector_;

		if (arg_.PriceMode == PriceMode::FAK) {
			buySelector = &fakBuySelector_;
			sellSelector = &fakSellSelector_;
		}

		if (IsBuyOpenPosition() && buySelector->IsCloseReady(arg_)) {
			type::Volume maxCloseableQuantity = minorPosition_.Volume;
			auto maxCloseVolume = std::min(maxCloseableQuantity, arg_.Quantity);
			auto closeParam = buySelector->GetMinorClosePrice(maxCloseVolume);

			return Running{ this, RunningMode::BuySpreadClose, &arg_, closeParam, buySelector, sellSelector};
		}

		if (IsSellOpenPosition() && sellSelector->IsCloseReady(arg_)) {
			type::Volume maxCloseableQuantity = minorPosition_.Volume;
			auto maxCloseVolume = std::min(maxCloseableQuantity, arg_.Quantity);
			auto closeParam = sellSelector->GetMinorClosePrice(maxCloseVolume);

			return Running{ this, RunningMode::SellSpreadClose, &arg_, closeParam, buySelector, sellSelector };
		}
		return Running::None(arg_);
	}

	bool Context::QuoteCloseToLimit() const {
		const double LIMIT_BUFFER = 0.005;
		auto const& minorQuote = GetMinorQuote();
		auto const& majorQuote = GetMajorQuote();

		if (minorQuote.GetBuy().GetPrice() < minorQuote.GetLowerLimit() * (1 + LIMIT_BUFFER)) {
			return true;
		}
		if (minorQuote.GetSell().GetPrice() > minorQuote.GetUpperLimit() * (1 - LIMIT_BUFFER)) {
			return true;
		}

		if (majorQuote.GetBuy().GetPrice() < majorQuote.GetLowerLimit() * (1 + LIMIT_BUFFER)) {
			return true;
		}
		if (majorQuote.GetSell().GetPrice() > majorQuote.GetUpperLimit() * (1 - LIMIT_BUFFER)) {
			return true;
		}
		return false;

	}

	bool Context::InvalidQuote() const {
		auto const& major = GetMajorQuote();
		auto const& minor = GetMinorQuote();
		if (major.GetSell().GetQuantity() < 1) {
			return true;
		}
		if (major.GetBuy().GetQuantity() < 1) {
			return true;
		}
		if (minor.GetSell().GetQuantity() < 1) {
			return true;
		}
		if (minor.GetBuy().GetQuantity() < 1) {
			return true;
		}
		return false;
	}

	bool Context::IsOverCancelLimit() const {
		return CancellationLimiter::Instance().Get(minorInst_.InstID).IsOverLimit() ||
			CancellationLimiter::Instance().Get(majorInst_.InstID).IsOverLimit();
	}

	Running Context::TryRunning() {
		if (InvalidQuote()) {
			return Running::None(arg_);
		}

		if (IsOverCancelLimit()) {
			return Running::None(arg_);
		}

#ifdef _DEBUG
// 		auto const& majorSell = GetMajorQuote().GetSell();
// 		auto const& majorBuy = GetMajorQuote().GetBuy();
// 		auto const& minorSell = GetMinorQuote().GetSell();
// 		auto const& minorBuy = GetMinorQuote().GetBuy();
// 		auto const& info = fmt::format("Business[{}] tryrunning: Minor[{:.3f}:{} , {:.3f}:{}] Major[{:.3f}:{} , {:.3f}:{}]",
// 			business_->GetId(),	minorSell.GetPrice(), minorSell.GetQuantity(), minorBuy.GetPrice(), minorBuy.GetQuantity(),
// 			majorSell.GetPrice(), majorSell.GetQuantity(), majorBuy.GetPrice(), majorBuy.GetQuantity());
// 		Log::Info(info);
#endif

		if (QuoteCloseToLimit()) {
			return Running::None(arg_);
		}

		auto running = ReadyToClose();
		if (!running.IsNone()) {
			return running;
		}
		return ReadyToOpen();
	}

	void Context::FireMinor(Running const& running) {
		running_ = running;
		auto instrument = ServiceSinglton::Services().Instrument->Get(arg_.Minor);
		auto minorRequest = running.CreateMinorOrderRequest();
		if (minorRequest.OffsetFlag == type::OffsetFlag::Close) {
			minorRequest.OffsetFlag = GetRightOffsetFlag(instrument.Value.value());
		}
		Dealing dealing(business_, this, minorRequest);
		auto& savedDealing = this->GetDealingStore().Add(dealing);
		running_.SetMinorDealing(&savedDealing);
		business_->PushOrder(minorRequest);
	}

	BusinessDealingStore& Context::GetDealingStore() {
		return business_->GetDealingStore();
	}

	void Context::CancelMinor() {
		if (running_.MinorDealing->IsCancelSent()) {
			return;
		}
		auto request = running_.CreateMinorCancelRequest();
		running_.MinorDealing->SendCancel();
		business_->PushAction(request);
	}

	void Context::OppositeMinor(type::Volume volume) {
		auto request = running_.CreateMinorOppositeInsertRequest(volume);
		Dealing dealing(business_, this, request);
		auto& savedDealing = this->GetDealingStore().Add(dealing);
		business_->PushOrder(request);
	}

	void Context::OpenMajor(type::Volume volume) {
		auto request = running_.CreateMajorInsertRequest(volume);

		Dealing dealing(business_, this, request);
		auto& savedDealing = this->GetDealingStore().Add(dealing);
		business_->PushOrder(request);
	}


	type::Price BuyOpenPriceSelector::GetOpenSpread() const {
		auto const& minorBuy1 = minorQuote_.GetBuy().GetPrice();
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorBuy1 - majorBuy1;
	}
	type::Price BuyOpenPriceSelector::GetCloseSpread() const {
		auto const& minorSell1 = minorQuote_.GetSell().GetPrice();
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorSell1 - majorSell1;
	}
	type::Price BuyOpenPriceSelector::GetOpenSpread(type::Price minorOrderPrice) const {
		auto const& minorBuy1 = minorOrderPrice;
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorBuy1 - majorBuy1;
	}
	type::Price BuyOpenPriceSelector::GetCloseSpread(type::Price minorOrderPrice) const {
		auto const& minorSell1 = minorOrderPrice;
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorSell1 - majorSell1;
	}
	bool BuyOpenPriceSelector::IsOpenReady(AutoArgument const& arg) const {
		return GetOpenSpread() <= arg.BuyOpenSpread;
	}
	bool BuyOpenPriceSelector::IsCloseReady(AutoArgument const& arg) const 	{
		return GetCloseSpread() >= arg.BuyCloseSpread;
	}
	bool BuyOpenPriceSelector::IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return GetOpenSpread(minorOrderPrice) > arg.BuyOpenSpread;
	}
	bool BuyOpenPriceSelector::IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return GetCloseSpread(minorOrderPrice) < arg.BuyCloseSpread;
	}
	AvailablePrice BuyOpenPriceSelector::GetMinorOpenPrice(type::Volume maxVolume) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& buy1 = minorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = std::min(buy1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice BuyOpenPriceSelector::GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& buy1 = majorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price -= instrument->UnitPrice * majorLostTick;
		return result;
	}	
	AvailablePrice BuyOpenPriceSelector::GetMinorClosePrice(type::Volume maxVolume) const {
		//买方向平仓:价差涨到平仓价差获利（卖平)
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& sell1 = minorQuote_.GetSell();
		result.Price = sell1.GetPrice();
		result.Volume = std::min(sell1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice BuyOpenPriceSelector::GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& sell = majorQuote_.GetSell();
		result.Price = sell.GetPrice();
		result.Volume =  maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price += instrument->UnitPrice * majorLostTick;
		return result;
	}

	type::Price SellOpenPriceSelector::GetOpenSpread() const {
		auto const& minorSell1 = minorQuote_.GetSell().GetPrice();
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorSell1 - majorSell1;
	}
	type::Price SellOpenPriceSelector::GetCloseSpread() const {
		auto const& minorBuy1 = minorQuote_.GetBuy().GetPrice();
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorBuy1 - majorBuy1;
	}
	type::Price SellOpenPriceSelector::GetOpenSpread(type::Price minorOrderPrice) const {
		auto const& minorSell1 = minorOrderPrice;
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorSell1 - majorSell1;
	}
	type::Price SellOpenPriceSelector::GetCloseSpread(type::Price minorOrderPrice) const {
		auto const& minorBuy1 = minorOrderPrice;
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorBuy1 - majorBuy1;
	}
	bool SellOpenPriceSelector::IsOpenReady(AutoArgument const& arg) const {
		return GetOpenSpread() >= arg.SellOpenSpread;
	}
	bool SellOpenPriceSelector::IsCloseReady(AutoArgument const& arg) const {
		return GetCloseSpread() <= arg.SellCloseSpread;
	}
	bool SellOpenPriceSelector::IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return GetOpenSpread(minorOrderPrice) < arg.SellOpenSpread;
	}
	bool SellOpenPriceSelector::IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return GetCloseSpread(minorOrderPrice) > arg.SellCloseSpread;
	}
	AvailablePrice SellOpenPriceSelector::GetMinorOpenPrice(type::Volume maxVolume) const {
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& sell1 = minorQuote_.GetSell();
		result.Price = sell1.GetPrice();
		result.Volume = std::min(sell1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice SellOpenPriceSelector::GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& sell1 = majorQuote_.GetSell();
		result.Price = sell1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price += instrument->UnitPrice * majorLostTick;
		return result;
	}	
	AvailablePrice SellOpenPriceSelector::GetMinorClosePrice(type::Volume maxVolume) const {
		//卖方向平仓-价差跌到平仓价差 获利（买平)
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& buy1 = minorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = std::min(buy1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice SellOpenPriceSelector::GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& buy1 = majorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price -= instrument->UnitPrice * majorLostTick;
		return result;
	}


	type::Price FAKBuyOpenPriceSelector::GetOpenSpread() const {
		auto const& minorSell1 = minorQuote_.GetSell().GetPrice();
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorSell1 - majorBuy1;	//卖近买远->远卖-近买
	}
	type::Price FAKBuyOpenPriceSelector::GetCloseSpread() const {
		auto const& minorBuy1 = minorQuote_.GetBuy().GetPrice();
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorBuy1 - majorSell1;	//买近卖远->远买-近卖
	}
	type::Price FAKBuyOpenPriceSelector::GetOpenSpread(type::Price minorOrderPrice) const {
		auto const& minorSell1 = minorOrderPrice;
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorSell1 - majorBuy1;	//卖近买远->远卖-近买
	}
	type::Price FAKBuyOpenPriceSelector::GetCloseSpread(type::Price minorOrderPrice) const {
		auto const& minorBuy1 = minorOrderPrice;
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorBuy1 - majorSell1;	//买近卖远->远买-近卖
	}
	bool FAKBuyOpenPriceSelector::IsOpenReady(AutoArgument const& arg) const {
		return GetOpenSpread() <= arg.BuyOpenSpread;
	}
	bool FAKBuyOpenPriceSelector::IsCloseReady(AutoArgument const& arg) const {
		return GetCloseSpread() >= arg.BuyCloseSpread;
	}
	bool FAKBuyOpenPriceSelector::IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return false;	//对价不判断
	}
	bool FAKBuyOpenPriceSelector::IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return false;	//对价不判断
	}
	AvailablePrice FAKBuyOpenPriceSelector::GetMinorOpenPrice(type::Volume maxVolume) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& sell1 = minorQuote_.GetSell();		//买价差时，对价委托取卖价
		result.Price = sell1.GetPrice();
		result.Volume = std::min(sell1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice FAKBuyOpenPriceSelector::GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& buy1 = majorQuote_.GetBuy();		//买价差时，对价委托取买价
		result.Price = buy1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price -= instrument->UnitPrice * majorLostTick;
		return result;
	}
	AvailablePrice FAKBuyOpenPriceSelector::GetMinorClosePrice(type::Volume maxVolume) const {
		//买方向平仓:价差涨到平仓价差 获利（卖平)
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& buy1 = minorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = std::min(buy1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice FAKBuyOpenPriceSelector::GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& sell1 = majorQuote_.GetSell();
		result.Price = sell1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price += instrument->UnitPrice * majorLostTick;
		return result;
	}


	type::Price FAKSellOpenPriceSelector::GetOpenSpread() const {
		auto const& minorBuy1 = minorQuote_.GetBuy().GetPrice();
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorBuy1 - majorSell1;
	}
	type::Price FAKSellOpenPriceSelector::GetCloseSpread() const {
		auto const& minorSell1 = minorQuote_.GetSell().GetPrice();
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorSell1 - majorBuy1;
	}
	type::Price FAKSellOpenPriceSelector::GetOpenSpread(type::Price minorOrderPrice) const {
		auto const& minorBuy1 = minorOrderPrice;
		auto const& majorSell1 = majorQuote_.GetSell().GetPrice();
		return minorBuy1 - majorSell1;
	}
	type::Price FAKSellOpenPriceSelector::GetCloseSpread(type::Price minorOrderPrice) const {
		auto const& minorSell1 = minorOrderPrice;
		auto const& majorBuy1 = majorQuote_.GetBuy().GetPrice();
		return minorSell1 - majorBuy1;
	}
	bool FAKSellOpenPriceSelector::IsOpenReady(AutoArgument const& arg) const {
		return GetOpenSpread() >= arg.SellOpenSpread;
	}
	bool FAKSellOpenPriceSelector::IsCloseReady(AutoArgument const& arg) const {
		return GetCloseSpread() <= arg.SellCloseSpread;
	}
	bool FAKSellOpenPriceSelector::IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return false;	//对价不判断
	}
	bool FAKSellOpenPriceSelector::IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const {
		return false;	//对价不判断
	}
	AvailablePrice FAKSellOpenPriceSelector::GetMinorOpenPrice(type::Volume maxVolume) const {
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& buy1 = minorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = std::min(buy1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice FAKSellOpenPriceSelector::GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Open;
		auto const& sell1 = majorQuote_.GetSell();
		result.Price = sell1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price += instrument->UnitPrice * majorLostTick;
		return result;
	}
	AvailablePrice FAKSellOpenPriceSelector::GetMinorClosePrice(type::Volume maxVolume) const {
		AvailablePrice result;
		result.Directin = type::Direction::Buy;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& sell1 = minorQuote_.GetSell();
		result.Price = sell1.GetPrice();
		result.Volume = std::min(sell1.GetQuantity(), maxVolume.value());
		return result;
	}
	AvailablePrice FAKSellOpenPriceSelector::GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const {
		AvailablePrice result;
		result.Directin = type::Direction::Sell;
		result.OffsetFlag = type::OffsetFlag::Close;
		auto const& buy1 = majorQuote_.GetBuy();
		result.Price = buy1.GetPrice();
		result.Volume = maxVolume;
		auto instrument = ServiceSinglton::Services().Instrument->Get(majorQuote_.GetInstID());
		result.Price -= instrument->UnitPrice * majorLostTick;
		return result;
	}
}
