#include "stdafx.h"
#include "ZtpFtdUserMDApiImpl.h"


CZtpFtdMdApi* CZtpFtdMdApi::CreateZtpFtdMdApi() {
// 	Log::Debug("Step1: CreateZtpFtdMdApi");
	return new CZtpFtdMdApiImpl();
}

CZtpFtdMdApiImpl::CZtpFtdMdApiImpl()
:m_pUserApi(CZtpFtdUserApi::CreateFtdUserApi()),m_pMdSpi(NULL) {
// 	Log::Debug("Step5: CZtpFtdMdApiImpl constructed");
	m_pUserApi->RegisterSpi(this);
}
void CZtpFtdMdApiImpl::Init() {
	m_pUserApi->Init();
}
void CZtpFtdMdApiImpl::Release() {
	m_pUserApi->Release();
	m_pUserApi = NULL;
	delete this;
}
void CZtpFtdMdApiImpl::RegisterSpi(CZtpFtdMdSpi* pSpi) {
	m_pMdSpi = pSpi;
}
void CZtpFtdMdApiImpl::RegisterFront(char* pszFrontAddress) {
	m_pUserApi->RegisterFront(pszFrontAddress);
}
void CZtpFtdMdApiImpl::SubscribeMarketTopic(int nTopicID) {
	m_pUserApi->SubscribeMarketTopic(nTopicID);
}
int CZtpFtdMdApiImpl::ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID) {
	return m_pUserApi->ReqUserLogin(pReqUserLogin, nRequestID);
}
int CZtpFtdMdApiImpl::ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID) {
	return m_pUserApi->ReqUserLogout(pReqUserLogout, nRequestID);
}
int CZtpFtdMdApiImpl::ReqSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID) {
	return m_pUserApi->ReqSubMarketData(pSpecificInstrument, nRequestID);
}
int CZtpFtdMdApiImpl::ReqUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID) {
	return m_pUserApi->ReqUnSubMarketData(pSpecificInstrument, nRequestID);
}


void CZtpFtdMdApiImpl::OnFrontConnected() {
	if(NULL != m_pMdSpi) {
		m_pMdSpi->OnFrontConnected();
	}
}
void CZtpFtdMdApiImpl::OnFrontDisconnected(int nReason) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnFrontDisconnected(nReason);
	}
}
void CZtpFtdMdApiImpl::OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnRspError(pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdMdApiImpl::OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnRspUserLogin(pRspUserLogin, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdMdApiImpl::OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnRspUserLogout(pRspUserLogout, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdMdApiImpl::OnRspSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnRspSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdMdApiImpl::OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnRspUnSubMarketData(pSpecificInstrument, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdMdApiImpl::OnRtnDepthMarketData(CZtpApiDepthMarketDataField* pDepthMarketDataField) {
	if (NULL != m_pMdSpi) {
		m_pMdSpi->OnRtnDepthMarketData(pDepthMarketDataField);
	}
}

