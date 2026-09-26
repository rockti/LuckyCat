#include "Dispatcher.h"
#include "service/Service.h"
#include "utils/Log.h"
#include "strategy/auto/AutoStrategy.h"
#include "service/CancellationLimiter.h"
using namespace automate;

void Dispatcher::Dispatch(Data& data) {
	auto& services = ServiceSinglton::Services();
	switch (data.Type) {
	case DataType::Quote: {
		auto& quote = *(Quote*)data.Payload;
		auto const& savedQuote = services.Quote->Update(quote);
		services.Position->OnQuote(savedQuote);
		break;
	}
	case DataType::Order: {
		auto& order = *(InvestorOrder*)data.Payload;
		Log::Info(fmt::format("dispatch order: Unique[{}] Local[{}] Sys[{}]",
			order.UniqueID, order.OrderLocalID, order.OrderSysID));
		services.Order->InitAdd(order);
		services.Position->OnInitOrder(order);
		if (CancellationLimiter::IsCountCancel(order)) {
			auto const& cancelCount = CancellationLimiter::Instance().Add(order.InstID);
			auto inst = ServiceSinglton::Services().Instrument->Get(order.InstID);
			if (cancelCount.IsOverLimit()) {
				Log::Info(fmt::format("OverCancelLimit: Inst[{}] Max[{}] Current[{}] Buff[{}]",
					inst->Code, cancelCount.Max, cancelCount.Current, cancelCount.CancelBuff));
			} else {
				Log::Info(fmt::format("CancelCount: Inst[{}] Max[{}] Current[{}] Buff[{}]",
					inst->Code, cancelCount.Max, cancelCount.Current, cancelCount.CancelBuff));
			}
		}
		break;
	}
	case DataType::OrderRtn: {
		auto& orderRtn = *(OrderRtnNotification*)data.Payload;
		dispatch_order_rtn(orderRtn);
		if (CancellationLimiter::IsCountCancel(orderRtn.Order)) {
			auto const& cancelCount = CancellationLimiter::Instance().Add(orderRtn.Order.InstID);
			if (cancelCount.IsOverLimit()) {
				auto inst = ServiceSinglton::Services().Instrument->Get(orderRtn.Order.InstID);
				if (inst) {
					Log::Info(fmt::format("OverCancelLimit: Inst[{}] Max[{}] Current[{}] Buff[{}]",
						inst->Code, cancelCount.Max, cancelCount.Current, cancelCount.CancelBuff));
				}
			}
		}
		break;
	}
	case DataType::OrderErrorRtn: {
		auto& order = *(OrderErrorNotification*)data.Payload;
		dispatch_order_error(order);
		break;
	}
	case DataType::Trade: {
		auto& trade = *(InvestorTrade*)data.Payload;
		dispatch_trade(trade);
		break;
	}
	case DataType::TradeRtn: {
		auto& trade = *(InvestorTrade*)data.Payload;
		dispatch_trade(trade, true);
		break;
	}

	case DataType::Error: {
		auto& error = *(ErrorNotification*)data.Payload;
		services.TradeSystem->OnError(error);
		break;
	}

	case DataType::Instrument: {
		auto& inst = *(Instrument*)data.Payload;
		auto const& savedInst = services.Instrument->Add(inst);
		CancellationLimiter::Instance().Init(savedInst.InstID, 400, 0);
		services.TradeSystem->SubscribeQuote(inst.Exchange, inst.Code);
		auto platform = services.Exchange->toPlatform(inst.Exchange);
		break;
	}
	case DataType::Fee: {
		auto& fee = *(InvestorFee*)data.Payload;
		auto platform = services.Exchange->toPlatform(fee.Exchange);
		services.Instrument->AddFee(platform, fee);
		break;
	}
	case DataType::Margin: {
		auto& margin = *(InvestorMargin*)data.Payload;
		auto platform = services.Exchange->toPlatform(margin.Exchange);
		services.Instrument->AddMargin(platform, margin);
		break;
	}
	case DataType::Position: {
		auto& pos = *(InvestorPosition*)data.Payload;
		services.Position->InitAdd(pos);
		break;
	}
	case DataType::Account: {
		auto& account = *(InvestorAccount*)data.Payload;
		services.Account->Update(account.Platform, account);
		break;
	}
	case DataType::Exchange: {
		auto& en = *(ExchangeNotification*)data.Payload;
		auto& ts = ServiceSinglton::Services().TradeSystem;
		ts->OnStatus(en.Exchange, en.Status);
		break;
	}
	case DataType::TradePlatform: {
		auto& en = *(TradePlatformNotification*)data.Payload;
		dispatch(en);
		break;
	}
	case DataType::InitStatus: {
		auto& isn = *(InitStatusNotification*)data.Payload;
		services.Init->OnStatusUpdate(isn.Platform, isn.Process, isn.Status, isn.Error);
		break;
	}
	case DataType::CleanPosition: {
		auto& en = *(CleanPositionNotification*)data.Payload;
		services.Position->ClearPlatform(en.Platform);
		break;
	}
	case DataType::BusinessUpdate: {
		auto& bu = *(AutoSnapshot*)data.Payload;
		services.Strategy->OnUpdate(bu.ID, bu);
	}
	default:
		break;
	}
}

