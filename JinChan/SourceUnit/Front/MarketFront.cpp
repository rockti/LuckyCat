#include "stdafx.h"
#include "MarketFront.h"
#include "UserFlow.h"
#include <fmt/format.h>
#include "ErrorDefine.h"

CMarketFront::CMarketFront(InvestorInfoRepository& investorRepo, CReactorThread* pFrontReactor, CMarketFlowManager* pFlowManager, CEventHandler* pFrontManager)
:m_investorRepo(investorRepo),CEventHandler(pFrontReactor), m_pFlowManager(pFlowManager), m_pFrontManager(pFrontManager) {
	m_rspPackage.ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);
}
CMarketFront::~CMarketFront() {
	DisconnectAll(0);
}
int CMarketFront::HandleEvent(int nEventID, DWORD dwParam, void *pParam) {
	switch(nEventID) {
	case UM_MDFRONT_ADD_SESSION: return AddSession((CFTDSession*)pParam);
	case UM_MDFRONT_DEL_SESSION: return DelSession(dwParam, 0);
	default: break;
	}
	return 0;
}

void CMarketFront::rspError(CFTDSession* pSession, Error error) {
	CFTDRspInfoField ftdRspInfoField;
	ftdRspInfoField.ErrorID = error.Code;
	ftdRspInfoField.ErrorMsg = error.Msg;
	m_rspPackage.PreparePackage(FTD_TID_RspError, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);

	SendToDialogFlow(pSession, &m_rspPackage);
}
int CMarketFront::HandlePackage(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (FTD_TID_ReqUserLogin != pFTDPackage->GetTID()) {
		if (!IsSessionLogin(pSession->GetSessionID())) {
			RspNotLogin(pSession);
			return 0;
		}
	}

	int nRet = 0;
	switch(pFTDPackage->GetTID())  {
	case FTD_TID_ReqUserLogin:
		nRet = OnReqUserLogin(pFTDPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqUserLogout:
		OnReqUserLogout(pFTDPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqSubMarketData:
		nRet = OnReqSubMarketData(pFTDPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqUnSubMarketData:
		nRet = OnReqUnSubMarketData(pFTDPackage, (CFTDSession*)pSession);
		break;
	default:
		rspError(pSession, InvalidRequestError);
		Log::Debug(fmt::format("CMarketFront: received unknown FTDPackageID[{}]", pFTDPackage->GetTID()));
		break;
	}
	return nRet;
}
int CMarketFront::AddSession(CFTDSession* pSession) {
	OnSessionConnected(pSession);
	m_pIOThread->RegisterIOHandler(pSession);
	pSession->RegisterPackageHandler(this);
	pSession->RegisterSessionCallback(this);
	return 0;
}
int CMarketFront::DelSession(DWORD dwSessionID, int nErrorCode) {
	auto iter = m_sessionInfoMap.find(dwSessionID);
	if (iter == m_sessionInfoMap.end()) {
		return -1;
	}
	iter->second.pSession->Disconnect(nErrorCode);
	return 0;
}
CFTDSession* CMarketFront::GetSession(DWORD dwSessionID) {
	auto iter = m_sessionInfoMap.find(dwSessionID);
	if (iter == m_sessionInfoMap.end()) {
		return nullptr;
	}
	return (CFTDSession*)iter->second.pSession;
}

void CMarketFront::OnSessionConnected(CSession* pSession) {
	Log::Info(fmt::format("CMarketFront: Session[{}] connected, IP[{}]", pSession->GetSessionID(), pSession->GetChannel()->GetRemoteName()));
	TMarketFrontSessionInfo sInfo;
	sInfo.pSession = pSession;
	sInfo.nSessionID = pSession->GetSessionID();
	sInfo.bIsLogin = false;
	m_sessionInfoMap.emplace(sInfo.nSessionID, sInfo);
}
void CMarketFront::OnSessionDisconnected(CSession* pSession, int nReason) {
	Log::Info(fmt::format("CMarketFront: Session[{}] disconnected, reason[{:08x}] IP[{}]", pSession->GetSessionID(), nReason, pSession->GetChannel()->GetRemoteName()));

	CLoginIDType LoginID;
	auto iter = m_sessionInfoMap.find(pSession->GetSessionID());
	if (iter != m_sessionInfoMap.end()) {
		m_sessionInfoMap.erase(iter);
		((CFTDSession*)pSession)->UnPublish(DIALOG_FLOW);
	}
}
void CMarketFront::OnSessionWarning(CSession *pSession, int nReason, int nParam) {
	Log::Info(fmt::format("CMarketFront: Session[{}] warning, reason[{:08x}] IP[{}]", pSession->GetSessionID(), nReason, pSession->GetChannel()->GetRemoteName()));
}

void CMarketFront::DisconnectAll(int nReason) {
	for (auto& kv : m_sessionInfoMap) {
		kv.second.pSession->Disconnect(nReason);
	}
	m_sessionInfoMap.clear();
}
bool CMarketFront::IsSessionLogin(DWORD nSessionID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter == m_sessionInfoMap.end()) {
		return false;
	}

	return iter->second.bIsLogin;
}
bool CMarketFront::GetSessionOwner(DWORD nSessionID, CLoginIDType& LoginID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter == m_sessionInfoMap.end()) {
		return false;
	}

	LoginID = iter->second.LoginID;
	return true;
}
void CMarketFront::SetSessionOwner(DWORD nSessionID, CReadOnlyLoginIDType& LoginID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter != m_sessionInfoMap.end()) {
		iter->second.LoginID = LoginID;
		iter->second.bIsLogin = true;
	}
	
}
void CMarketFront::ClearSessionOwner(DWORD const nSessionID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter != m_sessionInfoMap.end()) {
		iter->second.LoginID.clear();
		iter->second.bIsLogin = false;
	}
}
void CMarketFront::InitRspInfo(CFTDRspInfoField* pRspInfoField) {
	pRspInfoField->ErrorID = 0;
	pRspInfoField->ErrorMsg = "正确";
}
void CMarketFront::SendToDialogFlow(CFTDSession* pSession, CFTDPackage* pPackage) {
	auto iter = m_sessionInfoMap.find(pSession->GetSessionID());
	if (iter != m_sessionInfoMap.end()) {

		FTDHeader* pHeader = pPackage->GetFTDHeader();
		pHeader->SequenceSeries = DIALOG_FLOW;
		pHeader->SequenceNumber = ++(iter->second.nDialogFlowSeqNo);
		pSession->SendRequestPackage(pPackage);
	}
}
void CMarketFront::RspNotLogin(CFTDSession* pSession) {
	rspError(pSession, NotLoginError);
}

