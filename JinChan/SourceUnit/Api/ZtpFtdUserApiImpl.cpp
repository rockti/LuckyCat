#include "stdafx.h"
#include "ZtpFtdUserApiImpl.h"
#include "CTime.h"
#include <fmt/format.h>

CZtpFtdUserApi* CZtpFtdUserApi::CreateFtdUserApi(char const* pszFlowPath, char const* pszUserApiType)
{
	Log::Debug("Step2: CreateFtdUserApi, new IOThread but not Start");
	return new CZtpFtdUserApiImpl(pszFlowPath, pszUserApiType, new CSelectThread());
}

static CProductClassType ConvertProductClass2ZTP(TZtpApiProductClassType const& productClass)
{
	switch (productClass)
	{
	case ZTP_API_PCT_Spot:
		return ZTP_PCT_Spot;
	case ZTP_API_PCT_Defer:
		return ZTP_PCT_Defer;
	case ZTP_API_PCT_Delivery:
		return ZTP_PCT_Delivery;
	case ZTP_API_PCT_Neutral:
		return ZTP_PCT_Neutral;
	default:
		return ZTP_PCT_Unknown;
	}
}
static CDirectionType ConvertDirection2ZTP(TZtpApiDirectionType const& direction)
{
	return (ZTP_API_D_Buy==direction) ? ZTP_D_Buy : ZTP_D_Sell;
}
static COffsetFlagType ConvertOperation2ZTP(TZtpApiOffsetFlagType const& operation)
{
	switch (operation)
	{
	case ZTP_API_OF_Open:
		return ZTP_OF_Open;			//开仓
	case ZTP_API_OF_Close:
		return ZTP_OF_Close;		//平仓
	case ZTP_API_OF_CloseToday:
		return ZTP_OF_CloseToday;	//平今
	case ZTP_API_OF_ForceClose:
		return ZTP_OF_ForceClose;	//强平
	default:
		return ZTP_OF_ForceClose;
	}
}
static CHedgeFlagType ConvertHedgeFlag2ZTP(TZtpApiHedgeFlagType const& hedgeFlag)
{
	switch (hedgeFlag)
	{
	case ZTP_API_Speculation:
		return ZTP_HF_Speculation;		//投机|延期
	case ZTP_API_Arbitrage:
		return ZTP_HF_Arbitrage;		//套利|中立仓
	case ZTP_API_Hedge:
		return ZTP_HF_Hedge;			//套保|交割
	default:
		return ZTP_HF_MarketMaker;		//做市商
	}
}
static COrderPriceTypeType ConvertPriceType2ZTP(TZtpApiOrderPriceType const& type)
{
	switch (type)
	{
	case ZTP_API_LimitPrice_FAK:
		return ZTP_OPT_FAKPrice;	//FAK
	case ZTP_API_LimitPrice_FOK:
		return ZTP_OPT_FOKPrice;	//FOK
	default:
		return ZTP_OPT_LimitPrice;	//限价
	}
}

static TZtpApiOrderStatusType ConvertStatusFromZTP(COrderStatusType const& status)
{
	switch (status)
	{
	case ZTP_OS_NoTradeQueueing:
		return ZTP_API_IS_NoTrade;		//未成交
	case ZTP_OS_PartTradedQueueing:
		return ZTP_API_IS_PartTraded;	//部分成交
	case ZTP_OS_AllTraded:
		return ZTP_API_IS_AllTraded;	//全部成交
	case ZTP_OS_Canceled:
		return ZTP_API_IS_Canceled;		//撤单
	case ZTP_OS_InsertQueueing:
	case ZTP_OS_NoTradeNotQueueing:
		return ZTP_API_IS_Queueing;		//正在报入
	case ZTP_OS_CancelFail:
	case ZTP_OS_InsertFail:
		return ZTP_API_IS_Failed;		//正在报入
	default:
		Log::Debug(fmt::format("ConvertDirectionFromZTP: unknown status[{}]", status));
		return ZTP_API_IS_Queueing;		//正在报入
	}
}
static TZtpApiProductClassType ConvertProductClassFromZTP(CProductClassType const& productClass)
{
	switch (productClass)
	{
	case ZTP_PCT_Spot:
		return ZTP_API_PCT_Spot;
	case ZTP_PCT_Defer:
		return ZTP_API_PCT_Defer;
	case ZTP_PCT_Delivery:
		return ZTP_API_PCT_Delivery;
	case ZTP_PCT_Neutral:
		return ZTP_API_PCT_Neutral;
	default:
		Log::Debug(fmt::format("ConvertProductClassFromZTP: unknown class[{}]", productClass.getValue()));
		return ZTP_API_PCT_Unknown;		//正在报入
	}
}
static TZtpApiDirectionType ConvertDirectionFromZTP(CDirectionType const& direction)
{
	return (ZTP_D_Buy==direction) ? ZTP_API_D_Buy : ZTP_API_D_Sell;
}
static TZtpApiOffsetFlagType ConvertOperationFromZTP(COffsetFlagType const& operation)
{
	switch (operation)
	{
	case ZTP_OF_Open:
		return ZTP_API_OF_Open;			//开仓
	case ZTP_OF_Close:
		return ZTP_API_OF_Close;		//平仓
	case ZTP_OF_CloseToday:
		return ZTP_API_OF_CloseToday;	//平今
	case ZTP_OF_ForceClose:
		return ZTP_API_OF_ForceClose;	//强平
	default:
		throw 1;
	}
}
static TZtpApiHedgeFlagType ConvertHedgeFlagFromZTP(CHedgeFlagType const& hedgeFlag)
{
	switch (hedgeFlag)
	{
	case ZTP_HF_Speculation:
		return ZTP_API_Speculation;		//投机|延期
	case ZTP_HF_Arbitrage:
		return ZTP_API_Arbitrage;		//套利|中立仓
	case ZTP_HF_Hedge:
		return ZTP_API_Hedge;			//套保|交割
	default:
		Log::Debug(fmt::format("ConvertHedgeFlagFromZTP: unknown type[{}]", hedgeFlag));
		return ZTP_API_MarketMaker;		//做市商
	}
}
static TZtpApiOrderPriceType ConvertPriceTypeFromZTP(COrderPriceTypeType const& type)
{
	switch (type)
	{
	case ZTP_OPT_FAKPrice:
		return ZTP_API_LimitPrice_FAK;	//未成交
	case ZTP_OPT_FOKPrice:
		return ZTP_API_LimitPrice_FOK;	//部分成交
	default:
		Log::Debug(fmt::format("ConvertPriceTypeFromZTP: unknown type[{}]", type));
		return ZTP_API_LimitPrice;		//系统拒绝
	}
}

CZtpFtdUserApiImpl::CZtpFtdUserApiImpl(char const* pszFlowPath, char const* pszUserApiType, CSelectThread* pReactor)
:CFtdUserApiImplBase(pszFlowPath,pszUserApiType,pReactor)
{
	Log::Debug("Step4: CZtpFtdUserApiImpl constructed");
}

