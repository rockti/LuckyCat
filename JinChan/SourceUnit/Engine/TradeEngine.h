#pragma once
#include "IFlow.h"
#include "Scheduler.h"
#include "IService.h"
#include "InitServiceLink.h"
#include "OrderSyncLink.h"
#include "LoginServiceLink.h"
#include "TradeServiceLink.h"
#include "Triggers.h"
#include "Repository/MemoryRepositoryImpl.h"
#include "QueryServiceLink.h"
#include "OperatorServiceLink.h"
#include "IServer.h"

using namespace ztp;
class Database;
struct Repos {
	MyInvestorRepository InvestorRepo;
	MyInstrumentRepository InstRepo;
	MemSystemStatusRepository SystemStatusRepo;
	MyBaseInfoRepository BaseInfoRepo;
	MemSessionRepository SessionRepo;
	MyOperatorRepository OperRepo;
	MemInitOrderStore OrderStore;
};

//内部交易引擎
class CTradeEngine {
public:
	CTradeEngine(Repos& repos,Database& database,IZTPServer& ztpServer, bool& isInitFinished,IFlow* pTradeResultFlow, CScheduler* pScheduler);
	virtual ~CTradeEngine();

	void HandleMessage(CZTPPackage& package);
	void handleInstrumentStatus(CZTPPackage& package, data::CSystemStatus* pStatus);
	void rspInvalidSystemStatus(DWORD tID, CZTPPackage& package);
private:
	type::SystemStatus GetSystemStatus();
	void startSyncData(CZTPPackage& package);
	void endSyncData();
	void clearOrderLocalID();
	void toExchSyncingStatus();

private:
	Repos& m_repos;
	Database& m_database;
	IZTPServer& m_ztpServer;
	bool& m_isInitFinished;

	IFlow* m_pTradeResultFlow;					//TradeCore中创建
	CScheduler* m_pScheduler;					//流转换器

	std::unique_ptr<CInitServiceLink> m_pInitServiceLink;
	std::unique_ptr<COrderSyncLink> m_pOrderSyncLink;
	std::unique_ptr<CLoginServiceLink> m_pLoginServiceLink;
	std::unique_ptr<CTradeServiceLink> m_pTradeServiceLink;
	std::unique_ptr<CQueryServiceLink> m_pQueryServiceLink;
	std::unique_ptr<COperatorServiceLink> m_pOperatorServiceLink;

	CZTPPackage m_rspPackage;
};
