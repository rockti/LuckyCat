#include "stdafx.h"
#include "LoginServiceLink.h"
#include "ZtpDataStruct.h"
#include <fmt/format.h>

CLoginServiceLink::CLoginServiceLink(SystemStatusRepository& systemRepo, 
	SessionRepository& sessionRepo,
	InvestorRepository& investorRepo,
	OperatorRepository& operRepo, 
	Database& database,
	IFlow* pTradeResultFlow)
:m_systemRepo(systemRepo),m_loginService(sessionRepo, investorRepo, operRepo, database),
m_pTradeResultFlow(pTradeResultFlow){
	m_rspPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
}
CLoginServiceLink::~CLoginServiceLink() {
	m_rspPackage.Release();	
}

template<class Field>
void CLoginServiceLink::PushRspToResultFlow(DWORD tID, CZTPPackage& package, Field& field, Error error) {
	m_rspPackage.PreparePublish(tID);
	m_rspPackage.GetZTPHeader()->SessionID = package.GetSessionID();
	m_rspPackage.GetZTPHeader()->RequestID = package.GetRequestID();
	data::CRspInfo rspField{ error.Code,type::ErrorMsg(error.Msg) };
	m_rspPackage.AddField(rspField);
	m_rspPackage.AddField(field);
	m_rspPackage.MakePackage();
	m_pTradeResultFlow->Append(m_rspPackage.GetAddress(), m_rspPackage.GetLength());
}

Error CLoginServiceLink::HandleMessage(CZTPPackage& package) {
	Error error = NoError;
	switch (package.GetTID()) {
	case TID_ReqUserLogin: {
		data::CUserLogin loginField;
		package.GetSingleField(loginField);
		error = m_loginService.UserLogin(loginField, package.GetSessionID());
		if (error.IsSuccess()) {
			Log::Info(fmt::format("IP[{}] Login Success: BrokerID[{}] LoginID[{}] UserType[{}] UserProductInfo[{}] SessionID[{}]", loginField.IPAddress
				, loginField.BrokerID, loginField.LoginID, loginField.UserType, loginField.UserProductInfo, package.GetSessionID()));
		} else {
			Log::Info(fmt::format("IP[{}] Login Failed[{}]: BrokerID[{}] LoginID[{}] UserType[{}] Password[{}] UserProductInfo[{}] SessionID[{}]", 
				loginField.IPAddress, error.Msg, loginField.BrokerID, 
				loginField.LoginID, loginField.UserType, loginField.Password, 
				loginField.UserProductInfo, package.GetSessionID()));
		}
		loginField.TradingDay = m_systemRepo.Get()->TradingDay;
		PushRspToResultFlow(TID_RspUserLogin, package, loginField, error);
	}
		break;
	case TID_ReqUserLogout: {
		data::CUserLogout logoutField;
		package.GetSingleField(logoutField);
		error = m_loginService.UserLogout(logoutField.BrokerID, logoutField.LoginID, package.GetSessionID());
		if (error.IsSuccess()) {
			Log::Info(fmt::format("Logout Success: BrokerID[{}] UserID[{}]", logoutField.BrokerID, logoutField.LoginID));
		}
		PushRspToResultFlow(TID_RspUserLogout, package, logoutField, error);
	}
		break;
	case TID_ReqForceLogout: {
// 		CForceInvestorExitField forceUserExit;
// 		ZTP_GET_SINGLE_FIELD(package, &forceUserExit);
// 		error = m_loginService.ForceLogout(forceUserExit.BrokerID, forceUserExit.InvestorID, package.GetSessionID());
// 		if (error.IsSuccess()) {
// 			Log::Info(fmt::format("ForceLogout: Success: BrokerID[{}] InvestorID[{}]", forceUserExit.BrokerID, forceUserExit.InvestorID));
// 		} else {
// 			Log::Info(fmt::format("ForceLogout: failed[{}]: BrokerID[{}] InvestorID[{}]", error.Msg, forceUserExit.BrokerID, forceUserExit.InvestorID));
// 		}
// 		PushRspToResultFlow(TID_RspForceLogout, package, forceUserExit,error);

	}
		break;
	case TID_ReqUserPasswordUpdate: {
		data::CUserPasswordUpdate field;
		package.GetSingleField(field);
		error = m_loginService.ChangePassword(field, package.GetSessionID());
		if (error.IsSuccess()) {
			Log::Info(fmt::format("ChangePassword Success: BrokerID[{}] UserID[{}]", field.BrokerID, field.InvestorID));
		}
		PushRspToResultFlow(TID_RspUserPasswordUpdate, package, field, error);
	}
		break;
	default:
		break;
	}

	return error;
}


