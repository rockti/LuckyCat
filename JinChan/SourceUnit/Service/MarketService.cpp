#include "stdafx.h"
#include "MarketService.h"

MarketService::MarketService(CReactorThread* pThread, CCachedFlow* depthMarketFlow) : CServerSessionFactory(pThread, 1000),
depthMarketFlow_(depthMarketFlow),
loginService_(ReposInstance().SessionRepo, ReposInstance().InvestorRepo, ReposInstance().OperRepo, *DBInstance()) {
	rspPackage_.ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);
}

inline CSession* MarketService::CreateSession(CChannel* pChannel, DWORD dwMark) {
	//在物理连接建立后创建会话
	CFTDSession* pSession = new CFTDSession(m_pIOThread, pChannel);
	pSession->RegisterPackageHandler(this);
	pSession->SetHeartbeatMode(XMPHBT_SLAVE_MODE);
	return pSession;
}

inline void MarketService::rspError(CFTDSession* pSession, Error error) {
	CFTDRspInfoField ftdRspInfoField;
	ftdRspInfoField.ErrorID = error.Code;
	ftdRspInfoField.ErrorMsg = error.Msg;
	rspPackage_.PreparePackage(FTD_TID_RspError, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&rspPackage_, &ftdRspInfoField);

	sendToDialogFlow(pSession);
}

inline void MarketService::sendToDialogFlow(CFTDSession* pSession) {
	FTDHeader* pHeader = rspPackage_.GetFTDHeader();
	pHeader->SequenceSeries = DIALOG_FLOW;
	pHeader->SequenceNumber = ++seqNo_;
	pSession->SendRequestPackage(&rspPackage_);
}

inline int MarketService::HandlePackage(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (FTD_TID_ReqUserLogin != pFTDPackage->GetTID()) {
		if (!loginService_.IsLogined(pSession->GetSessionID())) {
			return 0;
		}
	}

	int nRet = 0;
	switch (pFTDPackage->GetTID()) {
	case FTD_TID_ReqUserLogin:
		nRet = onReqUserLogin(pFTDPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqUserLogout:
		onReqUserLogout(pFTDPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqSubMarketData:
		nRet = onReqSubMarketData(pFTDPackage, (CFTDSession*)pSession);
		break;
	case FTD_TID_ReqUnSubMarketData:
		nRet = onReqUnSubMarketData(pFTDPackage, (CFTDSession*)pSession);
		break;
	default:
		rspError(pSession, InvalidRequestError);
		Log::Debug(fmt::format("CHKMarketFront: received unknown FTDPackageID[{}]", pFTDPackage->GetTID()));
		break;
	}
	return nRet;
}

inline int MarketService::onReqUserLogin(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (NULL == pSession)	return -1;

	CFTDReqUserLoginField reqUserLoginField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &reqUserLoginField) <= 0) {
		Log::Fatal("CMarketFront: OnReqUserLogin get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}
	data::CUserLogin userLogin;
	CopyLoginBaseInfoEntity(&userLogin, &reqUserLoginField);

	rspPackage_.PrepareResponse(pFTDPackage, FTD_TID_RspUserLogin, FTD_CHAIN_LAST);
	CFTDRspUserLoginField rspUserLoginField;
	//rspUserLoginField.TradingDay = m_pFlowManager->GetTradingDay(); todo
	rspUserLoginField.BrokerID = reqUserLoginField.BrokerID;
	rspUserLoginField.LoginID = reqUserLoginField.LoginID;
	rspUserLoginField.UserType = reqUserLoginField.UserType;
	rspUserLoginField.TradingSystemName = "JinChan:Market";
	rspUserLoginField.SessionID = pSession->GetSessionID();
	rspUserLoginField.MaxOrderLocalID = 0;

	auto error = loginService_.UserLogin(userLogin, pSession->GetSessionID());
	CFTDRspInfoField rspInfoField;
	rspInfoField.ErrorID = error.Code;
	rspInfoField.ErrorMsg = error.Msg;

	FTD_ADD_FIELD(&rspPackage_, &rspUserLoginField);
	FTD_ADD_FIELD(&rspPackage_, &rspInfoField);

	sendToDialogFlow(pSession);
	return 0;
}

inline int MarketService::onReqUserLogout(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (NULL == pSession)	return -1;

	CFTDReqUserLogoutField req;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &req) <= 0) {
		Log::Fatal("CMarketFront: OnReqUserLogin get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}
	data::CUserLogout userLogout;
	CopyUserLogoutEntity(&userLogout, &req);
	auto error = loginService_.UserLogout(userLogout.BrokerID, userLogout.LoginID, pSession->GetSessionID());
	CFTDRspUserLogoutField rspUserLogoutField;
	rspUserLogoutField.BrokerID = req.BrokerID;
	rspUserLogoutField.LoginID = req.LoginID;

	CFTDRspInfoField rspInfoField;
	rspInfoField.ErrorID = error.Code;
	rspInfoField.ErrorMsg = error.Msg;

	FTD_ADD_FIELD(&rspPackage_, &rspUserLogoutField);
	FTD_ADD_FIELD(&rspPackage_, &rspInfoField);
	sendToDialogFlow(pSession);
	pSession->Disconnect(0);
	return 0;
}

inline int MarketService::onReqSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	//如果没有合约信息，订阅也没有用
	if (NULL == pSession)
		return 0;

	CFTDRspInfoField rspInfoField;
	pSession->CreatSubInstrument();

	CFTDSpecificInstrumentField specificInstrumentField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &specificInstrumentField) <= 0) {
		Log::Fatal("CMarketFront: OnReqSubMarketData get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}

	auto instrumentIndex = FTDInstance().GetIndex(specificInstrumentField.InstrumentID);
	if (!instrumentIndex.IsValid()) {
		rspInfoField.ErrorID = InvalidInstrumentError.Code;
		rspInfoField.ErrorMsg = InvalidInstrumentError.Msg;
	} else {
		if (pSession->InsertInstrumentID(instrumentIndex.Index(), specificInstrumentField.InstrumentID)) {
			rspInfoField.ErrorID = NoError.Code;
			rspInfoField.ErrorMsg = "成功";
			publishTopicFlows(pFTDPackage, pSession);	//再处理请求包里的订阅流号
		} else {
			rspInfoField.ErrorID = DuplicateInstrumentError.Code;
			rspInfoField.ErrorMsg = DuplicateInstrumentError.Msg;
		}
	}

	rspPackage_.PrepareResponse(pFTDPackage, FTD_TID_RspSubMarketData, FTD_CHAIN_LAST);

	FTD_ADD_FIELD(&rspPackage_, &specificInstrumentField);
	FTD_ADD_FIELD(&rspPackage_, &rspInfoField);

	sendToDialogFlow(pSession);	//先返回订阅应答
	if (rspInfoField.ErrorID == NoError.Code) {
		auto* snapshot = FTDInstance().GetSnapshot(instrumentIndex);
		if (snapshot) {
			sendSnapshot(snapshot, pFTDPackage, pSession);
		}
	}
	return 0;
}

inline void MarketService::sendSnapshot(CFTDDepthMarketDataField* snapshot, CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	CFTDDisseminationField disseminationField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &disseminationField) <= 0) {
		Log::Fatal("CMarketFront: get disseminationField failed, errorPackage");
		return;
	}
	DWORD topicID = disseminationField.SequenceSeries;
	rspPackage_.PrepareResponse(pFTDPackage, FTD_TID_RtnDepthMarketData, FTD_CHAIN_LAST);
	FTD_ADD_FIELD(&rspPackage_, snapshot);
	rspPackage_.SetRequestID(topicID);
	pSession->SendRequestPackage(&rspPackage_);
}

