#pragma once
#include "ZQuote/ZQQuoteAPI.h"
#include "FTDPackage.h"
#include "IFlow.h"
class ZQuoteQuoteHandler : public IZQuoteAPINotify {
public:
	ZQuoteQuoteHandler(IFlow* flow);
	void SetZQuoteAPI(IZQQuoteAPI* quoteAPI) { quoteAPI_ = quoteAPI; }
protected:
	virtual void ZQ_CDECL OnRspLogin(ZQINT32 errorCode, const ZQAPIQuotLoginRspInfo* info) override;
	virtual void ZQ_CDECL OnAPIReady() override;
	virtual void ZQ_CDECL OnDisconnect(ZQINT32 reasonCode) override;
	virtual void ZQ_CDECL OnRspQryCommodity(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIQuoteCommodityInfo* info) override;
	virtual void ZQ_CDECL OnRspQryContract(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIQuoteContractInfo* info) override;
	virtual void ZQ_CDECL OnRspSubscribeQuote(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIQuoteWhole* info) override;
	virtual void ZQ_CDECL OnRspUnSubscribeQuote(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIContract* info) override;
	virtual void ZQ_CDECL OnRtnQuote(const ZQAPIQuoteWhole* info) override;
	
private:
	IZQQuoteAPI* quoteAPI_ = nullptr;
	IFlow* flow_;
	CFTDPackage package_;
};


