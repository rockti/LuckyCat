#include "stdafx.h"
#include "QueryServiceLink.h"
#include "ErrorDefine.h"
#include "Investor.h"

template<class Field>
void CQueryServiceLink::PushRspToResultFlow(DWORD tID, CZTPPackage& package, Field& field, Error error, bool isLast) {
	char chainFlag = isLast ? CHAIN_LAST : CHAIN_CONTINUE;
	m_rspPackage.PreparePublish(tID, chainFlag);
	m_rspPackage.GetZTPHeader()->SessionID = package.GetSessionID();
	m_rspPackage.GetZTPHeader()->RequestID = package.GetRequestID();
	data::CRspInfo rspField{ error.Code, type::ErrorMsg(error.Msg) };
	m_rspPackage.AddField(rspField);
	if (error == NoError) {
		m_rspPackage.AddField(field);
	}
	m_rspPackage.MakePackage();
	m_pQueryResultFlow->Append(m_rspPackage.GetAddress(), m_rspPackage.GetLength());
}

void CQueryServiceLink::PushEmptySuccessRspToResultFlow(DWORD tID, CZTPPackage& package) {
	m_rspPackage.PreparePublish(tID, CHAIN_LAST);
	m_rspPackage.GetZTPHeader()->SessionID = package.GetSessionID();
	m_rspPackage.GetZTPHeader()->RequestID = package.GetRequestID();
	data::CRspInfo rspField{ NoError.Code, type::ErrorMsg(NoError.Msg) };
	m_rspPackage.AddField(rspField);
	m_rspPackage.MakePackage();
	m_pQueryResultFlow->Append(m_rspPackage.GetAddress(), m_rspPackage.GetLength());
}

CQueryServiceLink::CQueryServiceLink(SessionRepository& sessionRepo, InvestorRepository& investorRepo, 
	InstrumentRepository& instRepo, SystemStatusRepository& systemRepo,IFlow* pTradeResultFlow)
:SessionCheckService(sessionRepo),m_investorRepo(investorRepo),
m_instRepo(instRepo),m_systemRepo(systemRepo),m_pQueryResultFlow(pTradeResultFlow){
	m_rspPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
}
CQueryServiceLink::~CQueryServiceLink() {
	m_rspPackage.Release();
}

void CQueryServiceLink::HandleMessage(CZTPPackage& package) {
	switch (package.GetTID()) {
	case TID_ReqQryInvestorAccount:				//资金查询
		handelQueryAccount(package);
		break;
	case TID_ReqQryInvestorPosition:			//持仓查询
		handleQueryPosition(package);
		break;
	case TID_ReqQryInstrument:					//合约查询
		handleQueryInstrument(package);
		break;
	case TID_ReqQryInvestorMargin:				//保证金查询
		handleQueryMargin(package);
		break;
	case TID_ReqQryInvestorFee:					//手续费查询
		handleQueryFee(package);
		break;
	case TID_ReqQryInvestorOrder:				//报单查询
		handleQueryOrder(package);
		break;
	case TID_ReqQryInvestorTrade:				//成交查询
		handleQueryTrade(package);
		break;
	default:
		Log::Error(fmt::format("CQueryServiceLink: do not support TID[{}]", package.GetTID()));
		break;
	}
    return;
}

void CQueryServiceLink::handelQueryAccount(CZTPPackage& package) {
	data::CQryInvestorAccount reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInvestorAccount;
	data::CInvestorAccount rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}

	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, reqField.BrokerID, reqField.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("RspOrderInsert:can not find order by InvestorID [{}]", reqField.InvestorID));
		PushRspToResultFlow(tID, package, rspField, investor.Error);
		return;
	}

	data::CInvestorAccount account = investor->GetAccount();
	account.TradingDay = m_systemRepo.Get()->TradingDay;
	CopyInvestorAccountEntity(&rspField, &account);
	m_rspPackage.AddField(rspField);
	PushRspToResultFlow(tID, package, rspField);
}

void CQueryServiceLink::handleQueryPosition(CZTPPackage& package) {
	data::CQryInvestorPosition reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInvestorPosition;
	data::CInvestorPosition rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}

	auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, reqField.BrokerID, reqField.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("RspOrderInsert:can not find order by InvestorID [{}]", reqField.InvestorID));
		PushRspToResultFlow(tID, package, rspField, investor.Error);
		return;
	}


	int count = 0;
	m_investorRepo.PositionForEach(reqField.InvestorID, [&](data::CInvestorPosition const& pos, bool isLast) {
		CopyInvestorPositionEntity(&rspField, &pos);
		m_rspPackage.AddField(rspField);
		PushRspToResultFlow(tID, package, rspField, NoError, isLast);
		count++;
		});
	if (count == 0) {
		PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
	}
}

