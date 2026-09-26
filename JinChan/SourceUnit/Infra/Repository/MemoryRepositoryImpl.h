#pragma once

#include "Repository.h"
#define BOOST_NO_MT
#include <boost/pool/pool_alloc.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>

#include <tuple>

typedef boost::container::deque_options<boost::container::block_size<128u>>::type block_128_option_t;

template<class T>
using Deque = boost::container::deque<T, void, block_128_option_t>;

class MemInitOrderStore : public InitOrderStore {
public:
	virtual void Add(data::COrder const& field) override {
		m_orders.emplace(field.OrderLocalID, field);
	}
	virtual std::optional<data::COrder> Get(type::OrderLocalID const& orderLocalID) override {
		auto iter = m_orders.find(orderLocalID);
		if (iter != m_orders.end()) {
			return iter->second;
		}
		return {};
	}
private:
	std::map<type::OrderLocalID, data::COrder> m_orders;
};

template<class T, class Base>
class SingletonRepository : public Base {
public:
	virtual void Update(T const& o) override { m_object = o; }
	virtual T* Get() override { return &m_object; }
private:
	T m_object;
};

class MemSystemStatusRepository : public SingletonRepository<data::CSystemStatus,SystemStatusRepository> {

};

class MyBaseInfoRepository : public BaseInfoRepository {
public:
	virtual void InitAdd(data::CSeatInfo const& seatInfo) override {
		m_seatInfo = seatInfo;
	}
	virtual data::CSeatInfo* Get() override {
		return &m_seatInfo;
	}
private:
	data::CSeatInfo m_seatInfo;
};

class MemSessionRepository : public SessionRepository {
public:
	virtual void Add(data::CUserSession const& session) override;
	virtual void Delete(data::CUserSession const& session) override;
	virtual data::CUserSession* Get(type::SessionID const& SessionID) override;
	std::map<int, data::CUserSession> m_userSessions;
};

class MemInvestorInfoRepository : public InvestorInfoRepository {
	typedef std::tuple<type::BrokerID, type::InvestorID> Key;
public:
	virtual void InitAdd(data::CInvestorInfo const& info) override {
		m_investorInfos.emplace(std::tie(info.BrokerID, info.InvestorID), info);
	}
	virtual data::CInvestorInfo* Get(type::BrokerID brokerID, type::InvestorID investorID) override {
		auto iter = m_investorInfos.find(std::tie(brokerID, investorID));
		if (iter != m_investorInfos.end()) {
			return &iter->second;
		}
		return nullptr;
	}
private:
	std::map<Key, data::CInvestorInfo> m_investorInfos;
};

class MyOperatorRepository : public OperatorRepository {
public:
	MyOperatorRepository() {
		m_opInvestors.reserve(100);
	}
	virtual void InitAdd(data::COperatorInfo const& info) override {
		m_opers.emplace(info.OperatorID, info);
	}
	virtual void InitAdd(data::COperatorInvestor const& operInvestor) override {
		m_opInvestors.push_back(operInvestor);
	}
	virtual data::COperatorInfo* Get(type::BrokerID const& brokerID, type::OperatorID const& operatorID) override {
		auto iter = m_opers.find(operatorID);
		if (iter != m_opers.end()) {
			return &iter->second;
		}
		return nullptr;
	}
private:
	std::map<type::OperatorID, data::COperatorInfo> m_opers;
	std::vector<data::COperatorInvestor>  m_opInvestors;
};


class MyInstrumentRepository : public InstrumentRepository {
public:
	MyInstrumentRepository(){
	}
	virtual void InitAdd(data::CInstrument const& instrument) override {
		m_insts.emplace(instrument.InstrumentID, instrument);
	}
	virtual void AddOrUpdate(data::CInstrument const& instrument) override {
		m_insts[instrument.InstrumentID] = instrument;
	}
	virtual data::CInstrument* Get(type::InstrumentID const& instrumentID) override {
		auto iter = m_insts.find(instrumentID);
		if (iter != m_insts.end()) {
			return &iter->second;
		}
		return nullptr;
	}

