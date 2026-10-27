#pragma once
#include "model/Constants.h"
#include "SGE/SGEDataType.h"

LocalOrderID const SGE_LOCALID = 521000000;			// SGE起始委托号(007421)
// LocalOrderID const SGE_LOCALID = 522000000;			// SGE起始委托号(007422)
LocalOrderID const GRAB_PREFIX = 512823810;			// 抢单起始委托号
LocalOrderID const MAX_LOCALID = 2147483647;		// 本地最大委托号：int max	UInt max：4294967295

class LocalIdACC {
public:
	inline static LocalOrderID NextNormal() {
		LocalOrderID& base = GetBase();
		base += 1;
		return base * 10;
	}

	inline static LocalOrderID NextBuyMarketing() {
		LocalOrderID& base = GetBase();
		base += 1;
		return base * 10 + 1;
	}
	inline static LocalOrderID BuyMarketingID(LocalOrderID sellLocalID) {
		return sellLocalID - 1;
	}
	inline static LocalOrderID SellMarketingID(LocalOrderID buyLocalID) {
		return buyLocalID + 1;
	}

	inline static LocalOrderID PairID(LocalOrderID localId) {
		return IsMarketingSell(localId) ? LocalIdACC::BuyMarketingID(localId) : LocalIdACC::SellMarketingID(localId);
	}

	static void Init(LocalOrderID base) {
		LocalOrderID& b = GetBase();
		b = base / 10;
	}
	static void InitGrab() {
		LocalOrderID& b = GetBase();
		if (b >= (GRAB_PREFIX / 10)) {
			return;
		}
		b = GRAB_PREFIX / 10;
	}

	inline static bool IsMarketing(LocalOrderID localId) {
		return localId % 10 > 0;
	}
	inline static bool IsMarketingSell(LocalOrderID localId) {
		return localId % 10 == 2;
	}
	static LocalOrderID GetCurrentID() { return GetBase() * 10; }
private:
	static LocalOrderID& GetBase() {
		static LocalOrderID s_base = 100;
		return s_base;
	}
};

struct OrderInfo {
	Exchange Exchange = Exchange::HKEX;
	InstID InstID = INVALID_INSTRUMENT_ID;
	Direction Direction = Direction::BUY;
	Operation Operation = Operation::UNKNOWN;
	OrderType OrderType = OrderType::NORMAL;
	InstCodeT InstCode;
	LocalOrderID LocalId = INVALID_LOCAL_ID;
	int Quantity = 0;
	money Price = 0.0;
	char ProductClass = SGE_OC_Unknown;
};

class OrderCancel {
public:
	OrderCancel(Exchange exchange, SystemNoT const& systemNo, InstCodeT const& instCode, InstID instID, int localId, int actionId, char productClass = SGE_OC_Unknown)
		:m_exchage(exchange), m_instID(instID), m_localId(localId), m_actionId(actionId), m_systemNo(systemNo), m_instCode(instCode), m_productClass(productClass) {
	}

	Exchange GetExchange() const { return m_exchage; }
	InstID GetInstID() const { return m_instID; }
	LocalOrderID GetLocalId() const { return m_localId; }
	LocalOrderID GetActionId() const { return m_actionId; }
	SystemNoT const& GetSystemNo() const { return m_systemNo; }
	InstCodeT const& GetInstCode() const { return m_instCode; }
	TSGEProductClassType GetProductClass() const { return m_productClass; }
private:
	Exchange m_exchage;
	SystemNoT m_systemNo;
	InstCodeT m_instCode;
	InstID m_instID;
	LocalOrderID m_localId;
	LocalOrderID m_actionId;
	char m_productClass = SGE_OC_Unknown;
};

struct MarketingOrderInfo {
	Exchange Exchange;
	InstID InstID;
	InstCodeT InstCode;
	LocalOrderID BuyLocalId;
	LocalOrderID SellLocalId;
	int Quantity;
	money BuyPrice;
	money SellPrice;
	inline OrderInfo GetBuyOrder() const {
		return OrderInfo{ Exchange, InstID, Direction::BUY, Operation::UNKNOWN, OrderType::NORMAL, InstCode, BuyLocalId, Quantity, BuyPrice };
	}
	inline OrderInfo GetSellOrder() const {
		return OrderInfo{ Exchange, InstID, Direction::SELL, Operation::UNKNOWN, OrderType::NORMAL, InstCode, SellLocalId, Quantity, SellPrice };
	}
};

struct MarketingOrderCancel {
	Exchange Exchange;
	InstID InstID;
	LocalOrderID BuyLocalId;
	LocalOrderID SellLocalId;
	SystemNoT BuySystemNo;
	SystemNoT SellSystemNo;
};
