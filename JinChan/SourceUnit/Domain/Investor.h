#pragma once

#include <optional>
#include "EntityCopy.h"
#include "ErrorDefine.h"
#include "RateAlgorithms.h"
#include "Repository.h"
#include "Order.h"
#include <array>

template<class T>
struct Result {
	// default is error
	Result():Error(SystemError){}
	// error result, no value
	Result(Error error) : Error(error) {
		if (error == NoError){
			throw std::logic_error("");
		}
	}
	// success result
	Result(T const& t) : Error(NoError), Value(t) {}

	T* operator->() {
		if (!IsSuccess()) {
			throw std::logic_error("");
		}
		return Value.operator->();
	}

	inline bool IsSuccess() const {
		return Error.IsSuccess();
	}

	operator bool() const {
		return IsSuccess();
	}

	Error Error;
	std::optional<T> Value;
};

using OrderResult = Result<Order>;



using InvestorResult = Result<Investor>;

struct UnilateralMargin {
	type::Direction Direction;
	type::Money UsedMargin;
};

struct UnilateralMarginChange {
	UnilateralMargin Existed;
	UnilateralMargin New;

	bool IsLessEqual() const {
		return New.UsedMargin <= Existed.UsedMargin;
	}

	bool IsGreatEqual() const {
		return New.UsedMargin >= Existed.UsedMargin;
	}

	bool IsDirectionChanged() const {
		return Existed.Direction != New.Direction;
	}

	bool IsSameDirection() const {
		return !IsDirectionChanged();
	}

	bool IsChanged() const {
		return New.UsedMargin != Existed.UsedMargin || IsDirectionChanged();
	}

	double ChangedMargin() const {
		return fabs(New.UsedMargin - Existed.UsedMargin);
	}

	double ExistedMargin() const {
		return Existed.UsedMargin;
	}

	double NewMargin() const {
		return New.UsedMargin;
	}
};

#undef max
class UnilateralPosition {
	const int MAX_INST_SIZE = 5;
public:
	UnilateralMargin CalcNewUsedMargin() const {
		double longMargin = 0.0;
		double shortMargin = 0.0;
		for (auto* pos : m_buyPositions) {
			if (pos != nullptr) {
				longMargin += pos->UsedMargin;
			}
		}
		for (auto* pos : m_sellPositions) {
			if (pos != nullptr) {
				shortMargin += pos->UsedMargin;
			}
		}
		if (longMargin >= shortMargin) {
			return { type::Direction::Buy, longMargin };
		} else {
			return { type::Direction::Sell, shortMargin };
		}
	}

	UnilateralMargin GetUsedMargin() const {
		return m_usedMargin;
	}
	void SetUsedMargin(UnilateralMargin usedMargin) {
		m_usedMargin = usedMargin;
	}

	data::CInvestorPosition* GetSubPosition(type::Direction direction, int index) const {
		if (index < 0 || index >= MAX_INST_SIZE) {
			return nullptr;
		}
		if (type::IsBuy(direction)) {
			return m_buyPositions[index];
		} else {
			return m_sellPositions[index];
		}

	}
	void SetSubPosition(type::Direction direction, int index, data::CInvestorPosition* pPosition) {
		if (index < 0 || index >= MAX_INST_SIZE) {
			return;
		}
		if (type::IsBuy(direction)) {
			m_buyPositions[index] = pPosition;
		} else {
			m_sellPositions[index] = pPosition;
		}
	}
private:
	std::array<data::CInvestorPosition*, 5> m_buyPositions = { nullptr,nullptr, nullptr,nullptr,nullptr };
	std::array<data::CInvestorPosition*, 5> m_sellPositions = { nullptr,nullptr, nullptr,nullptr,nullptr };
	UnilateralMargin m_usedMargin = { type::Direction::Buy,0.0 };
};

class InvestorUnilateralPositionStore {
	const int MAX_SIZE = 5;
public:
	InvestorUnilateralPositionStore() {

	}
	UnilateralPosition* Set(data::CInstrument const& inst, type::Direction direction, data::CInvestorPosition* position) {
		if (inst.UnilateralGroup < 0 || inst.UnilateralGroup >= MAX_SIZE) {
			return nullptr;
		}
		m_unilateralPositions[inst.UnilateralGroup].SetSubPosition(direction, inst.UnilateralSubIndex, position);
		return &m_unilateralPositions[inst.UnilateralGroup];
	}
	data::CInvestorPosition* Get(data::CInstrument const& inst, type::Direction direction) const {
		if (inst.UnilateralGroup < 0 || inst.UnilateralGroup >= MAX_SIZE) {
			return nullptr;
		}
		return m_unilateralPositions[inst.UnilateralGroup].GetSubPosition(direction, inst.UnilateralSubIndex);
	}
	UnilateralPosition* Get(data::CInstrument const& inst) {
		if (inst.UnilateralGroup < 0 || inst.UnilateralGroup >= MAX_SIZE) {
			return nullptr;
		}
		return &m_unilateralPositions[inst.UnilateralGroup];
	}

