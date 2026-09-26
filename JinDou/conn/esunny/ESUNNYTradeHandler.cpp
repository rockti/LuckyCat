#include "conn/esunny/ESUNNYTradeHandler.h"
#include "conn/esunny/ESUNNYConnection.h"
#include "utils/Log.h"
#include "notify/ConnPublisher.h"
#include "config/Config.h"
#include "notify/Notify.h"
#include "conn/ExchangeConn.h"
#include "model/Instrument.h"
#include "model/dealing/ExchangeRequest.h"
#include "utils/TimeChecker.h"
#include "service/Service.h"
#include "service/InstrumentProgressManager.h"

inline type::Exchange ConvertExchangeFromESUNNY(char const* exchange) {
	std::string_view ex(exchange);
	if (ex == "COMEX") {
		return type::Exchange::CME;
	}
	return type::Exchange::HKEX;
}
inline type::Direction ConvertDirectionFromESUNNY(ITapTrade::TAPISideType direction) {
	return direction == ITapTrade::TAPI_SIDE_BUY ? type::Direction::Buy : type::Direction::Sell;
}

inline type::OrderPriceType ConvertOrderTypeFromESUNNY(ITapTrade::TapAPIOrderInfo const& orderInfo) {
	if (orderInfo.TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_FAK) {
		return type::OrderPriceType::FAK;
	}
	if (orderInfo.TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_FOK) {
		return type::OrderPriceType::FOK;
	}
	
// 	if (orderInfo.TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_GFD) {
// 		return type::OrderPriceType::LimitPrice;
// 	}
	return type::OrderPriceType::LimitPrice;
}
char const* lastN(char const* str, int n) {
	auto len = strlen(str);
	auto start = len - n;
	if (start < 0) {
		start = 0;
	}
	return str + start;
}

std::string fromDateTimeToTime(char const* str) {
	std::string_view result(str);
	if (result.length() > 13) {
		auto time = result.substr(6, 6);
		return fmt::format("{}:{}:{}", time.substr(0, 2), time.substr(2, 2), time.substr(4, 2));
	}
	return "";
}

inline std::pair<bool,LocalOrderID> GetLocalIdByDirection(type::Direction direction, int refInt) {
	return std::make_pair(false,refInt);
}

ESUNNYTradeHandler::ESUNNYTradeHandler(TradeSystemConfig const& config, ConnPublisher& publisher)
:m_config(config),m_publisher(publisher),m_pTradeApi(NULL), m_sessionID(0), m_isApiReady(false) {
}

void ESUNNYTradeHandler::OnConnect() {
	Log::Info("ESUNNYTradeHandler: FrontConnected");
}

void ESUNNYTradeHandler::OnDisconnect(ITapTrade::TAPIINT32 reasonCode) {
	m_isApiReady = false;
	TradePlatformNotification en;
	en.Platform = TradePlatform::HKEX_ESUNNY;
	en.Type = TradePlatformNotification::DISCONNECTED;
	en.SubSystem = SubSystem::Trade;
	m_publisher.NotifyUI(en);

	auto msg = fmt::format("ESUNNYTradeHandler:FrontDisconnected, {}", GetErrorDescription(reasonCode));
	m_publisher.NotifyUI(msg);
	Log::Warn(msg);

	m_publisher.PushResponse(en);

}
void ESUNNYTradeHandler::OnExpriationDate(ITapTrade::TAPIDATE date, int days) {
	Log::Warn(fmt::format("ESUNNYTradeHandler: API is still [{}] days to expire, {} ]", days));
}
void ESUNNYTradeHandler::OnAPIReady(ITapTrade::TAPIINT32 errorCode) {
	TradePlatformNotification tn;
	tn.Platform = TradePlatform::HKEX_ESUNNY;
	tn.SubSystem = SubSystem::Trade;
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		m_isApiReady = true;
		tn.Type = TradePlatformNotification::CONNECTED;

		auto msg = fmt::format("ESUNNYTradeHandler: APIInit Success");
		Log::Error(msg);
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::TradeLogin);
	} else {
		tn.Error = Error( errorCode, GetErrorDescription(errorCode).c_str() );
		auto msg = fmt::format("ESUNNYTradeHandler: APIInit Failed, {}", tn.Error.Msg);
		Log::Error(msg);
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::TradeLogin, tn.Error);
	}
}

void ESUNNYTradeHandler::OnRspLogin(ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPITradeLoginRspInfo* loginRspInfo) {
	TradePlatformNotification tn;
	tn.Platform = TradePlatform::HKEX_ESUNNY;
	tn.SubSystem = SubSystem::Trade;
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ESUNNYTradeHandler: UserLogin success, waiting for API initialize...");
		tn.Type = TradePlatformNotification::LOGIN;
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::TradeConnect);
	} else {
		auto msg = fmt::format("ESUNNYTradeHandler: UserLogin Failed, {}", GetErrorDescription(errorCode));
		Log::Error(msg);
		tn.Error = Error{ errorCode,GetErrorDescription(errorCode).c_str() };
		tn.Type = TradePlatformNotification::LOGIN_FAILED;
		m_publisher.NotifyUI(tn);
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::TradeConnect, tn.Error);
	}
}


void ESUNNYTradeHandler::OnRspQryFund(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIFundData* info) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode && NULL != info) {
		if (0 == strcmp(info->CurrencyNo, "HKD")) {
			InvestorAccount account;
			account.InvestorID = info->AccountNo;
			account.Balance = info->Balance;
			account.Available = info->Available;
			account.Margin = info->AccountMaintenanceMargin;	//占用保证金
			account.Fee = info->FrozenFee;						//占用手续费
			account.CloseProfit = info->CloseProfit;
			account.FrozenFee = info->FrozenFee;
			account.FrozenMargin = info->FrozenDeposit;
			account.PreBalance = info->PreBalance;
			account.Platform = TradePlatform::HKEX_ESUNNY;

			m_publisher.NotifyUI(account);
			m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryAccount);

			Log::Info(fmt::format("ESUNNYTradeHandler: QryFund Success, Capital[{:.2f}] Available[{:.2f}] Margin[{:.2f}] Profit[{:.2f}] Fee[{:.2f}]"
				, info->Balance, info->Available, info->AccountMaintenanceMargin, info->CloseProfit, info->FrozenFee));
		}
		Log::Info(fmt::format("ESUNNYTradeHandler: QryFund CurrencyGroup[{}],Currency[{}] TradeRate[{}], Capital[{:.2f}] Available[{:.2f}] Margin[{:.2f}] Profit[{:.2f}] Fee[{:.2f}]",
			info->CurrencyGroupNo,info->CurrencyNo,info->TradeRate, info->Balance, info->Available, info->AccountMaintenanceMargin, info->CloseProfit, info->FrozenFee));
	} else {
		Error error(errorCode, GetErrorDescription(errorCode).c_str());
		auto msg = fmt::format("ESUNNYTradeHandler: QryFund Failed, ErrorCode[{}] ErrorMsg[{}]", 
			errorCode, error.Msg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryAccount, error);

	}
}

