#pragma once

#include <queue>
class CEventHandler;

struct CTimerHeapNode
{
	CEventHandler* pTimer;	//定时处理器的的指针
	int nIDEvent;			//定时处理器的标示
	int nElapse	;			//定时时间间隔
	DWORD nExpire;			//到期时间
};

struct CCompareTimerHeapNode //: public binary_function<CTimerHeapNode, CTimerHeapNode, bool>
{
    bool operator()(const CTimerHeapNode& x, const CTimerHeapNode& y) const
	{
		return y.nExpire < x.nExpire;
	}
};

//CTimerHandler是利用堆（Heap）实现的一个定时器管理器
class CTimerHandler : public std::priority_queue<CTimerHeapNode, std::vector<CTimerHeapNode>, CCompareTimerHeapNode>
{
public:
	CTimerHandler(DWORD nClock);
	virtual ~CTimerHandler();

	/**注册一个定时类可运行对象
	* @param pEventHandler 定时器的指针 
	* @param nIDEvent 定时器的标示 
	* @param nElapse 定时器的时间间隔 毫秒
	*/
	virtual void RegisterTimer(CEventHandler* pEventHandler, int nIDEvent, int nElapse);

	/**从定时器链表中移去一个定时器对象
	* @param pEventHandler 定时器对象的指针 
	* @param nIDEvent 定时器在CReatctor的标示，=0 移去该定时器所有注册
	*        >0 移去该定时器指定注册
	*/
	virtual void RemoveTimer(CEventHandler* pEventHandler, int nIDEvent);

	//检查各注册的定时器是否到期	@param nClock 当前时间（毫秒计）
	virtual void Expire(DWORD nClock);
	
	//检查各注册的定时器是否到期	@param nClock 当前时间（毫秒计）
	virtual bool CheckExpire(DWORD nClock);
		
private:
	//同步时间
	void SyncTime(DWORD nClock);
private:
	DWORD m_nClock;
	DWORD m_nClockAlter;
};
