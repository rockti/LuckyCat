#pragma once

#include "ESUNNY/TapAPICommDef.h"
#include "ESUNNY/TapAPIError.h"
#include "ESUNNY/TapQuoteAPI.h"
#include "ESUNNY/TapQuoteAPIDataType.h"
#include "FtdDataStruct.h"

/*Commit
 *1:合约2：用于组合套利合约（包括期货、期权），订阅期权行情且不用合约2时TAPICallOrPutFlagType字段要设为TAPI_CALLPUT_FLAG_NONE
 *2:合约只能一个个订阅，内盘数量不限，外盘最多50个
 *3:品种要用服务器列表中的值，如果不一致会订阅失败（据说有坑）
 *4:合约不存在可能服务器暂时没有数据，此时服务器会进行订阅，用户需要Retry
 *5:sessionID非常重要，是保证请求和返回的唯一标识，OnRspSubscribeQuote会自动触发，需要过滤
 */
class ConnPublisher;
struct ConnConfig;
class CMarketFlowManager;
class CFTDPackage;
class ESUNNYQuoteHandler : public ITapQuoteAPINotify
{
public:
	ESUNNYQuoteHandler(CMarketFlowManager* pFlowManager, int topicID);

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
	void SubscribeQuote(char const* instrumentCode);
	void UnsubscribeQuote(char const* instrumentCode);
	void DoSubscribe(char const* instrumentCode);
	void DoUnsubscribe(char const* instrumentCode);

private:
	void FillingContractInfo(char const* instCode, TapAPIContract& contractInfo);

	bool IsInvalidQuote(const TapAPIQuoteWhole* pDepthMarketData);
	CFTDDepthMarketDataField CreateQuote(const TapAPIQuoteWhole* pDepthMarketData);

	std::string GetErrorDescription(TAPIINT32 errorCode);
private:
	CMarketFlowManager* m_pMarketFlowManager;
	CFTDPackage* m_pMarketPackage;				//行情流数据包：行情直接以FTD协议返回，不用转换ZTP了
	int m_topicID;

	ITapQuoteAPI* m_pQuoteApi = nullptr;
	TAPIUINT32 m_sessionID;
	bool m_isApiReady;

	std::vector<std::string> m_subscribeList;		//订阅列表
};
