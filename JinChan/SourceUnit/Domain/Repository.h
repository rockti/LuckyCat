#pragma once

#include "Order.h"
//#include "Investor.h"
#include "ZTPType.h"
#include <functional>

using namespace ztp;

class Investor;

class InitOrderStore {
public:
	virtual void Add(data::COrder const& field) = 0;
	virtual std::optional<data::COrder> Get(type::OrderLocalID const& orderLocalID) = 0;
};

class BaseInfoRepository {
public:
	virtual void InitAdd(data::CSeatInfo const& seatInfo) = 0;
	virtual data::CSeatInfo* Get() = 0;
};

class SessionRepository {
public:
	virtual void Add(data::CUserSession const& session) = 0;
	virtual void Delete(data::CUserSession const& session) = 0;
	virtual data::CUserSession* Get(type::SessionID const& SessionID) = 0;
};

class InvestorInfoRepository {
public:
	virtual void InitAdd(data::CInvestorInfo const& info) = 0;
	virtual data::CInvestorInfo* Get(type::BrokerID, type::InvestorID) = 0;
};

class SystemStatusRepository {
public:
	virtual void Update(data::CSystemStatus const& systemStatus) = 0;
	virtual data::CSystemStatus* Get() = 0;
};

class OperatorRepository {
public:
	virtual void InitAdd(data::COperatorInfo const& info) = 0;
	virtual void InitAdd(data::COperatorInvestor const& operInvestor) = 0;
	virtual data::COperatorInfo* Get(type::BrokerID const& brokerID, type::OperatorID const& operatorID) = 0;
};

class InstrumentRepository {
public:
	using ForEachFunc = std::function<void (data::CInstrument const& inst, bool isLast)>;
	virtual void InitAdd(data::CInstrument const& instrument) = 0;
	virtual void AddOrUpdate(data::CInstrument const& instrument) = 0;
	virtual data::CInstrument* Get(type::InstrumentID const& instrumentID) = 0;
	virtual void ForEach(type::ProductClass ProductClass, type::InstrumentID const& InstrumentID, ForEachFunc func) = 0;
};

template<class T>
class Trigger {
public:
	virtual void OnAdd(T const& object) = 0;
	virtual void OnUpdate(T const& object) = 0;
	virtual void OnDelete(T const& object) = 0;
};


class InvestorRepository {
public:
	using OrderTrigger = Trigger<data::COrder>;
	using TradeTrigger = Trigger<data::CTrade>;
	using BalanceChangeTrigger = Trigger<data::CBalanceChange>;
	using PositionChangeTrigger = Trigger<data::CPositionChange>;
	using ForEachInvestor = std::function<void(data::CInvestorInfo const&, bool isLast)>;
	using ForEachPosition = std::function<void(data::CInvestorPosition const&, bool isLast)>;
	using ForEachMargin = std::function<void(data::CInvestorMargin const&, bool isLast)>;
	using ForEachFee = std::function<void(data::CInvestorFee const&, bool isLast)>;
	using ForEachOrder = std::function<void(data::COrder const&, bool isLast)>;
	using ForEachTrade = std::function<void(data::CTrade const&, bool isLast)>;
	using ForEachBalanceChange = std::function<void(data::CBalanceChange const&, bool isLast)>;
	using ForEachPositionChange = std::function<void(data::CPositionChange const&, bool isLast)>;
	virtual void InitAdd(data::CInvestorAccount& account) = 0;
	virtual void InitAdd(data::CInvestorMargin const& margin) = 0;
	virtual void InitAdd(data::CInvestorFee const& fee) = 0;
	virtual void InitAdd(data::CInvestorInfo const& info) = 0;
	virtual void InitAdd(data::CBalanceChange const& change) = 0;
	virtual void InitAdd(data::CPositionChange const& change) = 0;
	virtual data::CInvestorPosition* InitAdd(data::CInvestorPosition const& position) = 0;

	virtual void SetLastSGESeqNo(type::SequenceNo seqNo) = 0;
	virtual type::SequenceNo GetLastSGESeqNo() const = 0;

	virtual void Update(data::CInvestorAccount const& account) = 0;
	virtual void Update(data::CInvestorPosition const& position) = 0;

	virtual void AddOrderSysNoIndex(data::COrder const& order) = 0;
	virtual data::COrder const* Add(data::COrder const& order) = 0;
	virtual data::CTrade const* Add(data::CTrade const& trade) = 0;
	virtual data::CBalanceChange const* Add(data::CBalanceChange const& balanceChange) = 0;
	virtual data::CPositionChange const* Add(data::CPositionChange const& positionChange) = 0;
	virtual void Update(data::COrder const& order) = 0;
	virtual void Update(data::CTrade const& trade) = 0;