void ESUNNYTradeHandler::OnRspQryCommodity(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode,
	ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPICommodityInfo* info) {
	if (info != nullptr && errorCode != ITapTrade::TAPIERROR_SUCCEED) {
		Log::Error(fmt::format("ESUNNYTradeHandler: OnRspQryContract failed Error[{}] Msg[{}]", errorCode, GetErrorDescription(errorCode)));
		return;
	}
	if (info == nullptr) {
		Log::Error("ESUNNYTradeHandler: OnRspQryInstrument No Instrument");
		return;
	}
	auto& manager = VarietyProgressManager::Instance();
	VarietyInfo varietyInfo;
	varietyInfo.Code = info->CommodityNo;
	varietyInfo.UnitPrice = info->CommodityTickSize;
	varietyInfo.VolumeMultiple = info->ContractSize;

	Log::Info(fmt::format("ESUNNYTradeHandler: OnRspQryCommodity {} {} {}", varietyInfo.Code, varietyInfo.UnitPrice, varietyInfo.VolumeMultiple));

	if (manager.IsRemain(varietyInfo.Code)) {
		manager.AddVarietyDone(varietyInfo.Code);
		manager.AddVarietyInfo(varietyInfo);
		ITapTrade::TAPIUINT32 sessionID = RequestIdACC::Generate();
		ITapTrade::TapAPICommodity commodity;
		strcpy_s(commodity.CommodityNo, varietyInfo.Code.value());
		commodity.CommodityType = TAPI_COMMODITY_TYPE_FUTURES;
		strcpy_s(commodity.ExchangeNo, info->ExchangeNo);
		auto rtn = m_pTradeApi->QryContract(&sessionID, &commodity);
	}
}

void ESUNNYTradeHandler::OnRspQryContract(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, 
	ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPITradeContractInfo* info) {
	if (info != nullptr && errorCode != ITapTrade::TAPIERROR_SUCCEED) {
		Log::Error(fmt::format("ESUNNYTradeHandler: OnRspQryContract failed Error[{}] Msg[{}]", errorCode, GetErrorDescription(errorCode)));
		return;
	}
	if (info == nullptr) {
		Log::Error("ESUNNYTradeHandler: OnRspQryInstrument No Instrument");
		return;
	}

	auto varietyInfo = VarietyProgressManager::Instance().GetInfo(type::VarietyCode(info->CommodityNo));
	if (!varietyInfo) {
		Log::Error(fmt::format("ESUNNYTradeHandler: OnRspQryContract failed Error[No Commodity INfo]"));
		return;
	}

	std::string instCode = info->CommodityNo;
	instCode += info->ContractNo1;

	Instrument inst;
	inst.Platform = TradePlatform::HKEX_ESUNNY;
	inst.Exchange = type::Exchange::HKEX;
	inst.Code = instCode.c_str();
	inst.LongPosLimit = 100000;
	inst.ShortPosLimit = 100000;
	inst.MaxOrderVolume = 100000;
	inst.MinOrderVolume = 1;
	inst.Name = info->ContractName;
	inst.ProductClass = type::ProductClass::Future;
	inst.UnitPrice = varietyInfo->UnitPrice;
	inst.VolumeMultiple = varietyInfo->VolumeMultiple;
	inst.VarityCode = info->CommodityNo;
	inst.YearMonth = info->ContractNo1;
	auto instService = ServiceSinglton::Services().Instrument;
	inst = instService->Add(inst);
	m_publisher.NotifyUI(inst);
	Log::Info(fmt::format("ESUNNYTradeHandler: OnRspQryContract: InstID[{}] Code[{}] UnitPrice[{}] VolumeMultiple[{}]", 
		inst.InstID, inst.Code, inst.UnitPrice, inst.VolumeMultiple));
	if (isLast == ITapTrade::APIYNFLAG_YES) {
		if (VarietyProgressManager::Instance().IsDone()) {
			std::vector<type::InstrumentCode> codes;
			auto insts = ServiceSinglton::Services().Instrument->GetAll(TradePlatform::HKEX_ESUNNY);
			for (auto const& inst : insts) {
				codes.push_back(inst.Code);
			}
			InstrumentProgressManager::HKEX().InitInstruments(codes);
			for (auto const& inst : insts) {
				InstrumentProgressManager::HKEX().AddInstDone(inst.Code);
			}
			Log::Info(fmt::format("ESUNNYTradeHandler: OnRspQryInstrument Finished Total[{}]", insts.size()));
			m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryInstrument);
		}
	}
}

void ESUNNYTradeHandler::OnRspQryAccountFeeRent(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, 
	ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountFeeRentQryRsp* info) {
	if (info == nullptr) {
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryFee, SystemError);
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryAccountFeeRent No Fees");
		return;
	}
	if (info != nullptr && errorCode != ITapTrade::TAPIERROR_SUCCEED) {
		Error error{ errorCode, GetErrorDescription(errorCode).c_str() };
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryFee, error);

		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryAccountFeeRent failed Error[{}] Msg[{}]", error.Code, error.Msg);
		Log::Error(msg);
		return;
	}
	// hk的Fee是在品种上，不在合约上，也就是所有合约是一样的fee?那多方式是什么？

	InvestorFee fee;
	fee.Platform = TradePlatform::HKEX_ESUNNY;
	fee.Exchange = type::Exchange::CME;
	fee.InvestorID = m_investor;
	fee.VarityCode = info->CommodityNo;
 	auto instService = ServiceSinglton::Services().Instrument;
	if (VarietyProgressManager::Instance().Has(fee.VarityCode)) {

		Log::Info(fmt::format("ESUNNYTradeHandler: OnRspQryAccountFeeRent Mode[{}] OpenClose[{}] CloseToday[{}]",
			info->CalculateMode, info->OpenCloseFee, info->CloseTodayFee));
		fee.InstrumentCode = fee.VarityCode.value();
		fee.InstID = INVALID_INSTRUMENT_ID;
// 		//!比例+定额（仅限手续费）大于0.01部分为定额，小于0.01部分为比例，如：0.001为比例收取1%。
// 		const TAPICalculateModeType		TAPI_CALULATE_MODE_COMBINE = '0';
// 		//! 比例
// 		const TAPICalculateModeType		TAPI_CALCULATE_MODE_PERCENTAGE = '1';
// 		//! 定额
// 		const TAPICalculateModeType		TAPI_CALCULATE_MODE_QUOTA = '2';
		if (info->CalculateMode == ITapTrade::TAPI_CALULATE_MODE_COMBINE) {
			fee.OffsetFeeAmt = int(info->OpenCloseFee * 100);
			fee.OffsetFeeRate = (info->OpenCloseFee * 100 - fee.OffsetFeeAmt) / 10.0;
			fee.OffsetTodayFeeAmt = int(info->CloseTodayFee * 100);
			fee.OffsetTodayFeeRate = (info->CloseTodayFee * 100 - fee.OffsetTodayFeeAmt) / 10.0;
			fee.OpenFeeAmt = fee.OffsetFeeAmt;
			fee.OpenFeeRate = fee.OffsetFeeRate;
		} else if (info->CalculateMode == ITapTrade::TAPI_CALCULATE_MODE_PERCENTAGE) {
			fee.OffsetFeeAmt = 0;
			fee.OffsetFeeRate = info->OpenCloseFee;
			fee.OffsetTodayFeeAmt = 0;
			fee.OffsetTodayFeeRate = info->CloseTodayFee;
			fee.OpenFeeAmt = fee.OffsetFeeAmt;
			fee.OpenFeeRate = fee.OffsetFeeRate;
		} else if (info->CalculateMode == ITapTrade::TAPI_CALCULATE_MODE_QUOTA) {
			fee.OffsetFeeAmt = info->OpenCloseFee;
			fee.OffsetFeeRate = 0;
			fee.OffsetTodayFeeAmt = info->CloseTodayFee;
			fee.OffsetTodayFeeRate = 0;
			fee.OpenFeeAmt = fee.OffsetFeeAmt;
			fee.OpenFeeRate = fee.OffsetFeeRate;
		} else {
			fee.OffsetFeeAmt = 0.1;
			fee.OffsetFeeRate = 0;
			fee.OffsetTodayFeeAmt = 0.1;
			fee.OffsetTodayFeeRate = 0;
			fee.OpenFeeAmt = fee.OffsetFeeAmt;
			fee.OpenFeeRate = fee.OffsetFeeRate;
		}
		instService->AddVarietyFee(TradePlatform::HKEX_ESUNNY, fee.VarityCode, fee);
		InstrumentProgressManager::HKEX().AddVarietyFeeDone(fee.VarityCode);
//		m_publisher.NotifyUI(fee);
		if (InstrumentProgressManager::HKEX().IsFeeDone()) {
			m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryFee);
		}
	}

}