inline int MarketService::onReqUnSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession) {
	if (NULL == pSession)
		return 0;

	CFTDRspInfoField rspInfoField;
	rspInfoField.ErrorID = NoError.Code;
	rspInfoField.ErrorMsg = NoError.Msg;

	rspPackage_.PrepareResponse(pFTDPackage, FTD_TID_RspUnSubMarketData, FTD_CHAIN_LAST);

	CFTDSpecificInstrumentField specificInstrumentField;
	if (FTD_GET_SINGLE_FIELD(pFTDPackage, &specificInstrumentField) <= 0) {
		Log::Fatal("CMarketFront: OnReqUnSubMarketData get ErrorPackage");
		pSession->Disconnect(0);
		return -1;
	}

	auto instrumentIndex = FTDInstance().GetIndex(specificInstrumentField.InstrumentID);
	if (!instrumentIndex.IsValid()) {
		rspInfoField.ErrorID = -1;
		rspInfoField.ErrorMsg = "合约不存在";
	} else {
		if (pSession->DeleteInstrumentID(instrumentIndex.Index())) {
			rspInfoField.ErrorMsg = "退订成功";
		} else {
			rspInfoField.ErrorID = SystemError.Code;
			rspInfoField.ErrorMsg = "退订失败";
		}
	}
	FTD_ADD_FIELD(&rspPackage_, &specificInstrumentField);
	FTD_ADD_FIELD(&rspPackage_, &rspInfoField);
	sendToDialogFlow(pSession);
	return 0;
}

inline void MarketService::publishTopicFlows(CFTDPackage* pSubscribePackage, CFTDSession* pSession) {
	CFTDDisseminationField disseminationField;
	if (FTD_GET_SINGLE_FIELD(pSubscribePackage, &disseminationField) <= 0) {
		Log::Fatal("CMarketFront: get disseminationField failed, errorPackage");
		return;
	}

	DWORD topicID = disseminationField.SequenceSeries;
	if (NULL != depthMarketFlow_) {
		pSession->Publish(depthMarketFlow_, disseminationField.SequenceSeries, depthMarketFlow_->GetCount()/*从流上最新的一个包开始发*/);
	} else {
		Log::Fatal(fmt::format("CMarketFront: can not find topic[{}] flow", topicID));
	}
}
