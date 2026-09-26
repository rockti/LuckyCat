#pragma once
/*
#include "FlowReader.h"
#include "DBLoadThread.h"
#include "SelectThread.h"
#include "UserDataType.h"
#include "ZtpDataStruct.h"
#include "ZTPPackage.h"
#include "IEngine.h"

//初始化Handler,每次交易核心启动的时候读取基础数据，不支持数据库盘中修改同步
class CInitHandler: public CEventHandler
{
public:
	CInitHandler(CReactorThread* pInitThread, IEngine* pEngine);
	virtual ~CInitHandler();

	virtual void OnTimer(int nIDEvent);
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);

	bool DoInit();
	bool ConnectDB(char const* databaseName, char const* userName, char const* password);
	bool IsInited() const { return m_isInited; }

private:
	void LoadTradingDay(CTradingDayField& tradingDay);
	int OnInitAppendData(CCachedFlow* pDataFlow, int startPos);
	int OnInitAppendEnd();

private:
	bool m_isInited;						//初始化标志
	CTradingDayField m_TradingDay;			//交易日
	CFlowReader m_flowReader;				//流读取器

	CZTPPackage* m_pDataPackage;			//数据包
	IEngine* m_pEngine;			//交易引擎
	CDbConnection* m_pDbConnection;			//数据库连接
	CInitDataLoader* m_pInitDataLoader;		//数据库读取实例
};
*/