void ESUNNYTradeHandler::OnRspQryAccountMarginRent(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, 
	ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountMarginRentQryRsp* info) {
	if (info == nullptr) {
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryMargin, SystemError);
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryAccountMarginRent No Fees");
		return;
	}
	if (info != nullptr && errorCode != ITapTrade::TAPIERROR_SUCCEED) {
		Error error{ errorCode, GetErrorDescription(errorCode).c_str() };
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryMargin, error);

		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryAccountMarginRent failed Error[{}] Msg[{}]", error.Code, error.Msg);
		Log::Error(msg);
		return;
	}
	// hk的Fee是在品种上，不在合约上，也就是所有合约是一样的fee?那多方式是什么？

	InvestorMargin margin;
	margin.Platform = TradePlatform::HKEX_ESUNNY;
	margin.Exchange = type::Exchange::SHFE;
	margin.InvestorID = m_investor;
	margin.VarityCode = info->CommodityNo;
	auto instService = ServiceSinglton::Services().Instrument;
	if (VarietyProgressManager::Instance().Has(margin.VarityCode)) {

		margin.InstrumentCode = margin.VarityCode.value();
		margin.InstID = INVALID_INSTRUMENT_ID;
		// 		//!比例+定额（仅限手续费）大于0.01部分为定额，小于0.01部分为比例，如：0.001为比例收取1%。
		// 		const TAPICalculateModeType		TAPI_CALULATE_MODE_COMBINE = '0';
		// 		//! 比例
		// 		const TAPICalculateModeType		TAPI_CALCULATE_MODE_PERCENTAGE = '1';
		// 		//! 定额
		// 		const TAPICalculateModeType		TAPI_CALCULATE_MODE_QUOTA = '2';
		if (info->CalculateMode == ITapTrade::TAPI_CALULATE_MODE_COMBINE) {
			margin.LongMarginAmt = info->InitialMargin;
			margin.LongMarginRate = info->MaintenanceMargin;
			margin.ShortMarginAmt = info->SellInitialMargin;
			margin.ShortMarginRate = info->SellMaintenanceMargin;
// 			fee.OffsetTodayFeeAmt = int(info->CloseTodayFee * 100);
// 			fee.OffsetTodayFeeRate = (info->CloseTodayFee * 100 - fee.OffsetTodayFeeAmt) / 10.0;
		} else if (info->CalculateMode == ITapTrade::TAPI_CALCULATE_MODE_PERCENTAGE) {
			margin.LongMarginAmt = info->InitialMargin;
			margin.LongMarginRate = info->MaintenanceMargin;
			margin.ShortMarginAmt = info->SellInitialMargin;
			margin.ShortMarginRate = info->SellMaintenanceMargin;
		} else if (info->CalculateMode == ITapTrade::TAPI_CALCULATE_MODE_QUOTA) {
			margin.LongMarginAmt = info->InitialMargin;
			margin.LongMarginRate = info->MaintenanceMargin;
			margin.ShortMarginAmt = info->SellInitialMargin;
			margin.ShortMarginRate = info->SellMaintenanceMargin;
		} else {
			margin.LongMarginAmt = info->InitialMargin;
			margin.LongMarginRate = info->MaintenanceMargin;
			margin.ShortMarginAmt = info->SellInitialMargin;
			margin.ShortMarginRate = info->SellMaintenanceMargin;
		}
		instService->AddVarietyMargin(TradePlatform::HKEX_ESUNNY, margin.VarityCode, margin);
		InstrumentProgressManager::HKEX().AddVarietyMarginDone(margin.VarityCode);
//		m_publisher.NotifyUI(margin);
		if (InstrumentProgressManager::HKEX().IsFeeDone()) {
			m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryMargin);
		}
	}
}

