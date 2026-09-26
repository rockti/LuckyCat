#pragma once

#include "thirdapi/ESUNNY/TapAPICommDef.h"
#include "thirdapi/ESUNNY/TapAPIError.h"
#include "thirdapi/ESUNNY/TapQuoteAPI.h"
#include "thirdapi/ESUNNY/TapQuoteAPIDataType.h"
#include "conn/ExchangeConn.h"
#include "model/Instrument.h"
#include "model/Quote.h"

/*Commit
 *1:合约2：用于组合套利合约（包括期货、期权），订阅期权行情且不用合约2时TAPICallOrPutFlagType字段要设为TAPI_CALLPUT_FLAG_NONE
 *2:合约只能一个个订阅，内盘数量不限，外盘最多50个
 *3:品种要用服务器列表中的值，如果不一致会订阅失败（据说有坑）
 *4:合约不存在可能服务器暂时没有数据，此时服务器会进行订阅，用户需要Retry
 *5:sessionID非常重要，是保证请求和返回的唯一标识，OnRspSubscribeQuote会自动触发，需要过滤
 */
class ConnPublisher;
struct TradeSystemConfig;

class ESUNNYQuoteHandler : public ITapQuoteAPINotify
{
public:
	ESUNNYQuoteHandler(TradeSystemConfig const& config, ConnPublisher& publisher);

	virtual void TAP_CDECL OnRspLogin(TAPIINT32 errorCode, const TapAPIQuotLoginRspInfo* info);
	virtual void TAP_CDECL OnAPIReady();
	virtual void TAP_CDECL OnDisconnect(TAPIINT32 reasonCode);
	virtual void TAP_CDECL OnRspQryCommodity(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteCommodityInfo* info);
	virtual void TAP_CDECL OnRspQryContract(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteContractInfo* info);
	virtual void TAP_CDECL OnRspSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIQuoteWhole* info);
	virtual void TAP_CDECL OnRspUnSubscribeQuote(TAPIUINT32 sessionID, TAPIINT32 errorCode, TAPIYNFLAG isLast, const TapAPIContract* info);
	virtual void TAP_CDECL OnRtnQuote(const TapAPIQuoteWhole* info);

	bool IsAPIReady() const { return m_isApiReady; }
	void SetQuoteApi(ITapQuoteAPI* pQuoteApi) { m_pQuoteApi = pQuoteApi; }
	void SubscribeQuote(type::InstrumentCode const& instCode);
	void UnsubscribeQuote(type::InstrumentCode const& instCode);
	void DoSubscribe(type::InstrumentCode const& instCode);
	void DoUnsubscribe(type::InstrumentCode const& instCode);

private:
	void FillingContractInfo(Instrument const& inst, TapAPIContract& contractInfo);

	bool IsInvalidQuote(const TapAPIQuoteWhole* pDepthMarketData);
	Quote CreateQuote(type::InstID instID, const TapAPIQuoteWhole* pDepthMarketData); 

	std::string GetErrorDescription(TAPIINT32 errorCode);
private:
	TradeSystemConfig const& m_config;
	ConnPublisher& m_publisher;
	ITapQuoteAPI* m_pQuoteApi;
	TAPIUINT32 m_sessionID;
	bool m_isApiReady;

	std::vector<type::InstrumentCode> m_subscribeList;		//订阅列表
};
