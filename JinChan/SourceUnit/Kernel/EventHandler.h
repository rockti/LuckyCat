#pragma once
#include "BaseThread.h"
class  CReactorThread;
class  CEventHandler;

struct CEvent
{
	CEventHandler* pEventHandler;
	int nEventID;
	DWORD dwParam;
	void* pParam;
	void* pAdd;
};

//CEventHandler是一个抽象的运行对象。它是各种协议、控制器、定时器的抽象基类。
//有两类事件可触发运行，一种为IOEvent操作，另一种为TimerEvent定时操作。
//一个CEventHandler对象只有注册到CIOThread才能获得核心引擎的调用
class CEventHandler
{
public:
	CEventHandler(CReactorThread* pReactor);
	virtual ~CEventHandler();
	
	//处理IO操作的读操作，由CReactor调用
	virtual int HandleInput() { return 0; };
	
	//处理IO操作的写操作，由CReactor调用
	virtual int HandleOutput() { return 0; };
 	
	//处理一条事件
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam)	{ return 0; };

	//处理定时操作，由CReactor调用
	virtual void OnTimer(int nIDEvent) {};
	
	//获取读、写socket，供select使用
	virtual void GetIds(int* pReadId, int* pWriteId) {};

	//向CEventHandler对象发送异步事件
	bool PostEvent(int nEventID, DWORD dwParam, void* pParam);

	//向CEventHandler对象发送同步事件
	int SendEvent(int nEventID, DWORD dwParam, void* pParam);

	DWORD GetIOThreadID() const { return ((CThreadBase*)m_pIOThread)->GetThreadID(); }

protected:
	void SetTimer(int nIDEvent, int nElapse);
	void KillTimer(int nIDEvent);

protected:
	CReactorThread* m_pIOThread;    //当前对象所归属引擎的指针
};
