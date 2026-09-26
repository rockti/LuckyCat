#include "stdafx.h"
#include "AdapterEngine.h"
#include "FtdDataStruct.h"
#include "EntityCopy.h"
#include <fmt/format.h>


template<class FTDClass>
struct RspHelper {
//	static const int TID = 0;
};

template<>
struct RspHelper<CFTDUserPasswordUpdateField> {
	typedef CFTDUserPasswordUpdateField FTDField;
	typedef data::CUserPasswordUpdate ZTPField;
	static const int TID = FID_TID_RspUserPasswordUpdate;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyUserPasswordUpdateReqEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDInputOrderField> {
	typedef CFTDInputOrderField FTDField;
	typedef data::COrder ZTPField;
	static const int TID = FTD_TID_RspOrderInsert;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyAPIInputOrderEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDOrderActionField> {
	typedef CFTDOrderActionField FTDField;
	typedef data::COrderAction ZTPField;
	static const int TID = FTD_TID_RspOrderAction;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyAPIOrderActionEntity(ftdField, ztpField);
	}
};


template<>
struct RspHelper<CFTDRspInstrumentField> {
	typedef CFTDRspInstrumentField FTDField;
	typedef data::CInstrument ZTPField;
	static const int TID = FTD_TID_RspQryInstrument;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyZTP2FTDInstrumentInfoEntity(ftdField, ztpField);
	}
};
template<>
struct RspHelper<CFTDRspInvestorAccountField> {
	typedef CFTDRspInvestorAccountField FTDField;
	typedef data::CInvestorAccount ZTPField;
	static const int TID = FTD_TID_RspQryInvestorAccount;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyInvestorAccountEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDRspInvestorPositionField> {
	typedef CFTDRspInvestorPositionField FTDField;
	typedef data::CInvestorPosition ZTPField;
	static const int TID = FTD_TID_RspQryInvestorPosition;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyInvestorPositionEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDRspInvestorMarginField> {
	typedef CFTDRspInvestorMarginField FTDField;
	typedef data::CInvestorMargin ZTPField;
	static const int TID = FTD_TID_RspQryInvestorMargin;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyInvestorMarginEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDRspInvestorFeeField> {
	typedef CFTDRspInvestorFeeField FTDField;
	typedef data::CInvestorFee ZTPField;
	static const int TID = FTD_TID_RspQryInvestorFee;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyInvestorFeeEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDOrderField> {
	typedef CFTDOrderField FTDField;
	typedef data::COrder ZTPField;
	static const int TID = FTD_TID_RspQryInvestorOrder;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyFTDOrderEntity(ftdField, ztpField);
	}
};

template<>
struct RspHelper<CFTDTradeField> {
	typedef CFTDTradeField FTDField;
	typedef data::CTrade ZTPField;
	static const int TID = FTD_TID_RspQryInvestorTrade;
	static void Copy(FTDField* ftdField, ZTPField* ztpField) {
		CopyFTDTradeEntity(ftdField, ztpField);
	}
};


template<class ZTPClass>
struct ForwardHelper;

template<>
struct ForwardHelper<data::CUserPasswordUpdate> {
	typedef CFTDUserPasswordUpdateField FTDField;
	typedef data::CUserPasswordUpdate ZTPField;
	static const int TID = TID_ReqUserPasswordUpdate;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyUserPasswordUpdateReqEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::CUserLogout> {
	typedef CFTDReqUserLogoutField FTDField;
	typedef data::CUserLogout ZTPField;
	static const int TID = TID_ReqUserLogout;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyUserLogoutEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::CQryInvestorAccount> {
	typedef CFTDQryInvestorAccountField FTDField;
	typedef data::CQryInvestorAccount ZTPField;
	static const int TID = TID_ReqQryInvestorAccount;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyAPIQryInvestorAccountEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = true;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
		ztpField->LoginUserID = LoginID;
		ztpField->LoginBrokerID = BrokerID;
	}
};

template<>
struct ForwardHelper<data::CQryInvestorPosition> {
	typedef CFTDQryInvestorPositionField FTDField;
	typedef data::CQryInvestorPosition ZTPField;
	static const int TID = TID_ReqQryInvestorPosition;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyAPIQryInvestorPositionEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = true;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
		ztpField->LoginUserID = LoginID;
		ztpField->LoginBrokerID = BrokerID;
	}
};

template<>
struct ForwardHelper<data::CQryInstrument> {
	typedef CFTDQryInstrumentField FTDField;
	typedef data::CQryInstrument ZTPField;
	static const int TID = TID_ReqQryInstrument;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyQryInstrumentReqEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::CQryInvestorMargin> {
	typedef CFTDQryInvestorMarginField FTDField;
	typedef data::CQryInvestorMargin ZTPField;
	static const int TID = TID_ReqQryInvestorMargin;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyQryInvestorMarginEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::CQryInvestorFee> {
	typedef CFTDQryInvestorFeeField FTDField;
	typedef data::CQryInvestorFee ZTPField;
	static const int TID = TID_ReqQryInvestorFee;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyQryInvestorFeeEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::CQryInvestorOrder> {
	typedef CFTDQryOrderField FTDField;
	typedef data::CQryInvestorOrder ZTPField;
	static const int TID = TID_ReqQryInvestorOrder;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyQryInvestorOrderEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::CQryInvestorTrade> {
	typedef CFTDQryTradeField FTDField;
	typedef data::CQryInvestorTrade ZTPField;
	static const int TID = TID_ReqQryInvestorTrade;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyQryInvestorTradeEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::COrder> {
	typedef CFTDInputOrderField FTDField;
	typedef data::COrder ZTPField;
	static const int TID = TID_ReqOrderInsert;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyAPIInputOrderEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<>
struct ForwardHelper<data::COrderAction> {
	typedef CFTDOrderActionField FTDField;
	typedef data::COrderAction ZTPField;
	static const int TID = TID_ReqOrderAction;
	static void Copy(ZTPField* ztpField, FTDField* ftdField) {
		CopyAPIOrderActionEntity(ztpField, ftdField);
	}
	constexpr static bool NeedSessionLoginInfo = false;
	static void AttachLogin(ZTPField* ztpField, CLoginIDType const& LoginID, CBrokerIDType const& BrokerID) {
	}
};

template<class ForwardHelperClass>
void CAdapteEngine::ForwardFtdRequest(CFTDPackage* pFTDCPackage, CFTDSession* pSession) {
	ForwardHelper<ForwardHelperClass>::FTDField ftdField;
	if (FTD_GET_SINGLE_FIELD(pFTDCPackage, &ftdField) <= 0) {
		pSession->Disconnect(MSG_FTDERR_BADPACKAGE);
	}
	m_pFrontForwarder->ForwardHead(&m_reqPackage, ForwardHelper<ForwardHelperClass>::TID, pSession->GetSessionID(), pFTDCPackage->GetRequestID());
	ForwardHelper<ForwardHelperClass>::ZTPField ztpField;
	ForwardHelper<ForwardHelperClass>::Copy(&ztpField, &ftdField);
	m_reqPackage.AddField(ztpField);
	if (ForwardHelper<ForwardHelperClass>::NeedSessionLoginInfo) {
		CLoginIDType  LoginID;
		CBrokerIDType BrokerID;
		GetSessionOwner(pSession->GetSessionID(), LoginID, BrokerID);
		ForwardHelper<ForwardHelperClass>::AttachLogin(&ztpField, LoginID, BrokerID);
	}
	m_pFrontForwarder->ForwardTransaction(&m_reqPackage);
}

template<class RspHelperClass>
void CAdapteEngine::RspToDialog(CZTPPackage* pMessage, CFTDSession* pSession) {
	DWORD FtdTID = RspHelper<RspHelperClass>::TID;
	m_rspPackage.PreparePackage(FtdTID, pMessage->GetChain());
	m_rspPackage.SetRequestID(pMessage->GetRequestID());			
	data::CRspInfo ztpRspInfoField;					
	CFTDRspInfoField ftdRspInfoField;
	FillRsp(ftdRspInfoField, SystemError);

	auto itor = pMessage->GetFieldTypeIterator();
	if (itor.Retrieve(ztpRspInfoField) > 0) {
		CopyRspInfoEntity(&ftdRspInfoField, &ztpRspInfoField);	
	}
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);
	itor.Next();
	RspHelper<RspHelperClass>::ZTPField ztpField;
	RspHelper<RspHelperClass>::FTDField ftdField;
	while (!itor.IsEnd()) {
			itor.Retrieve(ztpField);
			RspHelper<RspHelperClass>::Copy(&ftdField, &ztpField);
			FTD_ADD_FIELD(&m_rspPackage, &ftdField);
			itor.Next();
	}
	SendToDialogFlow(pSession, &m_rspPackage);
}


#define ZTP_PUBLIC_FORWARD(FtdTID, FtdFieldType, ZtpFieldType, CopyField)	\
{																			\
	ZtpFieldType ztpField;													\
	FtdFieldType ftdField;													\
	auto itor = pMessage->GetFieldTypeIterator();\
	while (!itor.IsEnd())													\
	{																		\
		itor.Retrieve(ztpField);											\
		m_rspPackage.PreparePackage(FtdTID, FTD_CHAIN_LAST);				\
		CopyField;															\
		FTD_ADD_FIELD(&m_rspPackage, &ftdField);							\
		SendToPublicFlow(&m_rspPackage);									\
		itor.Next();														\
	}																		\
}



CAdapteEngine::CAdapteEngine(CReactorThread* pReactor, CFrontForwarder* pFrontForwarder)
:CFrontEngine(pReactor,pFrontForwarder) {
	m_rspPackage.ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);
}
CAdapteEngine::~CAdapteEngine() {
}

void CAdapteEngine::OnTimer(int nIDEvent) {
	CFrontEngine::OnTimer(nIDEvent);
}
int CAdapteEngine::HandlePackage(CFTDPackage* pFTDCPackage, CFTDSession* pSession) {
	if (NULL == pSession) {
		Log::Fatal("CAdapterEngine: can not handleFTDPackage without session!");
		return 0;
	}

	if (FTD_TID_ReqUserLogin!=pFTDCPackage->GetTID() && !IsSessionLogin(pSession->GetSessionID())) {
		Log::Fatal(fmt::format("CAdapterEngine: session[{}] is not login! can not handle tID[{}]", pSession->GetSessionID(), pFTDCPackage->GetTID()));
		return 0;
	}
//	Log::Info(fmt::format("CAdapterEngine: start to handlepackage sessionID[{}] tID[{}]", pSession->GetSessionID(), pFTDCPackage->GetTID()));

	int rtnCode = 0;
	switch (pFTDCPackage->GetTID()) {
	case FTD_TID_ReqUserLogin:
		rtnCode = ReqUserLogin(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqUserLogout:
		ForwardFtdRequest<data::CUserLogout>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FID_TID_ReqForceLogout:
//		ForwardFtdRequest<CForceInvestorExitField>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FID_TID_ReqUserPasswordUpdate:
		ForwardFtdRequest<data::CUserPasswordUpdate>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInstrument:
		ForwardFtdRequest<data::CQryInstrument>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInvestorAccount:
		ForwardFtdRequest<data::CQryInvestorAccount>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInvestorPosition:
		ForwardFtdRequest<data::CQryInvestorPosition>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInvestorMargin:
		ForwardFtdRequest<data::CQryInvestorMargin>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInvestorFee:
		ForwardFtdRequest<data::CQryInvestorFee>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInvestorOrder:
		ForwardFtdRequest<data::CQryInvestorOrder>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqQryInvestorTrade:
		ForwardFtdRequest<data::CQryInvestorTrade>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqOrderInsert:
		ForwardFtdRequest<data::COrder>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqOrderAction:
		ForwardFtdRequest<data::COrderAction>(pFTDCPackage, (CFTDSession*)pSession);
		break;
	default:
		SendErrorRsp(pFTDCPackage, pSession, -99, fmt::format("UnRecognized FTD_TID[{}]", pFTDCPackage->GetTID()).c_str());
		break;
	}
	return rtnCode;
}
void CAdapteEngine::HandleMessage(CZTPPackage* pMessage) {
	if (!HandleZTPNotify(pMessage)) {
		HandleZTPResponse(pMessage);
	}
}
void CAdapteEngine::InitOperatorInvestor() {
// 	auto* pOperatorInvestor = m_memoryDB.m_pOperatorInvestorFactory->getFirst();
// 	while (pOperatorInvestor) {
// 		m_sessionManager.AddOperatorInvestor(pOperatorInvestor->OperatorID, pOperatorInvestor->InvestorID);
// 		pOperatorInvestor = m_memoryDB.m_pOperatorInvestorFactory->getNext();
// 	}
}

CSession* CAdapteEngine::CreateSession(CChannel* pChannel, DWORD dwMark) {
	CFTDSession* pSession =  new CFTDSession(m_pIOThread, pChannel);
	pSession->RegisterPackageHandler(this);
	pSession->SetCompressMethod(CRPCM_ZERO);
	pSession->SetHeartbeatMode(XMPHBT_SLAVE_MODE);
	return pSession;
}
void CAdapteEngine::OnSessionDisconnected(CSession *pSession, int nReason) {
	m_sessionManager.SessionDisconnected(pSession->GetSessionID());
	CFrontEngine::OnSessionDisconnected(pSession, nReason);
}

bool CAdapteEngine::HandleZTPNotify(CZTPPackage* pMessage) {
	switch (pMessage->GetTID()) {
// 	case TID_NtfDelUserSession:
// 		OnDelUserSession(pMessage);	
// 		break;
	case TID_ErrOrderInsert:
		NotifyOrderError(pMessage);
		break;
	case TID_ErrOrderAction:		
		NotifyActionError(pMessage);
		break;
	case TID_NotifyOrder:			
		NotifyOrder(pMessage);
		break;
	case TID_NotifyTrade:			
		NotifyTrade(pMessage);
		break;
	case TID_NotifyInstrumentStatus:
		ZTP_PUBLIC_FORWARD(FTD_TID_RtnInstrumentStatus, CFTDInstrumentStatusField, data::CInstrumentStatus, { CopyInstrumentStatusEntity(&ftdField,&ztpField); });
		break;
	case TID_RtnDeferFee:
		NotifyDeferFee(pMessage);
		break;
	default:
		return false;
	}
	return true;
}
void CAdapteEngine::HandleZTPResponse(CZTPPackage* pMessage) {
	CFTDSession* pSession = (CFTDSession*)GetSession(pMessage->GetSessionID());
	if (NULL == pSession) {
		Log::Fatal(fmt::format("CAdapterEngine: can not handleZTPResponse without session!SessionID[{}] tID[{}]",pMessage->GetSessionID(),pMessage->GetTID()));
		return;
	}

	switch (pMessage->GetTID()) {
	case TID_RspUserLogin:					
		OnRspUserLogin(pMessage, pSession);
		break;
	case TID_RspUserLogout:					
		OnRspUserLogout(pMessage, pSession);
		break;
	case TID_RspUserPasswordUpdate:
		RspToDialog<CFTDUserPasswordUpdateField>(pMessage, pSession);
		break;
	case TID_RspOrderInsert:
		RspToDialog<CFTDInputOrderField>(pMessage, pSession);
		break;
	case TID_RspOrderAction:				
		RspToDialog<CFTDOrderActionField>(pMessage, pSession);
		break;
	case TID_RspQryInstrument:				
		RspToDialog<CFTDRspInstrumentField>(pMessage, pSession);
		break;
	case TID_RspQryInvestorAccount:			
		RspToDialog<CFTDRspInvestorAccountField>(pMessage, pSession);
		break;
	case TID_RspQryInvestorPosition:		
		RspToDialog<CFTDRspInvestorPositionField>(pMessage, pSession);
		break;
	case TID_RspQryInvestorMargin:			
		RspToDialog<CFTDRspInvestorMarginField>(pMessage, pSession);
		break;
	case TID_RspQryInvestorFee:				
		RspToDialog<CFTDRspInvestorFeeField>(pMessage, pSession);
		break;
 	case TID_RspQryInvestorOrder:			
		RspToDialog<CFTDOrderField>(pMessage, pSession);
		break;
	case TID_RspQryInvestorTrade:
		RspToDialog<CFTDTradeField>(pMessage, pSession);
		break;
	case TID_RspError: {
		rspErrorToDialog(pMessage, pSession);
	}
	break;
	default:

		return;
	}
}
void CAdapteEngine::rspErrorToDialog(CZTPPackage* pMessage, CFTDSession* pSession) {
	const int FtdTID = FTD_TID_RspError;
	m_rspPackage.PreparePackage(FtdTID, pMessage->GetChain());
	m_rspPackage.SetRequestID(pMessage->GetRequestID());

	data::CRspInfo ztpRspInfoField;
	if (pMessage->GetSingleField(ztpRspInfoField) > 0) {
		CFTDRspInfoField ftdRspInfoField;
		CopyRspInfoEntity(&ftdRspInfoField, &ztpRspInfoField);
		FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);
	}
	SendToDialogFlow(pSession, &m_rspPackage);
}
void CAdapteEngine::SendToDialogFlow(CFTDSession* pSession, CFTDPackage* pPackage) {
	TFrontSessionInfo* pSessionInfo = GetSessionInfo(pSession->GetSessionID());
	if (NULL == pSessionInfo)
		return;

	FTDHeader* pHeader = pPackage->GetFTDHeader();
	pHeader->SequenceSeries = DIALOG_FLOW;
	pHeader->SequenceNumber = ++(pSessionInfo->nDialogFlowSeqNo);
	pSession->SendRequestPackage(pPackage);
}

void CAdapteEngine::NotifyOrder(CZTPPackage* pPackage) {
	data::COrder ztpField;
	CFTDOrderField ftdField;
	if (!pPackage->GetSingleField(ztpField)) {
		return;
	}

	CopyFTDOrderEntity(&ftdField, &ztpField);
	m_rspPackage.PreparePackage(FTD_TID_RtnOrder, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &ftdField);
	
	m_sessionManager.Notify(ztpField.InvestorID, &m_rspPackage);
}
void CAdapteEngine::NotifyTrade(CZTPPackage* pPackage) {
	data::CTrade ztpField;
	CFTDTradeField ftdField;

	if (!pPackage->GetSingleField(ztpField)) {
		return;
	}

	CopyFTDTradeEntity(&ftdField, &ztpField);
	m_rspPackage.PreparePackage(FTD_TID_RtnTrade, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &ftdField);
	m_sessionManager.Notify(ztpField.InvestorID, &m_rspPackage);
}

void CAdapteEngine::NotifyDeferFee(CZTPPackage* pPackage) {
	data::CDeferFee ztpField;
	CFTDDeferFeeField ftdField;

	if (!pPackage->GetSingleField(ztpField)) {
		return;
	}
	ftdField.InstrumentID = ztpField.InstrumentID;
	ftdField.Direction = char(ztpField.Direction);
	ftdField.FeeRate = ztpField.FeeRate;
	m_rspPackage.PreparePackage(FTD_TID_RtnDeferFee, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &ftdField);
	m_sessionManager.NotifyAllInvestor(&m_rspPackage);
}
void CAdapteEngine::NotifyOrderError(CZTPPackage* pPackage) {
	data::COrder ztpField;
	data::CRspInfo ztpRspInfoField;
	CFTDInputOrderField ftdField;
	CFTDRspInfoField ftdRspInfoField;
	auto itor = pPackage->GetFieldTypeIterator();
	if (!itor.Retrieve(ztpRspInfoField)) {
		return;
	}
	itor.Next();
	if (!itor.Retrieve(ztpField)) {
		return;
	}
	CopyRspInfoEntity(&ftdRspInfoField,&ztpRspInfoField);
	CopyAPIInputOrderEntity(&ftdField, &ztpField);
	m_rspPackage.PreparePackage(FTD_TID_ErrRtnOrderInsert, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);
	FTD_ADD_FIELD(&m_rspPackage, &ftdField);
	m_sessionManager.Notify(ztpField.InvestorID, &m_rspPackage);
}
void CAdapteEngine::NotifyActionError(CZTPPackage* pPackage) {
	data::COrderAction ztpField;
	data::CRspInfo ztpRspInfoField;
	CFTDOrderActionField ftdField;
	CFTDRspInfoField ftdRspInfoField;

	auto itor = pPackage->GetFieldTypeIterator();
	if (!itor.Retrieve(ztpRspInfoField)) {
		return;
	}
	itor.Next();
	if (!itor.Retrieve(ztpField)) {
		return;
	}
	CopyRspInfoEntity(&ftdRspInfoField, &ztpRspInfoField);
	CopyAPIOrderActionEntity(&ftdField, &ztpField);
	m_rspPackage.PreparePackage(FTD_TID_ErrRtnOrderAction, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);
	FTD_ADD_FIELD(&m_rspPackage, &ftdField);
	m_sessionManager.Notify(ztpField.InvestorID, &m_rspPackage);
}
void CAdapteEngine::SendErrorRsp(CFTDPackage* pRequest, CFTDSession* pSession, int const nErrorCode, char const* pszErrorMsg) {
	CFTDRspInfoField rspInfoField = { 0 };
	rspInfoField.ErrorID = nErrorCode;
	rspInfoField.ErrorMsg = pszErrorMsg;
	m_rspPackage.PrepareResponse(pRequest, FTD_TID_RspError, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&m_rspPackage, &rspInfoField);
	SendToDialogFlow(pSession, &m_rspPackage);
}

int CAdapteEngine::ReqUserLogin(CFTDPackage* pPackage, CFTDSession* pSession) {	
	CFTDReqUserLoginField reqLoginField;
	if(FTD_GET_SINGLE_FIELD(pPackage, &reqLoginField) <= 0) {
		Log::Fatal("CAdapterEngine: reqUserLogin with errorPackage");
		pSession->Disconnect(MSG_FTDERR_BADPACKAGE);
		return -1;
	}

	Log::Info(fmt::format("CAdapteEngine: UserLogin Request Investor[{}]", reqLoginField.LoginID));
	
	m_pFrontForwarder->ForwardHead(&m_reqPackage, TID_ReqUserLogin, pSession->GetSessionID(), pPackage->GetRequestID());
	data::CUserLogin userLoginField;
	userLoginField.LoginTime = "";
	CopyLoginBaseInfoEntity(&userLoginField, &reqLoginField);
	m_reqPackage.AddField(userLoginField);
	m_pFrontForwarder->ForwardTransaction(&m_reqPackage);	
	return 0;
}
void CAdapteEngine::OnRspUserLogin(CZTPPackage* pMessage, CFTDSession* pSession) {
	m_rspPackage.PreparePackage(FTD_TID_RspUserLogin, FTD_CHAIN_LAST);
	m_rspPackage.SetRequestID(pMessage->GetRequestID());
	data::CRspInfo rspInfoField;
	auto itor = pMessage->GetFieldTypeIterator();
	if (itor.Retrieve(rspInfoField) <= 0)
		return;
	CFTDRspInfoField ftdRspInfoField;
	CopyRspInfoEntity(&ftdRspInfoField, &rspInfoField);
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);
	data::CUserLogin userLoginField;
	itor.Next();
	if(itor.Retrieve(userLoginField) <= 0)
		return;
	
	const char* ProductInfo = "JinChan 1.2.1";
	CFTDRspUserLoginField ftdRspUserLoginField;
	ftdRspUserLoginField.TradingDay = userLoginField.TradingDay.value();
	ftdRspUserLoginField.LoginTime = userLoginField.LoginTime.value();
	ftdRspUserLoginField.BrokerID = userLoginField.BrokerID.value();
	ftdRspUserLoginField.LoginID = userLoginField.LoginID.value();
	ftdRspUserLoginField.UserType = char(userLoginField.UserType);
	ftdRspUserLoginField.TradingSystemName = ProductInfo;
	ftdRspUserLoginField.SessionID = pSession->GetSessionID();
	ftdRspUserLoginField.MaxOrderLocalID = userLoginField.MaxOrderLocalID.value();
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspUserLoginField);
	pSession->SendRequestPackage(&m_rspPackage);

	if (0 == rspInfoField.ErrorID) {
		SetSessionOwner(pSession->GetSessionID(), userLoginField.LoginID.value(), char(userLoginField.UserType), userLoginField.BrokerID.value());
		SetSessionMaxCommFlux(pSession->GetSessionID(), INT_MAX);	//暂时不设流控
		auto iter = m_sessionInfoMap.find(pSession->GetSessionID());
		if (iter == m_sessionInfoMap.end()) {
			return;
		}
		if (userLoginField.UserType == type::UserType::Investor) {
			type::InvestorID investorID(userLoginField.LoginID.value());
			m_sessionManager.InvestorLogined(investorID, iter->second);
		} else {
			type::OperatorID operID(userLoginField.LoginID.value());
			m_sessionManager.OperatorLogined(operID, iter->second);
		}
	}
}
void CAdapteEngine::OnRspUserLogout(CZTPPackage* pMessage, CFTDSession* pSession) {
	m_rspPackage.PreparePackage(FTD_TID_RspUserLogout, FTD_CHAIN_LAST);
	m_rspPackage.SetRequestID(pMessage->GetRequestID());
	data::CRspInfo rspInfoField;
	auto itor = pMessage->GetFieldTypeIterator();
	if (itor.Retrieve(rspInfoField) <= 0)
		return;
	CFTDRspInfoField ftdRspInfoField;
	CopyRspInfoEntity(&ftdRspInfoField, &rspInfoField);	
	FTD_ADD_FIELD(&m_rspPackage, &ftdRspInfoField);
	
	if (0 == rspInfoField.ErrorID) {
		ClearSessionOwner(pSession->GetSessionID());
		if (auto sessionInfo = m_sessionManager.GetSessionInfo(pSession->GetSessionID()); sessionInfo) {
			if (sessionInfo->UserType == UT_Manager) {
				type::OperatorID operID(sessionInfo->LoginID);
				m_sessionManager.OperatorLogouted(operID, *sessionInfo);
			} else {
				type::InvestorID investorID(sessionInfo->LoginID);
				m_sessionManager.InvestorLogouted(investorID, *sessionInfo);
			}
		}

		data::CUserLogout userLogoutField;
		itor.Next();
		if(itor.Retrieve(userLogoutField) <= 0)
			return;
		CFTDRspUserLogoutField ftdRspUserLogoutField;
		CopyUserLogoutEntity(&ftdRspUserLogoutField, &userLogoutField);
		FTD_ADD_FIELD(&m_rspPackage, &ftdRspUserLogoutField);
	}
	SendToDialogFlow(pSession, &m_rspPackage);	
}
void CAdapteEngine::OnDelUserSession(CZTPPackage* pMessage) {
// 	CUserSessionField fieldUserSession;
// 	if (ZTP_GET_SINGLE_FIELD(pMessage, &fieldUserSession) <= 0)
// 		return;
// 
// 	CSession* pSession = GetSession(fieldUserSession.SessionID);
// 	if (NULL != pSession)
// 		pSession->Disconnect(0);
}

inline void InvestorSessionManager::InvestorLogined(type::InvestorID const& InvestorID, TFrontSessionInfo const& sessionInfo) {
	m_sessions.emplace(sessionInfo.pSession->GetSessionID(), sessionInfo);

	if (auto iter = m_investorFlows.find(InvestorID); iter != m_investorFlows.end()) {
		auto flow = iter->second;
		((CFTDSession*)sessionInfo.pSession)->Publish(iter->second, PRIVATE_FLOW, flow->GetCount());
//		sessionInfo.pSession->Disconnect(0);
	} else {
		auto flow = new CCachedFlow(true, 20000, 8096);
		m_investorFlows.emplace(InvestorID, FlowPtr(flow));
		((CFTDSession*)sessionInfo.pSession)->Publish(flow, PRIVATE_FLOW, 0);
	}

}

inline void InvestorSessionManager::InvestorLogouted(type::InvestorID const& InvestorID, TFrontSessionInfo const& sessionInfo) {
	m_sessions.erase(sessionInfo.pSession->GetSessionID());
}

inline void InvestorSessionManager::SessionDisconnected(DWORD SessionID) {
	auto iter = m_sessions.find(SessionID);
	if (iter != m_sessions.end()) {
		if (iter->second.UserType == UT_Manager) {
			type::OperatorID operID(iter->second.LoginID);
			OperatorLogouted(operID, iter->second);
		} else {
			type::InvestorID investorID(iter->second.LoginID);
			InvestorLogouted(investorID, iter->second);
		}
	}
}

inline void InvestorSessionManager::OperatorLogined(type::OperatorID const& OperatorID, TFrontSessionInfo const& sessionInfo) {

	if (auto iter = m_operFlows.find(OperatorID); iter != m_operFlows.end()) {
		auto flow = iter->second;
		((CFTDSession*)sessionInfo.pSession)->Publish(iter->second, PRIVATE_FLOW, flow->GetCount());
	} else {
		auto flow = new CCachedFlow(true, 100000, 8096);
		m_operFlows.emplace(OperatorID, FlowPtr(flow));
		((CFTDSession*)sessionInfo.pSession)->Publish(flow, DIALOG_FLOW, 0);
	}
}

inline void InvestorSessionManager::OperatorLogouted(type::OperatorID const& OperatorID, TFrontSessionInfo const& sessionInfo) {
	m_sessions.erase(sessionInfo.pSession->GetSessionID());
}

inline void InvestorSessionManager::Notify(type::InvestorID InvestorID, CFTDPackage* pPackage) {
	pPackage->MakePackage();
	auto iter = m_investorFlows.find(InvestorID);
	if (iter != m_investorFlows.end()) {
		iter->second->Append(pPackage->GetAddress(), pPackage->GetLength());
	}

	//Op可以管理所有Investor，以后再优化
	for (auto& opFlow : m_operFlows) {
		opFlow.second->Append(pPackage->GetAddress(), pPackage->GetLength());
	}
}

void InvestorSessionManager::NotifyAllInvestor(CFTDPackage* pPackage) {
	pPackage->MakePackage();
	for (auto& investorFlow : m_investorFlows) {
		investorFlow.second->Append(pPackage->GetAddress(), pPackage->GetLength());
	}
}

inline std::optional<TFrontSessionInfo> InvestorSessionManager::GetSessionInfo(DWORD sessionID) const {
	auto iter = m_sessions.find(sessionID);
	if (iter == m_sessions.end()) {
		return {};
	}
	return iter->second;
}
