#pragma once
#include "ZtpFtdUserApiImplBase.h"

//业务层面的指令都在这里完成
class CZtpFtdUserApiImpl : public CFtdUserApiImplBase
{
public:
	CZtpFtdUserApiImpl(char const* pszFlowPath, char const* pszUserApiType, CSelectThread* pReactor);

	virtual void HandleResponse(CFTDPackage* pMessage, WORD nSequenceSeries);

	//用户退出请求
	virtual int ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID);

	virtual int ReqUserPasswordUpdate(CZtpApiPasswordUpdateField* pReqUserPasswordUpdate, int nRequestID);

	//报单录入请求
	virtual int ReqOrderInsert(CZtpApiInputOrderField* pInputOrder, int nRequestID);
	//报单操作请求
	virtual int ReqOrderAction(CZtpApiInputOrderActionField* pOrderAction, int nRequestID);
	
	//合约查询请求(查询交易所下的所有合约信息)
	virtual int ReqQryInstrument(CZtpApiQryInstrumentField* pQryTopicInstrument, int nRequestID);
	//资金账户查询请求
	virtual int ReqQryInvestorAccount(CZtpApiQryAccountField* pQryInvestorAccount, int nRequestID);
	//客户持仓查询请求
	virtual int ReqQryInvestorPosition(CZtpApiQryPositionField* pQryInvestorPosition, int nRequestID);
	//保证金率查询请求
	virtual int ReqQryInvestorMargin(CZtpApiQryMarginField* pQryInvestorMargin, int nRequestID);
	//手续费率查询请求
	virtual int ReqQryInvestorFee(CZtpApiQryFeeField* pQryInvestorFee, int nRequestID);
	
	//报单查询请求(可根据指定报单号和指定合约进行索引查询)
	virtual int ReqQryInvestorOrder(CZtpApiQryOrderField* pQryOrder, int nRequestID);
	//成交单查询请求
	virtual int ReqQryInvestorTrade(CZtpApiQryTradeField* pQryTrade, int nRequestID);

private:
	//用户错误应答
	void OnRspError(CFTDPackage* pMessage, WORD nSequenceSeries);
	//用户登录应答
	void OnRspUserLogin(CFTDPackage* pMessage, WORD nSequenceSeries);
	//用户退出应答
	void OnRspUserLogout(CFTDPackage* pMessage, WORD nSequenceSeries);

	/****************************************TradeApi****************************************/
	//用户密码修改应答
 	void OnRspUserPasswordUpdate(CFTDPackage* pMessage, WORD nSequenceSeries);
	//查询合约应答
	void OnRspQryInstrument(CFTDPackage* pMessage, WORD nSequenceSeries);
	//客户资金账户查询应答
	void OnRspQryInvestorAccount(CFTDPackage* pMessage, WORD nSequenceSeries);
	//客户持仓查询应答
	void OnRspQryInvestorPosition(CFTDPackage* pMessage, WORD nSequenceSeries);
	//客户保证金率查询请求
	void OnRspQryInvestorMargin(CFTDPackage* pMessage, WORD nSequenceSeries);
	//客户手续费率查询请求
	void OnRspQryInvestorFee(CFTDPackage* pMessage, WORD nSequenceSeries);

	//报单录入应答
	void OnRspOrderInsert(CFTDPackage* pMessage, WORD nSequenceSeries);
	//报单操作应答
	void OnRspOrderAction(CFTDPackage* pMessage, WORD nSequenceSeries);
	//报单录入错误回报
	void OnErrRtnOrderInsert(CFTDPackage* pMessage, WORD nSequenceSeries);
	//报单操作错误回报
	void OnErrRtnOrderAction(CFTDPackage* pMessage, WORD nSequenceSeries);
	//报单回报
	void OnRtnOrder(CFTDPackage* pMessage, WORD nSequenceSeries);
	//成交回报
	void OnRtnTrade(CFTDPackage* pMessage, WORD nSequenceSeries);
	//合约交易状态通知
	void OnRtnInstrumentStatus(CFTDPackage* pMessage, WORD nSequenceSeries);
	//递延费通知
	void OnRtnDeferFee(CFTDPackage* pMessage, WORD nSequenceSeries);

	//报单查询应答
	void OnRspQryOrder(CFTDPackage* pMessage, WORD nSequenceSeries);
	//成交单查询应答
	void OnRspQryTrade(CFTDPackage* pMessage, WORD nSequenceSeries);

	/****************************************MDApi****************************************/
	//订阅合约的相关信息
	void OnRspSubMarketData(CFTDPackage* pMessage, WORD nSequenceSeries);
	//退订合约的相关信息
	void OnRspUnSubMarketData(CFTDPackage* pMessage, WORD nSequenceSeries);
	//深度行情通知
	void OnRtnDepthMarketData(CFTDPackage* pMessage, WORD nSequenceSeries);
};