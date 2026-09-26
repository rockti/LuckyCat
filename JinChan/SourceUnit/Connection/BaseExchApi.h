//Interface for the CBaseApi class.
#pragma once

#include "ZTPType.h"
#include "FlowReader.h"
#include "SeriesCachedFlow.h"
#include "EntityCopy.h"
#include "Logger.h"
#include <fmt/format.h>
#include <map>
using namespace ztp;

//交易核心对外连接标志
struct LinkApi {
	enum ID {NONE=-1, MARKET=0, FRONT, SGE, CTP, FEMAS, SGEMOCK};
};


class RequestIdACC {
public:
	static int Generate() {
		static int s_base = 1;
		return s_base++;
	}
};

//序列流读取对象
class CSeriesReader {
public:
	virtual bool GetNextUseNoBuf(CPackage* pPackage) { 
		return m_seriesReader.GetNextUseNoBuf(pPackage); 
	}
protected:
	CSeriesCachedFlow* m_pSeriesFlow = nullptr;
	CFlowReader m_seriesReader;
};

class CPackage;
class CBaseExchApi : public CSeriesReader
{
public:
	CBaseExchApi()
		:m_publicSeqNo(0),m_tradeSeqNo(0),m_isSeatInited(false),m_isConnected(false),m_isLogined(false){
		memset(&m_seatInfo, 0, sizeof(data::CSeatInfo));
	}
	virtual ~CBaseExchApi(){};

	virtual void InitSeatInfo(data::CSeatInfo const& seatField) {
		if (!m_isSeatInited) {
			CopySeatInfoEntity(&m_seatInfo, &seatField);
			m_isSeatInited = true;
		}
	}
	void AddClientInfo(type::ClientID const& clientID, type::InvestorID const& investorID) {
		m_clientInvestorMap.emplace(clientID, investorID);
		Log::Info(fmt::format("Add Client Info: [{}] [{}]", clientID, investorID));
	}

	void SetTradingDay(type::TradingDay const& tradingDay) { m_tradingDay = tradingDay; }

	virtual bool IsSeatInited() { return m_isSeatInited; }
	virtual bool IsConnected()	{ return m_isConnected; }
	virtual bool IsLogined()	{ return m_isLogined; }

	virtual void StartWork() {};
	virtual void InitInstrumentSet(type::InstrumentID instrumentID, data::CInstrument* pContract) {};
	//只有委托和撤单需要发给交易所，其他一切查询类全部由本系统应答
	virtual void Login() { }
	virtual bool ReqOrderInsert(data::COrder& OrderField)	{ return false; }
	virtual bool ReqOrderAction(data::COrderAction& ActionField)	{ return false; }

	data::CSeatInfo const& GetSeatInfo() const { return m_seatInfo; }

	void SetSequenceNo(int no) { m_tradeSeqNo = no; }

// 	void LogOrderInsert(int rtn, COrderField const& inputField) {
// 		char const* direction = (ZTP_D_Buy==inputField.Direction) ? "B" : "S";
// 		char const* operation = (ZTP_OF_Open==inputField.OffsetFlag) ? "OPEN" : "OFFSET";
// 		Log::Info(fmt::format("OrderInsert: rtn[{}] SeatID[{}] ClientID[{}] LocalID[{}] InstrumentID[{}] Flag[{}|{}] Price[{:.2f},{}] Condition[{}]",
// 			rtn, inputField.SeatID.getValue(), inputField.ClientID.getValue(), inputField.OrderLocalID.getValue(), inputField.InstrumentID.getValue(),
// 			direction, operation, inputField.LimitPrice.getValue(), inputField.Volume.getValue(), inputField.OrderPriceType.getValue()));
// 	}
// 	void LogOrderAction(int rtn, COrderActionField const& actionField) {
// 		Log::Info(fmt::format("OrderAction: rtn[{}] SeatID[{}] ClientID[{}] InstrumentID[{}] LocalID[{}] ActionID[{}] SystemID[{}]",
// 			rtn, actionField.SeatID.getValue(), actionField.ClientID.getValue(), actionField.InstrumentID.getValue(),
// 			actionField.OrderLocalID.getValue(), actionField.ActionLocalID.getValue(), actionField.OrderSysID.getValue()));
// 	}

protected:
	int m_publicSeqNo;		//公有流号（子类）
	int m_tradeSeqNo = -1;		//私有流号（子类）
	bool m_isSeatInited = false;	//席位信息是否初始化
	bool m_isConnected = false;		//接口是否连接（子类）
	bool m_isLogined = false;		//接口是否登录（子类）
	data::CSeatInfo m_seatInfo;
	std::map<type::ClientID, type::InvestorID > m_clientInvestorMap;
	type::TradingDay m_tradingDay; // only for mock
};
