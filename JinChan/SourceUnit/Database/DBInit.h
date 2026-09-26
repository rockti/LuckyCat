#pragma once

#include "Database.h"
#include "SessionFactory.h"
#include "SelectThread.h"
#include "Repository/MemoryRepositoryImpl.h"


struct Repos {
	MemSessionRepository SessionRepo;
	MyOperatorRepository OperRepo;
	MemSystemStatusRepository SystemStatusRepo;
	MyInvestorRepository InvestorRepo;
	MyBaseInfoRepository BaseRepo;
};
class DBInit {
public:
	void Init(std::string const& marketBorkerID);
	void HandleMessage(CZTPPackage* pPackage, std::string const& marketBorkerID);
private:
	bool isInited_ = false;

};

Repos& ReposInstance();
Database*& DBInstance();