#include "stdafx.h"
#include "ZtpFtdUserApiImplBase.h"
#include "UserFlow.h"
#include "CTime.h"
#include <fmt/format.h>

//应用程序线程和API线程会同时访问CFtdcUserDialogFlow，必须是线程安全的CCachedFlow
class CFtdUserDialogFlow : public CCachedFlow
{
public:
	CFtdUserDialogFlow(bool bSyncFlag, int nMaxObjects, int nDataBlockSize)
		:CCachedFlow(bSyncFlag,nMaxObjects,nDataBlockSize)
	{
		m_nCount = CCachedFlow::GetCount();
	}

	virtual int GetCount(void)	{ return m_nCount; }
	virtual bool Truncate(int nCount)
	{
		m_lock.Lock();
		bool bRet = CCachedFlow::Truncate(nCount);
		m_nCount = CCachedFlow::GetCount();
		m_lock.UnLock();
		return bRet;
	}
	virtual int Append(void *pObject, int length)
	{
		m_lock.Lock();
		int nRet = -1;
		if(m_nMaxObjects<=0 || (int)m_nodeQueue.size()-m_nFirstID<m_nMaxObjects)
		{
			nRet = CCachedFlow::Append(pObject, length);
			m_nCount = CCachedFlow::GetCount();
		}
		m_lock.UnLock();
		return nRet;
	}
	virtual int Get(int id, void *pObject, int length)
	{
		m_lock.Lock();
		int nRet = CCachedFlow::Get(id, pObject, length);
		m_nCount = CCachedFlow::GetCount();
		PopFront();
		m_lock.UnLock();
		return nRet;
	}

private:
	int m_nCount;
	CMutex m_lock;
};

