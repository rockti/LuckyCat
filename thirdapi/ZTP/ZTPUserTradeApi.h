#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZTPUserApiStruct.h"

#ifdef ZTP_TRADE_API_EXPORT
#define TRADE_API_EXPORT __declspec(dllexport)
#else
#define TRADE_API_EXPORT __declspec(dllimport)
#endif

class CZtpFtdTradeSpi	
{
public:
	//当客户端建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected()	{};

	//当客户端通信连接断开时，该方法被调用。	nReason： 2-连接错误 3-主动断开
	virtual void OnFrontDisconnected(int nReason) {};

	//错误应答
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//登录应答
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//录出应答
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	virtual void OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pRspUserPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//委托应答
	virtual void OnRspOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//撤单应答，只有撤单错误会调用，撤单成功在报单回报中体现
	virtual void OnRspOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//报单录入错误回报
	virtual void OnErrRtnOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo) {};

	//报单操作错误回报
	virtual void OnErrRtnOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo) {};

	//报单回报
	virtual void OnRtnOrder(CZtpApiOrderField* pOrderField) {};

	//成交回报
	virtual void OnRtnTrade(CZtpApiTradeField* pTradeField) {};

	//递延费率推送
	virtual void OnRtnDeferFee(CZtpApiRtnDeferFee* pDeferFeeField) {};

	//合约交易状态推送
	virtual void OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus)	{};

	//查询合约应答
	virtual void OnRspQryInstrument(CZtpApiRspInstrumentField* pRspInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//投资者资金账户查询应答
	virtual void OnRspQryInvestorAccount(CZtpApiRspAccountField* pRspAccount, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//投资者持仓查询应答
	virtual void OnRspQryInvestorPosition(CZtpApiRspPositionField* pRspPosition, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//投资者保证金率查询请求
	virtual void OnRspQryInvestorMargin(CZtpApiRspMarginField* pRspMargin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//投资者手续费率查询请求
	virtual void OnRspQryInvestorFee(CZtpApiRspFeeField* pRspFee, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//投资者报单查询应答
	virtual void OnRspQryInvestorOrder(CZtpApiOrderField* pOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//投资者成交查询应答
	virtual void OnRspQryInvestorTrade(CZtpApiTradeField* pTrade, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};
};

class TRADE_API_EXPORT CZtpFtdTradeApi
{
public:
	static CZtpFtdTradeApi* CreateZtpFtdTradeApi();

	//初始化接口对象
	virtual void Init() = 0;

	//删除接口对象
	virtual void Release() = 0;

	//注册回调接口
	virtual void RegisterSpi(CZtpFtdTradeSpi* pSpi) = 0;

	//注册前置地址
	virtual void RegisterFront(char* pszFrontAddress) = 0;

#ifdef OPERATER_DLL
	//订阅私有流(交易员用)
	virtual void SubscribePrivateTopic(TE_RESUME_TYPE nResumeType) = 0;
#else
	//订阅用户流(投资者用)
	virtual void SubscribeUserTopic(TE_RESUME_TYPE nResumeType) = 0;
#endif // OPERATER_DLL

	//订阅公共流
	virtual void SubscribePublicTopic(TE_RESUME_TYPE nResumeType) = 0;

	//登录请求
	virtual int ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID) = 0;

	//登出请求
	virtual int ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID) = 0;

	//登出请求
	virtual int ReqUserPasswordUpdate(CZtpApiPasswordUpdateField* pReqPasswordUpdate, int nRequestID) = 0;

	//报单录入请求
	virtual int ReqOrderInsert(CZtpApiInputOrderField* pInputOrder, int nRequestID) = 0;

	//报单操作请求
	virtual int ReqOrderAction(CZtpApiInputOrderActionField* pOrderAction, int nRequestID) = 0;

	//查询合约信息
	virtual int ReqQryInstrument(CZtpApiQryInstrumentField* pQryTopicInstrument, int nRequestID) = 0;

	//投资者资金账户查询请求
	virtual int ReqQryInvestorAccount(CZtpApiQryAccountField* pQryInvestorAccount, int nRequestID) = 0;

	//投资者持仓查询请求
	virtual int ReqQryInvestorPosition(CZtpApiQryPositionField* pQryInvestorPosition, int nRequestID) = 0;

	//投资者保证金率查询请求
	virtual int ReqQryInvestorMargin(CZtpApiQryMarginField* pQryInvestorMargin, int nRequestID) = 0;

	//投资者手续费率查询请求
	virtual int ReqQryInvestorFee(CZtpApiQryFeeField* pQryInvestorFee, int nRequestID) = 0;

	//报单查询请求(可根据指定报单号或指定合约进行索引查询)
	virtual int ReqQryInvestorOrder(CZtpApiQryOrderField* pQryInvestorOrder, int nRequestID) = 0;

	//成交单查询请求
	virtual int ReqQryInvestorTrade(CZtpApiQryTradeField* pQryInvestorTrade, int nRequestID) = 0;

protected:
	~CZtpFtdTradeApi(){};
};