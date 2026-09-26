#pragma once
#include "MarketFlowMgr.h"
#include "MarketRepository.h"
#include "SGEConnection.h"
#include "SGEQuoteHandler.h"

class SGEQuoteConnection : public SGEConnectionBase {
public:
	SGEQuoteConnection(ztp::data::CSeatInfo& seatInfo, IFlow* marketFlow, std::string const& sgeAddress, int sgePort, std::string const& safeAddress, int safePort);
	SGEQuoteHandler& GetHandler() {
		return m_quoteHandler;
	}
private:
	SGEQuoteHandler m_quoteHandler;
};