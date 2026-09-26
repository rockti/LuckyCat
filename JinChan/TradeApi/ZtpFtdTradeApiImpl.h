#pragma once
#include "ZtpFtdUserApi.h"
#include "ZTP/ZTPUserTradeApi.h"

class CZtpFtdTradeApiImpl : public CZtpFtdTradeApi, public CZtpFtdUserSpi
{
public:
	CZtpFtdTradeApiImpl();

	virtual void Init();
	virtual void Release();
	virtual void RegisterSpi(CZtpFtdTradeSpi* pSpi);
	virtual void RegisterFront(char* pszFrontAddress);
	virtual void SubscribeUserTopic(TE_RESUME_TYPE nResumeType);
#ifdef OPERATER_DLL
	virtual void SubscribePrivateTopic(TE_RESUME_TYPE nResumeType);
#else
	virtual void SubscribePublicTopic(TE_RESUME_TYPE nResumeType);
#endif
	virtual int ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID);
	virtual int ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID);
	virtual int ReqUserPasswordUpdate(CZtpApiPasswordUpdateField* pReqUserPasswordUpdate, int nRequestID);
	virtual int ReqOrderInsert(CZtpApiInputOrderField* pInputOrder, int nRequestID);
	virtual int ReqOrderAction(CZtpApiInputOrderActionField* pOrderAction, int nRequestID);
	virtual int ReqQryInstrument(CZtpApiQryInstrumentField* pQryTopicInstrument, int nRequestID);
	virtual int ReqQryInvestorAccount(CZtpApiQryAccountField* pQryInvestorAccount, int nRequestID);
	virtual int ReqQryInvestorPosition(CZtpApiQryPositionField* pQryInvestorPosition, int nRequestID);
	virtual int ReqQryInvestorMargin(CZtpApiQryMarginField* pQryInvestorMargin, int nRequestID);
	virtual int ReqQryInvestorFee(CZtpApiQryFeeField* pQryInvestorFee, int nRequestID);
	virtual int ReqQryInvestorOrder(CZtpApiQryOrderField* pQryInvestorOrder, int nRequestID);
	virtual int ReqQryInvestorTrade(CZtpApiQryTradeField* pQryInvestorTrade, int nRequestID);

private:
	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
 	virtual void OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnErrRtnOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo);
	virtual void OnErrRtnOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo);
	virtual void OnRtnOrder(CZtpApiOrderField* pOrderField);
	virtual void OnRtnTrade(CZtpApiTradeField* pTradeField);
	virtual void OnRtnDeferFee(CZtpApiRtnDeferFee* pDeferFeeField);
	virtual void OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus);
	virtual void OnRspQryInstrument(CZtpApiRspInstrumentField* pRspInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorAccount(CZtpApiRspAccountField* pRspAccount, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorPosition(CZtpApiRspPositionField* pRspPosition, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorMargin(CZtpApiRspMarginField* pRspMargin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorFee(CZtpApiRspFeeField* pRspFee, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorOrder(CZtpApiOrderField* pOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorTrade(CZtpApiTradeField* pTrade, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
private:
	CZtpFtdUserApi* m_pUserApi;
	CZtpFtdTradeSpi* m_pTradeSpi;
};
