#pragma once
#include "Package.h"
#include "FTDPackage.h"
#include "ZTPPackage.h"
#include "MarketRepository.h"
#include "SelectThread.h"
#include "CachedFlow.h"
#include "FlowReader.h"
#include "MarketEngine.h"
#include "MarketScheduler.h"
#include "ZtpDataStruct.h"
#include "EntityCopy.h"
#include "Database.h"
#include "Repository/MemoryRepositoryImpl.h"

class CMarketDBThread : public CSelectThread {
public:	
	CMarketDBThread(IFlow* marketFlow, CMarketEngine* pMarketEngine)
		:m_pMarketEngine(pMarketEngine) {
		m_ftdPackage.ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);
		m_ztpPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
		m_flowReader.AttachFlow(marketFlow, 0);
	}
	~CMarketDBThread()	{}

	virtual bool HandleOtherTask() {
		return true;
	}

private:
	CMarketEngine* m_pMarketEngine;			//行情处理引擎
	CFTDPackage m_ftdPackage;
	CZTPPackage m_ztpPackage;
	CFlowReader m_flowReader;
};

class CMarketCore : public CSelectThread{
public:
	CMarketCore(CSelectThread* pInitThread);
	~CMarketCore();
	bool InitInstance();
	void HandleMessage(CZTPPackage* pMessage);

private:
	bool CreateMarketConnection(CSelectThread* pFrontThread);
//	bool PrepareInitLoad();
private:
	CMarketFlowManager* m_pMarketFlowManager;
	CCachedFlow* m_pMarketFlow;				//行情流
	CSelectThread* m_pInitThread;			//初始化线程

	data::CSeatInfo m_seatInfo;
	Database* m_database;
	CFlowReader m_seriesReader;
	CZTPPackage m_initPackage;
	bool m_isInited = false;
	MemInvestorInfoRepository m_investorRepo;
	std::string m_marketBrokerID;
};

