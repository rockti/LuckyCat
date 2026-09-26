#pragma once
#include "Database.h"
#include "SessionFactory.h"
#include "Repository/MemoryRepositoryImpl.h"
#include "Profile.h"
#include "LoginServiceImpl.h"
#include "DBInit.h"
#include "FTDSession.h"

class MarketService : public CServerSessionFactory, public CFTDSessionCallback {
public:
	MarketService(CReactorThread* pThread, CCachedFlow* depthMarketFlow);
protected:
	virtual CSession* CreateSession(CChannel* pChannel, DWORD dwMark) override;
protected:
	virtual int HandlePackage(CFTDPackage* pFTDPackage, CFTDSession* pSession) override;

	int onReqUserLogin(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	int onReqUserLogout(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	int onReqSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	int onReqUnSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	void rspError(CFTDSession* pSession, Error error);
	void sendToDialogFlow(CFTDSession* pSession);
	void sendSnapshot(CFTDDepthMarketDataField* snapshot, CFTDPackage* pFTDPackage, CFTDSession* pSession);
	void publishTopicFlows(CFTDPackage* pSubscribePackage, CFTDSession* pSession);

private:
	CCachedFlow* depthMarketFlow_;
	std::string marketBrokerID_;
	CFTDPackage rspPackage_;
	CLoginServiceImpl loginService_;
	int seqNo_ = 0;
};
