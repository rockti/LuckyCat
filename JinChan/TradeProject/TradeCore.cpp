#include "stdafx.h"
#include "TradeCore.h"
#include "LinkManager.h"
#include "CachedFileFlow.h"
#include <fmt/format.h>
#include "ZTPServer.h"

CExchangeLinkManager* g_pExchangeLinkManager = NULL;
CTradeCore::CTradeCore(/*CSelectThread* pConnectThread,*/ CSelectThread* pInitThread)
:m_pipeID(1),m_isTradeCoreInited(false),m_pInitThread(pInitThread)/*,m_pConnectThread(pConnectThread)*/ {
	m_ztpPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
}

bool CTradeCore::Join() {
	if (m_dataBase) {
//		m_dataBase->Join();
	}
	return CThreadBase::Join();
}

void CTradeCore::ZTPRun() {
	m_ztpServer->Run();
}


bool CTradeCore::InitInstance() {

	//创建报盘管理线程(管理SeriesFlow)
	RUNTIME_INFO_REPORT("CTradeCore: Create ExchangeLinkManager");	
	g_pExchangeLinkManager = new CExchangeLinkManager();

// 	//创建内部行情流订阅线程，作为UDPSvr端接收行情数据广播	(当然也可以直接通过api接收，待评估)
// 	RUNTIME_INFO_REPORT("CTradeCore: Create MarketSubscribeThread");
// 	std::string marketMoudleUrl = theProfile().GetPrivateProfile("Module", "MarketDataUrl", "");
// 	m_pMarketSubThread = new CSubscribeThread(marketMoudleUrl.c_str());
// 	m_pMarketSubThread->Create();

	//创建内存变化流MemChangedFlow及通知器，用于通知下场数据发布网关
	RUNTIME_INFO_REPORT("CTradeCore: Create MemoryChangedFlow");
	m_pMemChangedFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x200000);



	RUNTIME_INFO_REPORT("CTradeCore: Create TradeResultFlow");
	m_pTradeResultFlow = new CCachedFlow(false, 2000000, 0x100000);
	//创建触发器和交易引擎，将TradeResultFlow转换成MemChangedFlow
	//创建交易结果流TradeResultFlow，ZTP包

	static Repos s_repos;
	auto port = theProfile().GetPrivateProfile("ZTPServer", "Port", 13811);
	m_ztpServer = new ZTPServer(s_repos.OperRepo, s_repos.InvestorRepo, port);

	RUNTIME_INFO_REPORT("CTradeCore: Create TradeEngine and MDBScheduler");

	m_dataBase = new Database(m_pMemChangedFlow, s_repos.SystemStatusRepo);
	m_dataBase->Create();

	m_pMDBScheduler = new CScheduler(m_pMemChangedFlow);
	m_pTradeEngine = new CTradeEngine(s_repos, *m_dataBase ,*m_ztpServer,m_isTradeCoreInited,m_pTradeResultFlow, m_pMDBScheduler);


	//创建交易前置线程
	RUNTIME_INFO_REPORT("CTradeCore: Create KernalTradeFront");
	m_pFrontKernalThread = new CKernalFront(m_pTradeResultFlow /*, m_pConnectThread*/);
	//创建TradeResultFlow通知器，用于通知交易前置线程
	CNotifyHandler* pTradeResultNotifyPipe = new CNotifyHandler(m_pFrontKernalThread, m_pipeID++);
	m_pTradeResultFlow->AttachObserver(pTradeResultNotifyPipe);

	g_pExchangeLinkManager->StartWork();		//启动交易所连接
// 	m_pMarketSubThread->SetRecvMdFlag(true);	//开始接收内部行情广播

// 	ztpServer.Run();
	::Sleep(1000);
//	m_isTradeCoreInited = true;
	return true;
}
void CTradeCore::ExitInstance() {
}
void CTradeCore::Run() {
	while(true) {
		process(m_pFrontKernalThread);	//先处理前置流内请求数据
		CBaseExchApi* pBaseApi = g_pExchangeLinkManager->GetSGEHandler();
		if (NULL != pBaseApi) {
			process(pBaseApi);			//再处理交易所返回流内数据
		}
		process(m_dataBase);
		process(m_ztpServer);
	}
}

void CTradeCore::process(CSeriesReader* pSeriesReader) {
	if (pSeriesReader == nullptr) return;
	for (int i=0; i<50; ++i) {
		if (!pSeriesReader->GetNextUseNoBuf(&m_ztpPackage))	//获取ZTP有序数据包
			break;
		m_pTradeEngine->HandleMessage(m_ztpPackage);		//丢给交易引擎处理
	}
}

char const* CTradeCore::GetSubjectDescription(DWORD SubjectID)
{
	switch (SubjectID) {
	case DIALOG_FLOW:
		return "DIALOG_FLOW";
	case PUBLIC_FLOW:
		return "PUBLIC_FLOW";
	case PRIVATE_FLOW:
		return "PRIVATE_FLOW";
	case QUERY_FLOW:
		return "QUERY_FLOW";
	default:
		return "USER_FLOW";
	}
}
char const* CTradeCore::GetTIDDescription(DWORD TID) {
	switch (TID)
	{
	case TID_DataSyncStart:
		return FT("数据同步开始[0x%x]", TID);
	case TID_DataSyncEnd:
		return FT("数据同步结束[0x%x]", TID);
	case TID_InitSeatInfo:
		return FT("初始化席位信息[0x%x]", TID);
	case TID_InitOperatorInfo:
		return FT("初始化操作员信息[0x%x]", TID);
	case TID_InitInvestorInfo:
		return FT("初始化投资者信息[0x%x]", TID);
	case TID_InitOperatorInvestor:
		return FT("初始化用户投资者关系[0x%x]", TID);
	case TID_InitInvestorAccount:
		return FT("初始化投资者资金账户[0x%x]", TID);
	case TID_InitInstrument:
		return FT("初始化合约列表[0x%x]", TID);
	case TID_InitInvestorMargin:
		return FT("初始化投资者保证金[0x%x]", TID);
	case TID_InitInvestorFee:
		return FT("初始化投资者手续费[0x%x]", TID);
	case TID_InitInvestorPosition:
		return FT("初始化投资者持仓[0x%x]", TID);
	case TID_NotifySeatLoginSuccess:
		return FT("席位登录成功通知[0x%x]", TID);
	case TID_NotifyInstrumentInfo:
		return FT("合约交易状态改变通知[0x%x]", TID);
	case TID_RspOrderInsert:
		return FT("报单录入应答[0x%x]", TID);
	case TID_RspOrderAction:
		return FT("报单撤销应答[0x%x]", TID);
	case TID_NotifyOrder:
		return FT("报单委托回报[0x%x]", TID);
	case TID_NotifyTrade:
		return FT("报单成交回报[0x%x]", TID);
	case TID_NotifyInitFinish:
		return FT("席位查询结束[0x%x]", TID);
	default:
		return FT("未知功能[0x%x]", TID);
	}
}