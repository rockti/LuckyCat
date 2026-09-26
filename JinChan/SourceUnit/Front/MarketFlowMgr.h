#pragma once
#include "IFlow.h"
#include "Channel.h"
#include "FTDPackage.h"
#include "Mutex.h"
#include "ZTPType.h"
using namespace ztp;

//行情内存流管理器，生成一个行情流并将交易所的行情放入其中，同时生成一个UDP通道向系统内部广播
class CMarketFlowManager : public std::map<DWORD, IFlow*>	//<topicID, flowObj>
{
public:
	CMarketFlowManager(char const* pszFlowPath, IFlow* marketFlow=NULL);
	~CMarketFlowManager();

	CMutex& GetLock() { return m_lock; }
	IFlow* GetTopicFlow(DWORD nTopicID);
	type::TradingDay const& GetTradingDay() const	{ return m_tradingDay; }

	bool RegisterTopicFlow(DWORD nTopicID);										//SGEConnection初始化时注册TopicID=1000
	void SetTradingDay(type::TradingDay& TradingDay, IFlow* topicflow);				//SGEHandler登录成功后调用
	int PulishToTopicFlow(CFTDPackage* pPackage, DWORD topicID);//SGEHandler发布行情

private:
	IFlow* m_pTradingDayFlow;	//存储行情包序号的文件流
	IFlow* m_pMarketFlow;		//需要转发的行情流
	type::TradingDay m_tradingDay;	
	CMutex m_lock;
};