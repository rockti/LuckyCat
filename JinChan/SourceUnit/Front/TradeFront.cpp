#include "stdafx.h"
#include "TradeFront.h"
#include "Profile.h"

CKernalFront::CKernalFront(IFlow* pTradeResultFlow)
:m_pAdapteEngine(NULL) {
	m_pSeriesFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x100000);	//创建交易前置收到的请求流，传输FTD数据包
	m_seriesReader.AttachFlow(m_pSeriesFlow, 0);

	m_ztpPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 100);
	m_resultReader.AttachFlow(pTradeResultFlow, 0);						//AdapterEngine返回的交易结果流，传输ZTP数据包

	InitFront();
}
CKernalFront::~CKernalFront() {
	delete m_pAdapteEngine;
}

void CKernalFront::StartTimer() {
// 	if(NULL == m_pFrontTimeTimer) {
// 		m_pFrontTimeTimer = new CFrontTimeTimer(this, m_pSeriesFlow);
// 	}
}
bool CKernalFront::HandleOtherTask() {
	int i = 0;
	for (i=0; i<100; ++i) {
		m_ztpPackage.Allocate(ZTP_PACKAGE_MAX_SIZE+ZTP_HEAD_LEN);
		if(!m_resultReader.GetNext(&m_ztpPackage))
			break;
		m_pAdapteEngine->HandleMessage(&m_ztpPackage);
	}

	return (i>0) ? true : CSelectThread::HandleOtherTask(); 
}

void CKernalFront::FrontStartWork() {
	Create();
	m_pAdapteEngine->InitOperatorInvestor();	//交易前置启动前先初始化操作员对应客户信息

	string reportMsg = "CKernalFront: TradeFront StartWork";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());
	m_pAdapteEngine->Start();
//	StartTimer();
}

void CKernalFront::InitFront() {	
	//创建转换引擎
	m_pAdapteEngine = new CAdapteEngine(this, new CFrontForwarder(m_pSeriesFlow));

	//交易前置开启监听
	string tradeFrontUrl = theProfile().GetPrivateProfile("Front", "TradeFrontUrl", "tcp://127.0.0.1:13810");
	m_pAdapteEngine->RegisterListener(tradeFrontUrl.c_str());
}