//做市商双边指令
void ESUNNYTradeHandler::OnRspHKMarketOrderInsert(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderMarketInsertRsp* info) {
	return;
}
void ESUNNYTradeHandler::OnRspHKMarketOrderDelete(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderMarketDeleteRsp* info) {
	if (NULL == info)	return;

	if (ITapTrade::TAPIERROR_SUCCEED != errorCode) {
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspHKMarketOrderDelete, {}", GetErrorDescription(errorCode));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}

//查询指令集合
void ESUNNYTradeHandler::OnRspQryTradingDate(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPITradingCalendarQryRsp* info) {

}
void ESUNNYTradeHandler::OnRspQryAccount(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIUINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIAccountInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED==errorCode && NULL!= info) {
		Log::Info(fmt::format("ESUNNYTradeHandler: QryAccount success, accountNo[{}], accountType[{}]]", info->AccountNo, info->AccountType));
	}
}

void ESUNNYTradeHandler::OnRtnFund(const ITapTrade::TapAPIFundData* info) {
	return;
	//推送资金变化，暂时不用，打个日志看看推送频率
	Log::Warn(fmt::format("ETH: OnRtnFund, Account[{}] Profit[{}]", info->AccountNo, info->CloseProfit));
}

type::OrderStatus GetOrderStatus(ITapTrade::TAPICHAR orderState) {
	switch (orderState) 	{
	case ITapTrade::TAPI_ORDER_STATE_SUBMIT: {
		break;
	}
	case ITapTrade::TAPI_ORDER_STATE_TRIGGERING: {
		break;
	}
	case ITapTrade::TAPI_ORDER_STATE_EXCTRIGGERING: {
		break;
	}
	case ITapTrade::TAPI_ORDER_STATE_QUEUED: {
		return type::OrderStatus::InsertQueueing;
	}
	case ITapTrade::TAPI_ORDER_STATE_ACCEPT: {
		return type::OrderStatus::NoTradeQueueing;
	}
	case ITapTrade::TAPI_ORDER_STATE_CANCELED:
	case ITapTrade::TAPI_ORDER_STATE_LEFTDELETED: {
		return type::OrderStatus::Canceled;
	}
	case ITapTrade::TAPI_ORDER_STATE_PARTFINISHED: {
		return type::OrderStatus::PartTradedQueueing;
	}
	case ITapTrade::TAPI_ORDER_STATE_FINISHED: {
		return type::OrderStatus::AllTraded;
	}
	case ITapTrade::TAPI_ORDER_STATE_CANCELING: {
		return type::OrderStatus::CancelQueueing;
	}
	case ITapTrade::TAPI_ORDER_STATE_MODIFYING:
	case ITapTrade::TAPI_ORDER_STATE_FAIL:
	case ITapTrade::TAPI_ORDER_STATE_DELETED:
	case ITapTrade::TAPI_ORDER_STATE_SUPPENDED:
	case ITapTrade::TAPI_ORDER_STATE_DELETEDFOREXPIRE:
	case ITapTrade::TAPI_ORDER_STATE_EFFECT:
	case ITapTrade::TAPI_ORDER_STATE_APPLY:
	default:
		return type::OrderStatus::Unknown;
		break;
	}
	return type::OrderStatus::Unknown;
}

void ESUNNYTradeHandler::OnRspQryPosition(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIPositionInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED != errorCode) {
		Error error = Error{ int(errorCode), GetErrorDescription(errorCode).c_str() };
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryPositionSummary Failed ErrorCode[{}] ErrorMsg[{}]", error.Code, error.Msg);
		Log::Error(msg);
		m_publisher.NotifyUI(msg);
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryPosition, error);
		return;
	}

	if (info == nullptr) {
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryPosition No position");
		Log::Info(msg);
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryPosition);
		return;
	}

	bool last = isLast == ITapTrade::APIYNFLAG_YES;
	std::string instCode = info->CommodityNo;
	instCode += info->ContractNo;
	InvestorPosition position;
	position.InstrumentCode = instCode;

	auto instID = ServiceSinglton::Services().Instrument->toID(position.InstrumentCode);
	if (instID != INVALID_INSTRUMENT_ID) {
		position.Exchange = type::Exchange::CME;
		position.InvestorID = m_investor;
		position.InstID = instID;
		position.ProductClass = type::ProductClass::Future;
		position.Direction = ConvertDirectionFromESUNNY(info->MatchSide);
		if (info->IsHistory) {
			position.YdPosition = info->PositionQty;
			position.YdPositionPrice = info->PositionPrice;
			position.PositionType = type::PositionType::HistoryOnly;

		} else {
			position.Position = info->PositionQty;
			position.PositionCost = info->PositionPrice;
			position.PositionType = type::PositionType::TodayOnly;
		}

		m_publisher.PushResponse(position);
		m_publisher.NotifyUI(position);
	}

	char const* direction = (type::Direction::Buy == position.Direction) ? "B" : "S";
	Log::Info(fmt::format("ESUNNYTradeHandler：OnRspQryPosition: UserID[{}] IsHistory[{}] Instrument[{}] Dir[{}] Total[{}] Today[{}] History[{}] AvgPrice[{:.3f}] YDAvg[{:.3f}]",
		position.InvestorID, info->IsHistory, position.InstrumentCode, direction, position.Position, position.Position, position.YdPosition, position.PositionCost, position.YdPositionPrice));

	if (last) {
		Log::Info("ESUNNYTradeHandler: QueryPosition Success");
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryPosition);
	}
}

void ESUNNYTradeHandler::OnRspQryOrder(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIOrderInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED != errorCode) {
		Error error = Error{ int(errorCode), GetErrorDescription(errorCode).c_str() };
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryOrder Failed ErrorCode[{}] ErrorMsg[{}]", error.Code, error.Msg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryOrder, error);
		return;
	}

	if (info == nullptr) {
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryOrder No orders");
		Log::Info(msg);
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryOrder);
		return;
	}
	bool last = isLast == ITapTrade::APIYNFLAG_YES;
	std::string instCode = info->CommodityNo;
	instCode += info->ContractNo;

	InvestorOrder order;
	order.InstrumentCode = type::InstrumentCode(instCode.c_str());
	order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
	order.Exchange = type::Exchange::CME;
	bool isEmpty = (0 == strlen(info->OrderNo));
	if ((order.InstID == INVALID_INSTRUMENT_ID) && (false == isEmpty)) {
		Log::Warn(fmt::format("ESUNNYTradeHandler: OnRspQryOrder filter not support Instrument[{}], UserID[{}] SystemID[{}] LocalID[{}]"
			, order.InstrumentCode, m_investor, info->OrderNo, order.OrderLocalID));
		if (last) {
			Log::Info("ESUNNYTradeHandler: QueryOrders Success");
			m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryOrder);
		}
		return;
	}
	order.InvestorID = m_investor;
	order.Direction = ConvertDirectionFromESUNNY(info->OrderSide);
	order.OffsetFlag = type::OffsetFlag::Unknown;
	order.OrderPriceType = info->TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_FAK ? type::OrderPriceType::FAK : type::OrderPriceType::LimitPrice;
	order.LimitPrice = info->OrderPrice;
	order.Volume = info->OrderQty;
	order.VolumeTrade = info->OrderMatchQty;
	order.VolumeRemain = info->OrderQty - info->OrderMatchQty;
	order.OrderSysID = info->OrderNo;
	order.OrderStatus = GetOrderStatus(info->OrderState);
	order.OrderLocalID = info->RefInt;
	order.InsertTime = fromDateTimeToTime(info->OrderInsertTime);
	if (order.OrderStatus == type::OrderStatus::Canceled) {
		order.CancelTime = fromDateTimeToTime(info->OrderUpdateTime);
	}

	if (LocalIdACC::IsMine(info->RefInt)) {
		char localID[16] = "";
		sprintf_s(localID, "%012d", info->RefInt);
		order.UniqueID = localID;
	} else {
		order.UniqueID = info->OrderNo;
	}

	m_publisher.NotifyUI(order);
	if (last) {
		Log::Info("ESUNNYTradeHandler: QueryOrders Success");
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryOrder);
	}

	char const* direction = IsBuy(order.Direction) ? "B" : "S";
	Log::Info(fmt::format("ESUNNYTradeHandler OnRspQryOrder: UserID[{}] Instrument[{}] Dir[{}] Price[{:.3f}] Quantity[{}] Deal[{}] Remain[{}] SystemID[{}] LocalID[{}] Type[{:c}] OrderStatus[{:c}]",
		order.InvestorID, order.InstrumentCode, direction,  order.LimitPrice.value(), order.Volume, order.VolumeTrade, order.VolumeRemain,
		order.OrderSysID, info->RefInt, order.OrderPriceType, info->OrderState));
}
void ESUNNYTradeHandler::OnRspQryFill(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TapAPIFillInfo* info) {
	if (ITapTrade::TAPIERROR_SUCCEED != errorCode) {
		Error error = Error{ int(errorCode), GetErrorDescription(errorCode).c_str() };
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryFill Failed ErrorCode[{}] ErrorMsg[{}]", error.Code, error.Msg);
		Log::Error(msg);
		m_publisher.NotifyFailToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryTrade, error);
		return;
	}

	if (info == nullptr) {
		auto msg = fmt::format("ESUNNYTradeHandler: OnRspQryFill No orders");
		Log::Info(msg);
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryTrade);
		return;
	}
	bool last = isLast == ITapTrade::APIYNFLAG_YES;
	std::string instCode = info->CommodityNo;
	instCode += info->ContractNo;

	InvestorTrade trade;
	trade.InstrumentCode = type::InstrumentCode(instCode.c_str());
	trade.InstID = ServiceSinglton::Services().Instrument->toID(trade.InstrumentCode);
	trade.Exchange = type::Exchange::CME;
