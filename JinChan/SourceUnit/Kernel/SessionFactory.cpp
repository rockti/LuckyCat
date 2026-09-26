#include "stdafx.h"
#include "SessionFactory.h"
#include <fmt/format.h>

int const TIMER_INTERVAL_CONNECT = 5000;	//重连间隔
CSessionConnecter::CSessionConnecter(char const* nameService, DWORD dwMark)
	:m_nameService(nameService), m_pChannel(NULL), m_dwMark(dwMark) {
}
CChannel* CSessionConnecter::Connect() {
	CChannel* pNewChannel = CNetworkFactory::GetInstance()->CreateChannel(&m_nameService);
	if (NULL == pNewChannel) {
		string reportMsg = fmt::format("CSessionConnecter: Connect to {}:{} error", m_nameService.GetHost(), m_nameService.GetPort());
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Fatal(reportMsg.c_str());
		Log::Error(fmt::format("CSessionConnecter: Connect to {}:{} error", m_nameService.GetHost(), m_nameService.GetPort()));
	} else {
		// 		string reportMsg = fmt::format("CSessionConnecter: Connect to {}:{} success", m_nameService.GetHost(),m_nameService.GetPort());
		// 		RUNTIME_INFO_REPORT(reportMsg.c_str());	
		// 		Log::Info(reportMsg.c_str();
		// 		Log::Info(fmt::format("CSessionConnecter: Connect to {}:{} success", m_nameService.GetHost(),m_nameService.GetPort()));
	}
	return pNewChannel;
}
void CSessionConnecter::OnConnected(CChannel* pChannel) {
	m_pChannel = pChannel;
}
void CSessionConnecter::CheckChannel(CChannel* pChannel) {
	if (m_pChannel == pChannel) {
		m_pChannel = NULL;
	}
}


CConnecterManager::CConnecterManager(CReactorThread* pReactor, CEventHandler* pSessionFactory)
	:CEventHandler(pReactor), m_pSessionFactory(pSessionFactory) {
	Clear();
}
CConnecterManager::~CConnecterManager() {
	Clear();
}
void CConnecterManager::Clear() {
	for (size_t i = 0; i < m_connecters.size(); i++) {
		delete m_connecters[i];
	}
	m_connecters.clear();
	m_nConnecterPos = -1;
}
void CConnecterManager::AppendConnecter(CSessionConnecter* pConnecter) {
	m_connecters.push_back(pConnecter);
}
void CConnecterManager::OnConnected(CChannel* pChannel) {
	m_connecters[m_nConnecterPos]->OnConnected(pChannel);
}
void CConnecterManager::OnDiscconnected(CChannel* pChannel) {
	for (size_t i = 0; i < m_connecters.size(); i++) {
		m_connecters[i]->CheckChannel(pChannel);
	}
	// 	Log::Debug(fmt::format("CConnecterManager->OnDisconnected PostEvent[重复连接-0x%x]", UM_CONNECT_LOOP));
	m_pSessionFactory->PostEvent(UM_CONNECT_LOOP, 0, this);
}
int CConnecterManager::Connect() {
	m_nConnecterPos = -1;
	return ConnectNext();
}
int CConnecterManager::ConnectNext() {
	for (m_nConnecterPos++; m_nConnecterPos < (int)m_connecters.size(); m_nConnecterPos++) {
		CSessionConnecter* pConnecter = m_connecters[m_nConnecterPos];
		if (NULL == pConnecter->GetChannel()) {
			// 			Log::Debug(fmt::format("CConnecterManager->ConnectNext: PostEvent[尝试连接-0x%x]", UM_TRY_CONNECT));
			PostEvent(UM_TRY_CONNECT, 0, pConnecter);
			return m_nConnecterPos;
		}
	}
	m_nConnecterPos = -1;
	m_pSessionFactory->SendEvent(UM_CONNECT_LOOP, 0, this);
	return m_nConnecterPos;
}
int CConnecterManager::HandleEvent(int nEventID, DWORD dwParam, void* pParam) {
	switch (nEventID) {
	case UM_TRY_CONNECT:
	{
		CSessionConnecter* pConnecter = (CSessionConnecter*)pParam;
		CChannel* pNewChannel = pConnecter->Connect();
//		Log::Debug(fmt::format("CConnecterManager->HandleEvent: SendEvent[连接结果-0x%x]", UM_CONNECT_RESULT));
		m_pSessionFactory->SendEvent(UM_CONNECT_RESULT, pConnecter->GetMark(), pNewChannel);
	}
	return 0;
	default:
		break;
	}
	return CEventHandler::HandleEvent(nEventID, dwParam, pParam);
}