void CZtpFtdUserApiImpl::HandleResponse(CFTDPackage* pMessage, WORD nSequenceSeries) {
	int fTID = pMessage->GetTID();
	switch(fTID) 
	{		
	case FTD_TID_RspError:				//错误应答
		OnRspError(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspUserLogin:			//用户登录应答
		OnRspUserLogin(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspUserLogout:			//用户退出应答
		OnRspUserLogout(pMessage, nSequenceSeries);
		break;
	case FID_TID_RspUserPasswordUpdate:			//用户退出应答
		OnRspUserPasswordUpdate(pMessage, nSequenceSeries);
		break;
	case FID_TID_RspForceLogout:		//强制用户退出应答(暂不提供)
		break;
	case FTD_TID_RspOrderInsert:		//报单录入应答
		OnRspOrderInsert(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspOrderAction:		//报单操作应答
		OnRspOrderAction(pMessage, nSequenceSeries);
		break;
	case FTD_TID_ErrRtnOrderInsert:		//错误报单录入回报
		OnErrRtnOrderInsert(pMessage, nSequenceSeries);
		break;
	case FTD_TID_ErrRtnOrderAction:		//错误报单操作回报
		OnErrRtnOrderAction(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RtnOrder:				//委托回报
		OnRtnOrder(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RtnTrade:				//成交回报
		OnRtnTrade(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RtnInstrumentStatus:	//交易状态改变
		OnRtnInstrumentStatus(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RtnDeferFee:	//交易状态改变
		OnRtnDeferFee(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInstrument:		//合约查询应答
		OnRspQryInstrument(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInvestorAccount:	//资金账户查询;
		OnRspQryInvestorAccount(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInvestorPosition://持仓查询
		OnRspQryInvestorPosition(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInvestorMargin:	//保证金查询
		OnRspQryInvestorMargin(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInvestorFee:		//手续费查询
		OnRspQryInvestorFee(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInvestorOrder:		//手续费查询
		OnRspQryOrder(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspQryInvestorTrade:		//手续费查询
		OnRspQryTrade(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspSubMarketData:		//订阅行情应答
		OnRspSubMarketData(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RspUnSubMarketData:	//退订行情应答
		OnRspUnSubMarketData(pMessage, nSequenceSeries);
		break;
// 	case FTD_TID_IntlRtnDissemination:	//内部订阅信息分发
// 		OnInternalRtnDissemination(pMessage, nSequenceSeries);
		break;
	case FTD_TID_RtnDepthMarketData:	//本地保存的行情快照
	case FTD_TID_IntlRtnDepthMarketData://前置直接发来的实时行情流
		OnRtnDepthMarketData(pMessage, nSequenceSeries);
		break;
	default:
		break;
	}
}
int CZtpFtdUserApiImpl::ReqUserLogout(CZtpApiReqLogoutField* pReqUserLogout, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqUserLogout, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDReqUserLogoutField reqUserLogoutField;
	memcpy(&reqUserLogoutField, pReqUserLogout, sizeof(CFTDReqUserLogoutField));
	FTD_ADD_FIELD(&m_reqPackage, &reqUserLogoutField);

	return RequestToDialogFlow();
}

int CZtpFtdUserApiImpl::ReqUserPasswordUpdate(CZtpApiPasswordUpdateField* pReqUserPasswordUpdate, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FID_TID_ReqUserPasswordUpdate, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDUserPasswordUpdateField reqUserPasswordUpdate;
	reqUserPasswordUpdate.BrokerID = pReqUserPasswordUpdate->BrokerID;
	reqUserPasswordUpdate.InvestorID = pReqUserPasswordUpdate->AccountID;
	reqUserPasswordUpdate.OldPassword = pReqUserPasswordUpdate->OldPassword;
	reqUserPasswordUpdate.NewPassword = pReqUserPasswordUpdate->NewPassword;
	FTD_ADD_FIELD(&m_reqPackage, &reqUserPasswordUpdate);

	return RequestToDialogFlow();
}

int CZtpFtdUserApiImpl::ReqQryInstrument(CZtpApiQryInstrumentField* pQryTopicInstrument, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInstrument, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryInstrumentField qryInstrumentField;
	memcpy(&qryInstrumentField, pQryTopicInstrument, sizeof(CFTDQryInstrumentField));
	FTD_ADD_FIELD(&m_reqPackage, &qryInstrumentField);
	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqQryInvestorAccount(CZtpApiQryAccountField* pQryInvestorAccount, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInvestorAccount, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryInvestorAccountField qryAccountField;
	memcpy(&qryAccountField, pQryInvestorAccount, sizeof(CFTDQryInvestorAccountField));
	FTD_ADD_FIELD(&m_reqPackage, &qryAccountField);
	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqOrderInsert(CZtpApiInputOrderField* pInputOrder, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqOrderInsert, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);
	
 	CFTDInputOrderField inputOrderField;
	inputOrderField.BrokerID = pInputOrder->BrokerID;
	inputOrderField.InvestorID = pInputOrder->InvestorID;
	inputOrderField.ProductClass = ConvertProductClass2ZTP(pInputOrder->ProductClass);
	inputOrderField.ExchangeID = pInputOrder->ExchangeID;
	inputOrderField.InstrumentID = pInputOrder->InstrumentID;
	inputOrderField.Direction = ConvertDirection2ZTP(pInputOrder->Direction);
	inputOrderField.OffsetFlag = ConvertOperation2ZTP(pInputOrder->OffsetFlag);
	inputOrderField.HedgeFlag = ConvertHedgeFlag2ZTP(pInputOrder->HedgeFlag);
	inputOrderField.Volume = pInputOrder->OrderQuantity;
	inputOrderField.LimitPrice = pInputOrder->OrderPrice;
	inputOrderField.OrderPriceType = ConvertPriceType2ZTP(pInputOrder->OrderPriceType);
	inputOrderField.UserOrderLocalID = pInputOrder->OrderLocalID;
	inputOrderField.UserCustom = pInputOrder->UserCustom;
	FTD_ADD_FIELD(&m_reqPackage, &inputOrderField);

	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqOrderAction(CZtpApiInputOrderActionField* pOrderAction, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqOrderAction, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDOrderActionField orderActionField;
	orderActionField.BrokerID = pOrderAction->BrokerID;
	orderActionField.InvestorID = pOrderAction->InvestorID;
	orderActionField.ExchangeID = pOrderAction->ExchangeID;
	orderActionField.ProductClass = ConvertProductClass2ZTP(pOrderAction->ProductClass);
	orderActionField.InstrumentID = pOrderAction->InstrumentID;
	orderActionField.HedgeFlag = pOrderAction->HedgeFlag;
	orderActionField.OrderSysID = pOrderAction->OrderSysID;
	orderActionField.UserOrderActionLocalID = pOrderAction->OrderActionLocalID;
	orderActionField.UserOrderLocalID = pOrderAction->OrderLocalID;
	FTD_ADD_FIELD(&m_reqPackage, &orderActionField);

	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqQryInvestorPosition(CZtpApiQryPositionField* pQryInvestorPosition, int nRequestID) {
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInvestorPosition, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryInvestorPositionField qryPositionField;
	memcpy(&qryPositionField, pQryInvestorPosition, sizeof(CFTDQryInvestorPositionField));
	FTD_ADD_FIELD(&m_reqPackage, &qryPositionField);
	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqQryInvestorMargin(CZtpApiQryMarginField* pQryInvestorMargin, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInvestorMargin, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryInvestorMarginField qryMarginField;
	memcpy(&qryMarginField, pQryInvestorMargin, sizeof(CFTDQryInvestorMarginField));
	FTD_ADD_FIELD(&m_reqPackage, &qryMarginField);
	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqQryInvestorFee(CZtpApiQryFeeField* pQryInvestorFee, int nRequestID)
{
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInvestorFee, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryInvestorFeeField qryFeeField;
	memcpy(&qryFeeField, pQryInvestorFee, sizeof(CFTDQryInvestorFeeField));
	FTD_ADD_FIELD(&m_reqPackage, &qryFeeField);
	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqQryInvestorOrder(CZtpApiQryOrderField* pQryOrder, int nRequestID) {
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInvestorOrder, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryOrderField qryOrderField;
	memcpy(&qryOrderField, pQryOrder, sizeof(CFTDQryOrderField));
	FTD_ADD_FIELD(&m_reqPackage, &qryOrderField);
	return RequestToDialogFlow();
}
int CZtpFtdUserApiImpl::ReqQryInvestorTrade(CZtpApiQryTradeField* pQryTrade, int nRequestID) {
	CAutoMutex autoMutex(&m_mutexAction);
	m_reqPackage.PreparePackage(FTD_TID_ReqQryInvestorTrade, FTD_CHAIN_LAST);
	m_reqPackage.SetRequestID(nRequestID);

	CFTDQryTradeField qryTradeField;
	memcpy(&qryTradeField, pQryTrade, sizeof(CFTDQryTradeField));
	FTD_ADD_FIELD(&m_reqPackage, &qryTradeField);
	return RequestToDialogFlow();
}

void CZtpFtdUserApiImpl::OnRspError(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;
	if (NULL != m_pSpi)
	{
		m_pSpi->OnRspError(pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspUserLogin(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;
	
	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspUserLogin(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDRspUserLoginField ftdRspUserLogin;
	CZtpApiRspLoginField apiRspLogin = {0};
	CNamedFieldIterator iter = pMessage->GetNamedFieldIterator(&CFTDRspUserLoginField::m_Describe);
	while (!iter.IsEnd())
	{
		iter.Retrieve(&ftdRspUserLogin);
		iter.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && iter.IsEnd())
			{
				bIsLast = true;
			}
			strcpy_s(apiRspLogin.TradeDate, ftdRspUserLogin.TradingDay.getValue());
			strcpy_s(apiRspLogin.BrokerID, ftdRspUserLogin.BrokerID.getValue());
			strcpy_s(apiRspLogin.AccountID, ftdRspUserLogin.LoginID.getValue());
			strcpy_s(apiRspLogin.SystemProductInfo, ftdRspUserLogin.TradingSystemName.getValue());
// 			strcpy_s(apiRspLogin.MaxOrderLocalID, ftdRspUserLogin.MaxOrderLocalID.getValue());
			apiRspLogin.SessionID = ftdRspUserLogin.SessionID.getValue();
			m_pSpi->OnRspUserLogin(&apiRspLogin, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}
}
void CZtpFtdUserApiImpl::OnRspUserLogout(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspUserLogout(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDRspUserLogoutField ftdRspUserLogout;
	CZtpApiRspLogoutField* pApiRspLogout = NULL;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDRspUserLogoutField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdRspUserLogout);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			pApiRspLogout = (CZtpApiRspLogoutField*)&ftdRspUserLogout;	//ZTP协议与FTD协议内结构相同，直接Copy指针
			m_pSpi->OnRspUserLogout(pApiRspLogout, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL == pApiRspLogout && NULL != m_pSpi) {
		m_pSpi->OnRspUserLogout(pApiRspLogout, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}

void CZtpFtdUserApiImpl::OnRspUserPasswordUpdate(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspUserPasswordUpdate(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDUserPasswordUpdateField ftdUserPasswordUpdate;
	CZtpApiPasswordUpdateField ApiRspPasswordUpdate = { 0 };

	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDUserPasswordUpdateField::m_Describe);
	if (!itor.IsEnd())
	{
		itor.Retrieve(&ftdUserPasswordUpdate);
		itor.Next();
		bool bIsLast = false;
		if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
		{
			bIsLast = true;
		}
		strcpy_s(ApiRspPasswordUpdate.BrokerID, ftdUserPasswordUpdate.BrokerID);
		strcpy_s(ApiRspPasswordUpdate.AccountID, ftdUserPasswordUpdate.InvestorID);
		strcpy_s(ApiRspPasswordUpdate.OldPassword, ftdUserPasswordUpdate.OldPassword);
		strcpy_s(ApiRspPasswordUpdate.NewPassword, ftdUserPasswordUpdate.NewPassword);
		ApiRspPasswordUpdate.UserType = ZTP_API_UT_Person;
		m_pSpi->OnRspUserPasswordUpdate(&ApiRspPasswordUpdate, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
	} else {
		m_pSpi->OnRspUserPasswordUpdate(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspQryInstrument(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInstrument(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDRspInstrumentField ftdInstrumentField;
	CZtpApiRspInstrumentField rspInstrument;	
	CZtpApiRspInstrumentField* pApiRspInstrument = NULL;	
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDRspInstrumentField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdInstrumentField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			memset(&rspInstrument, 0, sizeof(CZtpApiRspInstrumentField));
			strcpy_s(rspInstrument.ExchangeID, ftdInstrumentField.ExchangeID);
			strcpy_s(rspInstrument.InstrumentID, ftdInstrumentField.InstrumentID);
			strcpy_s(rspInstrument.InstrumentName, ftdInstrumentField.InstrumentName);
			rspInstrument.LongPosLimit = ftdInstrumentField.LongPosLimit;
			rspInstrument.MaxLimitOrderVolume = ftdInstrumentField.MaxLimitOrderVolume;
			rspInstrument.MaxMarketOrderVolume = ftdInstrumentField.MaxMarketOrderVolume;
			rspInstrument.MinLimitOrderVolume = ftdInstrumentField.MinLimitOrderVolume;
			rspInstrument.MinMarketOrderVolume = ftdInstrumentField.MinMarketOrderVolume;
			rspInstrument.PriceTick = ftdInstrumentField.PriceTick;
			rspInstrument.ProductClass = ftdInstrumentField.ProductClass;
			strcpy_s(rspInstrument.ProductID, ftdInstrumentField.ProductID);
			rspInstrument.ShortPosLimit = ftdInstrumentField.ShortPosLimit;
			rspInstrument.VolumeMultiple = ftdInstrumentField.VolumeMultiple;
			pApiRspInstrument = &rspInstrument;
			m_pSpi->OnRspQryInstrument(pApiRspInstrument, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	//没有查询结果-错误
	if (NULL==pApiRspInstrument && NULL!=m_pSpi)
	{
		m_pSpi->OnRspQryInstrument(pApiRspInstrument, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspQryInvestorAccount(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInvestorAccount(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDRspInvestorAccountField ftdAccountField;
	CZtpApiRspAccountField rspAccount;
	CZtpApiRspAccountField* pApiRspAccountField = nullptr;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDRspInvestorAccountField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdAccountField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			memset(&rspAccount, 0, sizeof(CZtpApiRspAccountField));
			rspAccount.Availlable = ftdAccountField.Available;
			rspAccount.Balance = ftdAccountField.Balance;
			strcpy_s(rspAccount.BrokerID, ftdAccountField.BrokerID);
			rspAccount.CloseProfit = ftdAccountField.CloseProfit;
			rspAccount.Deposit = ftdAccountField.Deposit;
			rspAccount.DynamicRights = ftdAccountField.DynamicRights;
			rspAccount.Fee = ftdAccountField.Fee;
			rspAccount.FrozenFee = ftdAccountField.FrozenFee;
			rspAccount.FrozenMargin = ftdAccountField.FrozenMargin;
			strcpy_s(rspAccount.InvestorID, ftdAccountField.InvestorID);
			rspAccount.LongFrozenMargin = ftdAccountField.LongFrozenMargin;
			rspAccount.LongMargin = ftdAccountField.LongMargin;
			rspAccount.Margin = ftdAccountField.Margin;
			rspAccount.OtherFee = ftdAccountField.OtherFee;
			rspAccount.PositionProfit = ftdAccountField.PositionProfit;
			rspAccount.PreAvaillable = ftdAccountField.PreAvailable;
			rspAccount.PreBalance = ftdAccountField.PreBalance;
			rspAccount.Risk = ftdAccountField.Risk;
			rspAccount.ShortFrozenMargin = ftdAccountField.ShortFrozenMargin;
			rspAccount.ShortMargin = ftdAccountField.ShortMargin;
			strcpy_s(rspAccount.TradingDay, ftdAccountField.TradingDay);
			rspAccount.Withdraw = ftdAccountField.Withdraw;
			pApiRspAccountField = &rspAccount;
			m_pSpi->OnRspQryInvestorAccount(pApiRspAccountField, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	//没有查询结果-错误
	if (NULL==pApiRspAccountField && NULL!=m_pSpi)
	{
		m_pSpi->OnRspQryInvestorAccount(pApiRspAccountField, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspQryInvestorPosition(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInvestorPosition(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}


	CFTDRspInvestorPositionField ftdPositionField;
	CZtpApiRspPositionField rspPosition;
	CZtpApiRspPositionField* pApiRspPosition = NULL;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDRspInvestorPositionField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdPositionField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			memset(&rspPosition, 0, sizeof(CZtpApiRspPositionField));
			strcpy_s(rspPosition.BrokerID, ftdPositionField.BrokerID);
			strcpy_s(rspPosition.InvestorID, ftdPositionField.InvestorID);
			strcpy_s(rspPosition.TradingDay, ftdPositionField.TradingDay);
			strcpy_s(rspPosition.ExchangeID, ftdPositionField.ExchangeID);
			strcpy_s(rspPosition.InstrumentID, ftdPositionField.InstrumentID);
			rspPosition.ProductClass = ftdPositionField.ProductClass;
			rspPosition.Direction = ftdPositionField.Direction;
			rspPosition.UsedMargin = ftdPositionField.UsedMargin;
			rspPosition.Position = ftdPositionField.Position;
			rspPosition.PositionCost = ftdPositionField.PositionCost;
			rspPosition.YdPosition = ftdPositionField.YdPosition;
			rspPosition.YdPositionPrice = ftdPositionField.YdPositionPrice;
			rspPosition.FrozenMargin = ftdPositionField.FrozenMargin;
			rspPosition.FrozenFee = ftdPositionField.FrozenFee;
			rspPosition.FrozenPosition = ftdPositionField.FrozenPosition;
			
			pApiRspPosition = &rspPosition;
			m_pSpi->OnRspQryInvestorPosition(pApiRspPosition, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL == pApiRspPosition && m_pSpi != NULL)
	{
		m_pSpi->OnRspQryInvestorPosition(pApiRspPosition, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspQryInvestorMargin(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInvestorMargin(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDRspInvestorMarginField ftdMarginField;
	CZtpApiRspMarginField rspMargin;
	CZtpApiRspMarginField* pApiRspMargin = NULL;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDRspInvestorMarginField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdMarginField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			memset(&rspMargin, 0, sizeof(CZtpApiRspMarginField));
			strcpy_s(rspMargin.BrokerID, ftdMarginField.BrokerID);
			strcpy_s(rspMargin.ExchangeID, ftdMarginField.ExchangeID);
			strcpy_s(rspMargin.InstrumentID, ftdMarginField.InstrumentID);
			strcpy_s(rspMargin.InvestorID, ftdMarginField.InvestorID);
			rspMargin.LongMarginAmt = ftdMarginField.LongMarginAmt;
			rspMargin.LongMarginRate = ftdMarginField.LongMarginRate;
			rspMargin.ShortMarginAmt = ftdMarginField.ShortMarginAmt;
			rspMargin.ShortMarginRate = ftdMarginField.ShortMarginRate;
			pApiRspMargin = &rspMargin;
			m_pSpi->OnRspQryInvestorMargin(pApiRspMargin, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL == pApiRspMargin && m_pSpi != NULL)
	{
		m_pSpi->OnRspQryInvestorMargin(pApiRspMargin, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspQryInvestorFee(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInvestorFee(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDRspInvestorFeeField ftdFeeField;
	CZtpApiRspFeeField rspFee;
	CZtpApiRspFeeField* pApiRspFee = NULL;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDRspInvestorFeeField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdFeeField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			memset(&rspFee, 0, sizeof(CZtpApiRspFeeField));
			strcpy_s(rspFee.BrokerID, ftdFeeField.BrokerID);
			strcpy_s(rspFee.ExchangeID, ftdFeeField.ExchangeID);
			strcpy_s(rspFee.InstrumentID, ftdFeeField.InstrumentID);
			strcpy_s(rspFee.InvestorID, ftdFeeField.InvestorID);
			rspFee.OffsetFeeAmt = ftdFeeField.OffsetFeeAmt;
			rspFee.OffsetFeeRate = ftdFeeField.OffsetFeeRate;
			rspFee.OffsetTodayFeeAmt = ftdFeeField.OffsetTodayFeeAmt;
			rspFee.OffsetTodayFeeRate = ftdFeeField.OffsetTodayFeeRate;
			rspFee.OpenFeeAmt = ftdFeeField.OpenFeeAmt;
			rspFee.OpenFeeRate = ftdFeeField.OpenFeeRate;
			pApiRspFee = &rspFee;
			m_pSpi->OnRspQryInvestorFee(pApiRspFee, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL == pApiRspFee && m_pSpi != NULL)
	{
		m_pSpi->OnRspQryInvestorFee(pApiRspFee, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}

void CZtpFtdUserApiImpl::OnRspOrderInsert(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;


	CFTDInputOrderField ftdInputOrderField;
	CZtpApiInputOrderField apiInputOrderField = {0};
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDInputOrderField::m_Describe);
	while (!itor.IsEnd())
	{
		memset(&ftdInputOrderField, 0, sizeof(CFTDInputOrderField));
		itor.Retrieve(&ftdInputOrderField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}

			strcpy_s(apiInputOrderField.BrokerID, ftdInputOrderField.BrokerID.getValue());
			strcpy_s(apiInputOrderField.InvestorID, ftdInputOrderField.InvestorID.getValue());
			strcpy_s(apiInputOrderField.ExchangeID, ftdInputOrderField.ExchangeID.getValue());
			apiInputOrderField.ProductClass = ftdInputOrderField.ProductClass.getValue();
			strcpy_s(apiInputOrderField.InstrumentID, ftdInputOrderField.InstrumentID.getValue());
			apiInputOrderField.Direction = ftdInputOrderField.Direction.getValue();
			apiInputOrderField.OffsetFlag = ftdInputOrderField.OffsetFlag.getValue();
			apiInputOrderField.HedgeFlag = ftdInputOrderField.HedgeFlag.getValue();
			apiInputOrderField.OrderQuantity = ftdInputOrderField.Volume.getValue();
			apiInputOrderField.OrderPrice = ftdInputOrderField.LimitPrice.getValue();
			apiInputOrderField.OrderPriceType = ConvertPriceTypeFromZTP(ftdInputOrderField.OrderPriceType.getValue());
			strcpy_s(apiInputOrderField.OrderLocalID, ftdInputOrderField.UserOrderLocalID.getValue());
			strcpy_s(apiInputOrderField.OrderSysID, ftdInputOrderField.OrderSysID.getValue());
			strcpy_s(apiInputOrderField.UserCustom, ftdInputOrderField.UserCustom.getValue());
			m_pSpi->OnRspOrderInsert(&apiInputOrderField, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}
}
void CZtpFtdUserApiImpl::OnRspOrderAction(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;


	CFTDOrderActionField ftdorderActionField;
	CZtpApiInputOrderActionField apiOrderActionField = {0};
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDOrderActionField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdorderActionField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}

			strcpy_s(apiOrderActionField.BrokerID, ftdorderActionField.BrokerID.getValue());
			strcpy_s(apiOrderActionField.InvestorID, ftdorderActionField.InvestorID.getValue());
			strcpy_s(apiOrderActionField.ExchangeID, ftdorderActionField.ExchangeID.getValue());
			apiOrderActionField.ProductClass = ftdorderActionField.ProductClass.getValue();
			strcpy_s(apiOrderActionField.InstrumentID, ftdorderActionField.InstrumentID.getValue());
			strcpy_s(apiOrderActionField.OrderSysID, ftdorderActionField.OrderSysID.getValue());
			strcpy_s(apiOrderActionField.OrderLocalID, ftdorderActionField.UserOrderLocalID.getValue());
			strcpy_s(apiOrderActionField.OrderActionLocalID, ftdorderActionField.UserOrderActionLocalID.getValue());
			m_pSpi->OnRspOrderAction(&apiOrderActionField, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}
}
void CZtpFtdUserApiImpl::OnRtnOrder(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDOrderField ftdOrderField;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDOrderField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdOrderField);

		CZtpApiOrderField apiOrderField = {0};
		strcpy_s(apiOrderField.BrokerID, ftdOrderField.BrokerID.getValue());
		strcpy_s(apiOrderField.InvestorID, ftdOrderField.InvestorID.getValue());
		strcpy_s(apiOrderField.ExchangeID, ftdOrderField.ExchangeID.getValue());
		apiOrderField.ProductClass = ConvertProductClassFromZTP(ftdOrderField.ProductClass);
		strcpy_s(apiOrderField.InstrumentID, ftdOrderField.InstrumentID.getValue());
		apiOrderField.Direction = ConvertDirectionFromZTP(ftdOrderField.Direction);
		apiOrderField.OffsetFlag = ConvertOperationFromZTP(ftdOrderField.OffsetFlag);
		apiOrderField.HedgeFlag = ConvertHedgeFlagFromZTP(ftdOrderField.HedgeFlag);
		apiOrderField.OrderQuantity = ftdOrderField.Volume.getValue();
		apiOrderField.OrderPrice = ftdOrderField.LimitPrice.getValue();
		apiOrderField.OrderPriceType = ConvertPriceTypeFromZTP(ftdOrderField.OrderPriceType);
		strcpy_s(apiOrderField.OrderLocalID, ftdOrderField.UserOrderLocalID.getValue());
		strcpy_s(apiOrderField.OrderSysID, ftdOrderField.OrderSysID.getValue());
		strcpy_s(apiOrderField.InsertTime, ftdOrderField.InsertTime.getValue());
		strcpy_s(apiOrderField.CancelTime, ftdOrderField.CancelTime.getValue());
		apiOrderField.OrderStatus = ConvertStatusFromZTP(ftdOrderField.OrderStatus);
		apiOrderField.TradeVolume = ftdOrderField.VolumeTrade.getValue();
		apiOrderField.RemainQuantity = ftdOrderField.VolumeRemain.getValue();
		strcpy_s(apiOrderField.UserCustom, ftdOrderField.UserCustom.getValue());
		if (NULL != m_pSpi)
		{
			m_pSpi->OnRtnOrder(&apiOrderField);
		}
		itor.Next();
	}
}
void CZtpFtdUserApiImpl::OnRtnTrade(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDTradeField ftdTradeField;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDTradeField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdTradeField);

		CZtpApiTradeField apiTradeField = {0};
		strcpy_s(apiTradeField.BrokerID, ftdTradeField.BrokerID.getValue());
		strcpy_s(apiTradeField.InvestorID, ftdTradeField.InvestorID.getValue());
		strcpy_s(apiTradeField.ExchangeID, ftdTradeField.ExchangeID.getValue());
		apiTradeField.ProductClass = ftdTradeField.ProductClass.getValue();
		strcpy_s(apiTradeField.InstrumentID, ftdTradeField.InstrumentID.getValue());
		apiTradeField.Direction = ftdTradeField.Direction.getValue();
		apiTradeField.OffsetFlag = ftdTradeField.OffsetFlag.getValue();
		apiTradeField.HedgeFlag = ftdTradeField.HedgeFlag.getValue();
		apiTradeField.TradeVolume = ftdTradeField.TradeVolume.getValue();
		apiTradeField.TradePrice = ftdTradeField.TradePrice.getValue();
		apiTradeField.TradeAmt = ftdTradeField.TradeAmt.getValue();
		strcpy_s(apiTradeField.TradeDate, ftdTradeField.TradeDate.getValue());
		strcpy_s(apiTradeField.TradeTime, ftdTradeField.TradeTime.getValue());	
		strcpy_s(apiTradeField.TradeID, ftdTradeField.TradeID.getValue());
		strcpy_s(apiTradeField.OrderSysID, ftdTradeField.OrderSysID.getValue());
		strcpy_s(apiTradeField.OrderLocalID, ftdTradeField.UserOrderLocalID.getValue());
		if (NULL != m_pSpi)
		{
			m_pSpi->OnRtnTrade(&apiTradeField);
		}
		itor.Next();
	}
}
void CZtpFtdUserApiImpl::OnErrRtnOrderInsert(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	CFTDInputOrderField ftdInputOrderField;
	CZtpApiInputOrderField apiInputOrderField = {0};
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDInputOrderField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdInputOrderField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			strcpy_s(apiInputOrderField.BrokerID, ftdInputOrderField.BrokerID.getValue());
			strcpy_s(apiInputOrderField.InvestorID, ftdInputOrderField.InvestorID.getValue());
			strcpy_s(apiInputOrderField.ExchangeID, ftdInputOrderField.ExchangeID.getValue());
			apiInputOrderField.ProductClass = ftdInputOrderField.ProductClass.getValue();
			strcpy_s(apiInputOrderField.InstrumentID, ftdInputOrderField.InstrumentID.getValue());
			apiInputOrderField.Direction = ftdInputOrderField.Direction.getValue();
			apiInputOrderField.OffsetFlag = ftdInputOrderField.OffsetFlag.getValue();
			apiInputOrderField.HedgeFlag = ftdInputOrderField.HedgeFlag.getValue();
			apiInputOrderField.OrderQuantity = ftdInputOrderField.Volume.getValue();
			apiInputOrderField.OrderPrice = ftdInputOrderField.LimitPrice.getValue();
			apiInputOrderField.OrderPriceType = ftdInputOrderField.OrderPriceType.getValue();
			strcpy_s(apiInputOrderField.OrderLocalID, ftdInputOrderField.UserOrderLocalID.getValue());
			strcpy_s(apiInputOrderField.OrderSysID, ftdInputOrderField.OrderSysID.getValue());
			m_pSpi->OnErrRtnOrderInsert(&apiInputOrderField, pApiRspInfo);
		}
	}
}
void CZtpFtdUserApiImpl::OnErrRtnOrderAction(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	CFTDOrderActionField ftdorderActionField;
	CZtpApiInputOrderActionField apiOrderActionField = {0};
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDOrderActionField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdorderActionField);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}

			strcpy_s(apiOrderActionField.BrokerID, ftdorderActionField.BrokerID.getValue());
			strcpy_s(apiOrderActionField.InvestorID, ftdorderActionField.InvestorID.getValue());
			strcpy_s(apiOrderActionField.ExchangeID, ftdorderActionField.ExchangeID.getValue());
			apiOrderActionField.ProductClass = ftdorderActionField.ProductClass.getValue();
			strcpy_s(apiOrderActionField.InstrumentID, ftdorderActionField.InstrumentID.getValue());
			strcpy_s(apiOrderActionField.OrderSysID, ftdorderActionField.OrderSysID.getValue());
			strcpy_s(apiOrderActionField.OrderLocalID, ftdorderActionField.UserOrderLocalID.getValue());
			strcpy_s(apiOrderActionField.OrderActionLocalID, ftdorderActionField.UserOrderActionLocalID.getValue());
			m_pSpi->OnErrRtnOrderAction(&apiOrderActionField, pApiRspInfo);
		}
	}
}
void CZtpFtdUserApiImpl::OnRtnInstrumentStatus(CFTDPackage* pMessage, WORD nSequenceSeries)
{
// 	CFTDInstrumentStatusField InstrumentStatus;
// 	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDInstrumentStatusField::m_Describe);
// 	while (!itor.IsEnd())
// 	{
// 		itor.Retrieve(&InstrumentStatus);
// 
// 		if (m_pSpi != NULL)
// 		{
// 			m_pSpi->OnRtnInstrumentStatus((CFTDInstrumentStatusField*)&InstrumentStatus);
// 		}
// 		itor.Next();
// 	}
// 	if (m_pSpi != NULL)
// 	{
// 		m_pSpi->OnPackageEnd((int)nSequenceSeries, pMessage->GetFTDCHeader()->SequenceNumber);
// 	}
}
void CZtpFtdUserApiImpl::OnRtnDeferFee(CFTDPackage* pMessage, WORD nSequenceSeries) {
 	CFTDDeferFeeField ftdDeferFeeField;
 	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDDeferFeeField::m_Describe);
	while (!itor.IsEnd()) {
		memset(&ftdDeferFeeField, 0, sizeof(CFTDDeferFeeField));
		itor.Retrieve(&ftdDeferFeeField);
		itor.Next();

		CZtpApiRtnDeferFee apiTradeField = { 0 };
		strcpy_s(apiTradeField.InstrumentID, ftdDeferFeeField.InstrumentID.getValue());
		apiTradeField.DeferDirection = ftdDeferFeeField.Direction.getValue();
		apiTradeField.DeferFee = ftdDeferFeeField.FeeRate;
		if (NULL != m_pSpi) {
			m_pSpi->OnRtnDeferFee(&apiTradeField);
		}
	}
}
void CZtpFtdUserApiImpl::OnRspQryOrder(CFTDPackage* pMessage, WORD nSequenceSeries) {

	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0) {
		pApiRspInfo = NULL;
	}

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInvestorOrder(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDOrderField ftdOrderField;
	CFTDRspInvestorFeeField ftdFeeField;
	CZtpApiOrderField apiOrderField = { 0 };
	CZtpApiOrderField* pApiOrderField = nullptr;

	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDOrderField::m_Describe);
	while (!itor.IsEnd()) {
		memset(&ftdOrderField, 0, sizeof(CFTDTradeField));
		itor.Retrieve(&ftdOrderField);
		itor.Next();
		if (NULL != m_pSpi) {
			bool bIsLast = false;
			if (FTD_CHAIN_LAST == pMessage->GetChain() && itor.IsEnd()) {
				bIsLast = true;
			}
			memset(&apiOrderField, 0, sizeof(CZtpApiOrderField));
			strcpy_s(apiOrderField.BrokerID, ftdOrderField.BrokerID.getValue());
			strcpy_s(apiOrderField.InvestorID, ftdOrderField.InvestorID.getValue());
			strcpy_s(apiOrderField.ExchangeID, ftdOrderField.ExchangeID.getValue());
			apiOrderField.ProductClass = ConvertProductClassFromZTP(ftdOrderField.ProductClass);
			strcpy_s(apiOrderField.InstrumentID, ftdOrderField.InstrumentID.getValue());
			apiOrderField.Direction = ConvertDirectionFromZTP(ftdOrderField.Direction);
			apiOrderField.OffsetFlag = ConvertOperationFromZTP(ftdOrderField.OffsetFlag);
			apiOrderField.HedgeFlag = ConvertHedgeFlagFromZTP(ftdOrderField.HedgeFlag);
			apiOrderField.OrderQuantity = ftdOrderField.Volume.getValue();
			apiOrderField.OrderPrice = ftdOrderField.LimitPrice.getValue();
			apiOrderField.OrderPriceType = ConvertPriceTypeFromZTP(ftdOrderField.OrderPriceType);
			strcpy_s(apiOrderField.OrderLocalID, ftdOrderField.UserOrderLocalID.getValue());
			strcpy_s(apiOrderField.OrderSysID, ftdOrderField.OrderSysID.getValue());
			strcpy_s(apiOrderField.InsertTime, ftdOrderField.InsertTime.getValue());
			strcpy_s(apiOrderField.CancelTime, ftdOrderField.CancelTime.getValue());
			apiOrderField.OrderStatus = ConvertStatusFromZTP(ftdOrderField.OrderStatus);
			apiOrderField.TradeVolume = ftdOrderField.VolumeTrade.getValue();
			apiOrderField.RemainQuantity = ftdOrderField.VolumeRemain.getValue();
			strcpy_s(apiOrderField.UserCustom, ftdOrderField.UserCustom.getValue());
			pApiOrderField = &apiOrderField;
			m_pSpi->OnRspQryInvestorOrder(&apiOrderField, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL == pApiOrderField && m_pSpi != NULL) {
		m_pSpi->OnRspQryInvestorOrder(pApiOrderField, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspQryTrade(CFTDPackage* pMessage, WORD nSequenceSeries) {

	CFTDRspInfoField ftdRspInfoField;
	CZtpApiRspInfoField* pApiRspInfo = (CZtpApiRspInfoField*)&ftdRspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &ftdRspInfoField) <= 0)
		pApiRspInfo = NULL;

	if (pApiRspInfo != nullptr && pApiRspInfo->ErrorID != 0 && m_pSpi != nullptr) {
		m_pSpi->OnRspQryInvestorTrade(nullptr, pApiRspInfo, pMessage->GetRequestID(), true);
		return;
	}

	CFTDTradeField ftdTradeField;
	CZtpApiTradeField apiTradeField = { 0 };
	CZtpApiTradeField* pApiTradeField = nullptr;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDTradeField::m_Describe);
	while (!itor.IsEnd()) {
		memset(&ftdTradeField, 0, sizeof(CFTDTradeField));
		itor.Retrieve(&ftdTradeField);
		itor.Next();
		if (NULL != m_pSpi) {

			bool bIsLast = false;
			if (FTD_CHAIN_LAST == pMessage->GetChain() && itor.IsEnd()) {
				bIsLast = true;
			}
			memset(&apiTradeField, 0, sizeof(CZtpApiTradeField));
			strcpy_s(apiTradeField.BrokerID, ftdTradeField.BrokerID.getValue());
			strcpy_s(apiTradeField.InvestorID, ftdTradeField.InvestorID.getValue());
			strcpy_s(apiTradeField.ExchangeID, ftdTradeField.ExchangeID.getValue());
			apiTradeField.ProductClass = ftdTradeField.ProductClass.getValue();
			strcpy_s(apiTradeField.InstrumentID, ftdTradeField.InstrumentID.getValue());
			apiTradeField.Direction = ftdTradeField.Direction.getValue();
			apiTradeField.OffsetFlag = ftdTradeField.OffsetFlag.getValue();
			apiTradeField.HedgeFlag = ftdTradeField.HedgeFlag.getValue();
			apiTradeField.TradeVolume = ftdTradeField.TradeVolume.getValue();
			apiTradeField.TradePrice = ftdTradeField.TradePrice.getValue();
			apiTradeField.TradeAmt = ftdTradeField.TradeAmt.getValue();
			strcpy_s(apiTradeField.TradeDate, ftdTradeField.TradeDate.getValue());
			strcpy_s(apiTradeField.TradeTime, ftdTradeField.TradeTime.getValue());
			strcpy_s(apiTradeField.TradeID, ftdTradeField.TradeID.getValue());
			strcpy_s(apiTradeField.OrderSysID, ftdTradeField.OrderSysID.getValue());
			strcpy_s(apiTradeField.OrderLocalID, ftdTradeField.UserOrderLocalID.getValue());

			pApiTradeField = &apiTradeField;
			m_pSpi->OnRspQryInvestorTrade(&apiTradeField, pApiRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL == pApiTradeField && m_pSpi != NULL) {
		m_pSpi->OnRspQryInvestorTrade(pApiTradeField, pApiRspInfo, pMessage->GetRequestID(), true);
	}
}

void CZtpFtdUserApiImpl::OnRtnDepthMarketData(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDDepthMarketDataField ftdDepthMarketData;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDDepthMarketDataField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&ftdDepthMarketData);
		if (NULL != m_pSpi)
		{
			CZtpApiDepthMarketDataField ztpMarketField = {0};	
			strcpy_s(ztpMarketField.InstrumentID, ftdDepthMarketData.InstrumentID.getValue());		//合约代码
			ztpMarketField.PreClosePrice = ftdDepthMarketData.PreClosePrice.getValue();				//昨收盘
			ztpMarketField.PreSettlementPrice = ftdDepthMarketData.PreSettlementPrice.getValue();
			ztpMarketField.OpenPrice = ftdDepthMarketData.OpenPrice.getValue();						//开盘价
			ztpMarketField.HighestPrice = ftdDepthMarketData.HighestPrice.getValue();				//最高价
			ztpMarketField.LowestPrice = ftdDepthMarketData.LowestPrice.getValue();					//最低价
			ztpMarketField.LastPrice = ftdDepthMarketData.LastPrice.getValue();						//最新价
			ztpMarketField.AvgPrice = ftdDepthMarketData.AvgPrice.getValue();						//均价
			ztpMarketField.TradeWeight = ftdDepthMarketData.Weight.getValue();						//成交重量
			ztpMarketField.ClosePrice = ftdDepthMarketData.ClosePrice.getValue();					//收盘价
			ztpMarketField.SettlementPrice = ftdDepthMarketData.SettlementPrice.getValue();			//结算价
			ztpMarketField.OpenInterest = ftdDepthMarketData.OpenInterest.getValue();				//持仓量
			ztpMarketField.UpperLimitPrice = ftdDepthMarketData.UpperLimitPrice.getValue();			//涨停板
			ztpMarketField.LowerLimitPrice = ftdDepthMarketData.LowerLimitPrice.getValue();			//跌停板
			ztpMarketField.TradeVolume = ftdDepthMarketData.Volume.getValue();						//成交量
			ztpMarketField.TopicNo = nSequenceSeries;												//主题号
			ztpMarketField.SequenceNo = ftdDepthMarketData.SequenceNo.getValue();					//行情序号
			ztpMarketField.Turnover = ftdDepthMarketData.Turnover.getValue();						//成交额
			strcpy_s(ztpMarketField.QuoteDate, ftdDepthMarketData.TradingDay.getValue());			//交易日
			strcpy_s(ztpMarketField.QuoteTime, ftdDepthMarketData.UpdateTime.getValue());			//行情时间
			ztpMarketField.Millisec = ftdDepthMarketData.Millisec;									//行情毫秒

			ztpMarketField.BidPrice1 = ftdDepthMarketData.BidPrice1.getValue();
			ztpMarketField.BidVolume1 = ftdDepthMarketData.BidVolume1.getValue();
			ztpMarketField.BidPrice2 = ftdDepthMarketData.BidPrice2.getValue();
			ztpMarketField.BidVolume2 = ftdDepthMarketData.BidVolume2.getValue();
			ztpMarketField.BidPrice3 = ftdDepthMarketData.BidPrice3.getValue();
			ztpMarketField.BidVolume3 = ftdDepthMarketData.BidVolume3.getValue();
			ztpMarketField.BidPrice4 = ftdDepthMarketData.BidPrice4.getValue();
			ztpMarketField.BidVolume4 = ftdDepthMarketData.BidVolume4.getValue();
			ztpMarketField.BidPrice5 = ftdDepthMarketData.BidPrice5.getValue();
			ztpMarketField.BidVolume5 = ftdDepthMarketData.BidVolume5.getValue();
// 			ztpMarketField.BidPrice6 = ftdDepthMarketData.BidPrice6.getValue();
// 			ztpMarketField.BidVolume6 = ftdDepthMarketData.BidVolume6.getValue();
// 			ztpMarketField.BidPrice7 = ftdDepthMarketData.BidPrice7.getValue();
// 			ztpMarketField.BidVolume7 = ftdDepthMarketData.BidVolume7.getValue();
// 			ztpMarketField.BidPrice8 = ftdDepthMarketData.BidPrice8.getValue();
// 			ztpMarketField.BidVolume8 = ftdDepthMarketData.BidVolume8.getValue();
// 			ztpMarketField.BidPrice9 = ftdDepthMarketData.BidPrice9.getValue();
// 			ztpMarketField.BidVolume9 = ftdDepthMarketData.BidVolume9.getValue();
// 			ztpMarketField.BidPrice10 = ftdDepthMarketData.BidPrice10.getValue();
// 			ztpMarketField.BidVolume10 = ftdDepthMarketData.BidVolume10.getValue();
			ztpMarketField.AskPrice1 = ftdDepthMarketData.AskPrice1.getValue();
			ztpMarketField.AskVolume1 = ftdDepthMarketData.AskVolume1.getValue();
			ztpMarketField.AskPrice2 = ftdDepthMarketData.AskPrice2.getValue();
			ztpMarketField.AskVolume2 = ftdDepthMarketData.AskVolume2.getValue();
			ztpMarketField.AskPrice3 = ftdDepthMarketData.AskPrice3.getValue();
			ztpMarketField.AskVolume3 = ftdDepthMarketData.AskVolume3.getValue();
			ztpMarketField.AskPrice4 = ftdDepthMarketData.AskPrice4.getValue();
			ztpMarketField.AskVolume4 = ftdDepthMarketData.AskVolume4.getValue();
			ztpMarketField.AskPrice5 = ftdDepthMarketData.AskPrice5.getValue();
			ztpMarketField.AskVolume5 = ftdDepthMarketData.AskVolume5.getValue();
// 			ztpMarketField.AskPrice6 = ftdDepthMarketData.AskPrice6.getValue();
// 			ztpMarketField.AskVolume6 = ftdDepthMarketData.AskVolume6.getValue();
// 			ztpMarketField.AskPrice7 = ftdDepthMarketData.AskPrice7.getValue();
// 			ztpMarketField.AskVolume7 = ftdDepthMarketData.AskVolume7.getValue();
// 			ztpMarketField.AskPrice8 = ftdDepthMarketData.AskPrice8.getValue();
// 			ztpMarketField.AskVolume8 = ftdDepthMarketData.AskVolume8.getValue();
// 			ztpMarketField.AskPrice9 = ftdDepthMarketData.AskPrice9.getValue();
// 			ztpMarketField.AskVolume9 = ftdDepthMarketData.AskVolume9.getValue();
// 			ztpMarketField.AskPrice10 = ftdDepthMarketData.AskPrice10.getValue();
// 			ztpMarketField.AskVolume10 = ftdDepthMarketData.AskVolume10.getValue();
			m_pSpi->OnRtnDepthMarketData(&ztpMarketField);
		}
		itor.Next();
	}
}
void CZtpFtdUserApiImpl::OnRspSubMarketData(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField rspInfoField;
	CZtpApiRspInfoField* pRspInfo = (CZtpApiRspInfoField*)&rspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &rspInfoField) <= 0)
	{
		pRspInfo = NULL;
	}

	CFTDSpecificInstrumentField rspSpecificInstrument;
	CZtpApiSpecificInstrumentField* pSpecificInstrument = NULL;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDSpecificInstrumentField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&rspSpecificInstrument);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			pSpecificInstrument = (CZtpApiSpecificInstrumentField*)&rspSpecificInstrument;
			m_pSpi->OnRspSubMarketData(pSpecificInstrument, pRspInfo, pMessage->GetRequestID(), bIsLast);
		}
	}

	if (NULL==pSpecificInstrument && NULL!=m_pSpi)
	{
		m_pSpi->OnRspSubMarketData(pSpecificInstrument, pRspInfo, pMessage->GetRequestID(), true);
	}
}
void CZtpFtdUserApiImpl::OnRspUnSubMarketData(CFTDPackage* pMessage, WORD nSequenceSeries)
{
	CFTDRspInfoField rspInfoField;
	CZtpApiRspInfoField* pRspInfo = (CZtpApiRspInfoField*)&rspInfoField;
	if (FTD_GET_SINGLE_FIELD(pMessage, &rspInfoField) <= 0)
	{
		pRspInfo = NULL;
	}

	CFTDSpecificInstrumentField rspSpecificInstrument;
	CZtpApiSpecificInstrumentField* pSpecificInstrument = NULL;
	CNamedFieldIterator itor = pMessage->GetNamedFieldIterator(&CFTDSpecificInstrumentField::m_Describe);
	while (!itor.IsEnd())
	{
		itor.Retrieve(&rspSpecificInstrument);
		itor.Next();
		if (NULL != m_pSpi)
		{
			bool bIsLast = false;
			if (FTD_CHAIN_LAST==pMessage->GetChain() && itor.IsEnd())
			{
				bIsLast = true;
			}
			pSpecificInstrument = (CZtpApiSpecificInstrumentField*)&rspSpecificInstrument;
			m_pSpi->OnRspUnSubMarketData(pSpecificInstrument, pRspInfo, pMessage->GetRequestID(), bIsLast);
		}

	}

	if (NULL==pSpecificInstrument && NULL!=m_pSpi)
	{
		m_pSpi->OnRspUnSubMarketData(pSpecificInstrument, pRspInfo, pMessage->GetRequestID(), true);
	}
}