void Dispatcher::dispatch(TradePlatformNotification& en) {
	auto& ts = ServiceSinglton::Services().TradeSystem;
	switch (en.Type) {
	case TradePlatformNotification::LOGIN_FAILED: {	
		ts->OnLoginFailed(en.Platform, en.SubSystem, en.Error);
		break;
	}
	case TradePlatformNotification::LOGIN: {
		ts->OnLogin(en.Platform, en.SubSystem, en.TradingDay);
		if (en.Platform == TradePlatform::ZTP && en.SubSystem == SubSystem::Trade) {
			ts->SetTradingDay(en.TradingDay);
			auto lastLocalID = LocalIdACC::Saver().Load(en.TradingDay);
			if (lastLocalID > 0 && LocalIdACC::IsMine(lastLocalID)) {
				LocalIdACC::Init(lastLocalID + 100);
			}
		}
		break;
	}
	case TradePlatformNotification::LOGOUT: {		
		ts->OnLogout(en.Platform, en.SubSystem);
		break;
	}
	case TradePlatformNotification::CONNECTED: {
		ts->OnConnected(en.Platform, en.SubSystem);
		break;
	}
	case TradePlatformNotification::DISCONNECTED: {
		ts->OnDisconnected(en.Platform, en.SubSystem);
		break;
	}
	case TradePlatformNotification::PASSWORD_CHANGE: {
		ts->OnPasswordChange(en.Platform, en.SubSystem, en.Error);
		break;
	}
	default:
		break;
	}
}

void Dispatcher::dispatch_order_rtn(OrderRtnNotification& en) {
	auto& services = ServiceSinglton::Services();
	auto platform = services.Exchange->toPlatform(en.Order.Exchange);

	auto status = en.Order.OrderStatus;
	auto order = services.Order->GetOrder(en.Order.UniqueID);
 	Log::Info(fmt::format("dispatch order rtn: Saved[{}] Unique[{}] Local[{}] Sys[{}]", 
 		order.IsSuccess(), en.Order.UniqueID, en.Order.OrderLocalID, en.Order.OrderSysID));
	if (order) {
		Log::Info(fmt::format("dispatch order rtn: Saved[{}] Unique[{}] Local[{}] Sys[{}] Inst[{}] Status[{}-{}] Traded[{}-{}] Remained[{}-{}]",
			order.IsSuccess(), en.Order.UniqueID, en.Order.OrderLocalID, en.Order.OrderSysID,
			en.Order.InstrumentCode, en.Order.OrderStatus, order->OrderStatus,
			en.Order.VolumeTrade, order->VolumeTrade, en.Order.VolumeRemain, order->VolumeRemain));
		// traded, remained and avg price are managed by OnRtnTrade
		services.Order->AddOrUpdate(en.Order);

		services.Position->OnOrderRtn(en.Order);
		trigger_query_account(status, platform);
		return;
	}
	//自己下的新订单或者是其它系统下的单
	if (en.Order.IsPendingAndNoTrade()) { 
		auto const& o = services.Order->AddOrUpdate(en.Order);
		if (!type::IsOpen(o)) { // 平仓单，冻结持仓
			if (PositionHasOffset(en.Order.InstID, en.Order.Exchange)) {
				services.Position->FreezePosition(en.Order);
			}
		}
		return;
	} 
	//其它系统的报单先存
	if (en.Order.IsSysIDEmpty()) { //不能没有系统号
		Log::Error(fmt::format("New OrderRtn without OrderSysID - LocalID[{}]", en.Order.OrderLocalID));
		return;
	}
	auto const& o = services.Order->AddOrUpdate(en.Order);
	
	trigger_query_account(status, platform);
}

void Dispatcher::trigger_query_account(type::OrderStatus status, TradePlatform platform) {
	switch (status) {
	case ztp::type::OrderStatus::New:
	case ztp::type::OrderStatus::InsertFail:
	case ztp::type::OrderStatus::CancelFail:
	case ztp::type::OrderStatus::InsertQueueing:
	case ztp::type::OrderStatus::CancelQueueing:
		return;
	case ztp::type::OrderStatus::NoTradeQueueing:
	case ztp::type::OrderStatus::PartTradedQueueing:
	case ztp::type::OrderStatus::AllTraded:
	case ztp::type::OrderStatus::Canceled:
	case ztp::type::OrderStatus::NoTradeNotQueueing:
		ServiceSinglton::Services().TradeSystem->QueryAccount(platform);
		return;
	case ztp::type::OrderStatus::Unknown:
	default:
		break;
	}
}
void Dispatcher::dispatch_order_error(OrderErrorNotification& en) {
	auto& services = ServiceSinglton::Services();
	auto order = services.Order->GetOrder(en.OrderLocalID);
	if (order) {
		switch (en.Type) {
		case OrderErrorNotification::Insert: {
			order->OnStatus(type::OrderStatus::InsertFail);
			order->InsertError = en.Error;
			services.Order->AddOrUpdate(order.Value.value());
			services.Position->UnfreezePosition(order.Value.value()); //其它系统的报单也需要冻结持仓
			break;
		}
		case OrderErrorNotification::Cancel: {
			order->OnStatus(type::OrderStatus::CancelFail);
			order->CancelError = en.Error;
			services.Order->AddOrUpdate(order.Value.value());
			break;
		}
		default:
			break;
		}
	}
	services.TradeSystem->OnOrderError(en);
}

void Dispatcher::dispatch_trade(InvestorTrade& trade, bool isRtn) {
	Log::Info(fmt::format("Dispatch Trade: isRtn[{}] Inst[{}] Dir[{}] Price[{}] Qty[{}]", 
		isRtn, trade.InstrumentCode, trade.Direction, trade.TradePrice, trade.TradeVolume));
	auto& services = ServiceSinglton::Services();
	services.Trade->Add(trade);

	if (isRtn) {
		services.Order->OnTrade(trade);
		services.Position->OnTrade(trade);
	}
}