//	trade.OrderLocalID = info->OrderNo; todo
	if (trade.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("ESUNNYTradeHandler: OnRspQryFill filter not support Instrument[{}], UserID[{}] SystemID[{}]", 
			trade.InstrumentCode, m_investor, info->OrderNo));
		if (last) {
			Log::Info("ESUNNYTradeHandler: QueryTrades Success");
			m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryTrade);
		}
		return;
	}
	trade.InvestorID = m_investor;
	trade.Direction = ConvertDirectionFromESUNNY(info->MatchSide);
	trade.OffsetFlag = type::OffsetFlag::Unknown;
//	trade.OrderPriceType = info->TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_FAK ? 
//		type::OrderPriceType::FAK : type::OrderPriceType::LimitPrice; //todo
	trade.OrderSysID = info->OrderNo;
	trade.ProductClass = type::ProductClass::Future;
	trade.TradeID = info->MatchNo;
	trade.TradePrice = info->MatchPrice;
	trade.TradeVolume = info->MatchQty;
	trade.TradeTime = lastN(info->MatchDateTime,8);


	m_publisher.NotifyUI(trade);
	if (last) {
		Log::Info("ESUNNYTradeHandler: QueryTrades Success");
		m_publisher.NotifyDoneToUI(TradePlatform::HKEX_ESUNNY, InitProcess::QueryTrade);
	}
	char const* direction = IsBuy(trade.Direction) ? "B" : "S";
	Log::Info(fmt::format("ESUNNYTradeHandler OnRspQryFill: UserID[{}] Instrument[{}] Dir[{}] Price[{:.3f}] Quantity[{}] SystemID[{}]",
		trade.InvestorID, trade.InstrumentCode, direction, trade.TradePrice, trade.TradeVolume, trade.OrderSysID));

}

void ESUNNYTradeHandler::OnRtnPosition(const ITapTrade::TapAPIPositionInfo* info) {
	return;
	//推送持仓明细，暂时不用，打个日志看看推送频率
	Log::Warn(fmt::format("ETH: OnRtnPosition, Account[{}] Instrument[{}{}] Direction[{}] Quantity[{}] AvgPrice[{}] MatchNo[{}]"
		,info->AccountNo, info->CommodityNo, info->ContractNo, info->MatchSide, info->PositionQty, info->PositionPrice, info->MatchNo));
}

void ESUNNYTradeHandler::OnRtnPositionSummary(const ITapTrade::TapAPIPositionSummary* info) {
	return;
	//推送持仓汇总，暂时不用，打个日志看看推送频率
	Log::Debug(fmt::format("ETH: OnRtnPositionSummary, Account[{}] Instrument[{}{}] Direction[{}] Quantity[{}] AvgPrice[{}]"
		, info->AccountNo, info->CommodityNo, info->ContractNo, info->MatchSide, info->PositionQty, info->PositionPrice));
}

//交易员补单指令
void ESUNNYTradeHandler::OnRspOrderLocalRemove(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalRemoveRsp* info) {

}
void ESUNNYTradeHandler::OnRspOrderLocalInput(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalInputRsp* info) {

}
void ESUNNYTradeHandler::OnRspOrderLocalModify(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderLocalModifyRsp* info) {

}
void ESUNNYTradeHandler::OnRspFillLocalInput(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIFillLocalInputRsp* info) {

}
void ESUNNYTradeHandler::OnRspFillLocalRemove(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIFillLocalRemoveRsp* info) {

}