	virtual data::COrder* GetOrder(type::InvestorID const& investorID, type::OrderSysID const& orderSysID) = 0;
	virtual data::COrder* GetOrder(type::InvestorID const& investorID, type::OrderLocalID const& localID) = 0;
	virtual data::COrder* GetOrder(type::InvestorID const& investorID, type::UserOrderLocalID const& localID) = 0;
//	virtual COrder* GetTrade() = 0;


	virtual data::CInvestorAccount* GetAccount(type::InvestorID const& investorID) = 0;
	virtual data::CInvestorInfo* GetInfo(type::InvestorID const& investorID) = 0;
	virtual data::CInvestorFee* GetFee(type::BrokerID const& brokerID, type::InvestorID const& investorID, type::InstrumentID const& instrumentID) = 0;
	virtual data::CInvestorMargin* GetMargin(type::BrokerID const& brokerID, type::InvestorID const& investorID, type::InstrumentID const& instrumentID) = 0;
	virtual data::CInvestorPosition* GetPosition(type::InvestorID const& investorID, type::InstrumentID const& instrumentID, type::Direction direction) = 0;
	virtual int InvestorForEach(ForEachInvestor func) = 0;
	virtual int PositionForEach(type::InvestorID const& investorID, ForEachPosition func) = 0;
	virtual int MarginForEach(type::InvestorID const& investorID, type::InstrumentID const& inst, ForEachMargin func) = 0;
	virtual int FeeForEach(type::InvestorID const& investorID, type::InstrumentID const& inst, ForEachFee func) = 0;
	virtual int OrderForEach(type::InvestorID const& investorID, type::InstrumentID const& instId, ForEachOrder func) = 0;
	virtual int TradeForEach(type::InvestorID const& investorID, type::OrderSysID const& sysID, ForEachTrade func) = 0;
	virtual int TradeForEach(type::InvestorID const& investorID, type::InstrumentID const& instId, ForEachTrade func) = 0;
	virtual int BalanceChangeForEach(type::InvestorID const& investorID, type::SequenceNo const& seqNo, ForEachBalanceChange func) = 0;
	virtual int BalanceChangeForEach(type::InvestorID const& investorID, ForEachBalanceChange func) = 0;
	virtual int PositionChangeForEach(type::InvestorID const& investorID, type::SequenceNo const& seqNo, ForEachPositionChange func) = 0;
	virtual int PositionChangeForEach(type::InvestorID const& investorID, ForEachPositionChange func) = 0;

	void AddTrigger(OrderTrigger* trigger) {
		m_orderTriggers.push_back(trigger);
	}
	void AddTrigger(TradeTrigger* trigger) {
		m_tradeTriggers.push_back(trigger);
	}
	void AddTrigger(BalanceChangeTrigger* trigger) {
		m_balanceChangeTriggers.push_back(trigger);
	}
	void AddTrigger(PositionChangeTrigger* trigger) {
		m_positionChangeTriggers.push_back(trigger);
	}

	void TriggerAdd(data::COrder const& order) {
		for (auto& trigger : m_orderTriggers) {
			trigger->OnAdd(order);
		}
	}
	void TriggerAdd(data::CTrade const& trade) {
		for (auto& trigger : m_tradeTriggers) {
			trigger->OnAdd(trade);
		}
	}
	void TriggerAdd(data::CBalanceChange const& balanceChange) {
		for (auto& trigger : m_balanceChangeTriggers) {
			trigger->OnAdd(balanceChange);
		}
	}
	void TriggerAdd(data::CPositionChange const& positionChange) {
		for (auto& trigger : m_positionChangeTriggers) {
			trigger->OnAdd(positionChange);
		}
	}
	void TriggerUpdate(data::COrder const& order) {
		for (auto& trigger : m_orderTriggers) {
			trigger->OnUpdate(order);
		}
	}
	void TriggerUpdate(data::CTrade const& trade) {
		for (auto& trigger : m_tradeTriggers) {
			trigger->OnUpdate(trade);
		}
	}

	void TriggerUpdate(data::CBalanceChange const& balanceChange) {
		for (auto& trigger : m_balanceChangeTriggers) {
			trigger->OnUpdate(balanceChange);
		}
	}
	void TriggerUpdate(data::CPositionChange const& positionChange) {
		for (auto& trigger : m_positionChangeTriggers) {
			trigger->OnUpdate(positionChange);
		}
	}
private:
	std::vector<OrderTrigger*> m_orderTriggers;
	std::vector<TradeTrigger*> m_tradeTriggers;
	std::vector<BalanceChangeTrigger*> m_balanceChangeTriggers;
	std::vector<PositionChangeTrigger*> m_positionChangeTriggers;
};


template<class Repo, class T>
struct Saver {
	Saver(Repo& repo, T const& t, bool isAdd = false) : repo_(repo), value_(t),isAdd_(isAdd) {}
	~Saver() {
		if (isAdd_) {
			repo_.TriggerAdd(value_);
		} else {
			repo_.TriggerUpdate(value_);
		}
	}
	T const& value_;
	Repo& repo_;
	bool isAdd_;
};