/////////////////////////////////////CFtdcUserSubscriber////////////////////////////////////
CFtdSequenceSubscriber::CFtdSequenceSubscriber(CFtdUserApiImplBase* pUserApiImplBase, IFlow* pFlow, WORD wSequenceSeries)
:m_pUserApiImplBase(pUserApiImplBase),m_pFlow(pFlow),m_sequenceSeries(wSequenceSeries),m_nResumeType(ZTP_TERT_RESUME)/*续传模式是默认值，对话流和查询流用*/
,m_commPhaseNo(0),m_sequenceNo(0),m_maxBlockCount(1),m_queryTimeout(30),m_currentPerSecond(1)
{
	if (DIALOG_FLOW == wSequenceSeries)
	{
		m_maxBlockCount = 1500;
		m_maxPerSecond = 3000;
		CleanController();
	}

	if (QUERY_FLOW == wSequenceSeries)
	{
		m_maxBlockCount = 1;
		m_maxPerSecond = 50;
		CleanController();
	}
}
CFtdSequenceSubscriber::~CFtdSequenceSubscriber()
{
}
void CFtdSequenceSubscriber::SetCommPhaseNo(WORD wCommPhaseNo)
{
	if (NULL != m_pFlow)
	{
		m_pFlow->SetCommPhaseNo(wCommPhaseNo);
	}

	if (m_commPhaseNo != wCommPhaseNo)
	{
		m_commPhaseNo = wCommPhaseNo;
		m_sequenceNo = 0;
	}
}
WORD CFtdSequenceSubscriber::GetCommPhaseNo()
{
	return (NULL!=m_pFlow) ? m_pFlow->GetCommPhaseNo() : m_commPhaseNo;
}
WORD CFtdSequenceSubscriber::GetSequenceSeries()
{
	return m_sequenceSeries;
}
DWORD CFtdSequenceSubscriber::GetReceivedCount()
{
	return (NULL!=m_pFlow) ? m_pFlow->GetCount() : m_sequenceNo;
}
void CFtdSequenceSubscriber::HandleMessage(CFTDPackage* pMessage)
{
	UpdateTimeStampList(pMessage);

	//提交给上层应用程序
	m_pUserApiImplBase->HandleResponse(pMessage, m_sequenceSeries);

	//将报文加入流中
	if (NULL != m_pFlow)
	{
		pMessage->Push(FTD_HEAD_LEN);
		m_pFlow->Append(pMessage->GetAddress(), pMessage->GetLength());
		pMessage->Pop(FTD_HEAD_LEN);
	}
	m_sequenceNo++;
}
void CFtdSequenceSubscriber::MoveTo(int nOffset)
{
	if (NULL != m_pFlow)
	{
		m_pFlow->Truncate(nOffset);
	}
	m_sequenceNo = nOffset;
}
void CFtdSequenceSubscriber::CleanController(void)
{
	CAutoMutex autoMutex(&m_mutex);
	m_lastTime = 0;
	m_currentPerSecond = 0;
	m_timeStampList.clear();
}
void CFtdSequenceSubscriber::UpdateTimeStampList(CFTDPackage* pResponse)
{
	CAutoMutex autoMutex(&m_mutex);
	if (DIALOG_FLOW==m_sequenceSeries || QUERY_FLOW==m_sequenceSeries) 
	{
		//最后一个包Dequeue才去掉超时时间
		if (FTD_CHAIN_LAST == pResponse->GetChain()) 
		{
			if (m_timeStampList.size() > 0)
				m_timeStampList.pop_front();
		}
	}
}
int CFtdSequenceSubscriber::TagTimeStamp(void)
{
	CAutoMutex autoMutex(&m_mutex);
	//只对对话流和查询流进行流控
	if (DIALOG_FLOW!=m_sequenceSeries && QUERY_FLOW!=m_sequenceSeries)
		return 0;

	time_t t = time(NULL);
	// check unFinished
	if (m_timeStampList.size() >= m_maxBlockCount) 
	{
		//对话流必须实时处理，上个包没处理完直接返回-2
		if (DIALOG_FLOW == m_sequenceSeries)
			return -2;

		list<time_t>::iterator pos = m_timeStampList.begin();
		if (pos != m_timeStampList.end()) 
		{
			time_t t2 = *pos;
			if (t - t2 > m_queryTimeout) 
			{
				m_timeStampList.pop_front();
			} 
			else 
			{	
				return -2;
			}
		}
	}

	// check pre-second
	if (t != m_lastTime) 
	{
		m_currentPerSecond = 1;
		m_lastTime = t;
	}
	else 
	{
		if (m_currentPerSecond >= m_maxPerSecond) 
		{
			return -3;	//超过流控
		}
		++m_currentPerSecond;
	}

	m_timeStampList.push_back(t);
	return 0;
}
/////////////////////////////////////CFtdcUserSubscriber////////////////////////////////////

///////////////////////////////////CMarketDataFlowManager///////////////////////////////////
CMarketTopicManager::CMarketTopicManager(char const* pszFlowPath)
:m_strFlowPath(pszFlowPath)
{
}
CMarketTopicManager::~CMarketTopicManager() {
	for (auto& kv : m_mapFlow) {
		delete kv.second;
	}
}
IFlow* CMarketTopicManager::GetFlow(DWORD nTopicID)
{
	auto iter = m_mapFlow.find(nTopicID);
	return (iter == m_mapFlow.end()) ? NULL : iter->second;
}
bool CMarketTopicManager::RegisterTopic(DWORD nTopicID)
{
	//注册过的不用再注册
	if (NULL != GetFlow(nTopicID))
	{
		return false;
	}

	IFlow* pFlow  = new CUserFlow(nTopicID, m_strFlowPath.c_str(), true);
	m_mapFlow.emplace(nTopicID, pFlow);
	Log::Info(fmt::format("CMarketTopicFlowManager: RegisterTopic CreateUserFlow[{}] Success", nTopicID));
	return true;
}
///////////////////////////////////CMarketDataFlowManager///////////////////////////////////