void ESUNNYTradeHandler::OnRtnContactInfo(ITapTrade::TAPIINT32 errorCode, ITapTrade::TAPIYNFLAG isLast, const ITapTrade::TAPISTR_40 ContactInfo) {
	ITapTrade::TAPIINT32 nRtn = m_pTradeApi->RequestVertificateCode(&m_sessionID, const_cast<char*>(ContactInfo));
	if (ITapTrade::TAPIERROR_SUCCEED != nRtn) {
		auto msg = fmt::format("ETH: RequestVertificateCode failed, {}", GetErrorDescription(nRtn));
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	} else {
// 		TradePlatformNotification notification;
// 		notification.Platform = TradePlatform::HKEX_ESUNNY;
// 		notification.Type = TradePlatformNotification::CODE_LOGIN;		//二次认证登陆
// 		m_publisher.NotifyUI(notification);
	}
}
void ESUNNYTradeHandler::OnRspRequestVertificateCode(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIRequestVertificateCodeRsp* rsp) {
	if (ITapTrade::TAPIERROR_SUCCEED == errorCode) {
		Log::Info("ETH: VertificateCode success");
	} else {
		Log::Error(fmt::format("ETH: VertificateCode Failed, {}", GetErrorDescription(errorCode)));
	}
}
std::string ESUNNYTradeHandler::GetErrorDescription(ITapTrade::TAPIINT32 errorCode) {
	switch (errorCode) {
	/***************************************登录过程错误***************************************/
	case ITapTrade::TAPIERROR_LOGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录过程执行错误");
	case ITapTrade::TAPIERROR_LOGIN_USER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户不存在");
	case ITapTrade::TAPIERROR_LOGIN_DDA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要进行动态认证");
	case ITapTrade::TAPIERROR_LOGIN_LICENSE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户未授权");
	case ITapTrade::TAPIERROR_LOGIN_MODULE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录模块不正确");
	case ITapTrade::TAPIERROR_LOGIN_FORCE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要强制修改密码");
	case ITapTrade::TAPIERROR_LOGIN_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录状态禁止登陆");
	case ITapTrade::TAPIERROR_LOGIN_PASS:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录密码不正确");
	case ITapTrade::TAPIERROR_LOGIN_RIGHT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有该模块登录权限");
	case ITapTrade::TAPIERROR_LOGIN_COUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录数量超限");
	case ITapTrade::TAPIERROR_LOGIN_NOTIN_SERVERFLAGUSRES:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户不在服务器标识下可登录用户列表中");
	case ITapTrade::TAPIERROR_LOGIN_FREEZE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "登录用户已被冻结");
	case ITapTrade::TAPIERROR_LOGIN_TOFREEZE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "密码错误，用户冻结");
	case ITapTrade::TAPIERROR_LOGIN_ACCOUNTSTATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户状态不允许登录");
	case ITapTrade::TAPIERROR_LOGIN_SECCERTIFI:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "需要进行二次认证");
	case ITapTrade::TAPIERROR_LOGIN_NOSECONDSET:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未绑定二次认证信息");
	case ITapTrade::TAPIERROR_LOGIN_NOTURSTHOST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不受信任的计算机登录");
	case ITapTrade::TAPITAPIERROR_SECONDCERTIFICATION_FAIL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "二次验证失败");
	case ITapTrade::TAPITAPIERROR_SECONDCERTIFICATION_TIMEOVER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "二次验证超时");
	/***************************************交易服务错误***************************************/
	case ITapTrade::TAPIERROR_ORDERINSERT_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "资金账号不存在");
	case ITapTrade::TAPIERROR_ORDERINSERT_ACCOUNT_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "资金账号状态不正确");
	case ITapTrade::TAPIERROR_ORDERINSERT_TRADECENT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "资金账号交易中心不一致");
	case ITapTrade::TAPIERROR_ORDERINSERT_CONTRACT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单无效的合约");
	case ITapTrade::TAPIERROR_ORDERINSERT_LME_NOTREADY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "LME未准备就绪");
	case ITapTrade::TAPIERROR_ORDERINSERT_ERROR_ORDER_TYPE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不支持的下单类型");
	case ITapTrade::TAPIERROR_ORDERINSERT_ORDER_TYPE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不合法的委托类型");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户权限禁止交易");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_COM_GROUP:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户品种分组禁止交易");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_SYSTEM:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "系统权限禁止交易");
	case ITapTrade::TAPIERROR_ORDER_CLOSE_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户权限只可平仓");
	case ITapTrade::TAPIERROR_ORDER_CLOSE_SYSTEM:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "系统权限只可平仓");
	case ITapTrade::TAPIERROR_ORDER_NOTRADE_RISK:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户品种风控权限禁止交易");
	case ITapTrade::TAPIERROR_ORDER_CLOSE_RISK:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "客户品种风控权限只可平仓");
	case ITapTrade::TAPIERROR_ORDERINSERT_POSITIONMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "持仓量超过最大限制");
	case ITapTrade::TAPIERROR_ORDERINSERT_ONCEMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单超过单笔最大量");
	case ITapTrade::TAPIERROR_ORDERINSERT_TRADEROUTE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单合约无交易路由");
	case ITapTrade::TAPIERROR_ORDER_IN_MOD_PRICE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托价格超出偏离范围");
	case ITapTrade::TAPIERROR_ORDER_IN_GIVEUP_POS_MAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "超过GiveUp最大持仓量");
	case ITapTrade::TAPIERROR_UPPERCHANNEL_NOT_LOGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未登录网关");
	case ITapTrade::TAPIERROR_UPPERCHANNEL_NOT_FOUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未找到网关信息");
	case ITapTrade::TAPIERROR_ORDERINSERT_NOTENOUGHFUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单资金不足");
	case ITapTrade::TAPIERROR_ORDERINSERT_FEE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "手续费参数错误");
	case ITapTrade::TAPIERROR_ORDERINSERT_MARGIN:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "保证金参数错误");
	case ITapTrade::TAPIERROR_ORDERINSERT_BASENOFUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "总基币资金不足");
	case ITapTrade::TAPIERROR_ORDERDELETE_NOT_SYSNO:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "撤单无此系统号");
	case ITapTrade::TAPIERROR_ORDERDELETE_NOT_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态不允许撤单");
	case ITapTrade::TAPIERROR_ORDERDELETE_NO_INPUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单不允许撤单");
	case ITapTrade::TAPIERROR_ORDERMODIFY_NOT_STATE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态不允许改单");
	case ITapTrade::TAPIERROR_ORDERINPUT_REPEAT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单重复");
	case ITapTrade::TAPIERROR_UPPER_ONCEMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单超过上手单笔最大量");
	case ITapTrade::TAPIERROR_UPPER_POSITIONMAX:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下单超过上手最大持仓量");
	case ITapTrade::TAPIERROR_ORDERINSERT_CLOSEMODE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "开平方式错误");
	case ITapTrade::TAPIERROR_CLOSE_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托平仓持仓不足");
	case ITapTrade::TAPIERROR_CLOSE_MATCH:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交平仓失败");
	case ITapTrade::TAPIERROR_MOD_DEL_NO_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未找到本地委托");
	case ITapTrade::TAPIERROR_MOD_DEL_GATEWAY_DISCON:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "与网关断开连接");
	case ITapTrade::TAPIERROR_MATCHINPUT_REPEAT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交重复");
	case ITapTrade::TAPIERROR_MATCHINPUT_NO_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交未找到对应委托");
	case ITapTrade::TAPIERROR_MATCHINPUT_NO_CONTRACT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交合约不存在");
	case ITapTrade::TAPIERROR_MATCHINPUT_PARM_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交参数错误");
	case ITapTrade::TAPIERROR_MATCHINPUT_OSTATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "录单成交委托状态错误");
	case ITapTrade::TAPIERROR_MATCHREMOVE_NO_MATCH:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交删除未找到成交");
	case ITapTrade::TAPIERROR_MATCHREMOVE_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态成交不可删");
	case ITapTrade::TAPIERROR_ORDERINPUT_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不允许录入此状态订单");
	case ITapTrade::TAPIERROR_ORDERINPUT_MOD_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "错误的修改订单请求");
	case ITapTrade::TAPIERROR_ORDERREMOVE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "订单不可删，存在对应成交");
	case ITapTrade::TAPIERROR_ORDERINPUT_MOD_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不合法的委托状态");
	case ITapTrade::TAPIERROR_ORDEREXCHANGE_STATE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "此状态不允许订单转移");
	case ITapTrade::TAPIERROR_ORDERREMOVE_NOT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "订单不允许删除");
	case ITapTrade::TAPIERROR_ORDERMARKET_DELETE_NOTFOUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单未找到委托");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_ACCOUNT_NE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单客户不一致");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_COMMODITY_NE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单品种不一致");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_CONTRACT_NE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单合约不一致");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_SIDE_EQ:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单买卖方向相同");
	case ITapTrade::TAPIERROR_ORDERMARKET_DEL_SIDE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商双边撤单买卖方向错误");
	case ITapTrade::TAPIERROR_ORDERMARKET_OTHER_SIDE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "做市商单边检查未通过");
	/***************************************网关错误代码***************************************/
	case ITapTrade::TAPIERROR_GW_NOT_READY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "网关未就绪，未连接上手");
	case ITapTrade::TAPIERROR_GW_INVALID_COMMODITY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "品种错误");
	case ITapTrade::TAPIERROR_GW_INVALID_CONTRACT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "合约错误");
	case ITapTrade::TAPIERROR_GW_INVALID_FIELD:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "报单字段有误");
	case ITapTrade::TAPIERROR_GW_INVALID_PRICE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "价格不合法");
	case ITapTrade::TAPIERROR_GW_INVALID_VOLUME:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "数量不合法");
	case ITapTrade::TAPIERROR_GW_INVALID_TYPE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "报单类型不合法");
	case ITapTrade::TAPIERROR_GW_INVALID_MODE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托模式不合法");
	case ITapTrade::TAPIERROR_GW_ORDER_NOT_EXIST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "委托不存在（改单、撤单)");
	case ITapTrade::TAPIERROR_GW_SEND_FAIL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "发送报单失败");
	/***************************************前置返回错误***************************************/
	case ITapTrade::TAPIERROR_TRADEFRONT_MODULETYPEERR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置不允许该模块登录");
	case ITapTrade::TAPIERROR_TRADEFRONT_TOOMANYDATA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "一次请求太多数据");
	case ITapTrade::TAPIERROR_TRADEFRONT_NODATA:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置没有所要数据");
	case ITapTrade::TAPIERROT_TRADEFRONT_NOUSER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "所查询的操作员信息不存在");
	case ITapTrade::TAPIERROR_TRADEFRONT_DISCONNECT_TRADE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置与交易断开");
	case ITapTrade::TAPIERROR_TRADEFRONT_DISCONNECT_MANAGE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "前置与管理断开");
	case ITapTrade::TAPIERROR_TRADEFRONT_ACCOUNT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "下属资金账号不存在");
	case ITapTrade::TAPIERROR_TRADEFRONT_ORDER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "该操作员不允许交易");
	case ITapTrade::TAPIERROR_TRADEFRONT_FREQUENCY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "查询频率过快");
	case ITapTrade::TAPIERROR_TRADEFRONT_RUFUSE:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "该授权不予许登录");
	case ITapTrade::TAPIERROR_TRADEFRONT_SELFMATCH:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "自成交验证不通过");
	/***************************************网络断开错误***************************************/
	case ITapTrade::TAPIERROR_ConnectFail:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "连接服务失败");
	case ITapTrade::TAPIERROR_LinkAuthFail:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "链路认证失败");
	case ITapTrade::TAPIERROR_HostUnavailable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "主机地址不可用");
	case ITapTrade::TAPIERROR_SendDataError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "发送数据错误");
	case ITapTrade::TAPIERROR_NOFrontAvailable:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没用可用的接入前置");
	case ITapTrade::TAPIERROR_RepeatLogin:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "重复登录");
	case ITapTrade::TAPIERROR_InnerError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "内部错误");
	case ITapTrade::TAPIERROR_LastReqNotFinish:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "上一次请求还没有结束");
	case ITapTrade::TAPIERROR_InputValueError:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "输入参数非法");
	case ITapTrade::TAPIERROR_AuthCode_Invalid:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码不合法");
	case ITapTrade::TAPIERROR_AuthCode_Expired:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码超期");
	case ITapTrade::TAPIERROR_AuthCode_TypeNotMatch:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "授权码类型不匹配");
	case ITapTrade::TAPIERROR_API_NotReady:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "API还没有准备好");
	case ITapTrade::TAPIERROR_UDP_LISTEN_FAILED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "UDP端口监听失败");
	case ITapTrade::TAPIERROR_UDP_LISTENING:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "UDP正在监听");
	case ITapTrade::TAPIERROR_NotImplemented:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "接口未实现");
	case ITapTrade::TAPIERROR_CallOneTimeOnly:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "每次登陆只允许调用一次");
	case ITapTrade::TAPIERROR_ORDER_FREQUENCY:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "超过下单频率");
	case ITapTrade::TAPIERROR_RENTQRY_TOOFAST:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "查询频率太快");
	case ITapTrade::TAPIERROR_CALL_NOCONDITION:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "不符合调用条件");
	case ITapTrade::TAPIERROR_ORDER_NOTFOUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "改单撤单时没有找到对应订单");
	case ITapTrade::TAPIERROR_RIGHT_ORDERINPUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有订单录入或者成交录入");
	case ITapTrade::TAPIERROR_RIGHT_LOCALOPERATION:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有订单修改和订单删除权限，成交删除权限");
	case ITapTrade::TAPIERROR_RIGHT_ORDERTRANSFER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "没有订单转移权限");
	case ITapTrade::TAPIERROR_FILLINPUT_SYSTEMNO:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交录入时系统号为空");
	case ITapTrade::TAPIERROR_FILLREMOVE_MATCHNO:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交删除时成交号为空");
	case ITapTrade::TAPIERROR_FILLREQMOVE_NOFUND:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "成交删除时没有找到对应的成交");
	/***************************************网络断开错误***************************************/
	case ITapTrade::TAPIERROR_DISCONNECT_CLOSE_INIT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "主动断开");
	case ITapTrade::TAPIERROR_DISCONNECT_CLOSE_PASS:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "被动断开");
	case ITapTrade::TAPIERROR_DISCONNECT_READ_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "读错误");
	case ITapTrade::TAPIERROR_DISCONNECT_WRITE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "写错误");
	case ITapTrade::TAPIERROR_DISCONNECT_BUF_FULL:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "缓冲区满");
	case ITapTrade::TAPIERROR_DISCONNECT_IOCP_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "异步操作错误");
	case ITapTrade::TAPIERROR_DISCONNECT_PARSE_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "解析数据错误");
	case ITapTrade::TAPIERROR_DISCONNECT_CONNECT_TIMEOUT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "连接超时");
	case ITapTrade::TAPIERROR_DISCONNECT_INIT_ERROR:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "初始化失败");
	case ITapTrade::TAPIERROR_DISCONNECT_HAS_CONNECTED:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "已经连接");
	case ITapTrade::TAPIERROR_DISCONNECT_HAS_EXIT:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "工作线程已结束");
	case ITapTrade::TAPIERROR_DISCONNECT_TRY_LATER:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "操作正在进行，请稍后重试");
	default:
		return fmt::format("ErrorCode[{}] ErrorMsg[{}]", errorCode, "未知错误");
	}
}

