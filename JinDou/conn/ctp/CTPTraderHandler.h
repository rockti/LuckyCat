#pragma once

#pragma warning(push)
#pragma warning(disable:4100)
#include "thirdapi/CTP/ThostFtdcTraderApi.h"
#pragma warning(pop)

#include "model/Instrument.h"
#include "conn/ExchangeConn.h"

struct TradeSystemConfig;
class ConnPublisher;
struct PositionNotification;
class CTPTraderHandler : public CThostFtdcTraderSpi {
public:
	CTPTraderHandler(TradeSystemConfig const& config, ConnPublisher& publisher);

	//当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected();

	//当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	//0x1001 网络读失败	0x1002 网络写失败		0x2001 接收心跳超时		0x2002 发送心跳失败		0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);

	//客户端认证响应
	void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//登出请求响应
	void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//报单录入请求响应
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//报单操作请求响应
	void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//投资者结算结果确认响应
	void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//请求查询交易编码响应
	void OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	//请求查询资金账户响应
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//请求查询投资者持仓响应
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//请求查询报单响应
	void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//请求查询成交响应
	void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//错误应答
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	//报单通知
	void OnRtnOrder(CThostFtdcOrderField *pOrder);

	//成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade);

	//报单录入错误回报
	void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

	//报单操作错误回报
	void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

	//合约交易状态通知
	void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);

	virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///请求查询合约保证金率响应
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///请求查询合约手续费率响应
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	void SetTraderApi(CThostFtdcTraderApi* pTraderApi) { m_pTraderApi = pTraderApi; }
	void SetTraderUrl(std::string const& traderUrl) { m_traderUrl = traderUrl; }

	void SetLogin(type::InvestorID const& investor, type::Password const& password) {
		m_investor = investor;
		m_password = password;
	}

	void SetQueryFeeInstrument(int requestID, type::InstrumentCode const& instCode) { m_queryFeeMapping[requestID] = instCode; }

private:
	void SettlementInfoConfirm();	//确认结算单
	void FillingPositionNotification(CThostFtdcInvestorPositionField const& position, InvestorPosition& notification, type::InstID instID);

	bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo);
	bool IsOrdered(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsSendFailed(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsCanceled(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsCancelFailed(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsPartial(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsAllTraded(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsAccepted(CThostFtdcOrderField const& order, bool isEmpty) const;
	bool IsFakOrder(CThostFtdcOrderField const& order, bool isEmpty) const;
	
private:
	TradeSystemConfig const& m_config;
	ConnPublisher& m_publisher;

	CThostFtdcTraderApi* m_pTraderApi;
	std::map<type::Exchange, ExchangeStatus> m_exchangeStatusMap;

	std::string m_traderUrl;

	type::InvestorID m_investor;
	type::Password m_password;
	std::map<int, type::InstrumentCode> m_queryFeeMapping;
};