int CMarketFront::OnReqUserLogin(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (NULL == pSession)	return -1;

	CFTDReqUserLoginField reqUserLoginField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &reqUserLoginField) <= 0) {
		Log::Fatal("CMarketFront: OnReqUserLogin get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}

	//用户端产品信息可以在这里校验，如果失败直接给出失败应答	TODO zty
	//。。。
	m_rspPackage.PrepareResponse(pFTDPackage, FTD_TID_RspUserLogin, FTD_CHAIN_LAST);
	CFTDRspUserLoginField rspUserLoginField;
	rspUserLoginField.TradingDay = m_pFlowManager->GetTradingDay();
	rspUserLoginField.BrokerID = reqUserLoginField.BrokerID;
	rspUserLoginField.LoginID = reqUserLoginField.LoginID;
	rspUserLoginField.UserType = reqUserLoginField.UserType;
	rspUserLoginField.TradingSystemName = "JinChan:Market";
	rspUserLoginField.SessionID = pSession->GetSessionID();
	rspUserLoginField.MaxOrderLocalID = 0;
	FTD_ADD_FIELD(&m_rspPackage, &rspUserLoginField);
	CFTDRspInfoField rspInfoField;
	InitRspInfo(&rspInfoField);
	rspInfoField.ErrorMsg = "登录成功";

	auto* info = m_investorRepo.Get(type::BrokerID(reqUserLoginField.BrokerID.getValue()), type::InvestorID(reqUserLoginField.LoginID.getValue()));

	if (info && info->Password == reqUserLoginField.Password.getValue()) {
		SetSessionOwner(pSession->GetSessionID(), reqUserLoginField.LoginID);
		Log::Info(fmt::format("CMarketFront: UserLogin success, Remote[{}] BroketID[{}] LoginID[{}] UserProductInfo[{}] SessionID[{}]", pSession->GetChannel()->GetRemoteName()
			, reqUserLoginField.BrokerID, reqUserLoginField.LoginID, reqUserLoginField.UserProductInfo, pSession->GetSessionID()));
	} else {
		rspInfoField.ErrorID = InvalidLoginError.Code;
		rspInfoField.ErrorMsg = InvalidLoginError.Msg;
		Log::Info(fmt::format("CMarketFront: UserLogin Failed, Remote[{}] BroketID[{}] LoginID[{}] UserProductInfo[{}] SessionID[{}]", pSession->GetChannel()->GetRemoteName()
			, reqUserLoginField.BrokerID, reqUserLoginField.LoginID, reqUserLoginField.UserProductInfo, pSession->GetSessionID()));
	}

	FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);
	SendToDialogFlow(pSession, &m_rspPackage);
	return 0;
}
void CMarketFront::OnReqUserLogout(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (NULL == pSession)	return;

	CFTDReqUserLogoutField reqUserLogoutField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &reqUserLogoutField) <=0) {
		Log::Fatal("CMarketFront: OnReqUserLogout get ErrorPackage");
		pSession->Disconnect(0);
		return;
	}

	//发送录出应答
	m_rspPackage.PrepareResponse(pFTDPackage, FTD_TID_RspUserLogout, FTD_CHAIN_LAST);
	CFTDRspUserLogoutField rspUserLogoutField;
	rspUserLogoutField.BrokerID = reqUserLogoutField.BrokerID;
	rspUserLogoutField.LoginID = reqUserLogoutField.LoginID;
	FTD_ADD_FIELD(&m_rspPackage, &rspUserLogoutField);

	CFTDRspInfoField rspInfoField;
	InitRspInfo(&rspInfoField);
	rspInfoField.ErrorMsg = "登出成功";
	FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);
	SendToDialogFlow(pSession, &m_rspPackage);
	ClearSessionOwner(pSession->GetSessionID());
	Log::Info(fmt::format("CMarketFront: UserLogout success, Remote[{}] BrokerID[{}] LoginID[{}] SessionID[{}]", pSession->GetChannel()->GetRemoteName()
		, rspUserLogoutField.BrokerID, rspUserLogoutField.LoginID, pSession->GetSessionID()));
}
int CMarketFront::OnReqSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	//如果没有合约信息，订阅也没有用
	if(NULL==pSession)
		return 0;

	CFTDRspInfoField rspInfoField;
	InitRspInfo(&rspInfoField);
	m_rspPackage.PrepareResponse(pFTDPackage, FTD_TID_RspSubMarketData, FTD_CHAIN_LAST);
	pSession->CreatSubInstrument();	//创建一个合约类，可以选择要发送的合约

	CFTDSpecificInstrumentField specificInstrumentField;	
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &specificInstrumentField) <= 0) {
		Log::Fatal("CMarketFront: OnReqSubMarketData get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}

	auto instrumentIndex = FTDInstance().GetIndex(specificInstrumentField.InstrumentID);
	if (!instrumentIndex.IsValid()) {
		rspInfoField.ErrorID = InvalidInstrumentError.Code;
		rspInfoField.ErrorMsg = InvalidInstrumentError.Msg;
	} else {
		if (pSession->InsertInstrumentID(instrumentIndex.Index(), specificInstrumentField.InstrumentID)) {
			rspInfoField.ErrorMsg = "成功";
			PublishTopicFlows(pFTDPackage, pSession);	//再处理请求包里的订阅流号
		} else {
			rspInfoField.ErrorID = DuplicateInstrumentError.Code;
			rspInfoField.ErrorMsg = DuplicateInstrumentError.Msg;
		}
	}
	FTD_ADD_FIELD(&m_rspPackage, &specificInstrumentField);
	FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);

	SendToDialogFlow(pSession, &m_rspPackage);	//先返回订阅应答
	auto* snapshot = FTDInstance().GetSnapshot(instrumentIndex);
	if (snapshot) {
		sendSnapshot(snapshot, pFTDPackage, pSession);
	}
	return 0;
}
void CMarketFront::sendSnapshot(CFTDDepthMarketDataField* snapshot, CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	CFTDDisseminationField disseminationField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &disseminationField) <= 0) {
		Log::Fatal("CMarketFront: get disseminationField failed, errorPackage");
		return;
	}
	DWORD topicID = disseminationField.SequenceSeries;
	m_rspPackage.PrepareResponse(pFTDPackage, FTD_TID_RtnDepthMarketData, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, snapshot);
	m_rspPackage.SetRequestID(topicID);
	pSession->SendRequestPackage(&m_rspPackage);
}
int CMarketFront::OnReqUnSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession) { 
	if(NULL==pSession)
		return 0;

	CFTDRspInfoField rspInfoField;
	InitRspInfo(&rspInfoField);
	m_rspPackage.PrepareResponse(pFTDPackage, FTD_TID_RspUnSubMarketData, FTD_CHAIN_LAST);

	CFTDSpecificInstrumentField specificInstrumentField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &specificInstrumentField) <= 0) {
		Log::Fatal("CMarketFront: OnReqUnSubMarketData get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}

	auto instrumentIndex = FTDInstance().GetIndex(specificInstrumentField.InstrumentID);
	if (!instrumentIndex.IsValid()) {
		rspInfoField.ErrorID = -1;
		rspInfoField.ErrorMsg = "合约不存在";
		FTD_ADD_FIELD(&m_rspPackage, &specificInstrumentField);
		FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);
	} else {
		if (pSession->DeleteInstrumentID(instrumentIndex.Index())) {
			rspInfoField.ErrorMsg = "退订成功";
			FTD_ADD_FIELD(&m_rspPackage, &specificInstrumentField);
			FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);
		} else {
			rspInfoField.ErrorID = -1;
			rspInfoField.ErrorMsg = "退订失败";
			FTD_ADD_FIELD(&m_rspPackage, &specificInstrumentField);
			FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);
		}
	}

	SendToDialogFlow(pSession, &m_rspPackage);
	return 0;
}
int CMarketFront::OnRspQryMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if(NULL==pSession)
		return 0;

	return 0;
}

