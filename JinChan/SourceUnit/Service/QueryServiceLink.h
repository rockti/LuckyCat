#pragma once
#include "IFlow.h"
#include "ZTPPackage.h"
#include "EntityCopy.h"
#include "Repository.h"
#include "BaseServiceLink.h"

class CInvestorAccount;
class CInvestorPosition;

class CQueryServiceLink : public SessionCheckService
{
public:
	CQueryServiceLink(
		SessionRepository& sessionRepo,
		InvestorRepository& investorRepo, 
		InstrumentRepository& instRepo, 
		SystemStatusRepository& systemRepo,
		IFlow* pTradeResult);
	virtual ~CQueryServiceLink();

	void HandleMessage(CZTPPackage& package);

protected:
	void handelQueryAccount(CZTPPackage& package);
	void handleQueryPosition(CZTPPackage& package);
	void handleQueryInstrument(CZTPPackage& package);
	void handleQueryMargin(CZTPPackage& package);
	void handleQueryFee(CZTPPackage& package);
	void handleQueryOrder(CZTPPackage& package);
	void handleQueryTrade(CZTPPackage& package);

	template<class Field>
	void PushRspToResultFlow(DWORD tID, CZTPPackage& package, Field& field, Error error = NoError, bool isLast = true);

	void PushEmptySuccessRspToResultFlow(DWORD tID, CZTPPackage& package);

private:
	InvestorRepository& m_investorRepo;
	InstrumentRepository& m_instRepo;
	SystemStatusRepository& m_systemRepo;
	IFlow* m_pQueryResultFlow;
	CZTPPackage m_rspPackage;
};