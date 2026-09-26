//Basic Input Output System
#pragma once

#include "model/Constants.h"
#include "model/Instrument.h"
#include <map>
struct OrderInsertRequest;
class OrderCancelRequest;
struct MarketingOrderInfo;
struct MarketingOrderCancel;

class RequestIdACC {
public:
	static int Generate() {
		int& base = get();
		return base++;
	}
	static void Init(int base) {
		int& b = get();
		b = base;
	}
private:
	static int& get() {
		static int s_base = 1;
		return s_base;
	}
};


class ExchangeConn
{
public:
	ExchangeConn(TradePlatform api) :m_api(api) {
	}
	virtual bool IsInited() {	return m_isInited;	}
	virtual void Init() = 0;
 	virtual void Release()  = 0;
	virtual void Login(type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
		type::InvestorID const& quoteInvestor, type::Password const& quotePassword) = 0;
	virtual void ReLogin(SubSystem subSystem) = 0;
	virtual void SetVertificateCode(char const* code) = 0;

	virtual void ChangePassword(type::Password const& oldPassword, type::Password const& newPassword) = 0;

	virtual void InsertOrder(OrderInsertRequest const& order) = 0;
	virtual void CancelOrder(OrderCancelRequest const& action) = 0;

	virtual void SubscribeQuote(type::InstrumentCode const& instCode) = 0;
	virtual void UnSubscribeQuote(type::InstrumentCode const& instCode) = 0;

	virtual void QueryOrders() = 0;
	virtual void QueryTrades() = 0;
	virtual void QueryPositions() = 0;
	virtual void QueryFees() = 0;
	virtual void QueryMargins() = 0;
	virtual void QueryInstruments() = 0;
	virtual void QueryAccount() = 0;

	virtual void QueryFee(type::InstrumentCode const& inst) = 0;
	virtual void QueryMargin(type::InstrumentCode const& inst) = 0;
	virtual void QueryInstrument(type::InstrumentCode const& inst) = 0;
public:
	void LogOrderInsert(int rtn, OrderInsertRequest const& order, int requestId);
	void LogOrderCancel(int rtn, OrderCancelRequest const& orderAction, int requestId);

protected:
	TradePlatform m_api;
	bool m_isInited = false;
};

class ExchangeConnInstrumentHelper {
public:
	ExchangeConnInstrumentHelper(Instruments const& instruments)
		:m_instruments(instruments) {
		initInstrumentMap();
	}
	type::InstID GetInstID(type::InstrumentCode const& instCode) const {
		auto iterator = m_instrumentMap.find(instCode);
		if (iterator == m_instrumentMap.end()) {
			return INVALID_INSTRUMENT_ID;
		}
		return iterator->second;
	}

	Instrument const& GetInstrument(type::InstID instID) const {
		return m_instruments.Get(instID);
	}

	Instrument const* FindInstrument(type::InstrumentCode const& instCode) const {
		type::InstID instID = GetInstID(instCode);
		if (INVALID_INSTRUMENT_ID == instID) {
			return nullptr;
		}
		return &m_instruments.Get(instID);
	}

	int GetMultiplier(type::InstID instID) const {
		return m_instruments.Get(instID).VolumeMultiple;
	}
private:
	Instruments const& m_instruments;
	std::map<type::InstrumentCode, type::InstID> m_instrumentMap;
	void initInstrumentMap() {
		for(Instrument const& inst : m_instruments.GetAll()) {
			m_instrumentMap[inst.Code] = inst.InstID;
		}
	}
};

char const* GetExchangeDescribe(type::Exchange flag);