void CMarketFront::PublishTopicFlows(CFTDPackage* pSubscribePackage, CFTDSession* pSession) {
	CFTDDisseminationField disseminationField;
	if (FTD_GET_SINGLE_FIELD(pSubscribePackage, &disseminationField) <= 0) {
		Log::Fatal("CMarketFront: get disseminationField failed, errorPackage");
		return;
	}
	
	DWORD topicID = disseminationField.SequenceSeries;
	IFlow* pFlow = m_pFlowManager->GetTopicFlow(topicID);			//找到TopicID对应的流
	if (NULL != pFlow) {
		pSession->Publish(pFlow, disseminationField.SequenceSeries, pFlow->GetCount()/*从流上最新的一个包开始发*/);
	} else {
		Log::Fatal(fmt::format("CMarketFront: can not find topic[{}] flow", topicID));
	}
}


CMarketFrontManager::CMarketFrontManager(CReactorThread* pReactor, CMarketFlowManager* pFlowManager)
:CServerSessionFactory(pReactor, 1000), m_pFlowManager(pFlowManager),  m_pMarketFront(NULL), m_pSelectThread(NULL) {
	Log::Info(fmt::format("CEventHandler->CSessionFactory->CMarketFrontManager: ThreadID[{}]", ((CThreadBase*)pReactor)->GetThreadID()));
}
CMarketFrontManager::~CMarketFrontManager() {
	m_pSelectThread->Stop();
	delete m_pMarketFront;
	delete m_pSelectThread;
}
int CMarketFrontManager::HandleEvent(int nEventID, DWORD dwParam, void* pParam) {
	if (nEventID == UM_MDFRONT_DEL_SESSION) {	//行情前置登录失败则删除Session
		m_pMarketFront->PostEvent(UM_MDFRONT_DEL_SESSION, dwParam, NULL);
	}

	return CServerSessionFactory::HandleEvent(nEventID, dwParam, pParam);
}
CSession* CMarketFrontManager::CreateSession(CChannel* pChannel, DWORD dwMark) {
	//在物理连接建立后创建会话
	CFTDSession* pSession =  new CFTDSession(m_pSelectThread, pChannel);
	m_pMarketFront->PostEvent(UM_MDFRONT_ADD_SESSION, 0, pSession);
	pSession->SetHeartbeatMode(XMPHBT_SLAVE_MODE);
	return NULL;	//一定要返回NULL，否则在SessionFactory里没法RegisterPackageHandler
}
CSelectThread* CMarketFrontManager::CreateFrontThread(InvestorInfoRepository& investorRepo) {
	m_pSelectThread = new CSelectThread;
	m_pMarketFront = new CMarketFront(investorRepo,m_pSelectThread, m_pFlowManager, this);
	m_pSelectThread->Create();

	string reportMsg = fmt::format("CMarketFrontManager->CreateFront: ThreadID[{}]", m_pSelectThread->GetThreadID());
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());
	return m_pSelectThread;
}
