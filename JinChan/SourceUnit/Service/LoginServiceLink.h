#pragma once
#include "IService.h"
#include "LoginServiceImpl.h"
#include "ErrorEngine.h"
#include "IFlow.h"
#include "ZTPPackage.h"
class Database;
//CLoginServiceLink是一个在内存运行的Login服务的连接
class CLoginServiceLink
{
public:
	//构造函数，根据一个login服务，创建一个使用ZTP包的连接
	CLoginServiceLink(SystemStatusRepository& systemRepo, SessionRepository& sessionRepo,
		InvestorRepository& investorRepo,
		OperatorRepository& operRepo,
		Database& database,
		IFlow* pTradeResultFlow);
	virtual ~CLoginServiceLink();

	Error HandleMessage(CZTPPackage& package);

private:

	template<class Field>
	void PushRspToResultFlow(DWORD tID, CZTPPackage& package, Field& field, Error error = NoError);

	SystemStatusRepository& m_systemRepo;
	CLoginServiceImpl m_loginService;
	IFlow* m_pTradeResultFlow;
	CZTPPackage m_rspPackage;
};

