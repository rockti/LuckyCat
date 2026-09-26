#include "stdafx.h"
#include "ZQuoteHandler.h"
#include "BaseExchApi.h"
#include "ZQuoteConnection.h"
#include <fmt/format.h>
#include "ZtpType.h"
#include "ServiceDef.h"
#include "BaseExchApi.h"
#include "FtdDataStruct.h"
#include "InstrumentManager.h"
#include "DBInit.h"

#pragma warning(disable:4244)

ZQuoteQuoteHandler::ZQuoteQuoteHandler(IFlow* flow) :flow_(flow) {
	package_.ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);
}


void ZQ_CDECL ZQuoteQuoteHandler::OnDisconnect(ZQINT32 reasonCode) {
	Log::Error(fmt::format("ZQuote Disconnected, reason[{}]", reasonCode));
}


void ZQ_CDECL ZQuoteQuoteHandler::OnAPIReady() {
	Log::Info("ZQuote Ready");
	ZQAPICommodity req = { 0 };
	strcpy_s(req.CommodityNo, "GC");
	req.CommodityType = ZQ_COMMODITY_TYPE_FUTURES;
	strcpy(req.ExchangeNo, "COMEX");
	uint32_t reqID = 0;
	auto rtn = quoteAPI_->QryContract(&reqID, &req);
	if (rtn != 0) {
		printNlog("ZQuote QryContract failed, rtn[{}]", rtn);
	}
}


void ZQ_CDECL ZQuoteQuoteHandler::OnRspLogin(ZQINT32 errorCode, const ZQAPIQuotLoginRspInfo* info) {
	Log::Info("ZQuote Logined, Quote GC Quote");
}


void ZQ_CDECL ZQuoteQuoteHandler::OnRspQryCommodity(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIQuoteCommodityInfo* info) {
}

void ZQ_CDECL ZQuoteQuoteHandler::OnRspQryContract(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIQuoteContractInfo* info) {
	if (errorCode != 0) {
		Log::Error(fmt::format("ZQuote QryContract Failed, Error[{}]", errorCode));
	}
	if (info != nullptr) {
		Log::Info(fmt::format("ZQuote OnQryContrct: Commodity[{}] Contract[{}]", info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1));
		uint32_t reqID = 0;
		ZQAPIContract req = { 0 };
		memcpy(&req, &info->Contract,sizeof(req));

		std::string instCode = info->Contract.Commodity.CommodityNo;
		instCode += info->Contract.ContractNo1;
		CInstrumentIDType inst = instCode.c_str();
		FTDInstance().Add(inst);
		Log::Info(fmt::format("ZQuote SubscribeQuote: Commodity[{}] Contract[{}]", 
			info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1));
		auto rtn = quoteAPI_->SubscribeQuote(&reqID, &req);
		if (rtn != 0) {
			printNlog("ZQuote SubscribeQuote: Commodity[{}] Contract[{}] failed, rtn[{}]", 
				info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1,rtn);
		}
	}
}

void ZQ_CDECL ZQuoteQuoteHandler::OnRspSubscribeQuote(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIQuoteWhole* info) {
	if (errorCode != 0) {
		Log::Error(fmt::format("ZQuote QryContract Failed, Error[{}]", errorCode));
	}
	if (info != nullptr) {
		Log::Info(fmt::format("ZQuote OnRspSubscribeQuote: Commodity[{}] Contract[{}]", info->Contract.Commodity.CommodityNo, info->Contract.ContractNo1));
	}

}

void ZQ_CDECL ZQuoteQuoteHandler::OnRspUnSubscribeQuote(ZQUINT32 sessionID, ZQINT32 errorCode, ZQYNFLAG isLast, const ZQAPIContract* info) {
}
void fillPrice(CFTDDepthMarketDataField& market, const ZQAPIQuoteWhole* info);
void ZQ_CDECL ZQuoteQuoteHandler::OnRtnQuote(const ZQAPIQuoteWhole* info) {
	if (info != nullptr) {
		CFTDDepthMarketDataField market;
		std::string instCode = info->Contract.Commodity.CommodityNo;
		instCode += info->Contract.ContractNo1;
		market.InstrumentID = instCode.c_str();

		fillPrice(market, info);

		market.LastPrice = info->QLastPrice;
		market.Volume = info->QTotalQty;

		market.ClosePrice = info->QClosingPrice;
		market.PreClosePrice = info->QPreClosingPrice;
		market.HighestPrice = info->QHighPrice;
		market.LowestPrice = info->QLowPrice;
		market.OpenPrice = info->QOpeningPrice;
		market.OpenInterest = info->QPositionQty;
		market.PreOpenInterest = info->QPrePositionQty;
		market.PreSettlementPrice = info->QPreSettlePrice;
		market.SettlementPrice = info->QSettlePrice;
		market.Turnover = info->QTurnoverRate;
		market.UpperLimitPrice = info->QLimitUpPrice;
		market.LowerLimitPrice = info->QLimitDownPrice;
//		market.UpdateTime = info->DateTimeStamp; todo

// 		fmt::print("OnRtnQuote: [{}] Last[{:.3f} Ask1[{:.3f}{}] Bid[{:.3f}{}]\n",
// 			market.InstrumentID, market.LastPrice,
// 			market.AskPrice1, market.AskVolume1,
// 			market.BidPrice1, market.BidVolume1);

		auto instIndex = FTDInstance().UpdateSnapshot(market);
		package_.PreparePackage(FTD_TID_RtnDepthMarketData, FTD_CHAIN_LAST);
		package_.SetInstrumentIDCount(instIndex.Index());
		FTD_ADD_FIELD(&package_, &market);
		package_.MakePackage();

		flow_->Append(package_.GetAddress(), package_.GetLength());
	}

}

void fillPrice(CFTDDepthMarketDataField& market, const ZQAPIQuoteWhole* info) {
	market.AskPrice1 = info->QAskPrice[0];
	market.AskPrice2 = info->QAskPrice[1];
	market.AskPrice3 = info->QAskPrice[2];
	market.AskPrice4 = info->QAskPrice[3];
	market.AskPrice5 = info->QAskPrice[4];
	market.AskVolume1 = info->QAskQty[0];
	market.AskVolume2 = info->QAskQty[1];
	market.AskVolume3 = info->QAskQty[2];
	market.AskVolume5 = info->QAskQty[4];
	market.AskVolume4 = info->QAskQty[3];

	market.BidPrice1 = info->QBidPrice[0];
	market.BidPrice2 = info->QBidPrice[1];
	market.BidPrice3 = info->QBidPrice[2];
	market.BidPrice4 = info->QBidPrice[3];
	market.BidPrice5 = info->QBidPrice[4];
	market.BidVolume1 = info->QBidQty[0];
	market.BidVolume2 = info->QBidQty[1];
	market.BidVolume3 = info->QBidQty[2];
	market.BidVolume4 = info->QBidQty[3];
	market.BidVolume5 = info->QBidQty[4];

	//todo hidden price
}