bool ESUNNYTradeHandler::IsOrdered(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为已排队；有系统号；成交量为0
	return (ITapTrade::TAPI_ORDER_STATE_QUEUED ==order.OrderState) && (0==order.OrderMatchQty);
}
bool ESUNNYTradeHandler::IsCanceled(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为完全撤单、已撤余单；有系统号；同时委托量大于成交量。PS：FAK当作撤单处理
	return (ITapTrade::TAPI_ORDER_STATE_CANCELED==order.OrderState || ITapTrade::TAPI_ORDER_STATE_LEFTDELETED==order.OrderState) && (order.OrderQty>order.OrderMatchQty);
}
bool ESUNNYTradeHandler::IsPartial(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为部分成交；有系统号；同时委托量大于成交量
	return (ITapTrade::TAPI_ORDER_STATE_PARTFINISHED==order.OrderState) && (order.OrderQty>order.OrderMatchQty);
}
bool ESUNNYTradeHandler::IsAllTraded(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为完全成交；成交量等于委托量
	return (ITapTrade::TAPI_ORDER_STATE_FINISHED==order.OrderState) && (order.OrderMatchQty==order.OrderQty);
}
bool ESUNNYTradeHandler::IsAccepted(ITapTrade::TapAPIOrderInfo const& order) const {
	//报单状态为已受理；没有系统号
	return (ITapTrade::TAPI_ORDER_STATE_ACCEPT==order.OrderState);
}
bool ESUNNYTradeHandler::IsSendFailed(ITapTrade::TapAPIOrderInfo const& order) const {
	//居然是通过ErrorCode来判断的，奇葩的选择
	return (ITapTrade::TAPIERROR_SUCCEED!=order.ErrorCode);
}

bool IsFakOrder(ITapTrade::TapAPIOrderInfo const& order) {
	return order.TimeInForce == ITapTrade::TAPI_ORDER_TIMEINFORCE_FAK;
}

