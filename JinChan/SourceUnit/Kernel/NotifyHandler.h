#pragma once

#include "EventHandler.h"

class CSelectThread;
//CNotifyHandler是一个通知管理器，主要用来触发CIOThread,防止其深度睡眠，从而导致不能及时处理来自队列的任务
class CNotifyHandler : public CEventHandler
{
public:
	CNotifyHandler(CSelectThread* pReactor, int pipeNo=0);
	virtual ~CNotifyHandler();
	
	//由CIOThread调用，处理IO操作的读操作
	virtual int HandleInput();
	
	//获取读、写socket，供select使用
	virtual void GetIds(int* pReadId, int* pWriteId);
	
	//产生通知，通知观察者对象
	virtual void Notify();
	
protected:
	//创建管道 参数p0为读管道文件符， p1 为写管道文件符。
	void CreatePipe(int& pRead, int& pWrite);

protected:
	CSelectThread* m_pSelectThread;
	int	m_pipeNo;
	int m_nReadPipe;
	int m_nWritePipe;
	DWORD m_nPreClock;
	DWORD m_nPerCnt;
	bool m_SendFlag;
	char m_buffer[10];
	
	volatile DWORD m_nReadCnt;
	volatile DWORD m_nWriteCnt;
};
