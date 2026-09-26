#pragma once

#include "BaseThread.h"
#include "EventQueue.h"
#include "EventHandler.h"
#include "TimerHandler.h"

const int SM_USER = 0x1000;
//事件调度线程
class CEventThread : public CThreadBase
{
public:
	CEventThread();
	virtual ~CEventThread();

	virtual void Run();
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);

	//注册一个定时类可运行对象
	void RegisterTimer(CEventHandler* pEventHandler, int nIDEvent, int nElapse);
	//从定时器链表中移去一个定时器对象	0表示移去所有注册对象
	void RemoveTimer(CEventHandler* pEventHandler, int nIDEvent);
	
	//向EventHandler发送异步事件
	bool PostEvent(CEventHandler* pEventHandler, int nEventID, DWORD dwParam, void* pParam);
	//向EventHandler发送同步事件
	int SendEvent(CEventHandler* pEventHandler, int nEventID, DWORD dwParam, void* pParam);	
	//清除事件
	void ClearEvent(CEventHandler* pEventHandler);

	//终止程序主循环
	void Stop(DWORD nStopCode = 0);	
	
	//获取当前系统时间
	DWORD Time() const { return m_nCurrTime; }
	//获取当前系统时间的毫秒部分
	DWORD GetMilTime() const { return m_nCurrMilTime; }	
	//获取当前系统滴哒数
	DWORD GetClock() const { return m_nCurrClockTick; }
	
protected:
	//同步当前系统时间，检查Timer
	void SyncTime();
	//从事件队列中取出事件，分派给相应的线程
	void DispatchEvents();

private:
	virtual void DispatchIOs() = 0;	//分派IO事件

//	string GetEventDesc(int nEventID);

protected:
	DWORD m_nCurrTime;				//当前系统时间秒
	DWORD m_nCurrMilTime;			//当前系统时间毫秒
	DWORD m_nCurrClockTick;			//当前系统滴哒数
	bool m_bShouldRun;				//循环运行标志

	CEventQueue m_queueEvent;		//事件队列(2048个)
	CTimerHandler m_timerHandler;	//定时器队列
	CMutex m_lockActive;
};