CFtdUserApiImplBase::CFtdUserApiImplBase(char const* pszFlowPath, char const* pszUserApiType, CSelectThread* pReactor)
:CClientSessionFactory(pReactor,1),m_strFlowPath(pszFlowPath),m_strUserApiType(pszUserApiType)
,m_pSpi(NULL),m_pNotifyHandler(NULL),m_sessionID(0)
,m_pDialogReqFlow(NULL),m_pQueryReqFlow(NULL),m_pUserFlow(NULL),m_pPrivateFlow(NULL),m_pPublicFlow(NULL)
{
	Log::Debug("Step3: CFtdUserApiImplBase constructed");
	m_reqPackage.ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);

	//对话和查询流是REQ/RSP模式，不用创建流
	m_sequenceSubscriberMap[DIALOG_FLOW] = new CFtdSequenceSubscriber(this, NULL, DIALOG_FLOW);
	m_sequenceSubscriberMap[QUERY_FLOW] = new CFtdSequenceSubscriber(this, NULL, QUERY_FLOW);
	m_pMarketTopicManager = new CMarketTopicManager(pszFlowPath);
}
CFtdUserApiImplBase::~CFtdUserApiImplBase()
{
	CClientSessionFactory::Stop();
	RemoveSubscriber();
	RemoveDialogFlow();
	RemoveQueryFlow();
	delete m_pPrivateFlow;
	m_pPrivateFlow = NULL;
	delete m_pPublicFlow;
	m_pPublicFlow = NULL;
	delete m_pUserFlow;
	m_pUserFlow = NULL;
	delete m_pMarketTopicManager;
	m_pMarketTopicManager = NULL;	
};
int CFtdUserApiImplBase::HandlePackage(CFTDPackage* pFTDPackage, CFTDSession* pSession)
{
	//登录应答在这里处理，其他的应答丢给子类完成
	if (pFTDPackage->GetTID() == FTD_TID_RspUserLogin)
	{
		CFTDRspUserLoginField rspUserLoginField;
		if (FTD_GET_SINGLE_FIELD(pFTDPackage, &rspUserLoginField) <= 0)
		{
			return 0;
		}
		CFTDRspInfoField rspInfoField;
		if (FTD_GET_SINGLE_FIELD(pFTDPackage, &rspInfoField) <= 0)
		{
			return 0;
		}

		m_tradingDay = rspUserLoginField.TradingDay.getValue();
		if (rspInfoField.ErrorID == 0)
		{
			for (SequenceSubscriberMap::iterator itor=m_sequenceSubscriberMap.begin(); itor!=m_sequenceSubscriberMap.end(); ++itor)
			{
				CFtdSequenceSubscriber* pSubscriber = (*itor).second;
				if (pSubscriber->GetSequenceSeries()==DIALOG_FLOW || pSubscriber->GetSequenceSeries()==QUERY_FLOW)
				{	
					continue;	//对话流和查询流的清理是和Session绑定的，会在SessionConnect的时候完成，这里不用处理
				}
				pSubscriber->SetCommPhaseNo((WORD)CDate::DateToLong(rspUserLoginField.TradingDay.getValue()));
			}
		}
	}
	else if(pFTDPackage->GetTID() == FTD_TID_RtnDepthMarketData) 
	{
		HandleResponse(pFTDPackage, (WORD)pFTDPackage->GetRequestID());
		return 0;
	}

	HandleResponse(pFTDPackage, pFTDPackage->GetFTDHeader()->SequenceSeries);
	return 0;
}
void CFtdUserApiImplBase::Init()
{	
	m_pNotifyHandler = new CNotifyHandler((CSelectThread*)m_pIOThread, 39600);
	m_pIOThread->Create();
	Start();
	Log::Debug(fmt::format("Step6: CFtdUserApiImplBase Init, IOThread Created[{}] SessionThread Created;", m_pIOThread->GetThreadID()));
}
void CFtdUserApiImplBase::Release()
{
	m_pIOThread->Stop();
	m_pIOThread->Join();

	CReactorThread* pFartherThread = m_pIOThread;
	delete m_pNotifyHandler;
	delete this;
	delete pFartherThread;
}
char const* CFtdUserApiImplBase::GetTradingDay()
{
	return m_tradingDay.c_str();
}
void CFtdUserApiImplBase::RegisterSpi(CZtpFtdUserSpi* pSpi)
{
	m_pSpi = pSpi;
}
void CFtdUserApiImplBase::RegisterFront(char* pszFrontAddress)
{
	RegisterConnecter(pszFrontAddress);
}
void CFtdUserApiImplBase::SubscribeMarketTopic(int nTopicID)
{
	m_pMarketTopicManager->RegisterTopic(nTopicID);
	CreateSubscriber(nTopicID, (CUserFlow*)m_pMarketTopicManager->GetFlow((DWORD)nTopicID), ZTP_TERT_QUICK);
}

