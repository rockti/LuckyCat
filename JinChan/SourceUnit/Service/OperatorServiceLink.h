#pragma once
#include "ErrorEngine.h"
#include "IFlow.h"
#include "ZTPPackage.h"
#include "Repository.h"
#include "IServer.h"

using namespace ztp;

class COperatorServiceLink
{
public:
	COperatorServiceLink(
		IZTPServer& ztpServer,
		SystemStatusRepository& systemStatusRepo, 
		BaseInfoRepository& baseInfoRepo, 
		InvestorRepository& investorRepo, 
		InstrumentRepository& instRepo);
	virtual ~COperatorServiceLink();

	void HandleMessage(CZTPPackage& package);

protected:
	void handleBalanceRequest(CZTPPackage& package);
	void handleQueryInvestors(CZTPPackage& package);
	void handleQueryAccount(CZTPPackage& package);
	void handleQueryPosition(CZTPPackage& package);
	void handleQueryBalanceChange(CZTPPackage& package);
	void handleQueryPositionChange(CZTPPackage& package);
	
	template<class Proto, class QueryAndForEach>
	void handleQuery(CZTPPackage& package, QueryAndForEach func);

private:
	IZTPServer& m_ztpServer;
	SystemStatusRepository& m_systemStatusRepo;
	BaseInfoRepository& m_baseInfoRepo;
	InvestorRepository& m_investorRepo;
	InstrumentRepository& m_instRepo;
	CZTPPackage m_ztpPackage;
};