#pragma once
#include "Repository.h"

class SessionCheckService {
public:
	SessionCheckService(SessionRepository& sessionRepo) :m_sessionRepo(sessionRepo) {}
	Error CheckSession(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID, type::SessionID const& SessionID);

private:
	SessionRepository& m_sessionRepo;
};