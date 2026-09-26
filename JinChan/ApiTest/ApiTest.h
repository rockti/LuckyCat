#pragma once

#include "ZTP/ZtpUserMDApi.h"
#include "ztp/ZTPUserTradeApi.h"
#include <string_view>


class TestMdSpi : public CZtpFtdMdSpi	
{
public:
	TestMdSpi();
	virtual void OnFrontConnected();;
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRtnDepthMarketData(CZtpApiDepthMarketDataField* pDepthMarketDataField);	
private:
	int m_requestID = 0;
};
class COrderField;
class TestTradeSpi: public CZtpFtdTradeSpi
{
public:
	TestTradeSpi();
	virtual void OnFrontConnected();
	virtual void OnFrontDisconnected(int nReason);
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pRspPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnErrRtnOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo);
	virtual void OnErrRtnOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo);
	virtual void OnRtnOrder(CZtpApiOrderField* pOrderField);
	virtual void OnRtnTrade(CZtpApiTradeField* pTradeField);
	virtual void OnRtnDeferFee(CZtpApiRtnDeferFee* pDeferFeeField) override;
	virtual void OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus);
	virtual void OnRspQryInstrument(CZtpApiRspInstrumentField* pRspInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorAccount(CZtpApiRspAccountField* pRspAccount, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorPosition(CZtpApiRspPositionField* pRspPosition, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorMargin(CZtpApiRspMarginField* pRspMargin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorFee(CZtpApiRspFeeField* pRspFee, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorOrder(CZtpApiOrderField* pOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorTrade(CZtpApiTradeField * pTrade, CZtpApiRspInfoField * pRspInfo, int nRequestID, bool bIsLast);
	void logErrRtnAction(CZtpApiRspInfoField* pRspInfo);
	void logRtnOrder(CZtpApiOrderField* pOrderField);
	void logRtnTrade(CZtpApiTradeField* pTradeField);
	void logRtnInst(TZtpApiInstrumentStatusType* pInstrumentStatus);
	void logQryInst(CZtpApiRspInstrumentField* pRspInstrument, int nRequestID, CZtpApiRspInfoField* pRspInfo, bool bIsLast);
	void logQryAccount(CZtpApiRspAccountField* pRspAccount, int nRequestID, CZtpApiRspInfoField* pRspInfo);
	void logQryPosition(CZtpApiRspPositionField* pRspPosition, int nRequestID, CZtpApiRspInfoField* pRspInfo, bool bIsLast);
	void logQryMargin(CZtpApiRspMarginField* pRspMargin, int nRequestID, CZtpApiRspInfoField* pRspInfo);
	void logQryFee(CZtpApiRspFeeField* pRspFee, int nRequestID, CZtpApiRspInfoField* pRspInfo);
	void logQryOrder(CZtpApiOrderField* pOrderField, int nRequestID, CZtpApiRspInfoField* pRspInfo, bool bIsLast);
	void logQryTrade(CZtpApiTradeField* pTradeField, int nRequestID, CZtpApiRspInfoField* pRspInfo);

	char const* GetNextLocalID(int cao = 0) const;

	void Login();
	void Logout();

	void InsertOrder(char const* instCode, char const dir, char const offsetFlag, int quantity, double price, char const orderType);
	void CancelOrder(CZtpApiOrderField* pOrderField);
	void QueryAccount();
	void QueryPositions();
	void QueryInstrument();
	void QueryMargin();
	void QueryFee();
	void QueryOrder();
	void QueryTrade(char const* sysID = nullptr);
	void ChangePassword();
private:
	void logConnected();
	void logDisconnected(int nReason);
	void logRspError(CZtpApiRspInfoField* pRspInfo);
	void logLogin(int nRequestID, CZtpApiRspInfoField* pRspInfo, CZtpApiRspLoginField* pRspUserLogin);
	void logLogout(int nRequestID, CZtpApiRspInfoField* pRspInfo);
	void logRspOrderInsert(CZtpApiRspInfoField* pRspInfo, int nRequestID, CZtpApiInputOrderField* pRspOrder);
	void logRspAction(CZtpApiRspInfoField* pRspInfo, int nRequestID, CZtpApiInputOrderActionField* pRspAction);
	void logErrRtnOrder(CZtpApiRspInfoField* pRspInfo);

	using LocalID = int;
	std::map<LocalID, CZtpApiInputOrderField> m_unfinisedOrders;

	int m_nRtn = 0;
	int m_count = 0;
	int m_requestId = 1;
	string m_orderSysID;
};
