#pragma once
#include "SelectThread.h"
#include "BaseExchApi.h"
#include "TradeFront.h"
#include "TradeEngine.h"
#include "FlowReader.h"
#include "ZTPPackage.h"
#include "Scheduler.h"
#include "ServiceName.h"
#include "UdpServer.h"
#include "Profile.h"
#include "UserFlow.h"
#include "Database.h"
#include "ZTPServer.h"
class CTradeCore : public CThreadBase
{
public:
	CTradeCore(/*CSelectThread* pConnectThread,*/ CSelectThread* pInitThread);

	bool IsInited() const { return m_isTradeCoreInited; }
	CKernalFront* GetTradeFront()	{ return m_pFrontKernalThread; }
	bool Join();
	void ZTPRun();
private:
	virtual bool InitInstance();
	virtual void ExitInstance();
	void Run();
	void process(CSeriesReader* pSeriesReader);

	char const* GetSubjectDescription(DWORD SubjectID);
	char const* GetTIDDescription(DWORD TID);

private:
	int m_pipeID;							//Pipe必须分配一个初始化的ID，不可重复
	bool m_isTradeCoreInited;				//交易核心初始化标志
	CZTPPackage m_ztpPackage;				//内部数据包

	CCachedFlow* m_pTradeResultFlow = nullptr;		//交易结果流(交易所返回的原始应答、回报信息)
	CCachedFlow* m_pMemChangedFlow = nullptr;			//内存变化流(交易核心处理过的各类数据走这里)

// 	CSubscribeThread* m_pMarketSubThread;	//内部行情订阅线程
	Database* m_dataBase = nullptr;

	CSelectThread* m_pInitThread = nullptr;			//初始化线程
	CSelectThread* m_pConnectThread = nullptr;		//交易前置连接管理线程
	CKernalFront* m_pFrontKernalThread = nullptr;		//交易前置数据处理线程

	CTradeEngine* m_pTradeEngine = nullptr;			//交易引擎
	CScheduler* m_pMDBScheduler = nullptr;			//触发器(触发内存表时调用，根据交易结果生成内存变化流)
	ZTPServer* m_ztpServer = nullptr;
};


