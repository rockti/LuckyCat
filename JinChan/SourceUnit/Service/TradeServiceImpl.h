#pragma once
#include "IFlow.h"
#include "ErrorEngine.h"
#include "Investor.h"
#include <utility>

class CZTPPackage;

//CTradeServiceImpl是一个完成交易功能的实例
class CTradeServiceImpl {
public:
	CTradeServiceImpl(SystemStatusRepository& systemRepo,InvestorRepository& investorRepo, InstrumentRepository& instRepo, IFlow* pTradeResultFlow);

	OrderResult ReqOrderInsert(Investor& investor, data::COrder& inputOrder);
	OrderResult ReqOrderAction(Investor& investor, data::COrderAction& orderAction);
	//注意，这里要把session带入委托，防止应答的时候找不到Session
	OrderResult RspOrderInsert(data::COrder& inputOrder, data::CRspInfo const& rspInfo);
	OrderResult RspOrderAction(data::COrderAction& orderAction, data::CRspInfo const& rspInfo);
	OrderResult OnRtnOrder(data::COrder& order);
	Result<Trade> OnRtnTrade(data::CTrade& trade);

	bool NotifyInstrumentStatus(data::CInstrumentStatus* pInstrumentStatus);

	void DynamicProfitCalculate();

	Result<Investor> GetInvestor(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID);

private:
	SystemStatusRepository& m_systemRepo;
	InvestorRepository& m_investorRepo;
	InstrumentRepository& m_instRepo;
	CAlgorithms m_algorithm;				//费率算法
};