	virtual void ForEach(type::ProductClass ProductClass, type::InstrumentID const& InstrumentID, ForEachFunc func) override {
		auto size = m_insts.size();
		data::CInstrument* pre = nullptr;
		auto instMatch = [](type::InstrumentID const& query, type::InstrumentID const& inDB) {
			if (query == "*") {
				return true;
			} 
			return query == inDB;
		};

		auto classMatch = [](type::ProductClass query, type::ProductClass inDB) {
			if (query == type::ProductClass::Unknown) {
				return true;
			}
			return query == inDB;
		};

		for (auto& kv : m_insts ) {
			if (classMatch(ProductClass, kv.second.ProductClass) && 
				instMatch(InstrumentID, kv.second.InstrumentID)) {
				if (pre != nullptr) {
					func(*pre, false);
				}
				pre = &kv.second;
			}
		}
		if (pre != nullptr) {
			func(*pre, true);
		}
	}
private:
	std::map<type::InstrumentID, data::CInstrument> m_insts;
};

class MyInvestorRepository : public InvestorRepository {
public:
	MyInvestorRepository() {

	}
	virtual void InitAdd(data::CInvestorAccount& account) override;
	virtual void InitAdd(data::CInvestorMargin const& margin) override {
		if (margin.InvestorID == "*") {
			auto& broker = m_brokers[margin.BrokerID];
			broker.Margins.InitAdd(margin);
		} else {
			auto iter = m_investors.find(margin.InvestorID);
			if (iter != m_investors.end()) {
				iter->second.Margins.InitAdd(margin);
			}
		}
	}
	virtual void InitAdd(data::CInvestorFee const& fee) override {
		if (fee.InvestorID == "*") {
			auto& broker = m_brokers[fee.BrokerID];
			broker.Fees.InitAdd(fee);
		} else {
			auto iter = m_investors.find(fee.InvestorID);
			if (iter != m_investors.end()) {
				iter->second.Fees.InitAdd(fee);
			}
		}
	}
	virtual void InitAdd(data::CInvestorInfo const& info) override {
		auto& investor = m_investors[info.InvestorID];
		CopyInvestorInfoEntity(&investor.Info, &info);
	}
	virtual void InitAdd(data::CBalanceChange const& change) override {
		auto iter = m_investors.find(change.InvestorID);
		if (iter != m_investors.end()) {
			if (change.BalanceSequenceNo > m_balanceSeqNo) {
				m_balanceSeqNo = change.BalanceSequenceNo;
			}
			iter->second.BalanceChanges.Add(change);
		}
	}
	virtual void InitAdd(data::CPositionChange const& change) override {
		auto iter = m_investors.find(change.InvestorID);
		if (iter != m_investors.end()) {
			if (change.PositionSequenceNo > m_positionSeqNo) {
				m_positionSeqNo = change.PositionSequenceNo;
			}
			iter->second.PositionChanges.Add(change);
		}
	}
	virtual data::CInvestorPosition* InitAdd(data::CInvestorPosition const& position) override {
		auto iter = m_investors.find(position.InvestorID);
		if (iter != m_investors.end()) {
			return iter->second.Positions.InitAdd(position);
		}
		return nullptr;
	}

	virtual void SetLastSGESeqNo(type::SequenceNo seqNo) override {
		if (seqNo > m_lastSeqNo) {
			m_lastSeqNo = seqNo;
		}
	}

	virtual type::SequenceNo GetLastSGESeqNo() const override {
		return m_lastSeqNo;
	}

	virtual void Update(data::CInvestorAccount const& account) override {
	}
	virtual void Update(data::CInvestorPosition const& position) override {
// 		if (position.Position <= 0 && position.YdPosition <=0) {
// 			auto& investor = m_investors[position.InvestorID];
// 			investor.Positions.Remove(position.InstrumentID, position.Direction);
// 		}
	}

	virtual void AddOrderSysNoIndex(data::COrder const& order) override {
		auto& investor = m_investors[order.InvestorID];
		investor.Orders.AddSynNoIndex(const_cast<data::COrder&>(order));
	}

	virtual data::COrder const* Add(data::COrder const& order) override {
		auto& investor = m_investors[order.InvestorID];
		auto result = investor.Orders.Add(order);
//		TriggerAdd(*result);
		return result;
	}
	virtual data::CTrade const* Add(data::CTrade const& trade) override {
		auto& investor = m_investors[trade.InvestorID];
		auto result = investor.Trades.Add(trade);
//		TriggerAdd(*result);
		return result;
	}

