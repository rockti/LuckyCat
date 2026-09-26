#include "stdafx.h"
#include "TradeEngine.h"
#include "LinkManager.h"
#include "LoginServiceImpl.h"
#include "TradeServiceImpl.h"
#include "Profile.h"
#include "SGEConnection.h"
#include "MockSGEConnection.h"
#include <fmt/format.h>

#include "MaxOrderLocalIDSinglton.h"


extern CExchangeLinkManager* g_pExchangeLinkManager;

typedef CNewUsTrigger<data::COrder, InvestorRepository::OrderTrigger> COrderTrigger;
typedef CNewUsTrigger<data::CTrade, InvestorRepository::TradeTrigger> CTradeTrigger;
typedef CNewUsTrigger<data::CBalanceChange, InvestorRepository::BalanceChangeTrigger> CBalanceChangeTrigger;
typedef CNewUsTrigger<data::CPositionChange, InvestorRepository::PositionChangeTrigger> CPositionChangeTrigger;


CTradeEngine::CTradeEngine(Repos& repos,Database& database,IZTPServer& ztpServer,bool& isInitFinished, IFlow* pTradeResultFlow, CScheduler* pScheduler)
:m_repos(repos),m_database(database),m_ztpServer(ztpServer),m_isInitFinished(isInitFinished),m_pTradeResultFlow(pTradeResultFlow),m_pScheduler(pScheduler) {
	m_rspPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
	m_repos.SystemStatusRepo.Get()->SystemStatus = type::SystemStatus::Start;

	//创建上场服务
	std::string brokerID = theProfile().GetPrivateProfile("Broker", "BrokerID", "");

	m_pInitServiceLink.reset(new CInitServiceLink(
		m_repos.SystemStatusRepo,
		m_repos.InvestorRepo,
		m_repos.OperRepo, 
		m_repos.InstRepo, 
		m_repos.BaseInfoRepo, 
		m_repos.OrderStore,
		brokerID));
	std::string reportMsg = "CTradeEngine: InitService Created";
	RUNTIME_INFO_REPORT(reportMsg.c_str());	
	Log::Info(reportMsg.c_str());

	//创建重放服务
	m_pOrderSyncLink.reset(new COrderSyncLink(m_repos.SystemStatusRepo, m_repos.InstRepo, m_repos.InvestorRepo, m_repos.OrderStore));
	reportMsg = "CTradeEngine: SyncService Created";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());

	//创建登录服务
	m_pLoginServiceLink.reset(new CLoginServiceLink(m_repos.SystemStatusRepo,m_repos.SessionRepo,m_repos.InvestorRepo,m_repos.OperRepo, m_database,pTradeResultFlow));
	reportMsg = "CTradeEngine: LoginService Created";
	RUNTIME_INFO_REPORT(reportMsg.c_str());	
	Log::Info(reportMsg.c_str());

	//创建交易服务
	m_pTradeServiceLink.reset(new CTradeServiceLink(m_repos.SessionRepo, m_repos.SystemStatusRepo,m_repos.BaseInfoRepo,m_repos.InvestorRepo,m_repos.InstRepo, pTradeResultFlow));
	reportMsg = "CTradeEngine: TradeService Created";
	RUNTIME_INFO_REPORT(reportMsg.c_str());	
	Log::Info(reportMsg.c_str());

	//创建查询服务
	m_pQueryServiceLink.reset(new CQueryServiceLink(m_repos.SessionRepo, m_repos.InvestorRepo, m_repos.InstRepo, m_repos.SystemStatusRepo,pTradeResultFlow));
	reportMsg = "CTradeEngine: QueryService Created";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());

	//创建查询服务
	m_pOperatorServiceLink.reset(new COperatorServiceLink(m_ztpServer, m_repos.SystemStatusRepo, m_repos.BaseInfoRepo, m_repos.InvestorRepo, m_repos.InstRepo));
	reportMsg = "CTradeEngine: OperatorService Created";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());

	m_repos.InvestorRepo.AddTrigger(new COrderTrigger(pScheduler, "t_Order"));
	m_repos.InvestorRepo.AddTrigger(new CTradeTrigger(pScheduler, "t_Trade"));
	m_repos.InvestorRepo.AddTrigger(new CBalanceChangeTrigger(pScheduler, "t_BalanceChange"));
	m_repos.InvestorRepo.AddTrigger(new CPositionChangeTrigger(pScheduler, "t_PositionChange"));
}

CTradeEngine::~CTradeEngine() {
	m_rspPackage.Release();
}

