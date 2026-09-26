#pragma once

#include "ESUNNY/iTapAPICommDef.h"
#include "ESUNNY/iTapAPIError.h"
#include "ESUNNY/iTapTradeAPI.h"
#include "ESUNNY/iTapTradeAPIDataType.h"
#include "model/Instrument.h"
#include "conn/ExchangeConn.h"

/*Commit
1：改单指令是易盛服务器先进行撤单动作再进行报单
2：可能会发生没有收到委托回报：需要查委托后调用订单录入、删除来补委托回报
3：可能会发生没有收到成交回报：需要查成交后进行成交录入、删除来补成交回报
4：断线后要重新登录，不会自动重登
5：QryOrder、QryFill、QryFund、QryPosition每次登陆，该接口只能进行一次调用
*/

class ConnPublisher;
struct ConnConfig;
//using namespace ITapTrade;
class ESUNNYTradeHandler : public ITapTrade::ITapTradeAPINotify {

public:
	ESUNNYTradeHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher);

	virtual void TAP_CDECL OnConnect();
	//登录期间的数据发送情况和登录的回馈信息传递到此回调函数中，登录成功后API将向服务器发送登录认证信息。
	//该回调返回成功，说明用户登录成功。但是不代表API准备完毕。
	virtual void TAP_CDECL OnRspLogin(ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPITradeLoginRspInfo* loginRspInfo);
	//API到期提醒回调-此函数为Login()登录成功后，如果到期日与当前日期小于30天，则进行回调提醒。
	virtual void TAP_CDECL OnExpriationDate(ITapTrade::TAPIDATE date, int days);
	//通知用户API准备就绪-只有用户回调收到此就绪通知时才能进行后续的各种行情数据查询操作。此回调函数是API能否正常工作的标志。
	virtual void TAP_CDECL OnAPIReady(ITapTrade::TAPIINT32 errorCode);
	void QueryTradingDate();
	virtual void TAP_CDECL OnDisconnect(ITapTrade::TAPIINT32 reasonCode);
	//返回系统交易日期和当天LME到期日
	virtual void TAP_CDECL OnRspQryTradingDate(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPITradingCalendarQryRsp* info);
	//返回用户信息-此回调接口向用户返回查询的资金账号的详细信息。用户有必要将得到的账号编号保存起来，然后在后续的函数调用中使用。
	virtual void TAP_CDECL OnRspQryAccount(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIUINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountInfo* info);
	//返回资金账户的资金信息
	virtual void TAP_CDECL OnRspQryFund(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIFundData* info);
	//用户资金变化通知-用户的委托成交后会引起资金数据的变化，因此需要向用户实时反馈。
	virtual void TAP_CDECL OnRtnFund(const ITapTrade::TapAPIFundData* info);
	//订单操作应答
	//下单、撤单、改单应答。下单都会有此应答回调，如果下单请求结构中没有填写合约或者资金账号，则仅返回错误号。
	//撤单、改单错误由应答和OnRtnOrder，成功仅返回OnRtnOrder回调。
	virtual void TAP_CDECL OnRspOrderAction(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderActionRsp* info);
	//返回新委托。新下的或者其他地方下的推送过来的。
	//服务器接收到客户下的委托内容后就会保存起来等待触发，同时向用户回馈一个新委托信息说明服务器正确处理了用户的请求，
	//返回的信息中包含了全部的委托信息，同时有一个用来标示此委托的委托号。
	virtual void TAP_CDECL OnRtnOrder(const ITapTrade::TapAPIOrderInfoNotice* info);
	//返回查询的委托信息-返回用户查询的委托的具体信息。
	virtual void TAP_CDECL OnRspQryOrder(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIOrderInfo* info);
	//返回查询的成交信息
	virtual void TAP_CDECL OnRspQryFill(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIFillInfo* info);
	//推送来的成交信息-用户的委托成交后将向用户推送成交信息。
	virtual void TAP_CDECL OnRtnFill(const ITapTrade::TapAPIFillInfo* info);
	//返回查询的持仓
	virtual void TAP_CDECL OnRspQryPosition(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIPositionInfo* info);
	//持仓变化推送通知
	virtual void TAP_CDECL OnRtnPosition(const ITapTrade::TapAPIPositionInfo* info);
	//返回查询的持仓汇总
	virtual void TAP_CDECL OnRspQryPositionSummary(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIPositionSummary* info);
	//持仓汇总变化推送通知
	virtual void TAP_CDECL OnRtnPositionSummary(const ITapTrade::TapAPIPositionSummary* info);
	//港交所做市商双边报价应答 Add:2017.08.29
	virtual void TAP_CDECL OnRspHKMarketOrderInsert(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderMarketInsertRsp* info);
	//港交所做市商双边撤单应答 Add:2017.08.29
	virtual void TAP_CDECL OnRspHKMarketOrderDelete(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderMarketDeleteRsp* info);
	//订单删除应答 Add:2017.12.05
	virtual void TAP_CDECL OnRspOrderLocalRemove(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalRemoveRsp* info);
	//订单录入应答 Add:2017.12.05
	virtual void TAP_CDECL OnRspOrderLocalInput(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalInputRsp* info);
	//订单修改应答 Add:2017.12.05
	virtual void TAP_CDECL OnRspOrderLocalModify(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalModifyRsp* info);
	//成交录入应答 Add:2017.12.05
	virtual void TAP_CDECL OnRspFillLocalInput(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIFillLocalInputRsp* info);
	//订单删除应答 Add:2017.12.05
	virtual void TAP_CDECL OnRspFillLocalRemove(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIFillLocalRemoveRsp* info);

	void SetTraderApi(ITapTrade::ITapTradeAPI* pTraderApi) { m_pTradeApi = pTraderApi; }

	/*********************************************************暂不实现接口*********************************************************/
	//登录完成后，如果需要二次认证（9.2.7后台），会收到联系方式的通知，可以选择通知消息的一个联系方式（邮箱或者电话）请求发送二次认证授权码（RequestVertificateCode）。
	virtual void TAP_CDECL OnRtnContactInfo(ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TAPISTR_40 ContactInfo);
	//请求获取二次认证授权码，后台发送邮件或者短信，并给出应答，包含发送序号以及认证码有效期。
	//errorCode 如果没有绑定联系，返回10016错误.
	//rsp二次认证码有效期，以秒返回，在二次认证有效期内，可以重复设置二次认证码，但是不能再重新申请二次认证码。
	//该回调返回成功，说明需要二次认证，并且需要选择一个联系方式然后调用RequestVertificateCode。
	virtual void TAP_CDECL OnRspRequestVertificateCode(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIRequestVertificateCodeRsp* rsp);
	//通知用户密码修改结果
	virtual void TAP_CDECL OnRspChangePassword(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode) {}
	//认证账户密码反馈
	virtual void TAP_CDECL OnRspAuthPassword(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode) {}
	//设置用户预留信息反馈
	virtual void TAP_CDECL OnRspSetReservedInfo(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TAPISTR_50 info) {}
	//返回系统中的交易所信息
	virtual void TAP_CDECL OnRspQryExchange(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIExchangeInfo* info) {}
	//返回系统中品种信息-此回调接口用于向用户返回得到的所有品种信息。
	virtual void TAP_CDECL OnRspQryCommodity(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPICommodityInfo* info) {}
	//返回系统中合约信息
	virtual void TAP_CDECL OnRspQryContract(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPITradeContractInfo* info) {}
	// 返回新增合约信息-向用户推送新的合约。主要用来处理在交易时间段中服务器添加了新合约时，向用户发送这个合约的信息。
	virtual void TAP_CDECL OnRtnContract(const ITapTrade::TapAPITradeContractInfo* info) {}
	//返回查询的委托变化流程信息
	virtual void TAP_CDECL OnRspQryOrderProcess(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIOrderInfo* info) {}
	//持仓盈亏通知
	virtual void TAP_CDECL OnRtnPositionProfit(const ITapTrade::TapAPIPositionProfitNotice* info) {}
	//返回系统中的币种信息
	virtual void TAP_CDECL OnRspQryCurrency(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPICurrencyInfo* info) {}
	//交易消息通知-返回查询的用户资金状态信息。信息说明了用户的资金状态，用户需要仔细查看这些信息。
	virtual void TAP_CDECL OnRspQryTradeMessage(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPITradeMessage* info) {}
	//交易消息通知-用户在交易过程中可能因为资金、持仓、平仓的状态变动使账户处于某些危险状态，或者某些重要的信息需要向用户通知。
	virtual void TAP_CDECL OnRtnTradeMessage(const ITapTrade::TapAPITradeMessage* info) {}
	//历史委托查询应答
	virtual void TAP_CDECL OnRspQryHisOrder(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIHisOrderQryRsp* info) {}
	//历史委托流程查询应答
	virtual void TAP_CDECL OnRspQryHisOrderProcess(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIHisOrderProcessQryRsp* info) {}
	//历史成交查询应答
	virtual void TAP_CDECL OnRspQryHisMatch(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIHisMatchQryRsp* info) {}
	//历史持仓查询应答
	virtual void TAP_CDECL OnRspQryHisPosition(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIHisPositionQryRsp* info) {}
	//历史交割查询应答
	virtual void TAP_CDECL OnRspQryHisDelivery(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIHisDeliveryQryRsp* info) {}
	//资金调整查询应答
	virtual void TAP_CDECL OnRspQryAccountCashAdjust(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountCashAdjustQryRsp* info) {}
	//查询用户账单应答 Add:2013.12.11
	virtual void TAP_CDECL OnRspQryBill(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIBillQryRsp* info) {}
	//查询账户手续费计算参数 Add:2017.01.14
	virtual void TAP_CDECL OnRspQryAccountFeeRent(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountFeeRentQryRsp* info) {}
	//查询账户保证金计算参数 Add:2017.01.14
	virtual void TAP_CDECL OnRspQryAccountMarginRent(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountMarginRentQryRsp* info) {}
	//订单转移应答 Add:2017.12.05
	virtual void TAP_CDECL OnRspOrderLocalTransfer(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalTransferRsp* info) {}
	//港交所询价通知
	virtual void TAP_CDECL OnHKMarketQuoteNotice(const ITapTrade::TapAPIOrderQuoteMarketNotice* info) {};

private:
	void QueryFund();
	void QueryPosition();
	std::string GetErrorDescription(ITapTrade::TAPIINT32 errorCode);

	bool IsOrdered(ITapTrade::TapAPIOrderInfo const& order) const;
	bool IsCanceled(ITapTrade::TapAPIOrderInfo const& order) const;
	bool IsPartial(ITapTrade::TapAPIOrderInfo const& order) const;
	bool IsAllTraded(ITapTrade::TapAPIOrderInfo const& order) const;
	bool IsAccepted(ITapTrade::TapAPIOrderInfo const& order) const;
	bool IsSendFailed(ITapTrade::TapAPIOrderInfo const& order) const;

private:
	ConnConfig const& m_config;
	ExchangeConnInstrumentHelper m_instHelper;
	ConnPublisher& m_publisher;
	ITapTrade::ITapTradeAPI* m_pTradeApi;
	ITapTrade::TAPIUINT32 m_sessionID;
	bool m_isApiReady;

};