CSessionListener::CSessionListener(CReactorThread* pReactor, CEventHandler* pSessionFactory, CServerBase* pServer, DWORD dwMark)
	:CListenHandler(pReactor, pServer), m_pSessionFactory(pSessionFactory), m_dwMark(dwMark) {
	Log::Info(fmt::format("CEventHandler->CListenHandler->CSessionListener: ThreadID[{}] Location[{}]", pReactor->GetThreadID(), pServer->GetServiceName()->GetLocation()));
}
int CSessionListener::OnConnected(CChannel* pChannel) {
	m_pSessionFactory->SendEvent(UM_LISTEN_RESULT, m_dwMark, pChannel);
	return 0;
}

CBaseSessionFactory::CBaseSessionFactory(CReactorThread* pReactor, int nMaxSession)
	:CEventHandler(pReactor), m_nMaxSession(nMaxSession) {
}

CSession* CBaseSessionFactory::GetSession(type::SessionID nSessionID) {
	auto iter = m_sessionMap.find(nSessionID);
	if (iter == m_sessionMap.end()) {
		return nullptr;
	}
	return iter->second;;
}

void CBaseSessionFactory::OnChannelCreated(CChannel* pChannel, DWORD dwMark) {
	CSession* pSession = CreateSession(pChannel, dwMark);
	if (NULL != pSession) {
		OnSessionConnected(pSession);
		m_pIOThread->RegisterIOHandler(pSession);
		pSession->RegisterSessionCallback(this);
	}
}
void CBaseSessionFactory::OnSessionConnected(CSession* pSession) {
	Log::Info(fmt::format("CSessionFactory: Session [{}] connected, IP[{}]", pSession->GetSessionID(), pSession->GetChannel()->GetRemoteName()));
	m_sessionMap.emplace(pSession->GetSessionID(), pSession);
}
void CBaseSessionFactory::OnSessionDisconnected(CSession* pSession, int nReason) {
	Log::Error(fmt::format("CSessionFactory: Session [{}] disconnected, reason[{:08x}] IP[{}] ", pSession->GetSessionID(), nReason, pSession->GetChannel()->GetRemoteName()));
	auto iter = m_sessionMap.find(pSession->GetSessionID());
	m_sessionMap.erase(iter);
}
void CBaseSessionFactory::OnSessionWarning(CSession* pSession, int nReason, int nParam) {
	Log::Info(fmt::format("CSessionFactory: Session[{}] warning, reason[{:08x}] IP[{}]", pSession->GetSessionID(), nReason, pSession->GetChannel()->GetRemoteName()));
}

void CBaseSessionFactory::DisconnectAll(int nReason) {
	for (auto& kv : m_sessionMap) {
		kv.second->Disconnect(nReason);
	}
}


CClientSessionFactory::CClientSessionFactory(CReactorThread* pReactor, int nMaxSession, CSelectThread* pConnReactor)
	:CBaseSessionFactory(pReactor, nMaxSession){
	srand((unsigned int)time(NULL));
	if (NULL == pConnReactor) {
		m_pConnReactor = new CSelectThread();
		m_bSessionCreateBySelf = true;
	} else {
		m_pConnReactor = pConnReactor;
		m_bSessionCreateBySelf = false;
	}
	m_pConnecterManager = new CConnecterManager(m_pConnReactor, this);
}
CClientSessionFactory::~CClientSessionFactory() {
	Stop();
	delete m_pConnecterManager;
	if (m_bSessionCreateBySelf) {
		delete m_pConnReactor;
	}
}
void CClientSessionFactory::Start() {
	if (m_bSessionCreateBySelf) {
		m_pConnReactor->Create();
		// 		Log::Debug(fmt::format("CSessionFactory Start ConnReactor[{}]", m_pIOThread->GetThreadID()));
	}
	OnTimer(TIMER_ID_CONNECT);
}
void CClientSessionFactory::OnTimer(int nIDEvent) {
	if (nIDEvent == TIMER_ID_CONNECT) {
		KillTimer(TIMER_ID_CONNECT);
		if (m_sessionMap.size() < m_nMaxSession && !m_pConnecterManager->IsConnecting()) {
			//最大连接数未满的情况下定时连接
			m_pConnecterManager->Connect();
		}
		return;
	}
	CEventHandler::OnTimer(nIDEvent);
}
int CClientSessionFactory::HandleEvent(int nEventID, DWORD dwParam, void* pParam) {
	switch (nEventID) {
	case UM_CONNECT_RESULT: {
		CChannel* pChannel = (CChannel*)pParam;
		if (NULL != pChannel) {
			m_pConnecterManager->OnConnected(pChannel);
			OnChannelCreated(pChannel, dwParam);
		}
		if (m_sessionMap.size() < m_nMaxSession) {
			m_pConnecterManager->ConnectNext();
		} else {
			m_pConnecterManager->CancelConnect();
		}
	}
	return 0;
	case UM_CONNECT_LOOP: {
		SetTimer(TIMER_ID_CONNECT, TIMER_INTERVAL_CONNECT);
		return 0;
	}
	default:
		break;
	}
	return CEventHandler::HandleEvent(nEventID, dwParam, pParam);
}
void CClientSessionFactory::Stop() {
	if (m_bSessionCreateBySelf) {
		m_pConnReactor->Stop();
		m_pConnReactor->Join();
	}
}

