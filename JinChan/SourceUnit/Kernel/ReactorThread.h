#pragma once
#include "EventThread.h"

//CReactorThread是建立在一个独立线程基础上的调度引擎，在需要的时候它会调用已经注册的CEventHandler对象
//IO类CEventHandler对象它保存在一个list中;Timer类CEventHandler对象它保存在自实现链表中。
class  CReactorThread : public CEventThread 
{
public:
	CReactorThread();
	virtual ~CReactorThread();
	
	//注册一个IO类可运行对象
	virtual void RegisterIOHandler(CEventHandler* pEventHandler);
	
	//从对象list中移去一个IO类可运行对象
	virtual void RemoveIOHandler(CEventHandler* pEventHandler);
	
	//设置特殊事件标志
	//@param bFlag true:有特殊事件发生 false:无特殊事件发生
	void SetSpeciaFlag(bool bFlag)	{ m_bSpeciaFlag = bFlag; }
	bool GetSpeciaFlag() const { return m_bSpeciaFlag; }
	
protected:
	//处理其它任务，派生类可继承来处理自已的一些需要反复执行的工作，比如行情落库
	virtual bool HandleOtherTask();

protected:
	typedef std::list<CEventHandler*> CEventHandlerList;
	CEventHandlerList m_IOHandlerList;	//存放IO事件处理器的列表
	bool m_bIOListHasNull;			    //IO事件处理器列表的内容标志
	bool m_bSpeciaFlag;					//特殊事件标志
};
