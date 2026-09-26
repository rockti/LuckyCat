#pragma once
#include "ZTP/ZTPUserApiStruct.h"

#ifdef ZTP_USER_API_EXPORT
#define USER_API_EXPORT __declspec(dllexport)
#else
#define USER_API_EXPORT __declspec(dllimport)
#endif

//所有用户接口都在这里定义，MDApi和TradeApi分别继承自本类
class CZtpFtdUserSpi
{
public:
	//当客户端建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected(){};

	//当客户端通信连接断开时，该方法被调用。	nReason： 2-连接错误 3-主动断开
	virtual void OnFrontDisconnected(int nReason){};

	//错误应答
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//用户登录应答
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//用户退出应答
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	virtual void OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pRspPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//报单录入应答
	virtual void OnRspOrderInsert(CZtpApiInputOrderField* pInputOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//报单操作应答
	virtual void OnRspOrderAction(CZtpApiInputOrderActionField* pOrderAction, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//报单录入错误回报（交易所返回）
	virtual void OnErrRtnOrderInsert(CZtpApiInputOrderField* pInputOrder, CZtpApiRspInfoField* pRspInfo) {};

	//报单操作错误回报（交易所返回）
	virtual void OnErrRtnOrderAction(CZtpApiInputOrderActionField* pOrderAction, CZtpApiRspInfoField* pRspInfo) {};

	//报单回报
	virtual void OnRtnOrder(CZtpApiOrderField* pOrder) {};

	//成交回报
	virtual void OnRtnTrade(CZtpApiTradeField* pTrade) {};

	//递延费率推送通知
	virtual void OnRtnDeferFee(CZtpApiRtnDeferFee* pDeferFeeField) {};

	//合约交易状态通知
	virtual void OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus) {};

	/********************************************************查询类请求********************************************************/
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

	/********************************************************行情类请求********************************************************/
	//订阅行情应答
	virtual void OnRspSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//退订行情应答
	virtual void OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};

	//深度行情通知
	virtual void OnRtnDepthMarketData(CZtpApiDepthMarketDataField* pDepthMarketData) {};
};

class USER_API_EXPORT CZtpFtdUserApi
{
public:
	static CZtpFtdUserApi* CreateFtdUserApi(char const* pszFlowPath="", char const* pszUserApiType="");

	/******************************Inherited by ZtpFtdUserApiImplBase******************************/
	//初始化
	virtual void Init() = 0;

	//删除接口对象本身
	virtual void Release() = 0;
	
	//获取当前交易日
	virtual char const* GetTradingDay() = 0;

	//注册回调接口
	virtual void RegisterSpi(CZtpFtdUserSpi* pSpi) = 0;

	//注册前置机网络地址
	virtual void RegisterFront(char* pszFrontAddress) = 0;

	//订阅公共流
	virtual void SubscribePublicTopic(TE_RESUME_TYPE nResumeType) = 0;

#ifdef OPERATER_DLL
	//订阅私有流
	virtual void SubscribePrivateTopic(TE_RESUME_TYPE nResumeType) = 0;
#else
	//订阅用户流
	virtual void SubscribeUserTopic(TE_RESUME_TYPE nResumeType) = 0;
#endif

	//订阅行情主题,然后才能订阅具体合约行情
	virtual void SubscribeMarketTopic(int nTopicID) = 0;

	//订阅合约行情。
	virtual int ReqSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID) = 0;

	//退订合约行情。
	virtual int ReqUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID) = 0;	

	//用户登录请求
	virtual int ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID) = 0;

	/******************************Inherited by ZtpFtdUserApiImpl******************************/
	//用户退出请求
	virtual int ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID) = 0;

	//用户退出请求
	virtual int ReqUserPasswordUpdate(CZtpApiPasswordUpdateField* pReqUserPasswordUpdate, int nRequestID) = 0;

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
	~CZtpFtdUserApi(){};
};