#ifdef OPERATER_DLL
void CFtdUserApiImplBase::SubscribePrivateTopic(TE_RESUME_TYPE nResumeType)
{
	if (NULL == m_pPrivateFlow)
	{
		m_pPrivateFlow = new CUserFlow("Private", m_strFlowPath.c_str(), true);
		Log::Info(fmt::format("CFtdUserApiImplBase: CreatePrivateFlow[{}] Success", PRIVATE_FLOW));
	}
	CreateSubscriber(PRIVATE_FLOW, m_pPrivateFlow, nResumeType);
}
#else
void CFtdUserApiImplBase::SubscribeUserTopic(TE_RESUME_TYPE nResumeType)
{
	if (NULL == m_pUserFlow)
	{
		m_pUserFlow = new CUserFlow("User", m_strFlowPath.c_str(), true);
		Log::Info(fmt::format("CFtdUserApiImplBase: CreateUserFlow[{}] Success", USER_FLOW));
	}
	CreateSubscriber(USER_FLOW, m_pUserFlow, nResumeType);
}
#endif
void CFtdUserApiImplBase::SubscribePublicTopic(TE_RESUME_TYPE nResumeType)
{
	if (NULL == m_pPublicFlow)
	{
		m_pPublicFlow = new CUserFlow("Public",  m_strFlowPath.c_str(), true);
		Log::Info(fmt::format("CFtdUserApiImplBase: CreatePublicFlow[{}] Success", PUBLIC_FLOW));
	}
	CreateSubscriber(PUBLIC_FLOW, m_pPublicFlow, nResumeType);
}

int CFtdUserApiImplBase::ReqUserLogin(CZtpApiReqLoginField* pReqUserLoginField, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	CFTDReqUserLoginField reqUserloginField;
	reqUserloginField.BrokerID = pReqUserLoginField->BrokerID;
	reqUserloginField.LoginID = pReqUserLoginField->AccountID;
	reqUserloginField.UserType = pReqUserLoginField->UserType;
	reqUserloginField.Password = pReqUserLoginField->Password;
	reqUserloginField.UserProductInfo = pReqUserLoginField->UserProductInfo;

	//没有连接建立Session直接退出
	CFTDSession* pSession = (CFTDSession*)GetSession(m_sessionID);
	if (NULL == pSession)
		return -1;
	reqUserloginField.IPAddress = pSession->GetChannel()->GetLocalName();

	m_reqPackage.PreparePackage(FTD_TID_ReqUserLogin, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);
	FTD_ADD_FIELD(&m_reqPackage, &reqUserloginField);

	CFTDDisseminationField disseminationField;
	for (SequenceSubscriberMap::iterator itor=m_sequenceSubscriberMap.begin(); itor!=m_sequenceSubscriberMap.end(); ++itor)
	{
		CFtdSequenceSubscriber* pSubscriber = (*itor).second;
		disseminationField.SequenceSeries = pSubscriber->GetSequenceSeries();
		switch(pSubscriber->GetResumeType())
		{
		case ZTP_TERT_RESTART:
			disseminationField.SequenceNo = 0;
			break;
		case ZTP_TERT_RESUME:
			disseminationField.SequenceNo = pSubscriber->GetReceivedCount();
			break;
		case ZTP_TERT_QUICK:
			disseminationField.SequenceNo = -1;
			break;
		default:
			break;
		}
		FTD_ADD_FIELD(&m_reqPackage, &disseminationField);
	}

	return RequestDirectly();	//登录请求发送时，服务端对话流还未创建，要用Session直接请求
}
int CFtdUserApiImplBase::ReqSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	CFTDSpecificInstrumentField specificInstrumentField;
	memcpy(&specificInstrumentField, pSpecificInstrument, sizeof(CFTDSpecificInstrumentField));
	m_reqPackage.PreparePackage(FTD_TID_ReqSubMarketData, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);
	FTD_ADD_FIELD(&m_reqPackage, &specificInstrumentField);
	m_marketSubscribeMap[specificInstrumentField.InstrumentID] = specificInstrumentField.InstrumentID;

	CFTDDisseminationField disseminationField;
	for (SequenceSubscriberMap::iterator itor=m_sequenceSubscriberMap.begin(); itor!=m_sequenceSubscriberMap.end(); ++itor)
	{
		CFtdSequenceSubscriber* pSubscriber = (*itor).second;
		//在行情API里不是对话流和查询流的都是SubscribeMarketTopic订阅过的各Topic行情流
		if (DIALOG_FLOW!=pSubscriber->GetSequenceSeries() && QUERY_FLOW!=pSubscriber->GetSequenceSeries())
		{
			disseminationField.SequenceSeries = pSubscriber->GetSequenceSeries();	//行情流流号
			disseminationField.SequenceNo = -1;	//默认是快速模式
			FTD_ADD_FIELD(&m_reqPackage, &disseminationField);
		}
	}

	return RequestToDialogFlow();
}
int CFtdUserApiImplBase::ReqUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	CFTDSpecificInstrumentField specificInstrumentField;
	memcpy(&specificInstrumentField, pSpecificInstrument, sizeof(CFTDSpecificInstrumentField));
	m_reqPackage.PreparePackage(FTD_TID_ReqUnSubMarketData, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);
	FTD_ADD_FIELD(&m_reqPackage, &specificInstrumentField);
	map<CInstrumentIDType,CInstrumentIDType>::iterator itertmp = m_marketSubscribeMap.find(specificInstrumentField.InstrumentID);
	if(itertmp != m_marketSubscribeMap.end())
	{
		m_marketSubscribeMap.erase(itertmp);
	}
	return RequestToDialogFlow();
}