	UnilateralPosition const* Get(int unilateralGroup) const {
		if (unilateralGroup < 0 || unilateralGroup >= MAX_SIZE) {
			return nullptr;
		}
		return &m_unilateralPositions[unilateralGroup];
	}

	double LongTotal() const {
		double total = 0.0;
		for (auto const& g : m_unilateralPositions) {
			auto margin = g.GetUsedMargin();
			if (type::IsBuy(margin)) {
				total += margin.UsedMargin.value();
			}
		}
		return total;
	}

	double ShortTotal() const {
		double total = 0.0;
		for (auto const& g : m_unilateralPositions) {
			auto margin = g.GetUsedMargin();
			if (type::IsSell(margin)) {
				total += margin.UsedMargin.value();
			}
		}
		return total;
	}
private:
	std::array<UnilateralPosition, 5> m_unilateralPositions;
};


class Investor {
	friend class Investors;
	Investor(InvestorRepository& investorRepo, InstrumentRepository& instRepo, data::CInvestorAccount* account, data::CInvestorInfo* pInfo) :
		m_investorRepo(investorRepo), m_instRepo(instRepo), m_account(account), m_info(pInfo), m_unilateralPositionStore(new InvestorUnilateralPositionStore) {
	}

public:

	static InvestorResult Get(InvestorRepository& investorRepo, InstrumentRepository& instRepo, type::BrokerID const& brokerID, type::InvestorID const& investorID) {
		data::CInvestorAccount* pInvestor = investorRepo.GetAccount(investorID);
		data::CInvestorInfo* pInfo = investorRepo.GetInfo(investorID);
		if (pInvestor == nullptr || pInfo == nullptr) {
			return {InvalidInvestorError};
		}
		return Investor(investorRepo, instRepo, (data::CInvestorAccount*)pInvestor, pInfo);
	}

	void ReplayChanges(type::SequenceNo seqNo) {
		m_investorRepo.BalanceChangeForEach(m_account->InvestorID,seqNo, [&](data::CBalanceChange const& change, bool isLast) {
			this->OnBalanceChange(change);
		});

		m_investorRepo.PositionChangeForEach(m_account->InvestorID, seqNo, [&](data::CPositionChange const& change, bool isLast) {
			this->OnPosition(change);
		});
	}

	bool CheckPassword(std::string const& password) {
		return m_info->Password == password.c_str() ;

	}

	void OnBalanceChange(data::CBalanceChange const& balanceChange) {
		if (balanceChange.BalanceChangeType == type::BalanceChangeFlag::Deposit) {
			Deposit(balanceChange);
		} else if (balanceChange.BalanceChangeType == type::BalanceChangeFlag::Withdraw) {
			Withdraw(balanceChange);
		}
	}

	data::CInvestorPosition* OnPosition(data::CPositionChange const& positionChange);

	template<class T>
	data::CInvestorPosition* createEmptyPosition(const T& positionChange);

	type::ClientID const& GetClientID() const {
		return m_info->ClientID;
	}

	inline Order from(data::COrder const* pOrder) {
		return Order(*this, (data::COrder*)pOrder, pOrder->getInstrument());
	}
	OrderResult GetOrNewOrder(data::COrder const& inputOrder, bool& isNew) {
		isNew = false;
		OrderResult result;
		if (inputOrder.OrderSysID == "") {
			result = FindOrder(inputOrder.OrderLocalID);

		} else {
			result = FindOrder(inputOrder.OrderSysID);
		}
		if (!result.IsSuccess()) {
			result = CreateOrder(inputOrder,false);
			isNew = true;
		}
		return result;
	}

	std::pair<Error,data::COrder*> SaveOrder(data::COrder& order) {
		auto const* pOrder = m_investorRepo.Add(order);
		if (pOrder == nullptr) {
			return { SystemError, nullptr };
		}
		return { NoError, (data::COrder*)pOrder};
	}


	OrderResult CreateOrder(data::COrder const& inputOrder, bool checkUserLocalId = true);

	Trade CreateTrade(Order& order, data::CTrade const& inputTrade);
	Result<Trade> CreateTrade(data::CTrade const& inputTrade);

	OrderResult FindOrder(type::OrderSysID const& orderSysID);
	OrderResult FindOrder(type::UserOrderLocalID const& localID);
	OrderResult FindOrder(type::OrderLocalID const& localID);

