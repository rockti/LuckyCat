#include "stdafx.h"
#include "SGEQuoteConnection.h"
#include "CachedFlow.h"
#include "Profile.h"
#include <fmt/format.h>


SGEQuoteConnection::SGEQuoteConnection(ztp::data::CSeatInfo& seatInfo, IFlow* marketFlow,
	std::string const& sgeAddress, int sgePort, std::string const& safeAddress, int safePort) :
	SGEConnectionBase(sgeAddress,sgePort,safeAddress,safePort),
	m_quoteHandler(marketFlow,sgeAddress,sgePort,m_safeContext) {
	m_sgeContext.SGECBList.on_connected_ = SGEQuoteHandler::OnFrontConnected;
	m_sgeContext.SGECBList.on_disconnected_ = SGEQuoteHandler::OnFrontDisconnected;
	m_sgeContext.SGECBList.on_read_ = SGEQuoteHandler::OnReceiveMsg;

	Log::Info("Register context");
	m_quoteHandler.InitSeatInfo(seatInfo);
	m_quoteHandler.RegiesterContext(m_sgeContext);

	SGEConnectionBase::InitInstance();
	Sleep(1000);
	m_quoteHandler.Login();
}

