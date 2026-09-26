#include "stdafx.h"
#include "FrontEngine.h"
#include "CachedFlow.h"
#include <fmt/format.h>


const int SESSION_CHECK_TIMER_ID = 9;
const int SESSION_CHECK_INTERVAL = 1000;
const int MAX_COMM_FLUX	= 50;	//每SESSION_CHECK_INTERVAL的最大通讯量

CFrontEngine::CFrontEngine(CReactorThread* pReactor, CFrontForwarder* pFrontForwarder)
:CServerSessionFactory(pReactor,500),m_pFrontForwarder(pFrontForwarder),m_maxCommFlux(MAX_COMM_FLUX)
{
	m_reqPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
	m_pPublicFlow = new CCachedFlow(false, 100000, 100000);
	m_pReturnFlow = new CCachedFlow(false, 10000000, 100000);

	//暂时不设置流控
//	SetTimer(SESSION_CHECK_TIMER_ID, SESSION_CHECK_INTERVAL);
}

CFrontEngine::~CFrontEngine() {
	ClearAllMap();
	delete m_pReturnFlow;
	delete m_pPublicFlow;
}

void CFrontEngine::ClearAllMap() {
	m_sessionInfoMap.clear();
}
void CFrontEngine::OnTimer(int nIDEvent)
{
	if (nIDEvent == SESSION_CHECK_TIMER_ID) {
		for (auto& kv : m_sessionMap) {
			auto iter = m_sessionInfoMap.find(kv.second->GetSessionID());
			if (iter != m_sessionInfoMap.end()) {
				CheckSession(kv.second, &iter->second);
			}
		}
	}
}

void CFrontEngine::OnSessionConnected(CSession* pSession) {
	//先调用基类的方法，更新SessionMap
	CServerSessionFactory::OnSessionConnected(pSession);

	TFrontSessionInfo sessionInfo = {0};
	sessionInfo.pSession = pSession;
	sessionInfo.nCommFlux = 0;
	sessionInfo.bIsLogin = false;
	sessionInfo.nConnectTime = m_pIOThread->Time();
	sessionInfo.nMaxCommFlux = m_maxCommFlux;
	m_sessionInfoMap.emplace(sessionInfo.pSession->GetSessionID(), sessionInfo);
}
void CFrontEngine::OnSessionDisconnected(CSession* pSession, int nReason) {
	CLoginIDType LoginID;
	CBrokerIDType BrokerID;
	if (GetSessionOwner(pSession->GetSessionID(), LoginID, BrokerID)) {
		//如果存在该会话的信息，则将其删除
		m_sessionInfoMap.erase(m_sessionInfoMap.find(pSession->GetSessionID()));

		//向交易引擎发出该交易员的登出请求，否则交易引擎无法得知其已断线，该交易员就再也无法登录了		
		m_pFrontForwarder->ForwardHead(&m_reqPackage, TID_ReqUserLogout, pSession->GetSessionID(), 0);
		data::CUserLogout userLogoutField;
		userLogoutField.LoginID = LoginID;
		userLogoutField.BrokerID = BrokerID;
		m_reqPackage.AddField(userLogoutField);
		m_pFrontForwarder->ForwardTransaction(&m_reqPackage);
		Log::Debug(fmt::format("CFrontEngine: Session[{}] disconnected, send TID_ReqUserLogout", pSession->GetSessionID()));
	}

	//最后调用基类的方法
	CServerSessionFactory::OnSessionDisconnected(pSession, nReason);
}

TFrontSessionInfo* CFrontEngine::GetSessionInfo(DWORD nSessionID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter == m_sessionInfoMap.end()) {
		return nullptr;
	}
	return &iter->second;
}
bool CFrontEngine::IsSessionLogin(DWORD nSessionID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	return (iter != m_sessionInfoMap.end()) ? iter->second.bIsLogin : false;
}
void CFrontEngine::SetSessionOwner(DWORD const nSessionID, CReadOnlyLoginIDType& LoginID, CUserTypeType const& userType, CReadOnlyBrokerIDType& brokerID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter != m_sessionInfoMap.end()) {
		iter->second.bIsLogin = true;
		iter->second.LoginID = LoginID;
		iter->second.UserType = userType;
		iter->second.BrokerID = brokerID;
	}
	CSession* pSession = GetSession(nSessionID);
	assert(NULL != pSession);	//强制验证一下基类Session存在
}
bool CFrontEngine::GetSessionOwner(DWORD nSessionID, CLoginIDType& loginID, CBrokerIDType& brokerID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter != m_sessionInfoMap.end()) {
		loginID = iter->second.LoginID;
		brokerID = iter->second.BrokerID;
		return true;
	}
	return false;
}
void CFrontEngine::ClearSessionOwner(DWORD const nSessionID) {
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter != m_sessionInfoMap.end()) {
		iter->second.bIsLogin = false;
		iter->second.LoginID.clear();
		iter->second.BrokerID.clear();
	}
}
bool CFrontEngine::SetSessionMaxCommFlux(DWORD nSessionID, int nMaxCommFlux) {
	assert(nMaxCommFlux > 0);
	auto iter = m_sessionInfoMap.find(nSessionID);
	if (iter != m_sessionInfoMap.end()) {
		iter->second.nMaxCommFlux = nMaxCommFlux;
		return true;
	}
	return false;
}
void CFrontEngine::CheckSession(CSession* pSession, TFrontSessionInfo* pSessionInfo) {
	//重新开始计算流量
	pSessionInfo->nCommFlux -= pSessionInfo->nMaxCommFlux;
	if (pSessionInfo->nCommFlux < 0) {
		pSessionInfo->nCommFlux = 0;
	}

	//若流量超过规定，则挂起该会话的读操作
	pSession->SuspendRead(pSessionInfo->nCommFlux >= pSessionInfo->nMaxCommFlux);
}
void CFrontEngine::CalcFlux(CSession* pSession, const int nFlux) {
	auto iter = m_sessionInfoMap.find(pSession->GetSessionID());
	if (iter != m_sessionInfoMap.end()) {
		auto& session = iter->second;
		session.nCommFlux++; //流量控制改为计数
		if (session.nCommFlux >= session.nMaxCommFlux) {
			pSession->SuspendRead(true);
		}
	}
}

void CFrontEngine::SendToPublicFlow(CPackage* pPackage) {
	pPackage->MakePackage();
	m_pPublicFlow->Append(pPackage->GetAddress(), pPackage->GetLength());
}
int CFrontEngine::AppendToReturnFlow(CPackage* pPackage) {
	pPackage->MakePackage();
	return m_pReturnFlow->Append(pPackage->GetAddress(), pPackage->GetLength());
}