	virtual data::CBalanceChange const* Add(data::CBalanceChange const& balanceChange) override {
		auto& investor = m_investors[balanceChange.InvestorID];
		auto result = investor.BalanceChanges.Add(balanceChange);
		result->BalanceSequenceNo = ++m_balanceSeqNo;
		return result;
	}
	virtual data::CPositionChange const* Add(data::CPositionChange const& positionChange) override {
		auto& investor = m_investors[positionChange.InvestorID];
		auto result = investor.PositionChanges.Add(positionChange);
		result->PositionSequenceNo = ++m_positionSeqNo;
		return result;
	}

	virtual void Update(data::COrder const& order) override {
//		TriggerUpdate(order);
	}
	virtual void Update(data::CTrade const& trade) override {
//		TriggerUpdate(trade);
	}

	virtual data::COrder* GetOrder(type::InvestorID const& investorID, type::OrderSysID const& orderSysID) override {
		auto& investor = m_investors[investorID];
		return investor.Orders.Get(orderSysID);
	}
	virtual data::COrder* GetOrder(type::InvestorID const& investorID, type::OrderLocalID const& localID) override {
		auto& investor = m_investors[investorID];
		return investor.Orders.Get(localID);
	}
	virtual data::COrder* GetOrder(type::InvestorID const& investorID, type::UserOrderLocalID const& userLocalID) override {
		auto& investor = m_investors[investorID];
		return investor.Orders.Get(userLocalID);
	}

	virtual data::CInvestorInfo* GetInfo(type::InvestorID const& investorID) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return &iter->second.Info;
		}
		return nullptr;
	}
	virtual data::CInvestorAccount* GetAccount(type::InvestorID const& investorID) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return &iter->second.Account;
		}
		return nullptr;
	}
	virtual data::CInvestorFee* GetFee(type::BrokerID const& brokerID, type::InvestorID const& investorID, type::InstrumentID const& instrumentID) override {
		if (investorID == "*") {
			auto iter = m_brokers.find(brokerID);
			if (iter != m_brokers.end()) {
				return iter->second.Fees.Get(instrumentID);
			}
			return nullptr;
		}
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Fees.Get(instrumentID);
		}
		return nullptr;

	}
	virtual data::CInvestorMargin* GetMargin(type::BrokerID const& brokerID, type::InvestorID const& investorID, type::InstrumentID const& instrumentID) override {
		if (investorID == "*") {
			auto iter = m_brokers.find(brokerID);
			if (iter != m_brokers.end()) {
				return iter->second.Margins.Get(instrumentID);
			}
			return nullptr;
		}
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Margins.Get(instrumentID);
		}
		return nullptr;
	}
	virtual data::CInvestorPosition* GetPosition(type::InvestorID const& investorID, type::InstrumentID const& instrumentID, type::Direction direction) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Positions.Get(instrumentID, direction);
		}
		return nullptr;
	}

	virtual int InvestorForEach(ForEachInvestor func) override {
		auto iter = m_investors.begin();
		auto end = m_investors.end();
		auto size = m_investors.size();
		int count = 1;
		for (; iter != end; ++iter,++count) {
			func(iter->second.Info, count == size);
		}
		return count;
	}
	virtual int PositionForEach(type::InvestorID const& investorID, ForEachPosition func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Positions.ForEach(func);
		}
		return 0;
	}
	virtual int MarginForEach(type::InvestorID const& investorID, type::InstrumentID const& inst, ForEachMargin func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Margins.ForEach(inst,func);
		}
		return 0;
	}
	virtual int FeeForEach(type::InvestorID const& investorID, type::InstrumentID const& inst, ForEachFee func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Fees.ForEach(inst, func);
		}
		return 0;
	}
	virtual int OrderForEach(type::InvestorID const& investorID, type::InstrumentID const& instID, ForEachOrder func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Orders.ForEach(instID,func);
		}
		return 0;
	}
	virtual int TradeForEach(type::InvestorID const& investorID, type::OrderSysID const& sysID, ForEachTrade func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Trades.ForEach(sysID, func);
		}
		return 0;
	}
	virtual int TradeForEach(type::InvestorID const& investorID, type::InstrumentID const& instId, ForEachTrade func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.Trades.ForEach(instId, func);
		}
		return 0;
	}

	virtual int BalanceChangeForEach(type::InvestorID const& investorID, type::SequenceNo const& seqNo, ForEachBalanceChange func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.BalanceChanges.ForEach(seqNo, func);
		}
		return 0;
	}


	virtual int BalanceChangeForEach(type::InvestorID const& investorID, ForEachBalanceChange func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.BalanceChanges.ForEach(func);
		}
		return 0;
	}

	virtual int PositionChangeForEach(type::InvestorID const& investorID, type::SequenceNo const& seqNo, ForEachPositionChange func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.PositionChanges.ForEach(seqNo, func);
		}
		return 0;
	}

	virtual int PositionChangeForEach(type::InvestorID const& investorID, ForEachPositionChange func) override {
		auto iter = m_investors.find(investorID);
		if (iter != m_investors.end()) {
			return iter->second.PositionChanges.ForEach(func);
		}
		return 0;
	}

