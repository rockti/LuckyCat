#pragma once
#include "ErrorEngine.h"
#include "Repository.h"
class Database;

class CLoginServiceImpl {
public:
	CLoginServiceImpl(
		SessionRepository& sessionRepo,
		InvestorRepository& investorRepo,
		OperatorRepository& operRepo,
		Database& database);

	//用户登录
	Error UserLogin(data::CUserLogin const& LoginInfo, type::SessionID const& SessionID); 

	//用户录出
	Error UserLogout(type::BrokerID const& BrokerID, type::LoginID const& LoginID, type::SessionID const& SessionID);

	//删除用户连接（操作员用）
	Error ForceLogout(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID, type::SessionID const& SessionID);

	//修改密码
 	Error ChangePassword(data::CUserPasswordUpdate const& LoginInfo, type::SessionID const& SessionID);

	bool IsLogined(type::SessionID const& sessionID) const;
private:
	Error baseCheck(type::BrokerID const& BrokerID, type::LoginID const& LoginID);
	Error operCheck(type::BrokerID const& BrokerID, type::LoginID const& LoginID, type::Password const& Password);
	Error investorCheck(type::BrokerID const& BrokerID, type::LoginID const& LoginID, type::Password const& Password);
	void addSession(data::CUserLogin const& LoginInfo, type::SessionID const& SessionID);
	std::string hashPassword(type::Password password, data::CInvestorInfo* pInvestorInfo);
private:
	SessionRepository& m_sessionRepo;
	InvestorRepository& m_investorRepo;
	OperatorRepository& m_operRepo;
	Database& m_database;
};
