#include "stdafx.h"
/*
#include "CTime.h"
#include "InitHandler.h"
#include "Profile.h"
#include "CachedFileFlow.h"
#include <fmt/format.h>

int const TIMER_CHECK_CONNECTION = 1;
CInitHandler::CInitHandler(CReactorThread* pInitThread, IEngine* pEngine)
:CEventHandler(pInitThread),m_pEngine(pEngine),m_isInited(false)
,m_pDbConnection(new CDbConnection),m_pInitDataLoader(NULL) {
	m_pDataPackage = CZTPPackage::CreatePackage(ZTP_HEAD_LEN);
	m_pInitDataLoader = new CInitDataLoader(this);
	SetTimer(TIMER_CHECK_CONNECTION, 1000);
}
CInitHandler::~CInitHandler() {
	delete m_pInitDataLoader;
	m_pDataPackage->Release();
	delete m_pDbConnection;
}

void CInitHandler::OnTimer(int nIDEvent) {
	if (nIDEvent == TIMER_CHECK_CONNECTION) {
		if (false == m_pInitDataLoader->IsBusy()) {
			if (!m_pDbConnection->isConnected()) {
				m_pDbConnection->ConnectDB();
			}
		}
	}
}
int CInitHandler::HandleEvent(int nEventID, DWORD dwParam, void* pParam)
{
	switch(nEventID) 
	{
	case UM_INIT_APPEND_DATA:
			return OnInitAppendData((CCachedFlow*)pParam, (int)dwParam);
	case UM_INIT_APPEND_END:
			return OnInitAppendEnd();
	case UM_INIT_RESULT:
		{
			string reportMsg = "CInitHandler: loadTable ";
			if (0 != dwParam) {
				reportMsg += fmt::format("failed[{}]", (char*)pParam);
				Log::Fatal(reportMsg.c_str());
				RUNTIME_INFO_REPORT(reportMsg.c_str());	
			} else {
				reportMsg += "success";
// 				Log::Info(reportMsg.c_str());
			}
			return 0;	
		}
	default:
		break;
	}
	return CEventHandler::HandleEvent(nEventID, dwParam, pParam);
}
bool CInitHandler::DoInit() {
	CSystemStatusField systemStatusField;
	try {
		SACommand cmdcount(m_pDbConnection, "SELECT count(*) FROM t_sync_systemstatus");
		cmdcount.Execute();
		cmdcount.FetchNext();
		long statusnumber = cmdcount.Field(1).asLong();
		if(1 != statusnumber) {
			string reportMsg = "CInitHandler: t_sync_systemstatus 记录不唯一 获取系统状态失败!";
			RUNTIME_INFO_REPORT(reportMsg.c_str());	
			Log::Warn(reportMsg.c_str());
			return false;
		}
		cmdcount.Close();
		SACommand cmd(m_pDbConnection, "SELECT * FROM t_sync_systemstatus");
		cmd.Execute();
		cmd.FetchNext();
		CDbConnection::GetRow(&cmd, (char*)&systemStatusField, &systemStatusField.m_Describe);
	}	
	catch(SAException& x) {
		try
		{
			m_pDbConnection->CheckConntectionError(x.ErrNativeCode());
			m_pDbConnection->Rollback();
			string reportMsg = "CInitHandler: 获取系统状态失败，与数据库断开连接!";
			RUNTIME_INFO_REPORT(reportMsg.c_str());	
			Log::Fatal(reportMsg.c_str());
			return false;
		}
		catch(SAException &) {
		}
	}

	CTradingDayField tradingDay;
	tradingDay.TradingDay = systemStatusField.TradingDay;
	LoadTradingDay(tradingDay);
	return true;
}
bool CInitHandler::ConnectDB(char const* databaseName, char const* userName, char const* password) {
	if(!m_pDbConnection->ConnectDB(databaseName, userName, password)) {
		return false;
	}
	return m_pInitDataLoader->ConnectDB(databaseName, userName, password);
}
void CInitHandler::LoadTradingDay(CTradingDayField& tradingDay) {
	m_TradingDay = tradingDay;
	m_pDataPackage->PreparePublish(TID_DataSyncStart);
	ZTP_ADD_FIELD(m_pDataPackage, &m_TradingDay);
	m_pEngine->HandleMessage(m_pDataPackage);

	m_pInitDataLoader->StartLoadData(m_TradingDay.TradingDay);
}
int CInitHandler::OnInitAppendData(CCachedFlow* pDataFlow, int startPos) {
	int const BULK_SIZE = 1000;		//单批次处理1000条
	int nCount = pDataFlow->GetCount() - startPos;
	if (nCount > BULK_SIZE) {
		nCount = BULK_SIZE;
	}

	m_flowReader.AttachFlow(pDataFlow, startPos);
	for (int i=0; i<nCount; ++i) {
		m_pDataPackage->Allocate(ZTP_PACKAGE_MAX_SIZE+ZTP_HEAD_LEN);
		if (!m_flowReader.GetNext(m_pDataPackage)) {
			break;
		} else {
			m_pEngine->HandleMessage(m_pDataPackage);
		}
	}

	return nCount;
}
int CInitHandler::OnInitAppendEnd() {
	m_pDataPackage->PreparePublish(TID_DataSyncEnd);
	ZTP_ADD_FIELD(m_pDataPackage, &m_TradingDay);
	m_pEngine->HandleMessage(m_pDataPackage);
	m_isInited = true;
	return 0;
}
*/