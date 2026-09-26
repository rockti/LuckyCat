#include "stdafx.h"
#include "BaseServiceLink.h"

Error SessionCheckService::CheckSession(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID, type::SessionID const& SessionID) {
	auto* pUserSession = m_sessionRepo.Get(SessionID);
	if (NULL == pUserSession) {
		return SessionNotFoundError;
	}
	if (pUserSession->UserType == type::UserType::Operator) {
		//OWEN TODO, 目前Operator可以管理所有Investor，以后再说
	} else {
		if (pUserSession->LoginID != InvestorID) {
			return SessionNotFoundError;
		}
	}
	return NoError;
}
