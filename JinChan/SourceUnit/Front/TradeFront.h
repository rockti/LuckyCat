#pragma once
#include "BaseExchApi.h"
#include "EventHandler.h"
#include "SelectThread.h"
#include "IFlow.h"
#include "FlowReader.h"
#include "ZTPPackage.h"
#include "FrontForwarder.h"
#include "AdapterEngine.h"

/*
const int FRONT_TIME_SYNC_TIMER_ID = 0x000009600;	//前置定时器
const int FRONT_TIME_SYNC_TIMER_FREQUENCY = 1000;	//毫秒
class CFrontTimeTimer : public CEventHandler
{
public:
	CFrontTimeTimer(CReactorThread* pReactor, IFlow* pFlow)
		:CEventHandler(pReactor),m_pLockFlow(pFlow)
		,m_nCurrTime(0),m_nCurrMilTime(0),m_nCurrClock(0)
	{
		m_pNotifyPackage = CZTPPackage::CreatePackage(1000);
		this->SetTimer(FRONT_TIME_SYNC_TIMER_ID, FRONT_TIME_SYNC_TIMER_FREQUENCY);
	}
protected:
	virtual void OnTimer(int nIDEvent)
	{
		switch(nIDEvent)
		{		
		case FRONT_TIME_SYNC_TIMER_ID:		//定时信息增加到kernel的输入流
			{
				SyncTime();
				CCurrentTimeField timeField = {0};
				timeField.CurrDate = "20180918";
				time_t dwCurrTime = (time_t)m_nCurrTime;
				strftime(m_timeBuffer, sizeof(timeField.CurrTime),"%H:%M:%S", localtime(&dwCurrTime));
				timeField.CurrTime = m_timeBuffer;
				timeField.CurrMillisec = m_nCurrMilTime;

// 				m_pNotifyPackage->PreparePublish(TID_NtfTimeSync);
// 				ZTP_ADD_FIELD(m_pNotifyPackage, &timeField);
// 				m_pNotifyPackage->MakePackage();				
// 				m_pLockFlow->Append(m_pNotifyPackage->GetAddress(), m_pNotifyPackage->GetLength());	
				break;
			}	
		}
	}

private:
	void SyncTime()
	{
		struct _timeb timebuffer;
		_ftime(&timebuffer);
		m_nCurrTime = (DWORD)timebuffer.time;	
		m_nCurrMilTime = timebuffer.millitm;
		m_nCurrClock = m_nCurrTime*1000 + timebuffer.millitm;
	}

private:
	IFlow* m_pLockFlow;				//流文件
	CZTPPackage* m_pNotifyPackage;	//时钟包
	char m_timeBuffer[20];			//存放当前时间	
	DWORD m_nCurrTime;		//当前系统时间秒
	DWORD m_nCurrMilTime;	//当前系统时间毫秒
	DWORD m_nCurrClock;		//当前滴答数
};
*/

class CKernalFront : public CSeriesReader, public CSelectThread
{
public:
	CKernalFront(IFlow* pTradeResultFlow);
	virtual ~CKernalFront();
	virtual bool InitInstance()	{ return true; }
	virtual void StartTimer();
	virtual bool HandleOtherTask();		//处理ZTP返回数据

	void FrontStartWork();
private:
	void InitFront();

private:
	CAdapteEngine* m_pAdapteEngine;		//FTD|ZTP转换引擎

	CZTPPackage m_ztpPackage;
	CFlowReader m_resultReader;
};
