#pragma once
#include "ErrorEngine.h"
#include "IFlow.h"
#include "ZTPPackage.h"
#include "EntityCopy.h"
#include "Repository.h"


class COrderSyncLink
{
public:
	COrderSyncLink(SystemStatusRepository& systemStatusRepo, InstrumentRepository& instRepo, InvestorRepository& investorRepo, InitOrderStore& orderStore);
	virtual ~COrderSyncLink();
	bool HandleMessage(CZTPPackage& package);


private:
	void syncTrade(CZTPPackage& package);
	void syncOrder(CZTPPackage& package);

	SystemStatusRepository& m_systemStatusRepo;
	InstrumentRepository& m_instRepo;
	InvestorRepository& m_investorRepo;
	InitOrderStore& m_orderStore;
};