void CQueryServiceLink::handleQueryInstrument(CZTPPackage& package) {
	data::CQryInstrument reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInstrument;
	data::CInstrument rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}
	int count = 0;
	m_instRepo.ForEach(reqField.ProductClass, reqField.InstrumentID, [&](data::CInstrument const& inst, bool isLast) {
		CopyInstrumentInfoEntity(&rspField, &inst);
		m_rspPackage.AddField(rspField);
		PushRspToResultFlow(tID, package, rspField, NoError, isLast);
		count++;
		});
	if (count == 0) {
		PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
	}
}

void CQueryServiceLink::handleQueryMargin(CZTPPackage& package) {
	data::CQryInvestorMargin reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInvestorMargin;
	data::CInvestorMargin rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}
	int count = 0;
	m_investorRepo.MarginForEach(reqField.InvestorID, reqField.InstrumentID,[&](data::CInvestorMargin const& margin, bool isLast) {
		CopyInvestorMarginEntity(&rspField, &margin);
		m_rspPackage.AddField(rspField);
		PushRspToResultFlow(tID, package, rspField, NoError, isLast);
		count++;
		});
	if (count == 0) {
		PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
	}
}
void CQueryServiceLink::handleQueryFee(CZTPPackage& package) {
	data::CQryInvestorFee reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInvestorFee;
	data::CInvestorFee rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}
	int count = 0;
	m_investorRepo.FeeForEach(reqField.InvestorID, reqField.InstrumentID,[&](data::CInvestorFee const& fee, bool isLast) {
		CopyInvestorFeeEntity(&rspField, &fee);
		m_rspPackage.AddField(rspField);
		PushRspToResultFlow(tID, package, rspField, NoError, isLast);
		count++;
		});
	if (count == 0) {
		PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
	}
}
void CQueryServiceLink::handleQueryOrder(CZTPPackage& package) {
	data::CQryInvestorOrder reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInvestorOrder;
	data::COrder rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}

	if (reqField.OrderSysID != "*") {
		auto* order = m_investorRepo.GetOrder(reqField.InvestorID, reqField.OrderSysID);
		if (order != nullptr) {
			m_rspPackage.AddField(*order);
			PushRspToResultFlow(tID, package, rspField, NoError, true);
		} else {
			PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
		}
	} else {
		int count = 0;
		m_investorRepo.OrderForEach(reqField.InvestorID, reqField.InstrumentID, [&](data::COrder const& order, bool isLast) {
			m_rspPackage.AddField(order);
			PushRspToResultFlow(tID, package, rspField, NoError, isLast);
			count++;
			});
		if (count == 0) {
			PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
		}
	}
}
void CQueryServiceLink::handleQueryTrade(CZTPPackage& package) {
	data::CQryInvestorTrade reqField;
	if (!package.GetSingleField(reqField)) {
		return;
	}
	const int tID = TID_RspQryInvestorTrade;
	data::CTrade rspField;
	if (auto error = CheckSession(reqField.BrokerID, reqField.InvestorID, package.GetSessionID()); !error.IsSuccess()) {
		PushRspToResultFlow(tID, package, rspField, error);
		return;
	}
	int count = 0;
	if (reqField.OrderSysID != "*") {
		m_investorRepo.TradeForEach(reqField.InvestorID, reqField.OrderSysID, [&](data::CTrade const& fee, bool isLast) {
			CopyTradeEntity(&rspField, &fee);
			m_rspPackage.AddField(rspField);
			PushRspToResultFlow(tID, package, rspField, NoError, isLast);
			count++;
			});
	} else {
		m_investorRepo.TradeForEach(reqField.InvestorID, reqField.InstrumentID, [&](data::CTrade const& fee, bool isLast) {
			CopyTradeEntity(&rspField, &fee);
			m_rspPackage.AddField(rspField);
			PushRspToResultFlow(tID, package, rspField, NoError, isLast);
			count++;
			});
	}
	if (count == 0) {
		PushEmptySuccessRspToResultFlow(tID, package);		//没有查询结果
	}
}