private:
	static constexpr int MaxInstrumentCount = 100;
	uint32_t m_balanceSeqNo = 0;
	uint32_t m_positionSeqNo = 0;
	uint32_t m_lastSeqNo = 0;
	struct MarginStore {
		MarginStore() {
			m_margins.reserve(MaxInstrumentCount);
		}
		void InitAdd(data::CInvestorMargin const& margin) {
			auto& value = m_margins.emplace_back(margin);
			m_instIndex.emplace(margin.InstrumentID, &value);
		}
		data::CInvestorMargin* Get(type::InstrumentID const& inst) {
			auto iter = m_instIndex.find(inst);
			if (iter == m_instIndex.end()) {
				return nullptr;
			}
			return iter->second;
		}

		int ForEach(type::InstrumentID const& inst, ForEachMargin func) {
			data::CInvestorMargin const* pre = nullptr;
			int count = 0;
			for (auto const& margin : m_margins) {
				if (inst == "*" || inst == margin.InstrumentID) {
					if (pre != nullptr) {
						func(*pre, false);
						++count;
					}
					pre = &margin;
				}
			}
			if (pre != nullptr) {
				func(*pre, true);
				++count;
			}
			return count;
		}

		std::vector<data::CInvestorMargin> m_margins;
		std::map<type::InstrumentID, data::CInvestorMargin*> m_instIndex;
	};

	struct FeeStore {
		FeeStore() {
			m_fees.reserve(MaxInstrumentCount);
		}
		void InitAdd(data::CInvestorFee const& fee) {
			auto& value = m_fees.emplace_back(fee);
			m_instIndex.emplace(fee.InstrumentID, &value);
		}
		data::CInvestorFee* Get(type::InstrumentID const& inst) {
			auto iter = m_instIndex.find(inst);
			if (iter == m_instIndex.end()) {
				return nullptr;
			}
			return iter->second;
		}
		int ForEach(type::InstrumentID const& inst, ForEachFee func) {
			data::CInvestorFee const* pre = nullptr;
			int count = 0;
			for (auto const& fee : m_fees) {
				if (inst == "*" || inst == fee.InstrumentID) {
					if (pre != nullptr) {
						func(*pre, false);
						++count;
					}
					pre = &fee;
				}
			}
			if (pre != nullptr) {
				func(*pre, true);
				++count;
			}
			return count;
		}
		std::vector<data::CInvestorFee> m_fees;
		std::map<type::InstrumentID, data::CInvestorFee*> m_instIndex;
	};

	struct PositionStore {

		PositionStore() {
		}
		struct Index {
			type::InstrumentID InstrumentID;
			type::Direction Direction;
			bool operator<(Index const& other) const {
				if (InstrumentID < other.InstrumentID) {
					return true;
				}
				if (InstrumentID == other.InstrumentID) {
					return Direction < other.Direction;
				}
				return false;
			}
		};
		data::CInvestorPosition* InitAdd(data::CInvestorPosition const& pos) {
			Index index;
			index.InstrumentID = pos.InstrumentID;
			index.Direction = pos.Direction;
			auto& value = m_positions.emplace(index, pos);
			return &value.first->second;
		}
		data::CInvestorPosition* Get(type::InstrumentID const& inst, type::Direction direct) {
			auto iter = m_positions.find(Index{ inst,direct });
			if (iter != m_positions.end()) {
				return &iter->second;
			}
			return nullptr;
		}
		void Remove(type::InstrumentID const& inst, type::Direction direct) {
			auto iter = m_positions.find(Index{ inst,direct });
			m_positions.erase(iter);
		}
		void Update(data::CInvestorPosition const& pos) {
			auto iter = m_positions.find(Index{ pos.InstrumentID, pos.Direction });
			if (iter != m_positions.end()) {
				CopyInvestorPositionEntity(&iter->second, &pos);
			}
		}
		int ForEach(ForEachPosition func) {
			auto size = m_positions.size();
			data::CInvestorPosition const* pre = nullptr;
			int count = 0;
			for (auto const& pos : m_positions) {
				if (pos.second.YdPosition > 0 || pos.second.Position > 0) {
					if (pre != nullptr) {
						func(*pre, false);
						++count;
					}
					pre = &pos.second;
				}
			}
			if (pre != nullptr) {
				func(*pre, true);
				++count;
			}
			return count;
		}
		using Map = std::map<Index, data::CInvestorPosition, std::less<Index>, boost::fast_pool_allocator<std::pair<const Index, data::CInvestorPosition>>>;
		Map m_positions;
	};

	struct OrderStore {
		static constexpr int MaxOrderCountPerInvestor = 3000;
		OrderStore() {
//			m_orders.reserve(MaxOrderCountPerInvestor);
		}
		data::COrder* Add(data::COrder const& order) {
			auto& value = m_orders.emplace_back(order);
			m_localIDIndex.emplace(order.OrderLocalID, &value);
			m_userLocalIDIndex.emplace(order.UserOrderLocalID, &value);
			m_instIndex.emplace(order.InstrumentID, &value);
			return &value;
		}

		void AddSynNoIndex(data::COrder& order) {
			m_sysIDIndex.emplace(order.OrderSysID, &order);
		}

		data::COrder* Get(type::OrderSysID const& orderSysID) {
			auto iter = m_sysIDIndex.find(orderSysID);
			if (iter != m_sysIDIndex.end()) {
				return iter->second;
			}
			return nullptr;
		}

		data::COrder* Get(type::OrderLocalID const& orderLocalID) {
			auto iter = m_localIDIndex.find(orderLocalID);
			if (iter != m_localIDIndex.end()) {
				return iter->second;
			}
			return nullptr;
		}

		data::COrder* Get(type::UserOrderLocalID const& userOrderLocalID) {
			auto iter = m_userLocalIDIndex.find(userOrderLocalID);
			if (iter != m_userLocalIDIndex.end()) {
				return iter->second;
			}
			return nullptr;
		}

		int ForEach(type::InstrumentID const& instID, ForEachOrder func) {
			auto count = 0;
			if (instID == "*") {
				auto size = m_orders.size();
				for (auto const& order : m_orders) {
					++count;
					func(order, count == size);
				}
				return count;
			} else {
				auto [begin, end] = m_instIndex.equal_range(instID);
				data::COrder const* pre = nullptr;
				auto iter = begin;
				while (iter != end) {
					auto next = iter;
					++next;
					func(*iter->second, next == end);
					++count;
					iter = next;
				}
			}
			return count;
		}

		using OrderSysIdIndex = std::map<type::OrderSysID, data::COrder*, std::less<type::OrderSysID>, 
										boost::fast_pool_allocator<std::pair<const type::OrderSysID, data::COrder*>>>;
		using OrderLocalIdIndex = std::map<type::OrderLocalID, data::COrder*, std::less<type::OrderLocalID>,
										boost::fast_pool_allocator<std::pair<const type::OrderLocalID, data::COrder*>>>;
		using UserOrderLocalIdIndex = std::map<type::UserOrderLocalID, data::COrder*, std::less<type::UserOrderLocalID>,
										boost::fast_pool_allocator<std::pair<const type::UserOrderLocalID, data::COrder*>>>;
		using InstrumentIndex = std::multimap<type::InstrumentID, data::COrder*, std::less<type::InstrumentID>,
										boost::fast_pool_allocator<std::pair<const type::InstrumentID, data::COrder*>>>;
		Deque<data::COrder> m_orders;
		OrderSysIdIndex m_sysIDIndex;
		OrderLocalIdIndex m_localIDIndex;
		UserOrderLocalIdIndex m_userLocalIDIndex;
		InstrumentIndex m_instIndex;
	};

	struct TradeStore {
		static constexpr int MaxTradeCount = 6000;
		TradeStore() {
//			m_trades.reserve(MaxTradeCount);
		}
		data::CTrade* Add(data::CTrade const& trade) {
			auto& value = m_trades.emplace_back(trade);
			m_sysIdIndex.emplace(value.OrderSysID, &value);
			m_instIndex.emplace(value.InstrumentID, &value);
			return &value;
		}

		int ForEach(type::InstrumentID const& instID, ForEachTrade func) {
			auto count = 0;
			if (instID == "*") {
				auto size = m_trades.size();
				for (auto const& trade : m_trades) {
					++count;
					func(trade, count == size);
				}
			} else {
				auto [begin, end] = m_instIndex.equal_range(instID);
				auto iter = begin;
				while (iter != end) {
					auto next = iter;
					++next;
					func(*iter->second, next == end);
					++count;
					iter = next;
				}
			}
			return count;
		}

		int ForEach(type::OrderSysID const& sysId, ForEachTrade func) {
			int count = 0;
			if (sysId != "*") {
				auto [begin, end] = m_sysIdIndex.equal_range(sysId);
				auto iter = begin;
				while (iter != end) {
					auto next = iter;
					++next;
					func(*iter->second, next == end);
					++count;
					iter = next;
				}
			}
			return count;
		}

		using OrderSysIDIndex = std::multimap<type::OrderSysID, data::CTrade*, std::less<type::OrderSysID>,
									boost::fast_pool_allocator<std::pair<const type::OrderSysID, data::CTrade*>>>;
		using InstrumentIndex = std::multimap<type::InstrumentID, data::CTrade*, std::less<type::InstrumentID>,
			boost::fast_pool_allocator<std::pair<const type::InstrumentID, data::CTrade*>>>;

		Deque<data::CTrade> m_trades;
		OrderSysIDIndex m_sysIdIndex;
		InstrumentIndex m_instIndex;

	};


	struct BalanceChangeStore {
		data::CBalanceChange* Add(data::CBalanceChange const& value) {
			auto& result = m_balanceChanges.emplace_back(value);
			m_seqNoIndex.emplace(value.ExchangeSequenceNo, &result);
			return &result;
		}

		int ForEach(type::SequenceNo const& seqNo, ForEachBalanceChange func) {
			auto count = 0;
			auto [begin, end] = m_seqNoIndex.equal_range(seqNo);
			auto iter = begin;
			while (iter != end) {
				auto next = iter;
				++next;
				func(*iter->second, next == end);
				++count;
				iter = next;
			}
			return count;
		}

		int ForEach(ForEachBalanceChange func) {
			auto count = 0;
			auto iter = m_balanceChanges.begin();
			auto end = m_balanceChanges.end();
			while (iter != end) {
				auto next = iter;
				++next;
				func(*iter, next == end);
				++count;
				iter = next;
			}
			return count;
		}
		using SeqNoIndex = std::multimap<type::SequenceNo, data::CBalanceChange*, std::less<type::SequenceNo>,
			boost::fast_pool_allocator<std::pair<const type::SequenceNo, data::CBalanceChange*>>>;

		Deque<data::CBalanceChange> m_balanceChanges;
		SeqNoIndex m_seqNoIndex;
	};

	struct PositionChangeStore {
		data::CPositionChange* Add(data::CPositionChange const& value) {
			auto& result = m_positionChanges.emplace_back(value);
			m_seqNoIndex.emplace(value.ExchangeSequenceNo, &result);
			return &result;
		}

		int ForEach(type::SequenceNo const& seqNo, ForEachPositionChange func) {
			auto count = 0;
			auto [begin, end] = m_seqNoIndex.equal_range(seqNo);
			auto iter = begin;
			while (iter != end) {
				auto next = iter;
				++next;
				func(*iter->second, next == end);
				++count;
				iter = next;
			}
			return count;
		}

		int ForEach(ForEachPositionChange func) {
			auto count = 0;
			auto iter = m_positionChanges.begin();
			auto end = m_positionChanges.end();
			while (iter != end) {
				auto next = iter;
				++next;
				func(*iter, next == end);
				++count;
				iter = next;
			}
			return count;
		}
		using SeqNoIndex = std::multimap<type::SequenceNo, data::CPositionChange*, std::less<type::SequenceNo>,
			boost::fast_pool_allocator<std::pair<const type::SequenceNo, data::CPositionChange*>>>;

		Deque<data::CPositionChange> m_positionChanges;
		SeqNoIndex m_seqNoIndex;

	};

	struct InvestorStore {
		data::CInvestorInfo Info;
		data::CInvestorAccount Account;
		MarginStore Margins;
		FeeStore Fees;
		PositionStore Positions;
		OrderStore Orders;
		TradeStore Trades;
		BalanceChangeStore BalanceChanges;
		PositionChangeStore PositionChanges;
	};
	struct BrokerStore {
		MarginStore Margins;
		FeeStore Fees;
	};
	std::map<type::InvestorID, InvestorStore> m_investors;
	std::map<type::BrokerID, BrokerStore> m_brokers;
};