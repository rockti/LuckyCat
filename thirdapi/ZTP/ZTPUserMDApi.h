#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZTPUserApiStruct.h"

#ifdef ZTP_MD_API_EXPORT
#define ZTP_MD_API_EXPORT __declspec(dllexport)
#else
#define ZTP_MD_API_EXPORT __declspec(dllimport)
#endif

class CZtpFtdMdSpi	
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

	//订阅行情应答，返回订阅的合约代码
	virtual void OnRspSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};
		
	//退订行情应答，返回退订的合约代码
	virtual void OnRspUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) {};
	
	//深度行情变化通知
	virtual void OnRtnDepthMarketData(CZtpApiDepthMarketDataField* pDepthMarketDataField) {};	
};

class ZTP_MD_API_EXPORT CZtpFtdMdApi
{
public:
	static CZtpFtdMdApi* CreateZtpFtdMdApi();

	//初始化接口对象
	virtual void Init() = 0;

	//删除接口对象
	virtual void Release() = 0;

	//注册回调接口
	virtual void RegisterSpi(CZtpFtdMdSpi* pSpi) = 0;

	//注册前置地址
	virtual void RegisterFront(char* pszFrontAddress) = 0;

	//订阅主题
	virtual void SubscribeMarketTopic(int nTopicID) = 0;

	//登录请求
	virtual int ReqUserLogin(CZtpApiReqLoginField* pReqUserLogin, int nRequestID) = 0;

	//登出请求
	virtual int ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID) = 0;

	//订阅行情，一次只能订阅一个行情
	virtual int ReqSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID) = 0;

	//退订行情，一次只能退订一个行情 
	virtual int ReqUnSubMarketData(CZtpApiSpecificInstrumentField* pSpecificInstrument, int nRequestID) = 0;

protected:
	~CZtpFtdMdApi(){};
};
