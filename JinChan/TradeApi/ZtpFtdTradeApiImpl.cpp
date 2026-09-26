#include "stdafx.h"
#include "ZtpFtdTradeApiImpl.h"

CZtpFtdTradeApi* CZtpFtdTradeApi::CreateZtpFtdTradeApi() {
// 	Log::Debug("Step1: CreateZtpFtdTradeApi");
	return new CZtpFtdTradeApiImpl();
}

CZtpFtdTradeApiImpl::CZtpFtdTradeApiImpl()
:m_pUserApi(CZtpFtdUserApi::CreateFtdUserApi()),m_pTradeSpi(NULL) {
// 	Log::Debug("Step5: CZtpFtdTradeApiImpl constructed");
	m_pUserApi->RegisterSpi(this);
}
void CZtpFtdTradeApiImpl::Init() {
	m_pUserApi->Init();
}
void CZtpFtdTradeApiImpl::Release() {
	m_pUserApi->Release();
	m_pUserApi = NULL;
	delete this;
}
void CZtpFtdTradeApiImpl::RegisterSpi(CZtpFtdTradeSpi* pSpi) {
	m_pTradeSpi = pSpi;
}
void CZtpFtdTradeApiImpl::RegisterFront(char* pszFrontAddress) {
	m_pUserApi->RegisterFront(pszFrontAddress);
}
#ifdef OPERATER_DLL
void CZtpFtdTradeApiImpl::SubscribePrivateTopic(TE_RESUME_TYPE nResumeType) {
	m_pUserApi->SubscribePrivateTopic(nResumeType);
}
#else
void CZtpFtdTradeApiImpl::SubscribeUserTopic(TE_RESUME_TYPE nResumeType) {
	m_pUserApi->SubscribeUserTopic(nResumeType);
}
#endif
void CZtpFtdTradeApiImpl::SubscribePublicTopic(TE_RESUME_TYPE nResumeType)
{
	m_pUserApi->SubscribePublicTopic(nResumeType);
}

int CZtpFtdTradeApiImpl::ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID)
{
	return m_pUserApi->ReqUserLogin(pReqUserLogin, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID)
{
	return m_pUserApi->ReqUserLogout(pReqUserLogout, nRequestID);
}

int CZtpFtdTradeApiImpl::ReqUserPasswordUpdate(CZtpApiPasswordUpdateField* pReqUserPasswordUpdate, int nRequestID)
{
	return m_pUserApi->ReqUserPasswordUpdate(pReqUserPasswordUpdate, nRequestID);
}

int CZtpFtdTradeApiImpl::ReqOrderInsert(CZtpApiInputOrderField* pInputOrder, int nRequestID)
{
	return m_pUserApi->ReqOrderInsert(pInputOrder, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqOrderAction(CZtpApiInputOrderActionField* pOrderAction, int nRequestID)
{
	return m_pUserApi->ReqOrderAction(pOrderAction, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInstrument(CZtpApiQryInstrumentField* pQryTopicInstrument, int nRequestID)
{
	return m_pUserApi->ReqQryInstrument(pQryTopicInstrument, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInvestorAccount(CZtpApiQryAccountField* pQryInvestorAccount, int nRequestID)
{
	return m_pUserApi->ReqQryInvestorAccount(pQryInvestorAccount, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInvestorPosition(CZtpApiQryPositionField* pQryInvestorPosition, int nRequestID)
{
	return m_pUserApi->ReqQryInvestorPosition(pQryInvestorPosition, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInvestorMargin(CZtpApiQryMarginField* pQryInvestorMargin, int nRequestID)
{
	return m_pUserApi->ReqQryInvestorMargin(pQryInvestorMargin, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInvestorFee(CZtpApiQryFeeField* pQryInvestorFee, int nRequestID)
{
	return m_pUserApi->ReqQryInvestorFee(pQryInvestorFee, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInvestorOrder(CZtpApiQryOrderField* pQryInvestorOrder, int nRequestID)
{
	return m_pUserApi->ReqQryInvestorOrder(pQryInvestorOrder, nRequestID);
}
int CZtpFtdTradeApiImpl::ReqQryInvestorTrade(CZtpApiQryTradeField* pQryInvestorTrade, int nRequestID)
{
	return m_pUserApi->ReqQryInvestorTrade(pQryInvestorTrade, nRequestID);
}


void CZtpFtdTradeApiImpl::OnFrontConnected()
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnFrontConnected();
	}
}
void CZtpFtdTradeApiImpl::OnFrontDisconnected(int nReason)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnFrontDisconnected(nReason);
	}
}
void CZtpFtdTradeApiImpl::OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspError(pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspUserLogin(pRspUserLogin, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspUserLogout(pRspUserLogout, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspUserPasswordUpdate(pPasswordUpdate, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspOrderInsert(pRspOrder, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspOrderAction(pRspAction, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnErrRtnOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnErrRtnOrderInsert(pRspOrder, pRspInfo);
	}
}
void CZtpFtdTradeApiImpl::OnErrRtnOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnErrRtnOrderAction(pRspAction, pRspInfo);
	}
}
void CZtpFtdTradeApiImpl::OnRtnOrder(CZtpApiOrderField* pOrderField)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRtnOrder(pOrderField);
	}
}
void CZtpFtdTradeApiImpl::OnRtnTrade(CZtpApiTradeField* pTradeField)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRtnTrade(pTradeField);
	}
}
void CZtpFtdTradeApiImpl::OnRtnDeferFee(CZtpApiRtnDeferFee* pDeferFeeField)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRtnDeferFee(pDeferFeeField);
	}
}
void CZtpFtdTradeApiImpl::OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRtnInstrumentStatus(pInstrumentStatus);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInstrument(CZtpApiRspInstrumentField* pRspInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInstrument(pRspInstrument, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInvestorAccount(CZtpApiRspAccountField* pRspAccount, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInvestorAccount(pRspAccount, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInvestorPosition(CZtpApiRspPositionField* pRspPosition, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInvestorPosition(pRspPosition, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInvestorMargin(CZtpApiRspMarginField* pRspMargin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInvestorMargin(pRspMargin, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInvestorFee(CZtpApiRspFeeField* pRspFee, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInvestorFee(pRspFee, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInvestorOrder(CZtpApiOrderField* pOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInvestorOrder(pOrder, pRspInfo, nRequestID, bIsLast);
	}
}
void CZtpFtdTradeApiImpl::OnRspQryInvestorTrade(CZtpApiTradeField* pTrade, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
	if(NULL != m_pTradeSpi)
	{
		m_pTradeSpi->OnRspQryInvestorTrade(pTrade, pRspInfo, nRequestID, bIsLast);
	}
}