#include "stdafx.h"
#include "MemoryRepositoryImpl.h"
#include "Investor.h"
void MemSessionRepository::Add(data::CUserSession const& session) {
	m_userSessions[session.SessionID] = session;
}

void MemSessionRepository::Delete(data::CUserSession const& session) {
	auto iter = m_userSessions.find(session.SessionID);
	if (iter != m_userSessions.end()) {
		m_userSessions.erase(iter);
	}
}

data::CUserSession* MemSessionRepository::Get(type::SessionID const& SessionID) {
	auto iter = m_userSessions.find(SessionID);
	if (iter != m_userSessions.end()) {
		return &iter->second;
	}
	return nullptr;
}

void MyInvestorRepository::InitAdd(data::CInvestorAccount& account) {
	auto& investor = m_investors[account.InvestorID];
	CopyInvestorAccountEntity(&investor.Account, &account);
	Investor::UpdateBalanceAndAvailable(&investor.Account);

}
