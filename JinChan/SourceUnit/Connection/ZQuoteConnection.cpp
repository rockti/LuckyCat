#include "stdafx.h"
#include "SGEQuoteConnection.h"
#include "CachedFlow.h"
#include "Profile.h"
#include <fmt/format.h>
#include "ZQuoteConnection.h"
#include "Profile.h"
#include "DBInit.h"
ZQuoteConnection::ZQuoteConnection(IFlow* flow):quoteHandler_(flow) {
	ZQAPIApplicationInfo appInfo;
	strcpy_s(appInfo.AuthCode, "");
	strcpy_s(appInfo.KeyOperationLogPath, "./ZQuote");
	int result = 0;
	quoteAPI_ = CreateZQuoteAPI(&appInfo,result);
	std::string address = theProfile().GetPrivateProfile("ZQuote", "Address", "");
	int port = theProfile().GetPrivateProfile("ZQuote", "Port", 0);
	if (port == 0 || address.empty()) {
		printNlog("load ZQuote address and port failed. Exit");
		exit(-1);
	}
	quoteAPI_->SetHostAddress(address.c_str(), port);
	quoteHandler_.SetZQuoteAPI(quoteAPI_);
	quoteAPI_->SetAPINotify(&quoteHandler_);
	
	::Sleep(1000);
	ZQAPIQuoteLoginAuth loginReq = { 0 };
	//行情账户GD01 密码Gd666888   地址139.196.194.205:8050  
	auto const& seatInfo = ReposInstance().BaseRepo.Get();
	strcpy_s(loginReq.UserNo, seatInfo->TraderID.value());
	strcpy_s(loginReq.Password, seatInfo->Password.value());
	loginReq.ISDDA = ZQYNFLAG_NO;
	loginReq.ISModifyPassword = ZQYNFLAG_NO;
	int rtn = quoteAPI_->Login(&loginReq);
	if (rtn != 0) {
		printNlog("login failed, rtn[{}]", rtn);
	}
}

ZQuoteConnection::~ZQuoteConnection() {
	if (quoteAPI_) {
		quoteAPI_->Disconnect();
	}
}

void ZQuoteConnection::Login() {
	ZQAPIQuoteLoginAuth loginReq;
	quoteAPI_->Login(&loginReq);
}

void ZQuoteConnection::SubMarket() {
}
