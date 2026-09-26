#include "stdafx.h"
#include "LoginServiceImpl.h"
#include "ErrorDefine.h"
#include "ToolFunctions.h"
#include "sha256.h"
#include "Database.h"

CLoginServiceImpl::CLoginServiceImpl(SessionRepository& sessionRepo,
	InvestorRepository& investorRepo,
	OperatorRepository& operRepo,
	Database& database)
:m_sessionRepo(sessionRepo), m_investorRepo(investorRepo),
m_operRepo(operRepo),m_database(database) {
}


Error CLoginServiceImpl::baseCheck(type::BrokerID const& BrokerID, type::LoginID const& LoginID) {
	if (BrokerID == "") {
		return BlankBrokerIDError;
	}
	if (LoginID == "") {
		return BlankOperatorIDError;
	}
	return NoError;
}
Error CLoginServiceImpl::operCheck(type::BrokerID const& BrokerID, type::LoginID const& LoginID, type::Password const& Password) {
	type::OperatorID operID(LoginID.value());
	data::COperatorInfo* pOper = m_operRepo.Get(BrokerID, operID);
	if (NULL == pOper) {
		return InvalidLoginError;
	}
	if (Password != pOper->Password) {
		return InvalidLoginError;
	}
	return NoError;
}

Error CLoginServiceImpl::investorCheck(type::BrokerID const& BrokerID, type::LoginID const& LoginID, type::Password const& password) {
	type::InvestorID investorID(LoginID.value());
	auto* pInvestorInfo = m_investorRepo.GetInfo(investorID);
	if (NULL == pInvestorInfo) {
		return InvalidLoginError;
	}

	if (pInvestorInfo->IsNewAuth == type::IsNewAuth::Old) {
		if (password != pInvestorInfo->Password) {
			return InvalidLoginError;
		}
	} else {
		auto newHash = hashPassword(password, pInvestorInfo);
		if (newHash != pInvestorInfo->PasswordHash.value()) {
			return InvalidLoginError;
		}
	}
	return NoError;
}

void CLoginServiceImpl::addSession(data::CUserLogin const& LoginInfo, type::SessionID const& SessionID) {
	data::CUserSession userSession;
	userSession.BrokerID = LoginInfo.BrokerID;
	userSession.UserType = LoginInfo.UserType;
	userSession.LoginID = LoginInfo.LoginID;
	userSession.SessionID = SessionID;
	userSession.LoginTime = GetLocalTime();
	userSession.IPAddress = LoginInfo.IPAddress;
	userSession.UserProductInfo = LoginInfo.UserProductInfo;
	m_sessionRepo.Add(userSession);
}


Error CLoginServiceImpl::UserLogin(data::CUserLogin const& LoginInfo, type::SessionID const& SessionID) {
	Error error = baseCheck(LoginInfo.BrokerID, LoginInfo.LoginID);
	if (error.IsSuccess()) {
		if (type::UserType::Operator == LoginInfo.UserType) {
			error = operCheck(LoginInfo.BrokerID, LoginInfo.LoginID, LoginInfo.Password);
		} else {
			error = investorCheck(LoginInfo.BrokerID, LoginInfo.LoginID, LoginInfo.Password);
		}

		if (error.IsSuccess()) {
			addSession(LoginInfo, SessionID);
		}
	}

	return error;
}

Error CLoginServiceImpl::UserLogout(type::BrokerID const& BrokerID, type::LoginID const& LoginID, type::SessionID const& SessionID) {
	Error error = baseCheck(BrokerID, LoginID);
	if (error.IsSuccess()) {
		auto* pUserSession = m_sessionRepo.Get(SessionID);
		if (NULL == pUserSession) {
			return NotLoginError;
		}
		m_sessionRepo.Delete(*pUserSession);
	}

	return error;
}
Error CLoginServiceImpl::ForceLogout(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID, type::SessionID const& SessionID) {
	type::LoginID loginID(InvestorID.value());
	Error error = baseCheck(BrokerID, loginID);
	auto* pInvestorInfo = m_investorRepo.GetInfo(InvestorID);
	if (NULL == pInvestorInfo) {
		return InvalidInvestorError;
	}

	return NoError;
}

inline std::string tail(std::string const& source, size_t const length) {
	if (length >= source.size()) { return source; }
	return source.substr(source.size() - length);
} // tail

std::string CLoginServiceImpl::hashPassword(type::Password password, data::CInvestorInfo* pInvestorInfo) {
	SHA256 sha256;
	auto passwordHash = sha256(password.value()).substr(0, 32);
	auto waitingForHash = fmt::format("{}{}{}", passwordHash, pInvestorInfo->Salt, tail(pInvestorInfo->InvestorID.value(), 6));
	std::string newHash = sha256(waitingForHash).substr(0, 32);
	return newHash;
}

Error CLoginServiceImpl::ChangePassword(data::CUserPasswordUpdate const& info, type::SessionID const& SessionID) {
	type::LoginID loginID(info.InvestorID.value());
	Error error = baseCheck(info.BrokerID, loginID);
	if (!error) return error;
	error = investorCheck(info.BrokerID, loginID, info.OldPassword);
	if (!error) return error;
	auto* pInvestorInfo = m_investorRepo.GetInfo(info.InvestorID);

	auto newHash = hashPassword(info.NewPassword,pInvestorInfo);
	auto result = m_database.ChangePassword(info.InvestorID, type::Hash(newHash.c_str()));
	pInvestorInfo->IsNewAuth = type::IsNewAuth::New;
	pInvestorInfo->PasswordHash = newHash;
	return result? NoError : SystemError;
}

bool CLoginServiceImpl::IsLogined(type::SessionID const& sessionID) const {
	return m_sessionRepo.Get(sessionID) != nullptr;
}

