#pragma once
#include "ZtpFtdUserApi.h"
#include "ZTP/ZTPUserMDApi.h"

class CZtpFtdMdApiImpl : public CZtpFtdMdApi, public CZtpFtdUserSpi
{
public:
	CZtpFtdMdApiImpl();

	virtual void Init();
	virtual void Release();
	virtual void RegisterSpi(CZtpFtdMdSpi* pSpi);
	virtual void RegisterFront(char* pszFrontAddress);
	virtual void SubscribeMarketTopic(int nTopicID);

	virtual int ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID);
	virtual int ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID);
	virtual int ReqSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID);
	virtual int ReqUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID);

private:
	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRtnDepthMarketData(CZtpApiDepthMarketDataField* pDepthMarketDataField);	

private:
	CZtpFtdUserApi* m_pUserApi;
	CZtpFtdMdSpi* m_pMdSpi;
};