void CClientSessionFactory::RegisterConnecter(char const* nameService, DWORD dwMark) {
	CSessionConnecter* pConnecter = new CSessionConnecter(nameService, dwMark);
	m_pConnecterManager->AppendConnecter(pConnecter);
}


void CClientSessionFactory::OnSessionDisconnected(CSession* pSession, int nReason) {
	CBaseSessionFactory::OnSessionDisconnected(pSession, nReason);
	if (NULL != m_pConnecterManager) {
		m_pConnecterManager->OnDiscconnected(pSession->GetChannel());
	}
}


CServerSessionFactory::CServerSessionFactory(CReactorThread* pReactor, int nMaxSession)
	:CBaseSessionFactory(pReactor, nMaxSession) {
}

void CServerSessionFactory::RegisterListener(char const* nameService, DWORD dwMark) {
	//创建Server
	CServiceName srvname(nameService);
	CServerBase* pServer = CNetworkFactory::GetInstance()->CreateServer(&srvname);
	if (NULL == pServer)
		return;

	//创建Listerer
	CSessionListener* pListener = new CSessionListener(m_pIOThread, this, pServer, dwMark);
	m_pIOThread->RegisterIOHandler(pListener);
	m_listenerSet.push_back(pListener);
}

CServerSessionFactory::~CServerSessionFactory() {
	Stop();
	for (size_t i = 0; i < m_listenerSet.size(); i++) {
		delete m_listenerSet[i];
	}
	m_listenerSet.clear();
}


void CServerSessionFactory::Start() {
}

