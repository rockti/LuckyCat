
#include "OrderService.h"
#include "model/dealing/ExchangeRequest.h"
#include "utils/Log.h"

void OrderService::InitAdd(InvestorOrder const& order) {
	initLocalID(order);
	auto iter = uniqueIndex_.find(order.UniqueID);
	if (iter == uniqueIndex_.end()) {
		auto& o = orders_.emplace_back(order);
		localIdIndex_.emplace(order.OrderLocalID, &o);
		uniqueIndex_.emplace(order.UniqueID, &o);
		o.Index = ++index_;
		notifyAdd(o);
		updateSysIdIndex(o);
	} else {
		*iter->second = order;
		notifyChange(*iter->second);
		updateSysIdIndex(*iter->second);
	}
}

void OrderService::initLocalID(const ztp::InvestorOrder& order) {
	static int MaxLocalId = 0;
	if (LocalIdACC::IsMine(order.OrderLocalID) && order.OrderLocalID > MaxLocalId) {
		uint32_t newMaxLocalId = order.OrderLocalID + 500; // 500 is buffer
		if (newMaxLocalId > LocalIdACC::GetCurrentId())	{
			LocalIdACC::Init(newMaxLocalId);
		}
		MaxLocalId = order.OrderLocalID;
	}
}

InvestorOrder const& OrderService::AddOrUpdate(InvestorOrder const& order) {
	auto iter = uniqueIndex_.find(order.UniqueID);
	if (iter == uniqueIndex_.end()) {
		auto& o = orders_.emplace_back(order);
		localIdIndex_.emplace(order.OrderLocalID, &o);
		uniqueIndex_.emplace(order.UniqueID, &o);
		o.Index = ++index_;
		notifyAdd(o);
		updateSysIdIndex(o);
		return o;
	} else {
		auto* o = iter->second;
		Log::Info(fmt::format("Order Service: [Before] Inst[{}] Dir[{}] OP[{}] Local[{}] Sys[{}] Price[{}] Volume[{}] Traded[{}] Remain[{}] Status[{}]",
			o->InstrumentCode,o->Direction,o->OffsetFlag,o->OrderLocalID,o->OrderSysID,o->LimitPrice, o->Volume,
			o->VolumeTrade,o->VolumeRemain,o->OrderStatus));
			
		iter->second->VolumeTrade = order.VolumeTrade;
		iter->second->VolumeRemain = order.VolumeRemain;
		iter->second->InsertTime = order.InsertTime;
		iter->second->CancelTime = order.CancelTime;
		iter->second->OrderSysID = order.OrderSysID;
		iter->second->OrderStatus = order.OrderStatus;
		iter->second->CancelLocalID = order.CancelLocalID;
		iter->second->InsertError = order.InsertError;
		iter->second->CancelError = order.CancelError;

		Log::Info(fmt::format("Order Service: [After] Inst[{}] Dir[{}] OP[{}] Local[{}] Sys[{}] Price[{}] Volume[{}] Traded[{}] Remain[{}] Status[{}]",
			o->InstrumentCode, o->Direction, o->OffsetFlag, o->OrderLocalID, o->OrderSysID, o->LimitPrice, o->Volume,
			o->VolumeTrade, o->VolumeRemain, o->OrderStatus));


		notifyChange(*iter->second);
		updateSysIdIndex(*iter->second);
		return *iter->second;
	}
}


void OrderService::notifyAdd(const InvestorOrder& order) {
	for (auto& o : observers_) {
		o->OnAdd(order);
	}
}

void OrderService::notifyChange(const InvestorOrder& order) {
	for (auto& o : observers_) {
		o->OnChange(order);
	}
}

void OrderService::updateSysIdIndex(InvestorOrder& order) {
	if (!order.IsSysIDEmpty()) {
		if (sysIdIndex_.find(order.OrderSysID) == sysIdIndex_.end()) {
			sysIdIndex_.emplace(order.OrderSysID, &order);
		}
	}
}

OrderResult OrderService::GetOrder(type::OrderUniquID uniqueID) const {
	auto iter = uniqueIndex_.find(uniqueID);
	if (iter == uniqueIndex_.end()) {
		return { NotFoundError };
	} else {
		return { *iter->second };
	}
}


OrderResult OrderService::GetOrder(type::OrderLocalID localID) const {
	auto iter = localIdIndex_.find(localID);
	if (iter == localIdIndex_.end()) {
		return { NotFoundError };
	} else {
		return { *iter->second };
	}
}

OrderResult OrderService::GetOrder(type::OrderSysID const& sysNo) const {
	auto iter = sysIdIndex_.find(sysNo);
	if (iter == sysIdIndex_.end()) {
		return {};
	} else {
		return { *iter->second };
	}
}
void OrderService::OnTrade(InvestorTrade const& trade) {
	auto iter = sysIdIndex_.find(trade.OrderSysID);
	if (iter != sysIdIndex_.end()) {
		iter->second->OnTrade(trade);
		for (auto& o : observers_) {
			o->OnChange(*iter->second);
		}
	}
}

Orders OrderService::GetAll() const {
	return orders_;
}