CSession* CFtdUserApiImplBase::CreateSession(CChannel* pChannel, DWORD)
{
	//连接创建后默认建立对话流和查询流，用户流|私有流、公共流需要订阅
	CFTDSession* pSession = new CFTDSession(m_pIOThread, pChannel);
	CreateDialogFlow();
	pSession->Publish(m_pDialogReqFlow, DIALOG_FLOW, 0);
	CreateQueryFlow();
	pSession->Publish(m_pQueryReqFlow, QUERY_FLOW, 0);

	for (SequenceSubscriberMap::iterator itor=m_sequenceSubscriberMap.begin(); itor!=m_sequenceSubscriberMap.end(); ++itor)
	{
		pSession->RegisterSubscriber((*itor).second);
	}

	pSession->RegisterPackageHandler(this);
	pSession->SetHeartbeatTimeout(10);		//心跳时间10s

	return pSession;
}
void CFtdUserApiImplBase::OnSessionConnected(CSession* pSession)
{
	m_sequenceSubscriberMap[DIALOG_FLOW]->CleanController();
	m_sequenceSubscriberMap[DIALOG_FLOW]->MoveTo(0);
	m_sequenceSubscriberMap[QUERY_FLOW]->CleanController();
	m_sequenceSubscriberMap[QUERY_FLOW]->MoveTo(0);
	m_sessionID = pSession->GetSessionID();
	CClientSessionFactory::OnSessionConnected(pSession);
	if (NULL != m_pSpi)
	{
		Log::Debug("CFtdUserApiImplBase::OnFrontConnected()");
		m_pSpi->OnFrontConnected();
	}
}
void CFtdUserApiImplBase::OnSessionDisconnected(CSession* pSession, int nReason)
{
	CAutoMutex autoMutex(&m_mutexAction);
	CClientSessionFactory::OnSessionDisconnected(pSession, nReason);
	m_sessionID = 0;

	if (NULL != m_pSpi)
	{
		m_pSpi->OnFrontDisconnected(nReason);
		Log::Debug(fmt::format("CFtdUserApiImplBase::OnFrontDisconnected({})", nReason));
	}

	RemoveDialogFlow();
	RemoveQueryFlow();
}
void CFtdUserApiImplBase::OnSessionWarning(CSession* pSession, int nReason, int nParam)
{
	Log::Debug(fmt::format("CFtdUserApiImplBase::OnHeartBeatWarning({})", nReason));
	return;	//不发送心跳超时通知
	if (nReason == MSG_XMPWARNING_RECVHEARTBEAT)
	{
		if (m_pSpi != NULL)
		{
// 			m_pSpi->OnHeartBeatWarning(nParam);
		}
	}
}
int CFtdUserApiImplBase::RequestDirectly()
{
	CFTDSession* pSession = (CFTDSession*)GetSession(m_sessionID);
	if (NULL == pSession)
		return -1;

	pSession->SendRequestPackage(&m_reqPackage);
	return 0;
}
int CFtdUserApiImplBase::RequestToDialogFlow()
{
	if (NULL == m_pDialogReqFlow)
		return -1;

	int nRtn = m_sequenceSubscriberMap[DIALOG_FLOW]->TagTimeStamp();
	if (nRtn)	return nRtn;

	m_reqPackage.MakePackage();
	nRtn = m_pDialogReqFlow->Append(m_reqPackage.GetAddress(), m_reqPackage.GetLength());
	return (nRtn < 0) ? -1 : 0;
}
int CFtdUserApiImplBase::RequestToQueryFlow()
{
	if (NULL == m_pQueryReqFlow)
		return -1;

	int nRtn = m_sequenceSubscriberMap[QUERY_FLOW]->TagTimeStamp();
	if (nRtn)	return nRtn;

	m_reqPackage.MakePackage();
	nRtn = m_pQueryReqFlow->Append(m_reqPackage.GetAddress(), m_reqPackage.GetLength());
	return (nRtn < 0) ? -1 : 0;
}
void CFtdUserApiImplBase::OnInternalRtnDissemination(CFTDPackage* pMessage, WORD wSequenceSeries)
{
// 	CFTDDisseminationField disseminationField;
// 	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDDisseminationField::m_Describe);
// 	while (!itor.IsEnd())
// 	{
// 		itor.Retrieve(&disseminationField);
// 		SequenceSubscriberMap::iterator i = m_sequenceSubscriberMap.find(disseminationField.SequenceSeries);
// 		if (i != m_sequenceSubscriberMap.end())
// 		{
// 			(*i).second->MoveTo(disseminationField.SequenceNo);
// 		}
// 		itor.Next();
// 	}
}
void CFtdUserApiImplBase::CreateDialogFlow()
{
	RemoveDialogFlow();
	m_pDialogReqFlow = new CFtdUserDialogFlow(false, 10000, 10000);
	((CFtdUserDialogFlow*)m_pDialogReqFlow)->AttachObserver(m_pNotifyHandler);
}
void CFtdUserApiImplBase::RemoveDialogFlow()
{
	delete m_pDialogReqFlow;
	m_pDialogReqFlow = NULL;
}
void CFtdUserApiImplBase::CreateQueryFlow()
{
	RemoveQueryFlow();
	m_pQueryReqFlow = new CFtdUserDialogFlow(false, 10000, 10000);
}
void CFtdUserApiImplBase::RemoveQueryFlow()
{
	delete m_pQueryReqFlow;
	m_pQueryReqFlow = NULL;
}
void CFtdUserApiImplBase::CreateSubscriber(int nTopicID, IFlow* pFlow, TE_RESUME_TYPE nResumeType)
{
	CFtdSequenceSubscriber* pSubscriber = NULL;
	SequenceSubscriberMap::iterator itor = m_sequenceSubscriberMap.find(WORD(nTopicID));
	if (itor == m_sequenceSubscriberMap.end())
	{
		pSubscriber = new CFtdSequenceSubscriber(this, pFlow, nTopicID);
		m_sequenceSubscriberMap[(WORD)nTopicID] = pSubscriber;
	}
	else
	{
		pSubscriber = (*itor).second;
	}
	pSubscriber->SetResumeType(nResumeType);
}
void CFtdUserApiImplBase::RemoveSubscriber()
{
	for (SequenceSubscriberMap::iterator itor=m_sequenceSubscriberMap.begin(); itor!= m_sequenceSubscriberMap.end(); ++itor)
	{
		delete (*itor).second;
	}
}
