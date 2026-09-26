#pragma once

#pragma warning(push)
#pragma warning(disable:4100)
#include "thirdapi/ZTP/ZTPUserTradeApi.h"
#pragma warning(pop)

#include "model/Instrument.h"
#include "conn/ExchangeConn.h"
#include "notify/Notify.h"

struct TradeSystemConfig;
class ConnPublisher;
struct PositionNotification;
class ZTPTraderHandler : public CZtpFtdTradeSpi
{
public:
	ZTPTraderHandler(TradeSystemConfig const& config, ConnPublisher& publisher);

	//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected();

	//当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	//0x1001 网络读失败	0x1002 网络写失败		0x2001 接收心跳超时		0x2002 发送心跳失败		0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);

	//错误应答
	virtual void OnRspError(CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//登录应答
	virtual void OnRspUserLogin(CZtpApiRspLoginField* pRspUserLogin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//录出应答
	virtual void OnRspUserLogout(CZtpApiRspLogoutField* pRspUserLogout, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspUserPasswordUpdate(CZtpApiPasswordUpdateField* pRspUserPasswordUpdate, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast) override;
	//委托应答
	virtual void OnRspOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//撤单应答，只有撤单错误会调用，撤单成功在报单回报中体现
	virtual void OnRspOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//报单录入错误回报
	virtual void OnErrRtnOrderInsert(CZtpApiInputOrderField* pRspOrder, CZtpApiRspInfoField* pRspInfo);

	//报单操作错误回报
	virtual void OnErrRtnOrderAction(CZtpApiInputOrderActionField* pRspAction, CZtpApiRspInfoField* pRspInfo);

	//报单回报
	virtual void OnRtnOrder(CZtpApiOrderField* pOrderField);

	//成交回报
	virtual void OnRtnTrade(CZtpApiTradeField* pTradeField);

	//递延费率推送
	virtual void OnRtnDeferFee(CZtpApiRtnDeferFee* pDeferFeeField) {};

	//合约交易状态推送
	virtual void OnRtnInstrumentStatus(TZtpApiInstrumentStatusType* pInstrumentStatus);

	//查询合约应答
	virtual void OnRspQryInstrument(CZtpApiRspInstrumentField* pRspInstrument, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//投资者资金账户查询应答
	virtual void OnRspQryInvestorAccount(CZtpApiRspAccountField* pRspAccount, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//投资者持仓查询应答
	virtual void OnRspQryInvestorPosition(CZtpApiRspPositionField* pRspPosition, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//投资者保证金率查询请求
	virtual void OnRspQryInvestorMargin(CZtpApiRspMarginField* pRspMargin, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//投资者手续费率查询请求
	virtual void OnRspQryInvestorFee(CZtpApiRspFeeField* pRspFee, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//投资者报单查询应答
	virtual void OnRspQryInvestorOrder(CZtpApiOrderField* pOrder, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	//投资者成交查询应答
	virtual void OnRspQryInvestorTrade(CZtpApiTradeField* pTrade, CZtpApiRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	void SetTraderApi(CZtpFtdTradeApi* pTraderApi) { m_pTraderApi = pTraderApi; }
	void SetInvestor(type::InvestorID const& investor) { m_investor = investor; }

private:

	void QueryTradingAccount();		//查询账户资金
	void QueryPosition();			//查询持仓
	void QueryTrade();				//查询成交
	void QueryOrder();				//查询委托

	void FillingPosition(CZtpApiRspPositionField const& position, InvestorPosition& notification);

	bool IsErrorRspInfo(CZtpApiRspInfoField* pRspInfo);

private:
	TradeSystemConfig const& m_config;
	ConnPublisher& m_publisher;

	CZtpFtdTradeApi* m_pTraderApi;
	type::InvestorID m_investor;
};