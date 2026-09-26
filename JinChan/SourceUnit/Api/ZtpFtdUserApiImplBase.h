#pragma once
#include "IFlow.h"
#include "CachedFlow.h"
#include "FTDSession.h"
#include "FTDSubscribe.h"
#include "FTDDataStruct.h"
#include "ZtpFtdUserApi.h"
#include "SessionFactory.h"
#include "Mutex.h"


//带流控的订阅者接口
class CFtdUserApiImplBase;
class CFtdSequenceSubscriber : public CFTDSubscriber
{
public:
	CFtdSequenceSubscriber(CFtdUserApiImplBase* pUserApiImplBase, IFlow* pFlow, WORD wSequenceSeries);
	virtual ~CFtdSequenceSubscriber();
	void SetCommPhaseNo(WORD wCommPhaseNo);
	WORD GetCommPhaseNo();
	WORD GetSequenceSeries();
	DWORD GetReceivedCount();
	void SetResumeType(TE_RESUME_TYPE nResumeType)	{ m_nResumeType = nResumeType; }
	TE_RESUME_TYPE GetResumeType()	{ return m_nResumeType; }

	void HandleMessage(CFTDPackage* pMessage);

	
	void MoveTo(int nOffset);
	void CleanController(void);
	void UpdateTimeStampList(CFTDPackage* pResponse);

	int TagTimeStamp(void);	//标记时间戳

private:
	CFtdUserApiImplBase* m_pUserApiImplBase;
	IFlow* m_pFlow;
	WORD m_sequenceSeries;			//流类型(对话、私有、公共、查询)
	WORD m_commPhaseNo;				//通讯阶段编号
	DWORD m_sequenceNo;				//已经通讯的序号
	TE_RESUME_TYPE m_nResumeType;	//重传模式

	DWORD m_maxBlockCount;			//最多未处理
	DWORD m_queryTimeout;			//查询超时秒数
	DWORD m_maxPerSecond;			//每秒流控数
	DWORD m_currentPerSecond;		//当前流速

	time_t m_lastTime;
	list<time_t> m_timeStampList;	//请求时间戳
	CMutex m_mutex;
};

//不同的Topic会有不同的UserFlow产生
class CMarketTopicManager
{
public:
	CMarketTopicManager(char const* pszFlowPath);
	~CMarketTopicManager();
	IFlow* GetFlow(DWORD nTopicID);
	bool RegisterTopic(DWORD nTopicID);
private:
	string m_strFlowPath;
	typedef std::map<DWORD, IFlow*> CFlowMap;
	CFlowMap m_mapFlow;
};

//Session层面的指令都在这里完成，包括登录
class CFtdUserApiImplBase : public CZtpFtdUserApi, public CClientSessionFactory, public CFTDSessionCallback
{
public:
	CFtdUserApiImplBase(char const* pszFlowPath, char const* pszUserApiType, CSelectThread* pReactor);	
	~CFtdUserApiImplBase();
	
	virtual void HandleResponse(CFTDPackage* pMessage, WORD wSequenceSeries) = 0;	//处理应答包	Inherited from CNsSession
	virtual int HandlePackage(CFTDPackage* pFTDPackage, CFTDSession* pSession);		//处理请求包	inherited from CNsSessionFactory

	virtual void Init();	
	virtual void Release();
	virtual char const* GetTradingDay();
	virtual void RegisterSpi(CZtpFtdUserSpi* pSpi);
	virtual void RegisterFront(char *pszFrontAddress);		//创建SessionConnecter

	virtual void SubscribeMarketTopic(int nTopicID);		//订阅行情流(主题不同行情源不同)，只能是Quick模式

#ifdef OPERATER_DLL
	virtual void SubscribePrivateTopic(TE_RESUME_TYPE nResumeType);
#else
	virtual void SubscribeUserTopic(TE_RESUME_TYPE nResumeType);
#endif
	virtual void SubscribePublicTopic(TE_RESUME_TYPE nResumeType);

	//前置登录其实是在这里转换成FTD协议的。对于SGE接口记得要重写
	virtual int ReqUserLogin(CZtpApiReqLoginField* pReqUserLoginField, int nRequestID);

	//行情前置API
	virtual int ReqSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID);
	virtual int ReqUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID);

protected:
	virtual CSession* CreateSession(CChannel* pChannel, DWORD);
	virtual void OnSessionConnected(CSession* pSession);
	virtual void OnSessionDisconnected(CSession* pSession, int nReason);
	virtual void OnSessionWarning(CSession* pSession, int nReason, int nParam);	

	int RequestDirectly();		//直接请求，不受流控
	int RequestToQueryFlow();	//查询流，流控和超时，暂时不用，查询都通过对话流实现
	int RequestToDialogFlow();	//对话流，受到流控
	void OnInternalRtnDissemination(CFTDPackage* pMessage, WORD wSequenceSeries);

private:
	void CreateDialogFlow();
	void RemoveDialogFlow();
	void CreateQueryFlow();
	void RemoveQueryFlow();
	void CreateSubscriber(int nTopicID, IFlow* pFlow, TE_RESUME_TYPE nResumeType);
	void RemoveSubscriber();

protected:
	CZtpFtdUserSpi* m_pSpi;		//回调指针
	CFTDPackage m_reqPackage;	//请求包
	CMutex m_mutexAction;

private:
	string m_strFlowPath;		//流文件路径
	string m_strUserApiType;	//API类型(MDMarket|Trader)
	CNotifyHandler* m_pNotifyHandler;	//通知线程，在Init的时候创建，用于Dialog流交互
	DWORD m_sessionID;			//前置号
	string m_tradingDay;		//交易日

	IFlow* m_pDialogReqFlow;	//会话流：请求应答
	IFlow* m_pQueryReqFlow;		//查询流：查询
	IFlow* m_pUserFlow;			//用户流：回报(客户级别)
	IFlow* m_pPrivateFlow;		//私有流：回报(会员级别)
	IFlow* m_pPublicFlow;		//公共流：通知
		
	typedef map<WORD, CFtdSequenceSubscriber*> SequenceSubscriberMap;
	SequenceSubscriberMap m_sequenceSubscriberMap;					//序列订阅列表(会话流、查询流、各种Topic用户流)

	map<CInstrumentIDType,CInstrumentIDType> m_marketSubscribeMap;	//行情订阅列表
	CMarketTopicManager* m_pMarketTopicManager;						//行情Topic管理器
};
