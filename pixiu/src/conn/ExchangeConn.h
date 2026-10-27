//Basic Input Output System
#pragma once

#include "model/Constants.h"
#include "model/Instrument.h"

struct OrderInfo;
class OrderCancel;
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


class ExchangeConn {
public:
	ExchangeConn(ConnPlatform api) :m_api(api) {
	}
	
	virtual void Init() = 0;
 	virtual void Release()  = 0;
	virtual void Login(char const* password) = 0;	
	virtual void SetVertificateCode(char const* code) = 0;

	virtual void ResetOrderSet() = 0;
	virtual void InsertOrder(OrderInfo const& order) = 0;
	virtual void InsertExOrder(OrderInfo const& order) = 0;		// 专为金交所多连接发单增加接口，只发FAK单，不另做撤单接口 20250101
	virtual int InsertMultiOrders(OrderInfo const* orders[]/*委托参数不可变*/) = 0;	// 批量报单接口，最多16笔	20250920
	virtual void CancelOrder(OrderCancel const& action) = 0;

	virtual void InsertMarketingOrder(MarketingOrderInfo const& order) = 0;
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel) = 0;
	virtual void SubscribeQuote(char const* instrumentCode) = 0;
	virtual void UnSubscribeQuote(char const* instrumentCode) = 0;
	virtual void QueryMblMarket(char const* instrumentCode) = 0;
	virtual void QueryFund() = 0;
	virtual void QueryOrder() = 0;
	virtual void QueryTrade() = 0;
	virtual void QueryPosition() = 0;
public:
	void LogOrderInsert(int rtn, OrderInfo const& order, int requestId);
	void LogOrderCancel(int rtn, OrderCancel const& orderAction, int requestId);

private:
	ConnPlatform m_api;

};

class ExchangeConnInstrumentHelper {
public:
	ExchangeConnInstrumentHelper(Instruments const& instruments)
		:m_instruments(instruments) {
		initInstrumentMap();
	}
	InstID GetInstID(std::string const& instCode) const {
		auto iterator = m_instrumentMap.find(instCode);
		if (iterator == m_instrumentMap.end()) {
			return INVALID_INSTRUMENT_ID;
		}
		return iterator->second;
	}

	Instrument const& GetInstrument(InstID instID) const {
		return m_instruments.Get(instID);
	}

	Instrument const* FindInstrument(std::string const& instCode) const {
		InstID instID = GetInstID(instCode);
		if (INVALID_INSTRUMENT_ID == instID) {
			return nullptr;
		}
		return &m_instruments.Get(instID);
	}

	int GetMultiplier(InstID instID) const {
		return m_instruments.Get(instID).GetMultiplier();
	}
private:
	Instruments const& m_instruments;
	std::map<std::string, InstID> m_instrumentMap;
	void initInstrumentMap() {
		for(Instrument const& inst : m_instruments.GetAll()) {
			m_instrumentMap[inst.GetInstCode().Str()] = inst.GetID();
		}
	}
};

std::pair<std::string, int> SeparatePort(std::string const& hostUrl);