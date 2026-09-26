#include "stdafx.h"
#include "LinkManager.h"
#include "SGEConnection.h"
#include "MockSGEConnection.h"
#include "Profile.h"

CExchangeLinkManager::CExchangeLinkManager() {
}

int const LINK_LOOP_INTERVAL = 20000;	//交易所连接检查Timer

CBaseExchApi* CExchangeLinkManager::GetSGEHandler(InstrumentRepository* instRepo, data::CSeatInfo const* seatInfo) {
	if (nullptr == m_pTradeLink) {
		if (nullptr == instRepo || nullptr == seatInfo) {
			return nullptr;
		}
		initExchLink(*instRepo, seatInfo);
	}
	return m_pTradeLink;
}

void CExchangeLinkManager::StartWork() {
	if (NULL != m_pTradeLink && false == m_pTradeLink->IsConnected()) {
		m_pTradeLink->StartWork();
	}
}
#include <iostream>
void CExchangeLinkManager::initExchLink(InstrumentRepository& instRepo,data::CSeatInfo const* seatInfo) {
#ifdef JINCHAN_MOCK
 	static MockSGEConnection connection(SgeLinkType::TRADE, NULL, NULL, 0, "", 0, "", 0);
 	m_pTradeLink = connection.GetHandler();
#else
	//SGEConnection里需要创建单独api线程，必须是静态对象
	int topicID = theProfile().GetPrivateProfile("SGE", "TopicID", 1000);
	string sgeAddr = theProfile().GetPrivateProfile("SGE", "SGEAddress", "188.31.25.50");
	int sgePort = theProfile().GetPrivateProfile("SGE", "SGEPort", 7777);
	string safeAddr = theProfile().GetPrivateProfile("SGE", "SafeAddress", "127.0.0.1");
	int safePort = theProfile().GetPrivateProfile("SGE", "SafePort", 6666);
//	static SGEConnection connection(SgeLinkType::QUOTE, m_pMarketFlowManager, m_pMarketRepository, topicID, sgeAddr, sgePort, safeAddr, safePort);
	static SGETradeConnection connection(instRepo, sgeAddr, sgePort, safeAddr, safePort);
	if (seatInfo != nullptr) {
		connection.GetHandler().InitSeatInfo(*seatInfo);
	}
	if (connection.InitInstance(nullptr)) {
// 		connection.GetHandler().StartWork();	//unit test
		m_pTradeLink = &connection.GetHandler();
		return;
	}
	std::cout << "init SGE handler failed" << endl;
	exit(-1);
#endif
}