	inline bool HasAvailable(Order const& order) {
		if (order.IsOffset()) {
			return true;
		}
		double margin = order.GetFreezeMargin();
		double fee = order.GetFreezeFee();
//		double margina = m_unilateralPositionStore.Get(order.GetInstrument())->GetFrozenMargin();
		return GetAvailable() > margin + fee;
	}

	//会保障无副作用
	Error Freeze(Order& order);
	void Unfreeze(Order const& order);
	void AddSysIDIndex(Order const& order);
	void UpdateOrder(Order const& order);
	void UpdateTrade(Trade const& trade);


	void OnTrade(Trade const& trade, double margin, double fee);
	void UnfreezeOrderMoney(Order const& order);
	double GetAvailable() const {
		return m_account->Available;
	}
	static void UpdateBalanceAndAvailable(data::CInvestorAccount* pInvestorAccount);

	data::CInvestorAccount const& GetAccount() const {
		return *m_account;
	}
	bool InitUnilateralPosition(data::CInstrument& inst, data::CInvestorPosition* pPos) {
		if (auto* uPos = m_unilateralPositionStore->Set(inst, pPos->Direction, pPos); uPos != nullptr) {
			uPos->SetUsedMargin(uPos->CalcNewUsedMargin());
			return true;
		}
		return false;
	}

	UnilateralMargin GetUnilateralPositionUsedMargin(int unilateralGroup) const {
		auto* uPos = m_unilateralPositionStore->Get(unilateralGroup);
		if (uPos == nullptr) {
			return {type::Direction::Buy,0.0};
		}
		return uPos->GetUsedMargin();
	}
	/*
	需要根据持仓重新计算LongMargin和ShortMargin并正确初始化Margin，因为初始化表格中没有LongMargin和ShortMargin
	如果不做初始化，保证金会不正确
	*/
	void InitAccountMarginByPositions();
private:
	void Deposit(data::CBalanceChange const& balanceChange) {
		m_account->Deposit += balanceChange.Amount;
		UpdateBalanceAndAvailable(m_account);
	}

	void Withdraw(data::CBalanceChange const& balanceChange) {
		m_account->Withdraw += balanceChange.Amount;
		UpdateBalanceAndAvailable(m_account);
	}
	bool FreezeMoney(Order const& order);

	inline int GetAvailablePosition(Order const& order) const {
		return order.GetInvestorPosition()->YdPosition +order.GetInvestorPosition()->Position - order.GetInvestorPosition()->FrozenPosition;
	}
	bool FreezePosition(Order const& order);
	void onOffsetTrade(Trade const& trade, double margin, double fee);

	std::tuple<double, double> GetFreeMargin(Trade const& trade, int YdTradeMatchPosition, int todayMatchPosition);

	void recalcMarginAndFee(const Trade& trade, double unfreezeMargin, double unfreezeFee, double freeMargin);
	void recalcMarginAndFee(const Trade& trade, double unfreezeMargin, double unfreezeFee, UnilateralMarginChange marginChange);
	void recalcFrozenMargin();
	void reduceMargin(const Trade& trade, double unfreezeMargin, double margin = 0);
	void recalcMargin();
	void increaseMargin(const Trade& trade, double unfreezeMargin, double margin = 0);
	void onOpenTrade(Trade const& trade, double unfreezeMargin, double unfreezeFee);

	int recalcTodayPositionAvgCost(data::CInvestorPosition& pos, int todayVolume, double todayPrice);

	void UnfreezeOrderPosition(Order const& order);

	OrderResult getOrder(type::OrderSysID const& orderSysId);
	Error isOverVolumeLimit(const data::COrder& inputOrder, data::CInstrument* pInst);
	Error link(const data::COrder& inputOrder, data::COrder& memOrder);
	Error linkMarginAndFee(const data::COrder& inputOrder, data::COrder& memOrder);
	Error linkPosition(const data::COrder& inputOrder, data::COrder& memOrder);
private:
	InvestorRepository& m_investorRepo;
	InstrumentRepository& m_instRepo;
	data::CInvestorAccount* m_account;
	data::CInvestorInfo* m_info;
	CAlgorithms m_baseAlgorithm;
	std::shared_ptr<InvestorUnilateralPositionStore> m_unilateralPositionStore;

};

class Investors {
public:
	static Result<Investor> GetInvestor(InvestorRepository& investorRepo, InstrumentRepository& instRepo, type::BrokerID const& BrokerID, type::InvestorID const& InvestorID) {
		if (auto iter = m_investors.find(InvestorID.value()); iter != m_investors.end()) {
			return iter->second;
		}
		auto result = Investor::Get(investorRepo, instRepo, BrokerID, InvestorID);
		if (result) {
			m_investors.emplace(InvestorID.value(), *result.Value);
		}
		return result;
	}

	static void Clear() {
		m_investors.clear();
	}
private:
	inline static std::map<std::string, Investor> m_investors;
};