int CServerSessionFactory::HandleEvent(int nEventID, DWORD dwParam, void* pParam) {
	switch (nEventID) {
	case UM_LISTEN_RESULT:		//收到外部连接请求
	{
		CChannel* pChannel = (CChannel*)pParam;
		if (m_sessionMap.size() > m_nMaxSession) {
			pChannel->Disconnect();	//超过最大连接数断开连接
		} else {
			OnChannelCreated(pChannel, dwParam);	//创建连接通道
		}
	}
	return 0;
	default:
		break;
	}
	return CEventHandler::HandleEvent(nEventID, dwParam, pParam);
}
void CServerSessionFactory::Stop() {
}
/*

CSessionFactory::CSessionFactory(CReactorThread* pReactor, int nMaxSession, CSelectThread* pConnReactor)
	:CEventHandler(pReactor), m_nMaxSession(nMaxSession), m_bConnecterEnable(true), m_bListenerEnable(true), m_bRandomConnect(true) {
	srand((unsigned int)time(NULL));
	if (NULL == pConnReactor) {
		m_pConnReactor = new CSelectThread();
		m_bSessionCreateBySelf = true;
	} else {
		m_pConnReactor = pConnReactor;
		m_bSessionCreateBySelf = false;
	}
	m_pConnecterManager = new CConnecterManager(m_pConnReactor, this);
}

CSessionFactory::~CSessionFactory() {
	Stop();
	for (size_t i = 0; i < m_listenerSet.size(); i++) {
		delete m_listenerSet[i];
	}
	m_listenerSet.clear();
	delete m_pConnecterManager;
	if (m_bSessionCreateBySelf) {
		delete m_pConnReactor;
	}
}

void CSessionFactory::Start() {
	if (m_bSessionCreateBySelf) {
		m_pConnReactor->Create();
// 		Log::Debug(fmt::format("CSessionFactory Start ConnReactor[{}]", m_pIOThread->GetThreadID()));
	}
	OnTimer(TIMER_ID_CONNECT);
}
void CSessionFactory::OnTimer(int nIDEvent) {
	if (nIDEvent == TIMER_ID_CONNECT) {
		KillTimer(TIMER_ID_CONNECT);
		if (m_sessionMap.size() < m_nMaxSession && IsConnecterEnable() && !m_pConnecterManager->IsConnecting()) {
			//最大连接数未满的情况下定时连接
			m_pConnecterManager->Connect(m_bRandomConnect);
		}
		return;
	}
	CEventHandler::OnTimer(nIDEvent);
}
int CSessionFactory::HandleEvent(int nEventID, DWORD dwParam, void* pParam) {
	// 	Log::Debug(fmt::format("CSessionFactory->HandleEvent: Dealwith EventID[0x%x]", nEventID));
	switch (nEventID) {
	case UM_CONNECT_RESULT:
	{
		CChannel* pChannel = (CChannel*)pParam;
		if (NULL != pChannel) {
			m_pConnecterManager->OnConnected(pChannel);
			OnChannelCreated(pChannel, dwParam);
		}
		if (m_sessionMap.size() < m_nMaxSession && IsConnecterEnable()) {
			m_pConnecterManager->ConnectNext();
		} else {
			m_pConnecterManager->CancelConnect();
		}
	}
	return 0;
	case UM_CONNECT_LOOP:
	{
		SetTimer(TIMER_ID_CONNECT, TIMER_INTERVAL_CONNECT);
	}
	return 0;
	case UM_LISTEN_RESULT:		//收到外部连接请求
	{
		CChannel* pChannel = (CChannel*)pParam;
		if (m_sessionMap.size() > m_nMaxSession || !IsListenerEnable()) {
			pChannel->Disconnect();	//超过最大连接数断开连接
		} else {
			OnChannelCreated(pChannel, dwParam);	//创建连接通道
		}
	}
	return 0;
	default:
		break;
	}
	return CEventHandler::HandleEvent(nEventID, dwParam, pParam);
}
void CSessionFactory::Stop() {
	if (m_bSessionCreateBySelf) {
		m_pConnReactor->Stop();
		m_pConnReactor->Join();
	}
//	DisconnectAll(0);
}
void CSessionFactory::DisconnectAll(int nReason) {
	for (auto& kv : m_sessionMap) {
		kv.second->Disconnect(nReason);
	}
}
void CSessionFactory::RegisterConnecter(char const* nameService, DWORD dwMark) {
	CSessionConnecter* pConnecter = new CSessionConnecter(nameService, dwMark);
	m_pConnecterManager->AppendConnecter(pConnecter);
}
void CSessionFactory::RegisterListener(char const* nameService, DWORD dwMark) {
	//创建Server
	CServiceName srvname(nameService);
	CServerBase* pServer = CNetworkFactory::GetInstance()->CreateServer(&srvname);
	if (NULL == pServer)
		return;

	//创建Listerer
	CSessionListener* pListener = new CSessionListener(m_pIOThread, this, pServer, dwMark);
	m_pIOThread->RegisterIOHandler(pListener);
	m_listenerSet.push_back(pListener);
}
void CSessionFactory::EnableConnecter(bool bEnable) {
	m_bConnecterEnable = bEnable;
	OnTimer(TIMER_ID_CONNECT);
}
void CSessionFactory::EnableListener(bool bEnable) {
	m_bListenerEnable = bEnable;
}
void CSessionFactory::SetConnectMode(bool bRandomConnect) {
	m_bRandomConnect = bRandomConnect;
}
CSession* CSessionFactory::GetSession(type::SessionID nSessionID) {
	auto iter = m_sessionMap.find(nSessionID);
	if (iter == m_sessionMap.end()) {
		return nullptr;
	}
	return iter->second;;
}

void CSessionFactory::OnChannelCreated(CChannel* pChannel, DWORD dwMark) {
	CSession* pSession = CreateSession(pChannel, dwMark);
	if (NULL != pSession) {
		OnSessionConnected(pSession);
		m_pIOThread->RegisterIOHandler(pSession);
		pSession->RegisterSessionCallback(this);
	}
}
void CSessionFactory::OnSessionConnected(CSession* pSession) {
	Log::Info(fmt::format("CSessionFactory: Session [{:08x}] connected, URL[{}]", pSession->GetSessionID(), pSession->GetChannel()->GetRemoteName()));
	m_sessionMap.emplace(pSession->GetSessionID(), pSession);
}
void CSessionFactory::OnSessionDisconnected(CSession* pSession, int nReason) {
	Log::Error(fmt::format("CSessionFactory: Session [{:08x}] disconnected, URL[{}] reason[{:08x}]", pSession->GetSessionID(), pSession->GetChannel()->GetRemoteName(), nReason)); 
	auto iter = m_sessionMap.find(pSession->GetSessionID());
	m_sessionMap.erase(iter);
	if (NULL != m_pConnecterManager) {
		m_pConnecterManager->OnDiscconnected(pSession->GetChannel());
	}
}
void CSessionFactory::OnSessionWarning(CSession* pSession, int nReason, int nParam) {
}

*/