void CTradeEngine::HandleMessage(CZTPPackage& package) {
	DWORD tID = package.GetTID();
	auto* pStatus = m_repos.SystemStatusRepo.Get();
	switch (pStatus->SystemStatus) {
	case type::SystemStatus::Start: {			//数据可操作	
		if (tID == TID_DataSyncStart) {
			startSyncData(package);
			return;
		}
		break;
	}
	case type::SystemStatus::DBLoading: {		//导入初始数据
		if (tID == TID_DataSyncEnd) {
			MaxOrderLocalIDSinglton::InitMaxOrderLocalID(m_pInitServiceLink->GetLocalID());
			m_pInitServiceLink->InitInvestorUsedMarginByPosition();
			m_pInitServiceLink->InitBalanceAndPositionChange();
			endSyncData();
		} else if (Internal::IsInit(tID)) {
			m_pInitServiceLink->HandleMessage(package);
		}
		return;
	}
	case type::SystemStatus::ExchSyncing: {
		if (Sync::IsMine(tID) || Public::InstrumentStatus::ID) {
			m_pOrderSyncLink->HandleMessage(package);
			if (pStatus->SystemStatus == type::SystemStatus::Ready) {
				m_isInitFinished = true;
			}
		} else if (Trading::IsMineReq(tID) || Auth::IsMine(tID) ) {
			rspInvalidSystemStatus(tID, package);
		}
		return;
	}
	case type::SystemStatus::Ready: {
		m_isInitFinished = true;
		if (tID == TID_NotifyInstrumentStatus) {
			handleInstrumentStatus(package, pStatus);
			return;
		}
		if (pStatus->TradingStatus == type::TradingStatus::NotTrading) {
			if (Trading::IsMineReq(tID)) {	//交易结束收到委托请求，直接给应答
				rspInvalidSystemStatus(tID, package);
				return;
			} 
		}
		if (Trading::IsMineReq(tID) || Internal::IsExchange(tID)) {
			m_pTradeServiceLink->HandleMessage(package);
			return;
		} 
		if (Auth::IsMine(tID)) {
			m_pLoginServiceLink->HandleMessage(package);
		} else if (Query::IsMine(tID)) {
			m_pQueryServiceLink->HandleMessage(package);
		} else if (Op::IsMine(tID)) {
			m_pOperatorServiceLink->HandleMessage(package);
		} else {
			std::string reportMsg = fmt::format("do not support TID[{}]", tID);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Info(reportMsg.c_str());
		}
		break;
	}
	default: {
		std::string reportMsg = fmt::format("do not support TID[{}]", tID);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Info(reportMsg.c_str());
	}
		break;
	}
}

void CTradeEngine::handleInstrumentStatus(CZTPPackage& package, data::CSystemStatus* pStatus) {
	data::CInstrumentStatus status;
	if (!package.GetSingleField(status)) {
		return;
	}
	pStatus->TradingStatus = convertToTradingStatus(status.InstrumentStatus);

	m_rspPackage.PreparePublish(TID_NotifyInstrumentStatus);
	m_rspPackage.GetZTPHeader()->SessionID = package.GetSessionID();
	m_rspPackage.GetZTPHeader()->RequestID = package.GetRequestID();
	m_rspPackage.AddField(status);
	m_rspPackage.MakePackage();
	m_pTradeResultFlow->Append(m_rspPackage.GetAddress(), m_rspPackage.GetLength());
}

void CTradeEngine::rspInvalidSystemStatus(DWORD tID, CZTPPackage& package) {
	data::CRspInfo rspField{ InvalidSystemStatusError.Code, type::ErrorMsg(InvalidSystemStatusError.Msg) };

	if (Trading::IsMineReq(tID)) {
		m_rspPackage.PreparePublish(tID + 1);
		m_rspPackage.GetZTPHeader()->SessionID = package.GetSessionID();
		m_rspPackage.GetZTPHeader()->RequestID = package.GetRequestID();
		m_rspPackage.AddField(rspField);
		switch (tID) {
		case TID_ReqOrderInsert: {
			data::COrder field;
			if (!package.GetSingleField(field)) {
				return;
			}
			m_rspPackage.AddField(field);
		}
			break;
		case TID_ReqOrderAction: {
			data::COrderAction field;
			if (!package.GetSingleField(field)) {
				return;
			}
			m_rspPackage.AddField(field);
		}
			break;
		default:
			break;
		}
	} else {
		m_rspPackage.PreparePublish(TID_RspError);
		m_rspPackage.GetZTPHeader()->SessionID = package.GetSessionID();
		m_rspPackage.GetZTPHeader()->RequestID = package.GetRequestID();
		m_rspPackage.AddField(rspField);
	}
	m_rspPackage.MakePackage();
	m_pTradeResultFlow->Append(m_rspPackage.GetAddress(), m_rspPackage.GetLength());
}

type::SystemStatus CTradeEngine::GetSystemStatus() {	
	data::CSystemStatus* pSystemStatus = m_repos.SystemStatusRepo.Get();
	if (NULL == pSystemStatus) {
		return type::SystemStatus::Start;
	}
	return pSystemStatus->SystemStatus;
}

void CTradeEngine::startSyncData(CZTPPackage& package) {
// 	CTradingDayField tradingDayField;
// 	CNamedFieldIterator fieldItor = package.GetNamedFieldIterator(&tradingDayField.m_Describe);
// 	fieldItor.Retrieve(&tradingDayField);

// 	data::CSystemStatus systemStatus;
// 	systemStatus.SystemStatus = type::SystemStatus::DBLoading;

	m_repos.SystemStatusRepo.Get()->SystemStatus = type::SystemStatus::DBLoading;;

	std::string reportMsg = "交易系统状态->[导入初始数据]";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());
}
void CTradeEngine::endSyncData() {
	clearOrderLocalID();
	toExchSyncingStatus();
	g_pExchangeLinkManager->GetSGEHandler()->Login();
}

void CTradeEngine::clearOrderLocalID() {
	m_repos.BaseInfoRepo.Get()->OrderLocalID = 0;
}
void CTradeEngine::toExchSyncingStatus() {
//	GetSystemStatus()->SystemStatus = ZTP_SS_ExchSyncing;
	m_repos.SystemStatusRepo.Get()->SystemStatus = type::SystemStatus::ExchSyncing;
//	m_memoryDB.m_pSystemStatusFactory->update(pSystemStatus, &systemStatus);

	std::string reportMsg = "交易系统状态->[同步交易流水]";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());
}