void ESUNNYTradeHandler::OnRtnOrder(const ITapTrade::TapAPIOrderInfoNotice* info) {
	if (NULL == info)	return;

	InvestorOrder order;
	order.Platform = TradePlatform::HKEX_ESUNNY;
	char instCode[16] = "";
	sprintf_s(instCode, "%s%s", info->OrderInfo->CommodityNo, info->OrderInfo->ContractNo);
	order.InstrumentCode = type::InstrumentCode(instCode);
	order.InstID = ServiceSinglton::Services().Instrument->toID(order.InstrumentCode);
	bool isEmpty = (0 == strlen(info->OrderInfo->OrderSystemNo));
	if ((order.InstID == INVALID_INSTRUMENT_ID) && (false == isEmpty)) {
		Log::Warn(fmt::format("ESUNNYTradeHandler: OnRtnOrder filter not support Instrument[{}], SystemID[{}] LocalID[{}]",
			instCode, info->OrderInfo->OrderSystemNo, info->OrderInfo->RefInt));
		return;
	}
	auto inst = ServiceSinglton::Services().Instrument->Get(order.InstID);
	order.Exchange = inst->Exchange;
	order.Direction = ConvertDirectionFromESUNNY(info->OrderInfo->OrderSide);
	order.OffsetFlag = type::OffsetFlag::Unknown;
	order.OrderPriceType = ConvertOrderTypeFromESUNNY(*info->OrderInfo);
	order.LimitPrice = info->OrderInfo->OrderPrice;
	order.Volume = info->OrderInfo->OrderQty;
	order.VolumeTrade = info->OrderInfo->OrderMatchQty;
	order.VolumeRemain = order.Volume - order.VolumeTrade;
	order.OrderLocalID = info->OrderInfo->RefInt;
	order.OrderSysID = info->OrderInfo->OrderNo;
	order.OrderStatus = GetOrderStatus(info->OrderInfo->OrderState);
	if (LocalIdACC::IsMine(order.OrderLocalID)) {
		sprintf_s(order.UniqueID.data(), "%012d", order.OrderLocalID.value());
	} else {
		order.UniqueID = info->OrderInfo->OrderNo;
	}
	order.InsertTime = fromDateTimeToTime(info->OrderInfo->OrderInsertTime);
	if (order.OrderStatus == type::OrderStatus::Canceled) {
		order.CancelTime = fromDateTimeToTime(info->OrderInfo->OrderUpdateTime);
	}

	auto* pOrder = info->OrderInfo;
	char const* direction = IsBuy(order.Direction) ? "B" : "S";
	auto msg = fmt::format(" Inst[{}] Dir[{}] Price[{:.3f}] Quantity[{}] Deal[{}] SystemID[{}] LocalID[{}] Type[{:c}] OrderStatus[{:c}]",
		instCode, direction, pOrder->OrderPrice, pOrder->OrderQty, pOrder->OrderMatchQty,
		pOrder->OrderNo, pOrder->RefInt, order.OrderPriceType, pOrder->OrderState);

	if (info->ErrorCode != ITapTrade::TAPIERROR_SUCCEED) {
		OrderErrorNotification noti;
		noti.Platform = TradePlatform::HKEX_ESUNNY;
		noti.Type = OrderErrorNotification::Insert;
		noti.OrderLocalID = info->OrderInfo->RefInt;
		noti.Error = Error { info->ErrorCode, GetErrorDescription(info->ErrorCode).c_str() };
		m_publisher.NotifyUI(noti);
		m_publisher.PushResponse(noti);
		Log::Error(fmt::format("ESUNNYTradeHandler OrderInsert Failed:Error[{}:{}] {}",noti.Error.Code, noti.Error.Msg, msg));
		m_publisher.NotifyUI(fmt::format("OrderInsert Failed:Code[{}] Msg[{}]", noti.Error.Code, noti.Error.Msg));

	} else {
		m_publisher.NotifyUI(OrderRtnNotification{ order });
		m_publisher.PushResponse(order);
		Log::Info(fmt::format("ESUNNYTradeHandler OnRtnOrder:{}", msg));
	}
}


void ESUNNYTradeHandler::OnRtnFill(const ITapTrade::TapAPIFillInfo* info) {
	//	return; // 全部使用OnRtnOrder，RtnFill里是增量成交
	if (NULL == info) return;

	char instCode[20] = "";
	sprintf_s(instCode, "%s%s", info->CommodityNo, info->ContractNo);

	Log::Warn(fmt::format("ETH:OnRtnFill, Inst[{}] Dir[{}] SysNo[{}] Deal[{:.2f},{}] ",
		instCode, info->MatchSide, info->OrderNo, info->MatchPrice, info->MatchQty));
	InvestorTrade trade;
	trade.Platform = TradePlatform::HKEX_ESUNNY;
	trade.InstrumentCode = type::InstrumentCode(instCode);
	trade.InstID = ServiceSinglton::Services().Instrument->toID(trade.InstrumentCode);
	if (trade.InstID == INVALID_INSTRUMENT_ID) {
		Log::Warn(fmt::format("ESUNNYTradeHandler: OnRtnTrade filter not support Instrument[{}], SystemID[{}] LocalID[{}]", 
			instCode, info->OrderNo));
		return;
	}
	trade.OrderSysID = info->OrderNo;
	auto order = ServiceSinglton::Services().Order->GetOrder(trade.OrderSysID);
	if (order) {
		trade.OrderLocalID = order->OrderLocalID;
	}
	trade.Exchange = ConvertExchangeFromESUNNY(info->ExchangeNo);
	trade.Direction = ConvertDirectionFromESUNNY(info->MatchSide);
	trade.OffsetFlag = type::OffsetFlag::Unknown;
	trade.TradeVolume = info->MatchQty;
	trade.TradeTime = lastN(info->MatchDateTime,8);
	trade.TradePrice = info->MatchPrice;
	trade.OrderLocalID = order ? order->OrderLocalID : 0;
	trade.TradeID = info->MatchNo;
	m_publisher.NotifyUI(TradeNotification{ trade });
	m_publisher.PushResponse(trade);

	char const* direction = IsBuy(trade.Direction) ? "B" : "S";
	Log::Info(fmt::format("ESUNNYTradeHandler OnRtnTrade: Instrument[{}] Dir[{}] Price[{:.3f}] Quantity[{}] SystemID[{}] SysID2[{}] LocalID[{}]",
		instCode, direction, trade.TradePrice, trade.TradeVolume, info->OrderNo, info->OrderSystemNo, trade.OrderLocalID));
}

//ESunny的委托错误应答在这里
void ESUNNYTradeHandler::OnRspOrderAction(ITapTrade::TAPIUINT32 sessionID, ITapTrade::TAPIINT32 errorCode, const ITapTrade::TapAPIOrderActionRsp* info) {
	if (NULL == info)	return;
	if (errorCode == ITapTrade::TAPIERROR_SUCCEED) return;

	OrderErrorNotification noti;
	noti.Platform = TradePlatform::HKEX_ESUNNY;
	if (info->ActionType == ITapTrade::APIORDER_INSERT) {
		noti.Type = OrderErrorNotification::Insert;
	} else {
		noti.Type = OrderErrorNotification::Cancel;
	}
	noti.OrderLocalID = info->OrderInfo->RefInt;
	noti.Error = Error{ errorCode, GetErrorDescription(errorCode).c_str() };
	m_publisher.NotifyUI(noti);
	m_publisher.PushResponse(noti);
	Log::Error(fmt::format("ESUNNYTradeHandler OrderInsert/Cancel Failed:Error[{}:{}] {}", noti.Error.Code, noti.Error.Msg, info->OrderInfo->RefInt));
	m_publisher.NotifyUI(fmt::format("OrderInsert/Cancel Failed:Code[{}] Msg[{}]", noti.Error.Code, noti.Error.Msg));
}
