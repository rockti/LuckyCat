#include "pch.h"
#include "SGEHandler.h"
#include "utils/Log.h"
#include "utils/String.h"
#include "notify/ConnPublisher.h"
#include "notify/Notify.h"
#include "model/dealing/ExchangeRequest.h"

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	if (i) {
		for (j = i; j < 3; j++) {
			char_array_3[j] = ' ';
		}
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
		while ((i++ < 3))
			ret += '=';
	}
	return ret;
}
//去除转义字符
static void FormatSafeInfo(char const* oriBuffer, size_t bufLen, char* destBuffer) {
	for (size_t i = 0, j = 0; i < bufLen; ++i) {
		if (oriBuffer[i] == '[' || oriBuffer[i] == ']' || oriBuffer[i] == '{' || oriBuffer[i] == '}' || oriBuffer[i] == '=' || oriBuffer[i] == ',' || oriBuffer[i] == '\\') {
			destBuffer[j++] = '\\';
			destBuffer[j++] = oriBuffer[i];
		}
		else {
			destBuffer[j++] = oriBuffer[i];
		}
	}
}

namespace {
	inline static ExchangeStatus ConvertExchangeStatusFromSGE(TSGEInstrumentStatusType status) {
		switch (status) {
		case SGE_IS_BeforeTrading:
			return ExchangeStatus::BEFORE_TRADING;	// 交易前
		case SGE_IS_AuctionOrdering:
			return ExchangeStatus::AUCTION_ORDERING;// 集合竞价报单
		case SGE_IS_AuctionMatch:
			return ExchangeStatus::AUCTION_MATCH;	// 集合竞价撮合
		case SGE_IS_Continous:
			return ExchangeStatus::CONTINOUS;		// 连续交易
		case SGE_IS_NoTrading:
			return ExchangeStatus::NO_TRADING;		// 暂停交易
		case SGE_IS_DeliveryAppend:
			return ExchangeStatus::DELIVERY_APPEND;	// 交割申报
		case SGE_IS_DeliveryEnd:
			return ExchangeStatus::DELIVERY_END;	// 交割结束
		case SGE_IS_MidAppend:
			return ExchangeStatus::MID_APPEND;		// 中立仓申报
		case SGE_IS_MidBalance:
			return ExchangeStatus::MID_BALANCE;		// 中立仓平衡
		case SGE_IS_Closed:
			return ExchangeStatus::CLOSED;			// 收盘
		default:
			return ExchangeStatus::UNKNOW;
		}
	}
	inline static Direction ConvertDirectionFromSGE(TSGEDirectionType direction) {
		return ('b' == direction) ? Direction::BUY : Direction::SELL;
	}
	inline static Operation ConvertOperationFromSGE(TSGEOffsetFlagType operation) {
		switch (operation) {
		case SGE_OP_Open: return Operation::OPEN;
		case SGE_OP_Close: return Operation::OFFSET;
		default: return Operation::UNKNOWN;
		}
	}
	inline static OrderType ConvertOrderTypeFromSGE(TSGEOrderPriceType orderTYpe) {
		switch (orderTYpe) {
		case SGE_LimitPrice_FOK: return OrderType::FOK;
		case SGE_LimitPrice_FAK: return OrderType::FAK;
		default: return OrderType::NORMAL;
		}
	}
	inline static char const* ConvertSpotVarietyFromSGE(TSGEVarietyID const& varietyID) {
		switch (varietyID) {
		case 201: return "Au99.99";		
		case 207: return "Au100g";
		default: return "";
		}
	}
}

void SGEHandler::OnTradeFrontConnected(void* ctx) {
	Log::Info("SGEHandler: OnTradeFrontConnected");
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::SGE;
	notification.Type = ExchangeNotification::TRADER_CONNECTED;
	m_pThis->m_publisher.NotifyUI(notification);
	m_pThis->Handshake(true);
}
void SGEHandler::OnTradeFrontDisconnected(int error_no, void* ctx) {
	std::string reportMsg = fmt::format("SGEHandler: OnTradeFrontDisconnected, errorNo[{}]", error_no);
	m_pThis->m_publisher.NotifyUI(reportMsg);
	Log::Error(reportMsg.c_str());

	ExchangeNotification notification;
	notification.Platform = ConnPlatform::SGE;
	notification.Type = ExchangeNotification::TRADER_DISCONNECTED;
	m_pThis->m_publisher.NotifyUI(notification);

	if (m_pThis->m_pTradeContext->SafeSessionPtr) {
		sge_safe_free_client_session(m_pThis->m_pTradeContext->SafeSessionPtr);
		m_pThis->m_pTradeContext->SafeSessionPtr = nullptr;
	}
}
void SGEHandler::OnTradeReceiveMsg(int type, const char* sge_msg, unsigned int msgLen, void* ctx) {
	int praseBufLens = msgLen + 1;	//留一位给'/0'
	char receivedMsg[SGE_MSG_LEN * 2] = "";
	if (MSG_TYPE_PLAIN == type) {
		if (false == m_pThis->m_pTradeContext->IsLogined) {
			memcpy(receivedMsg, sge_msg, msgLen);
			std::string_view gtpMsg(receivedMsg, msgLen);
			m_pThis->DispatchPlainMsg(gtpMsg, true);
		}	// 交易前置一旦登录成功后不再处理明文消息(行情)
	}	// 明文消息派发
	else if (MSG_TYPE_CIPHER == type) {
		if (m_pThis->m_safeHandler.DecryptMsg(sge_msg, msgLen, receivedMsg, praseBufLens, m_pThis->m_pTradeContext->SessionKey)) {
			std::string_view cipherMsg(receivedMsg, msgLen);
			m_pThis->DispatchCipherMsg(cipherMsg, true);
		}
	}	// 密文消息派发
	else {
		std::string reportMsg = fmt::format("SGEHandler: traderHandler received unknown msgType[{}]", type);
		m_pThis->m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
	}
}

void SGEHandler::OnQuoteFrontConnected(void* ctx) {
	Log::Info("SGEHandler: OnQuoteFrontConnected");
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::SGE;
	notification.Type = ExchangeNotification::QUOTE_CONNECTED;
	m_pThis->m_publisher.NotifyUI(notification);
	m_pThis->Handshake(false);
}
void SGEHandler::OnQuoteFrontDisconnected(int error_no, void* ctx) {
	std::string reportMsg = fmt::format("SGEHandler: OnQuoteFrontDisconnected, errorNo[{}]", error_no);
	m_pThis->m_publisher.NotifyUI(reportMsg);
	Log::Error(reportMsg.c_str());

	ExchangeNotification notification;
	notification.Platform = ConnPlatform::SGE;
	notification.Type = ExchangeNotification::QUOTE_DISCONNECTED;
	m_pThis->m_publisher.NotifyUI(notification);

	if (m_pThis->m_pQutoeContext->SafeSessionPtr) {
		sge_safe_free_client_session(m_pThis->m_pQutoeContext->SafeSessionPtr);
		m_pThis->m_pQutoeContext->SafeSessionPtr = nullptr;
	}
}
void SGEHandler::OnQuoteReceiveMsg(int type, const char* sge_msg, unsigned int msgLen, void* ctx) {
	int praseBufLens = msgLen + 1;	//留一位给'/0'
	char receivedMsg[SGE_MSG_LEN * 2] = "";
	if (MSG_TYPE_PLAIN == type) {
		memcpy(receivedMsg, sge_msg, msgLen);
		std::string_view gtpMsg(receivedMsg, msgLen);
		m_pThis->DispatchPlainMsg(gtpMsg, false);
// 		Log::Info(fmt::format("SGEHandler: MSG_TYPE_PLAIN Msg[{}]", gtpMsg));
	}	// 明文消息派发
	else if (MSG_TYPE_CIPHER == type) {
		if (m_pThis->m_safeHandler.DecryptMsg(sge_msg, msgLen, receivedMsg, praseBufLens, m_pThis->m_pQutoeContext->SessionKey)) {
			std::string_view praseMsg(receivedMsg, msgLen);
			m_pThis->DispatchCipherMsg(praseMsg, false);
		}
	}	// 密文消息派发
	else {
		std::string reportMsg = fmt::format("SGEHandler: quoteHandler received unknown msgType[{}]", type);
		m_pThis->m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
	}
}

SGEHandler* SGEHandler::m_pThis = nullptr;
SGEHandler::SGEHandler(ConnConfig& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher, SafeHandler& safeHandler)
	:m_config(config), m_instHelper(instHelper), m_publisher(publisher), m_safeHandler(safeHandler), m_requestNo(0), m_privateSeqNo(0) {

	m_pThis = this;

	// 两个前置需要有不同的交易员登录，可能会是不同的席位号
	auto panelVec = split(m_config.Credential.BrokerID, ":");
	if (panelVec.size() < 2) {
		std::string reportMsg = fmt::format("SGEConnection: Do not have enough brokerID[{}], check config", panelVec.size());
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg);
		return;
	}

	m_seatInfo.SeatID = panelVec[0].c_str();
	m_exSeatInfo.SeatID = panelVec[1].c_str();
	m_seatInfo.ClientID = m_config.Credential.InvestorID;
	m_exSeatInfo.ClientID = m_config.Credential.InvestorID;
	Log::Info(fmt::format("SGEConnection: LoadConfig, brokerID[{}]:[{}]", m_seatInfo.SeatID.c_str(), m_exSeatInfo.SeatID.c_str()));

	GetLocalIp();
}
SGEHandler::~SGEHandler() {
}

void SGEHandler::Login(bool isTrade) {
	bool isLogined = isTrade ? m_pTradeContext->IsLogined : m_pQutoeContext->IsLogined;
	if (isLogined) {
		Log::Error(fmt::format("SGEHandler: Trade[{}] is already Login", isTrade));
		return;
	}
	std::string seatID = isTrade ? m_seatInfo.SeatID : m_exSeatInfo.SeatID;
	std::string traderID = isTrade ? m_pTradeContext->SGESessionInfo.trader_id_ : m_pQutoeContext->SGESessionInfo.trader_id_;
	char randomCode[100] = "";
	char encryptedPwd[100] = "";
	if (encryptKeyDomain(traderID, encryptedPwd, randomCode)) {
		SGEFormatter sgeSerial(m_seatInfo);
		auto result = sgeSerial.ToLoginRequest(seatID.c_str(), traderID.c_str(), encryptedPwd, randomCode, m_localIp.c_str(), sge_get_version());
// 		Log::Info(fmt::format("SGELogin: [{}]", result.Message));

		SGEContext* theContext = isTrade ? m_pTradeContext : m_pQutoeContext;
		m_encryptedMsgLen = result.Size * 2;
		if (!m_safeHandler.EncryptMsg("Login", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, theContext->SessionKey)) {
			Log::Error(fmt::format("SGEHandler: encrypt LoginMsg failed, isTrade[{}]", isTrade));
			return;
		}

		sge_session_t* session = theContext->SGESessionPtr;
		int nRtn = sge_session_write(session, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
		if (0 != nRtn) {
			Log::Error(fmt::format("SGEHandler: send Login failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade));
			return;
		}
	}
}
void SGEHandler::SubscribeQuote(char const* instrumentCode) {
	InstID instID = m_instHelper.GetInstID(instrumentCode);
	m_subscribeList.insert(instID);
}
void SGEHandler::QueryFund() {
	SGEFormatter sgeSerial(m_seatInfo);
	auto const& result = sgeSerial.ToQueryFund();
// 	Log::Info(fmt::format("SGEQryFund: [{}]", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("QueryFund", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		Log::Error("SGEHandler: encrypt QueryFund failed");
		return;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		Log::Error(fmt::format("SGEHandler: send QueryFund failed, rtnCode[{}]", nRtn));
		return;
	}
}
void SGEHandler::QueryStorage() {
	SGEFormatter sgeSerial(m_seatInfo);
	auto const& result = sgeSerial.ToQueryStorage();
// 	Log::Info(fmt::format("SGEQryStorage: [{}]", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("QueryStorage", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		Log::Error("SGEHandler: encrypt QueryStorage failed");
		return;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		Log::Error(fmt::format("SGEHandler: send QueryStorage failed, rtnCode[{}]", nRtn));
		return;
	}
}
void SGEHandler::QueryPosition() {
	SGEFormatter sgeSerial(m_seatInfo);
	auto const& result = sgeSerial.ToQueryPosition();
// 	Log::Info(fmt::format("SGEQryPosition: [{}]", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("QueryPosition", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		Log::Error("SGEHandler: encrypt QueryPosition failed");
		return;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		Log::Error(fmt::format("SGEHandler: send QueryPosition failed, rtnCode[{}]", nRtn));
		return;
	}
}

void SGEHandler::ReqOrderInsert(OrderInfo const& orderInfo, bool isTrade) {
	if (SGE_OC_Unknown == orderInfo.ProductClass) {
		std::string reportMsg = fmt::format("SGEHandler: ReqOrderInsert get unknown productClass[{}]", orderInfo.ProductClass);
		m_publisher.NotifyUI(reportMsg);
		Log::Info(reportMsg.c_str());
		return;
	}

	CSeatInfo const& theSeatInfo = isTrade ? m_seatInfo : m_exSeatInfo;
	unsigned char const* theSessionKey = isTrade ? m_pTradeContext->SessionKey : m_pQutoeContext->SessionKey;
	sge_session_t* theSessionPtr = isTrade ? m_pTradeContext->SGESessionPtr : m_pQutoeContext->SGESessionPtr;

	FormatResult result;
	SGEFormatter formatter(theSeatInfo);
	result = formatter.ToInsertRequest(orderInfo);
// 	Log::Info(fmt::format("SGEHandler: OrderInsert [{}]", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("OrderInsert", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, theSessionKey)) {
		Log::Error("SGEHandler: encrypt OrderInsert failed");
		return;
	}

	int nRtn = sge_session_write(theSessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		Log::Error(fmt::format("SGEHandler: send OrderInsert failed, rtnCode[{}]", nRtn));
		return;
	}
}
void SGEHandler::ReqOrderAction(OrderCancel const& actionInfo) {
	if (SGE_OC_Unknown == actionInfo.GetProductClass()) {
		std::string reportMsg = fmt::format("SGEHandler: ReqOrderAction get unknown productClass[{}]", actionInfo.GetProductClass());
		m_publisher.NotifyUI(reportMsg);
		Log::Info(reportMsg);
		return;
	}

	FormatResult result;
	SGEFormatter formatter(m_seatInfo);
	result = formatter.ToCancelRequest(actionInfo);
// 	Log::Info(fmt::format("SGEHandler: OrderAction [{}]", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("OrderAction", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		Log::Error("SGEHandler: encrypt OrderAction failed");
		return;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		Log::Error(fmt::format("SGEHandler: send OrderAction failed, rtnCode[{}]", nRtn));
		return;
	}
}

void SGEHandler::OnRspError(CSGERspInfoField* pRspInfo) {
	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("SGEHandler: OnRspError, RspCode[{}] RspMsg[{}] isTrade[{}]", pRspInfo->RspCode, pRspInfo->RspMsg, pRspInfo->IsTrade);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
}
void SGEHandler::OnRspLogin(CSGERspInfoField* pRspInfo, CSGERspLoginField* pRspLogin) {
	if (IsErrorRspInfo(pRspInfo)) {
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::SGE;
		notification.Type = pRspInfo->IsTrade ? ExchangeNotification::TRADER_DISCONNECTED : ExchangeNotification::QUOTE_DISCONNECTED;
		m_publisher.NotifyUI(notification);

		auto msg = fmt::format("SGEHandler: Login failed, RspCode[{}] RspMsg[{}] isTrade[{}]", pRspInfo->RspCode, pRspInfo->RspMsg, pRspInfo->IsTrade);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		return;
	}
	else {
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::SGE;
		notification.Type = pRspInfo->IsTrade ? ExchangeNotification::TRADER_LOGINED : ExchangeNotification::QUOTE_LOGINED;
		m_publisher.NotifyUI(notification);
		pRspInfo->IsTrade ? m_pTradeContext->IsLogined = true : m_pQutoeContext->IsLogined = true;

		if (pRspInfo->IsTrade) {
			SYSTEMTIME nowTime;
			::GetLocalTime(&nowTime);
			int random = (nowTime.wSecond * 10 + nowTime.wMilliseconds) * 10;

			uint32_t maxOrderLocalID = atoi(pRspLogin->MaxOrderLocalID);
			uint32_t baseLocalID = (maxOrderLocalID > SGE_LOCALID) ? maxOrderLocalID : SGE_LOCALID + random/*防止双开程序的本地委托号相同*/;
			LocalIdACC::Init(baseLocalID);
			m_pTradeContext->LastLocalID = maxOrderLocalID;	// 交易前置最后本地委托号	

			m_publisher.PushResponse(CleanPositionNotification());
			m_seatInfo.TraderID = m_pTradeContext->SGESessionInfo.trader_id_;	// 指定委托交易员
			QueryStorage();		// 查客户库存
			QueryPosition();	// 查客户持仓
		}
		else {
			m_exSeatInfo.TraderID = m_pQutoeContext->SGESessionInfo.trader_id_;	// 指定额外委托交易员（仅用于抢单）
		}
		auto msg = fmt::format("SGEHandler: OnRspLogin[{}, {}] Success, LastLocalID[{}] CurrentLocalID[{}] isTrade[{}]"
			,pRspLogin->SeatID, pRspLogin->TraderID, pRspLogin->MaxOrderLocalID, LocalIdACC::GetCurrentID(), pRspInfo->IsTrade);
		m_publisher.NotifyUI(msg);
		Log::Info(msg);

		return;
	}
}
void SGEHandler::OnRspOrderInsert(CSGEInputOrderField* pInputOrder, CSGERspInfoField* pRspInfo) {
	OrderNotification notification = { 0 };
	notification.InstID = m_instHelper.GetInstID(pInputOrder->InstrumentID);
	notification.Direction = ConvertDirectionFromSGE(pInputOrder->Direction);
	notification.Operation = ConvertOperationFromSGE(pInputOrder->OffsetFlag);
	notification.LocalId = atoi(pInputOrder->OrderLocalID);
	notification.OrderPrice = pInputOrder->OrderPrice;
	notification.OrderQuantity = pInputOrder->OrderQuantity;
	notification.Status = OrderNotification::E_SENT_FAILED;
	m_publisher.PushResponse(notification);

	OrderType orderType = ConvertOrderTypeFromSGE(pInputOrder->OrderType);
	char const* direction = (Direction::SELL == notification.Direction) ? "S" : "B";
	auto msg = fmt::format("SGE OrderInsert Failed: ErrorCode[{}] ErrorMsg[{}] ClientID[{}] Inst[{}] Dir[{}] Opt[{}] Price[{:.3f}] Qty[{}] LocalID[{}] PriceType[{}] IsTrade[{}] SeqNo[{}] ReqNo[{}]"
		, pRspInfo->RspCode, pRspInfo->RspMsg, pInputOrder->ClientID, pInputOrder->InstrumentID, Desc(notification.Direction), Desc(notification.Operation)
		, pInputOrder->OrderPrice, notification.OrderQuantity, pInputOrder->OrderLocalID, orderType, pRspInfo->IsTrade, m_privateSeqNo, m_requestNo);
	if (14006!=pRspInfo->RspCode) {
		m_publisher.NotifyUI(msg);
	}
	Log::Error(msg);
}
void SGEHandler::OnRspOrderAction(CSGEOrderActionField* pInputOrderAction, CSGERspInfoField* pRspInfo) {	
	OrderNotification notification = { 0 };
	std::map<std::string, int>::const_iterator iterator = m_systemIDMap.find(pInputOrderAction->OrderSysID);
	if (iterator != m_systemIDMap.end()) {
		notification.LocalId = m_systemIDMap[pInputOrderAction->OrderSysID];
	}
	else {
		std::string errMsg = fmt::format("SGEHandler: OrderAction failed, Can't find localId by SystemId[{}]", pInputOrderAction->OrderSysID);
		Log::Error(errMsg);
		m_publisher.NotifyUI(errMsg);
		return;
	}

	if (15002 != pRspInfo->RspCode /*已撤*/ && 15008 != pRspInfo->RspCode /*已成*/ && 20006 != pRspInfo->RspCode/*撮合机succ_fail flag值错误*/) {
		notification.InstID = m_instHelper.GetInstID(pInputOrderAction->InstrumentID);
		strcpy_s(notification.SystemNo, pInputOrderAction->OrderSysID);
		notification.Status = OrderNotification::E_CANCEL_FAILED;
		m_publisher.PushResponse(notification);

		std::string errMsg = fmt::format("SGEHandler: OrderAction failed, ErrorCode[{}] ErrorMsg[{}] ClientID[{}] Instrument[{}] IsTrade[{}] SeqNo[{}] ReqNo[{}]"
			, pRspInfo->RspCode, pRspInfo->RspMsg, pInputOrderAction->ClientID, pInputOrderAction->InstrumentID, pRspInfo->IsTrade, m_privateSeqNo, m_requestNo);
		m_publisher.NotifyUI(errMsg);
		Log::Error(errMsg);
	}
}

void SGEHandler::OnRspQrySeatFund(CSGERspAccountField* pRspFund, CSGERspInfoField* pRspInfo) {	
	FundsNotification notification;
	memset(&notification, 0, sizeof(FundsNotification));
	notification.Platform = ConnPlatform::SGE;
	notification.Capital = pRspFund->Available / 100;
	notification.Available = pRspFund->Available / 100;
	m_publisher.PushResponse(notification);
	Log::Info(fmt::format("SGEHandler: OnRspQrySeatFund, SeatID[{}] Avaliable[{:.3f}]", pRspFund->SeatID, pRspFund->Available));

}
void SGEHandler::OnRspQryClientPosition(CSGERspClientPositionField* pRspPosition, CSGERspInfoField* pRspInfo, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo) && 16005 != pRspInfo->RspCode /*没有持仓*/) {
		auto msg = fmt::format("SGEHandler: QueryClientPosition failed, RspCode[{}] RspMsg[{}]", pRspInfo->RspCode, pRspInfo->RspMsg);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
	else {
		PositionNotification notification;
		memset(&notification, 0, sizeof(PositionNotification));

		notification.InstID = m_instHelper.GetInstID(pRspPosition->InstrumentID);
		if (notification.InstID == INVALID_INSTRUMENT_ID) {
			Log::Info(fmt::format("SGEHandler: QueryPosition filter not support Instrument[{}]", pRspPosition->InstrumentID));
			return;
		}
		notification.HedgeType = HedgeType::SPECULATE;
		if (0 != pRspPosition->LongPosition || 0 != pRspPosition->ShortPosition) {
			FillingPositionNotification(*pRspPosition, notification);
		}

		if (bIsLast) {
			Log::Info("SGEHandler: QueryPosition Success");
		}
	}
}
void SGEHandler::OnRspQryClientStorage(CSGERspClientStorageField* pRspStorage, CSGERspInfoField* pRspInfo, bool bIsLast) {
	if (IsErrorRspInfo(pRspInfo)) {
		auto msg = fmt::format("SGEHandler: OnRspQryClientStorage failed, RspCode[{}] RspMsg[{}]", pRspInfo->RspCode, pRspInfo->RspMsg);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
	}
	else {
		TSGEInstrumentIDType instrument = "";
		strcpy_s(instrument, ConvertSpotVarietyFromSGE(pRspStorage->VarietyID));
		if (0 != strlen(instrument)) {
			PositionNotification notification;
			memset(&notification, 0, sizeof(PositionNotification));
			notification.InstID = m_instHelper.GetInstID(instrument);
			if (notification.InstID == INVALID_INSTRUMENT_ID) {
				Log::Info(fmt::format("SGEHandler: QueryStorage filter not support Instrument[{}]", instrument));
				return;
			}

			if (0 != pRspStorage->TotalStorage) {
				notification.Direction = Direction::BUY;	// 库存都是买方向
				notification.HedgeType = HedgeType::SPECULATE;
				// 单位统一转换成手
				switch (pRspStorage->VarietyID) {
				case 201:	// Au99.99 报价10克/手，最大报单50000手
					notification.Quantity = int(pRspStorage->TotalStorage * 1000 / 10);					
					notification.Frozen = int(pRspStorage->Frozen * 1000 / 10);
					break;
				case 207:	// Au100g 报价100克/手，最大报单1000手
					notification.Quantity = int(pRspStorage->TotalStorage * 1000 / 100);					
					notification.Frozen = int(pRspStorage->Frozen * 1000 / 10);
					break;
				}
				notification.TodayQty = 0;

				m_publisher.PushResponse(notification);
				m_publisher.NotifyUI(notification);
				Log::Info(fmt::format("SGEHandler: QryStorageRsp, ClientiID[{}] Inst[{}] Total[{}] Frozen[{}]"
					, pRspStorage->ClientID, instrument, notification.Quantity, notification.Frozen));
			}
		}

		if (bIsLast) {
			Log::Info("SGEHandler: QueryStorage Success");
		}
	}
}

void SGEHandler::OnRtnOrder(CSGEOrderField* pOrder, bool isTrade) {
	if (m_seatInfo.ClientID.compare(pOrder->ClientID)) {
		return;
	}
	OrderNotification notification = { 0 };
	notification.InstID = m_instHelper.GetInstID(pOrder->InstrumentID);
	if (INVALID_INSTRUMENT_ID == notification.InstID) {
		return;
	}
	notification.LocalId = atoi(pOrder->OrderLocalID);
	if (m_pTradeContext->LastLocalID >= SGE_LOCALID && notification.LocalId < m_pTradeContext->LastLocalID - 200/*10的整数倍*/) {
// 		Log::Info(fmt::format("OnRtnOrder: LocalID[{}] < MaxLocalID[{}], filter hisOrder", notification.LocalId, m_pTradeContext->LastLocalID));
		return;
	}	// 如果重启，只处理最后的20笔委托（防止有挂单）

	notification.Exchange = InstIdExchange(notification.InstID);
	notification.Direction = ConvertDirectionFromSGE(pOrder->Direction);
	notification.Operation = ConvertOperationFromSGE(pOrder->OffsetFlag);
	notification.OrderPrice = pOrder->OrderPrice;
	notification.OrderQuantity = pOrder->OrderQuantity;
	notification.DealQuantity = pOrder->OrderQuantity - pOrder->RemainQuantity;	// 成交总量
	notification.DealPrice = pOrder->OrderPrice;
	strcpy_s(notification.SystemNo, pOrder->OrderSysID);

	if (SGE_IS_NoTrade == pOrder->OrderStatus || SGE_IS_Queueing == pOrder->OrderStatus) {
		notification.Status = OrderNotification::E_ORDERED;
	}	// 订单状态改变：挂单
	else if (SGE_IS_PartTraded/*Partial*/ == pOrder->OrderStatus || SGE_IS_AllTraded/*Traded*/ == pOrder->OrderStatus) {
		notification.Status = OrderNotification::E_ORDERED;
	}	// 成交状态改变：部成|全成	PS:SGE的成交回报先回来，不根据委托回报更新成交
	else if (SGE_IS_Canceled == pOrder->OrderStatus || SGE_IS_AutoCanceled == pOrder->OrderStatus || SGE_IS_PartFinish == pOrder->OrderStatus) {
		notification.Status = OrderNotification::E_CANCELED;
	}
	else if (SGE_IS_SysCanceled == pOrder->OrderStatus || SGE_IS_Abandon == pOrder->OrderStatus) {	//系统撤单；废单 
		notification.Status = OrderNotification::E_SENT_FAILED;
		auto msg = fmt::format("SGEHandler OnRtnOrder: Cancelled by System! ClientID[{}] SystemID[{}] LocalID[{}] Inst[{}] Status[{}]",
			pOrder->ClientID, pOrder->OrderSysID, pOrder->OrderLocalID, pOrder->InstrumentID, Desc(pOrder->OrderStatus));
		m_publisher.NotifyUI(msg);
	}	// 无效单和系统撤单
	else {
		notification.Status = OrderNotification::E_CANCELED;
		auto msg = fmt::format("SGEHandler OnRtnOrder: Cancelled by Urgency! ClientID[{}] SystemID[{}] LocalID[{}] Inst[{}] Status[{}]"
			, pOrder->ClientID, pOrder->OrderSysID, pOrder->OrderLocalID, pOrder->InstrumentID, Desc(pOrder->OrderStatus));
		m_publisher.NotifyUI(msg);
	}	// 柜台应急撤单	SGE_USTP_IS_UrgencyCanceled = 'u'
	m_publisher.PushResponse(notification);

	if (SGE_IS_Canceled == pOrder->OrderStatus || SGE_IS_NoTrade == pOrder->OrderStatus) {
		OrderType orderType = ConvertOrderTypeFromSGE(pOrder->OrderType);
		Log::Info(fmt::format("!SGEHandler OnRtnOrder: ClientID[{}] SysID[{}] LocalID[{}] Inst[{}] [{} {} {:.3f} {}] Status[{}] PriceType[{}] IsTrade[{}] SeqNo[{}] ReqNo[{}]"
			, pOrder->ClientID, notification.SystemNo, notification.LocalId, pOrder->InstrumentID, Desc(notification.Direction), Desc(notification.Operation)
			, notification.OrderPrice, notification.OrderQuantity, Desc(pOrder->OrderStatus), orderType, isTrade, m_privateSeqNo, m_requestNo));
	}	// 主动撤单记一下日志(最终状态不影响性能)
}
void SGEHandler::OnRtnTrade(CSGETradeField* pTrade, bool isTrade) {
	if (m_seatInfo.ClientID.compare(pTrade->ClientID)) {
		return;
	}
	OrderNotification notification = { 0 };
	notification.InstID = m_instHelper.GetInstID(pTrade->InstrumentID);
	if (INVALID_INSTRUMENT_ID == notification.InstID) {
		return;
	}

	notification.LocalId = atoi(pTrade->OrderLocalID);
	if (notification.LocalId < m_pTradeContext->LastLocalID) {
// 		Log::Info(fmt::format("OnRtnOrder: LocalID[{}] < MaxLocalID[{}], filter hisTrade", notification.LocalId, m_pTradeContext->MaxLocalID));
		return;
	}	// SGE重启后不处理之前的成交

	notification.Exchange = InstIdExchange(notification.InstID);
	notification.Direction = ConvertDirectionFromSGE(pTrade->Direction);
	notification.Operation = ConvertOperationFromSGE(pTrade->OffsetFlag);
	notification.DealQuantity = pTrade->TradeVolume;	// OnRtnTrade里的成交量是增量
	notification.DealPrice = pTrade->TradePrice;
	strcpy_s(notification.SystemNo, pTrade->OrderSysID);
	notification.Status = OrderNotification::E_TRADE;	// 只能给E_TRADE状态；没有OrderQuantity
	m_publisher.PushResponse(notification);

	// 成交记一下日志(最终状态不影响性能)
	Log::Info(fmt::format("!!SGEHandler OnRtnTrade: ClientID[{}] SysID[{}] LocalID[{}] Inst[{}] [{} {} {:.3f} {}] IsTrade[{}] SeqNo[{}] ReqNo[{}]", pTrade->ClientID, pTrade->OrderSysID
		, pTrade->OrderLocalID, pTrade->InstrumentID, Desc(notification.Direction), Desc(notification.Operation), pTrade->TradePrice, pTrade->TradeVolume, isTrade, m_privateSeqNo, m_requestNo));
}

void SGEHandler::OnRtnInstrumentStatus(TSGEInstrumentStatusType instStatus, bool isTrade) {
	if (instStatus != m_currentStatus) {	// 这里保证了只记录先来的前置状态
		m_currentStatus = instStatus;
		ExchangeNotification notification;
		notification.Platform = ConnPlatform::SGE;
		notification.Exchange = Exchange::SGE;
		notification.Type = ExchangeNotification::EXCHANGE_STATUS;
		notification.Status = ConvertExchangeStatusFromSGE(instStatus);
		m_publisher.NotifyUI(notification);
		m_publisher.PushResponse(notification);
		Log::Info(fmt::format("Exchange[{}] Status[{}] IsTrade[{}]", Desc(notification.Exchange), Desc(notification.Status), isTrade));
	}
}
void SGEHandler::OnRtnDepthMarketData(CSGEDepthMarketDataField* pMarketDataField) {
	if (nullptr == pMarketDataField)
		return;

	if (IsInvalidQuote(pMarketDataField))
		return;

	InstID instID = m_instHelper.GetInstID(pMarketDataField->InstrumentID);
	if (m_subscribeList.end() != m_subscribeList.find(instID)) {
		Quote quote(CreateQuote(instID, pMarketDataField));
		m_publisher.PushQuote(quote);
// 		Log::Info(fmt::format("SGEHandler Quote: Inst[{}] Open[{:.3f}] Last[{:.3f}] Interest[{}] Ask[{:.2f},{}] Bid[{:.2f},{}] Ask1[{:.2f},{}] Bid1[{:.2f},{}]"
// 			,pMarketDataField->InstrumentID, quote.GetOpenPrice(), quote.GetLastPrice(), quote.GetOpenInterest()
// 			,quote.GetSell().GetPrice(), quote.GetSell().GetQuantity(), quote.GetBuy().GetPrice(), quote.GetBuy().GetQuantity()
// 			,quote.GetSell(1).GetPrice(), quote.GetSell(1).GetQuantity(), quote.GetBuy(1).GetPrice(), quote.GetBuy(1).GetQuantity()));
	}
}

void SGEHandler::GetLocalIp() {
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (0 != WSAStartup(wVersionRequested, &wsaData)) {
		Log::Error("SGEHandler: get local IP failed");
	}

	char local[255] = { 0 };
	gethostname(local, sizeof(local));
	hostent* ph = gethostbyname(local);
	in_addr addr;
	memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr)); // 这里仅获取第一个ip
	m_localIp.assign(inet_ntoa(addr));
	WSACleanup();
}	 
bool SGEHandler::Handshake(bool isTrade) {
	std::string reportMsg = "SGEHandler: ";

	// 创建安全会话
	SGEContext* theContext = isTrade ? m_pTradeContext : m_pQutoeContext;
	theContext->SafeSessionPtr = sge_safe_create_client_session(m_pTradeContext->SafeApiPtr, &theContext->SGESessionInfo);
	if (nullptr == theContext->SafeSessionPtr) {
		reportMsg += fmt::format("Create SafeSession failed, isTrade[{}]", isTrade);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}

	fld_client_hello_t helloInfo;
	strcpy_s(helloInfo.trader_id_, theContext->SGESessionInfo.trader_id_);
	strcpy_s(helloInfo.sge_id_, theContext->SGESessionInfo.sge_id_);
	helloInfo.enc_mode_ = 0;
	strcpy_s(helloInfo.safe_api_trader_version_, sge_safe_client_get_api_version());

	// 创建握手信息并发送
	api_safe_client_session_t* pTheSafeSessionPtr = isTrade ? m_pTradeContext->SafeSessionPtr : m_pQutoeContext->SafeSessionPtr;
	int nRtn = sge_safe_client_create_hello(pTheSafeSessionPtr, &helloInfo);	// 创建安全信息
	if (0 == nRtn) {
		std::stringstream ssHello("");
		int requestId = RequestIdACC::Generate();
		auto const& helloBody = fmt::format("M60={},G04={},X83=0,X56={}", helloInfo.trader_id_, helloInfo.sge_id_, helloInfo.safe_api_trader_version_);
		ssHello << fmt::format("X01=GTP1.0,X03=AA00,X04=1,X05={},X06=L,X07={},", requestId, requestId) << helloBody;
// 		Log::Info(fmt::format("SGEHandshake: [{}], isTrade[{}]", ssHello.str().c_str(), isTrade));

		// 不同的前置要对应不同的Session握手
		sge_session_t* theSessionPtr = isTrade ? m_pTradeContext->SGESessionPtr : m_pQutoeContext->SGESessionPtr;
		nRtn = sge_session_write(theSessionPtr, MSG_TYPE_PLAIN, ssHello.str().c_str(), (unsigned int)ssHello.str().size());
		if (0 != nRtn) {
			reportMsg += fmt::format("Send helloInfo failed, rtnCode[{}]", nRtn);
			m_publisher.NotifyUI(reportMsg);
			Log::Error(reportMsg.c_str());
			return false;
		}
	}
	else {
		reportMsg += fmt::format("create helloInfo failed, rtnCode[{}]", nRtn);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}
	return true;
}

namespace {
	void assign(char* target, std::string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), std::min(str.length(), sizeof(tmp) - 1));
		strcpy_s(target, str.length() + 1, tmp);
	}
	void assign(char& target, std::string_view const& str) {
		target = str[0];
	}
	void assign(int& target, std::string_view const& str) {
		target = atoi(str.data());
	}
	void assign(long long& target, std::string_view const& str) {
		target = _atoi64(str.data());
	}
	void assign(double& target, std::string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), std::min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}
}

template<class Target>
bool copy_if_key(char const* expectedKey, std::string_view const& key, Target& target, Parser& parser) {
	if (expectedKey == key) {
		assign(target, parser.GetStringValue());
		return true;
	}
	return false;
}
inline bool copyRspCode(char const* expectedKey, std::string_view const& key, TSGERspCodeType& rspCode, Parser& parser) {
	if (expectedKey == key) {
		rspCode = parser.GetIntValue(3);
		return true;
	}
	return false;
}
#define COPY_RSPCODE(ekey, target) if (copyRspCode(ekey, key, target, m_parser)) continue;
#define COPY_CONTINUE(ekey, target) if(copy_if_key(ekey, key, target, m_parser)) continue;

void SGEHandler::DispatchPlainMsg(std::string_view const& plainMsg, bool isTrade) {
	m_parser.Init(plainMsg);
	if (false == isTrade/*行情前置*/ && MARKET_STREAM == plainMsg[15]) {
		ParseMarketMessage(plainMsg);
	}
	else {
		PraseSafeMessage(isTrade);
	}
}
void SGEHandler::DispatchCipherMsg(std::string_view const praseMsg, bool isTrade) {	
	m_parser.Init(praseMsg);
	switch (praseMsg[15]) {
	case TRADE_STREAM:		// 交易流:应答消息(委托应答，撤单应答，内含通用报错X39，X40字段)	
		ParseTradeMessage(isTrade);
		break;
	case PRIVATE_STREAM:	// 私有流(只有OnRtnOrder，OnRtnTrade);（交割申报、中立仓）的回报
		ParsePrivateMessage(isTrade);
		break;
	case PUBLIC_STREAM:		// 公共流(只处理合约状态变化)
		ParsePublicMessage(isTrade);
		break;
	case QUERY_STREAM:		// 查询流(从未出现-.-!!)
		Log::Info(fmt::format("SGEHandler isTrade[{}]: QUERY_STREAM praseMsg[{}] ", isTrade, praseMsg));
// 		ParseQueryMessage();
		break;
	default:				//其他情况当作错误处理
		ParseErrorMessage(isTrade);
		break;
	}
}

// 安全认证部分
void SGEHandler::PraseSafeMessage(bool isTrade) {
	CSGERspInfoField rspInfo;
	rspInfo.IsTrade = isTrade;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "M991") {		// 通用错误
				parseRspError(pRspInfo);
			}
			else if (value == "AA01") {	//握手应答
				fld_server_hello_t helloRtn = { 0 };
				if (PraseHandshakeMsg(pRspInfo, helloRtn)) {
					doRspHandshake(helloRtn, isTrade);
				}
			}
			else if (value == "AA11") {	//验证应答
				fld_sessionkey_info_t keyRtn = { 0 };
				if (PraseCertificationMsg(pRspInfo, keyRtn)) {
					doRspCertification(keyRtn, isTrade);
				}
			}
		}
	}
}
bool SGEHandler::PraseHandshakeMsg(CSGERspInfoField* pRspInfo, fld_server_hello_t& svrHello) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		COPY_CONTINUE("M60", svrHello.trader_id_);
		COPY_CONTINUE("G04", svrHello.sge_id_);
		COPY_CONTINUE("G05", svrHello.sge_random_);
		COPY_CONTINUE("X83", svrHello.enc_mode_);
	}

	if (0 != pRspInfo->RspCode) {
		OnRspError(pRspInfo);
		return false;
	}
	return true;
}
void SGEHandler::doRspHandshake(fld_server_hello_t const& svrHello, bool isTrade) {
	api_safe_client_link_t* theSafeLinkPtr = isTrade ? m_pTradeContext->SafeLinkPtr : m_pQutoeContext->SafeLinkPtr;
	api_safe_client_session_t* theSafeSessionPtr = isTrade ? m_pTradeContext->SafeSessionPtr : m_pQutoeContext->SafeSessionPtr;

	//处理Hello应答
	int nRtn = sge_safe_client_handle_hello_rsp(theSafeSessionPtr, theSafeLinkPtr, &svrHello);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: SafeHello rsp failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return;
	}

	//生成认证信息并发送
	fld_authentication_info_t auth_info = { 0 };
	nRtn = sge_safe_client_create_authentication(theSafeSessionPtr, theSafeLinkPtr, &auth_info);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: Create authentication failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return;
	}

	std::stringstream ssAuthenticate("");
	int requestId = RequestIdACC::Generate();
	auto const& authenticateBody = fmt::format("M60={},G04={},M63={},G05={},M64={}",
		auth_info.trader_id_, auth_info.sge_id_, auth_info.trader_random_, auth_info.sge_random_, auth_info.authen_signed_data_);
	ssAuthenticate << fmt::format("X01=GTP1.0,X03=AA10,X04=1,X05={},X06=L,X07={},", requestId, requestId) << authenticateBody;
// 	Log::Debug(fmt::format("SGEHandler: Authenticate [{}] isTrade[{}]", ssAuthenticate.str().c_str(), isTrade));

	sge_session_t* theSessionPtr = isTrade ? m_pTradeContext->SGESessionPtr : m_pQutoeContext->SGESessionPtr;
	nRtn = sge_session_write(theSessionPtr, MSG_TYPE_PLAIN, ssAuthenticate.str().c_str(), (unsigned int)ssAuthenticate.str().size());
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: Send authenticate request failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return;
	}
}
bool SGEHandler::PraseCertificationMsg(CSGERspInfoField* pRspInfo, fld_sessionkey_info_t& sessionKey) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		COPY_CONTINUE("M60", sessionKey.trader_id_);
		COPY_CONTINUE("G04", sessionKey.sge_id_);
		COPY_CONTINUE("X84", sessionKey.cipher_key_);
		COPY_CONTINUE("X86", sessionKey.enc_random_);
		COPY_CONTINUE("X88", sessionKey.skey_signed_data_);
	}

	if (0 != pRspInfo->RspCode) {
		OnRspError(pRspInfo);
		return false;
	}
	return true;
}
void SGEHandler::doRspCertification(fld_sessionkey_info_t const& sessionKey, bool isTrade) {
	api_safe_client_link_t* theSafeLinkPtr = isTrade ? m_pTradeContext->SafeLinkPtr : m_pQutoeContext->SafeLinkPtr;
	api_safe_client_session_t* theSafeSessionPtr = isTrade ? m_pTradeContext->SafeSessionPtr : m_pQutoeContext->SafeSessionPtr;

	//验证SGE签名
	int nRtn = sge_safe_client_accept_sessionkey(theSafeSessionPtr, theSafeLinkPtr, &sessionKey);
	if (0 != nRtn) {
		std::string reportMsg = fmt::format("SGEHandler: DoCertification failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return;
	}
// 	Log::Info(fmt::format("SGEHandler: DoCertification success! sessionKey[{}] isTrade[{}]", m_pThis->m_safeHandler.GetSessionKey(), isTrade));

	if (isTrade) {
		memcpy(m_pThis->m_pTradeContext->SessionKey, m_pThis->m_safeHandler.GetSessionKey(), 33);
		m_pThis->m_pTradeContext->IsHandshaked = true;
	}
	else {
		memcpy(m_pThis->m_pQutoeContext->SessionKey, m_pThis->m_safeHandler.GetSessionKey(), 33);
		m_pThis->m_pQutoeContext->IsHandshaked = true;
	}
}

// 通用错误
void SGEHandler::ParseErrorMessage(bool isTrade) {
	CSGERspInfoField rspInfo;
	rspInfo.IsTrade = isTrade;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("X03" == m_parser.GetKey() && "M991" == m_parser.GetStringValue()) {	// M991通用非业务报错
			parseRspError(pRspInfo);
			OnRspError(pRspInfo);
		}
	}
}
void SGEHandler::ParseTradeMessage(bool isTrade) {
	CSGERspInfoField rspInfo;
	rspInfo.IsTrade = isTrade;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();

		if (key == "X07") {
			m_requestNo = m_parser.GetIntValue();
		}
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "A101") {		// 登录应答
				parseRspLogin(pRspInfo);
			}
			else if (value == "T091") {	// 交割申报应答
				parseRspOrderInsert(pRspInfo, SGE_OC_Delivery);
			}
			else if (value == "T121") {	// 中立仓应答 
				parseRspOrderInsert(pRspInfo, SGE_OC_Neutral);
			}
			else if (value == "T201") {	// 委托应答
				parseRspOrderInsert(pRspInfo, SGE_OC_Unknown);
			}
			else if (value == "T101") { //交割撤单
				parseRspOrderAction(pRspInfo, SGE_OC_Delivery);
			}
			else if (value == "T131") {	// 中立仓撤单
				parseRspOrderAction(pRspInfo, SGE_OC_Neutral);
			}
			else if (value == "T211") {	// 委托撤单
				parseRspOrderAction(pRspInfo, SGE_OC_Unknown);
			}
			else if (value == "T451") {	// 席位资金查询
				parseRspQrySeatFund(pRspInfo);
			}
			else if (value == "T461") {	// 客户持仓查询
				Log::Info(fmt::format("PositionRsp isTrade[{}], Message:[{}]", isTrade, m_parser.GetRawMessage()));
				parseRspQryPosition(pRspInfo);
			}
			else if (value == "T471") {	// 客户库存查询
				Log::Info(fmt::format("StorageRsp isTrade[{}], Message:[{}]", isTrade, m_parser.GetRawMessage()));
				parseRspQryStorage(pRspInfo);
			}
		}
	}
}
void SGEHandler::ParsePrivateMessage(bool isTrade) {
	int privateSeqNo = 0;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X05") {
			privateSeqNo = m_parser.GetIntValue();
			if (m_privateSeqNo >= privateSeqNo) {
// 				Log::Info(fmt::format("SGEHandler: filtrate privateMessage, SelfSeq[{}] SeqNo[{}]", m_privateSeqNo, privateSeqNo));
				return;
			}
		}
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "T202") {		// 竞价委托回报
				parseOrderRtn(SGE_OC_Unknown, true, privateSeqNo, isTrade);
			}
			else if (value == "T212") {	// 竞价撤单回报
				parseOrderRtn(SGE_OC_Unknown, false, privateSeqNo, isTrade);
			}
			else if (value == "T092") {	// 交割申报回报
				parseOrderRtn(SGE_OC_Delivery, true, privateSeqNo, isTrade);
			}
			else if (value == "T102") {	// 交割撤单回报
				parseOrderRtn(SGE_OC_Delivery, false, privateSeqNo, isTrade);
			}
			else if (value == "T122") {	// 中立仓申报回报
				parseOrderRtn(SGE_OC_Neutral, true, privateSeqNo, isTrade);
			}
			else if (value == "T132") {	// 中立仓撤单回报
				parseOrderRtn(SGE_OC_Neutral, false, privateSeqNo, isTrade);
			}
			else if (value == "T112") {	// 交割申报成交
				parseTradeRtn(SGE_OC_Delivery, privateSeqNo, isTrade);
			}
			else if (value == "T222") {	// 竞价成交回报
				parseTradeRtn(SGE_OC_Unknown, privateSeqNo, isTrade);
			}
		}
	}
}
void SGEHandler::ParsePublicMessage(bool isTrade) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "B182") {		//合约状态推送
				parseInstrumentStatus(isTrade);
			}
			else if (value == "B302" && isTrade) {	// 140前置的递延费推送
				parseDeferFee();
			}
// 			else if (value == "B102") {	//合约信息推送
// 				parseInstrumentInfo();
// 			}
		}
	}
}
void SGEHandler::ParseMarketMessage(std::string_view const& plainMsg) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("X03" == m_parser.GetKey()) {
			auto value = m_parser.GetStringValue();
			if (value == "Q012") {	
				parseDepthMarket();
			}	// 竞价行情
// 			else if (value == "Q202") {
// 				parseDeliveryMarket();
// 			}	// 交割行情
		}
	}
}

// 对话流解析
void SGEHandler::parseRspError(CSGERspInfoField* pRspInfo) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
	}
	OnRspError(pRspInfo);
}
void SGEHandler::parseRspLogin(CSGERspInfoField* pRspInfo) {
	CSGERspLoginField loginInfo = { 0 };
	CSGERspLoginField* pRspLogin = &loginInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		COPY_CONTINUE("M20", pRspLogin->SeatID);
		COPY_CONTINUE("M60", pRspLogin->TraderID);
		COPY_CONTINUE("T13", pRspLogin->TradingDay);
		COPY_CONTINUE("O14", pRspLogin->MaxOrderLocalID);
	}
	OnRspLogin(pRspInfo, pRspLogin);
}
void SGEHandler::parseRspOrderInsert(CSGERspInfoField* pRspInfo, TSGEProductClassType productClass) {
	CSGEInputOrderField InputOrder = { 0 };
	CSGEInputOrderField* pInputOrder = &InputOrder;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		COPY_CONTINUE("M20", pInputOrder->SeatID);				// 席位号
		COPY_CONTINUE("M30", pInputOrder->ClientID);			// 交易编码
		COPY_CONTINUE("M60", pInputOrder->TraderID);			// 交易员代码
		COPY_CONTINUE("I00", pInputOrder->MarketID);			// 市场代码: '00'现货 '02'延期
		COPY_CONTINUE("I10", pInputOrder->InstrumentID);
		COPY_CONTINUE("O00", pInputOrder->OrderSysID);
		COPY_CONTINUE("O01", pInputOrder->OrderLocalID);
		COPY_CONTINUE("O02", pInputOrder->Direction);
		COPY_CONTINUE("O03", pInputOrder->OffsetFlag);
		COPY_CONTINUE("O06", pInputOrder->OrderPrice);
		COPY_CONTINUE("O07", pInputOrder->OrderQuantity);
		COPY_CONTINUE("O08", pInputOrder->RemainQuantity);
		COPY_CONTINUE("O09", pInputOrder->OrderStatus);
		COPY_CONTINUE("O15", pInputOrder->OrderType);
		COPY_CONTINUE("T12", pInputOrder->ApplyTime);
	}

	if (SGE_OC_Unknown == productClass) {
		pInputOrder->ProductClass = (pInputOrder->MarketID[1] == '2') ? SGE_OC_Defer : SGE_OC_Spot;
	}	// 未知状态可能：延期或现货
	else {
		pInputOrder->ProductClass = productClass;
	}	// 交割申报或中立仓

	if (0 != pRspInfo->RspCode) {
		OnRspOrderInsert(pInputOrder, pRspInfo);
	}	// OnRspOrderInsert只处理报单错误应答，订单成功会先收到委托回报
	else {
		m_systemIDMap[pInputOrder->OrderSysID] = atoi(pInputOrder->OrderLocalID);		// SGE撤单没有本地委托号，所以委托应答时候要做系统号与本地委托号的对应
	}
}
void SGEHandler::parseRspOrderAction(CSGERspInfoField* pRspInfo, TSGEProductClassType productClass) {
	CSGEOrderActionField orderAction = { 0 };
	CSGEOrderActionField* pOrderAction = &orderAction;

	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		COPY_CONTINUE("M20", pOrderAction->SeatID);
		COPY_CONTINUE("M30", pOrderAction->ClientID);
		COPY_CONTINUE("M60", pOrderAction->TraderID);
		COPY_CONTINUE("I00", pOrderAction->MarketID);			// 市场代码: '00'现货 '02'延期
		COPY_CONTINUE("I10", pOrderAction->InstrumentID);
		COPY_CONTINUE("O00", pOrderAction->OrderSysID);
	}

	if (SGE_OC_Unknown == productClass) {
		pOrderAction->ProductClass = (pOrderAction->MarketID[1] == '2') ? SGE_OC_Defer : SGE_OC_Spot;
	}	// 未知状态可能：延期或现货
	else {
		pOrderAction->ProductClass = productClass;
	}	// 交割申报或中立仓

	if (0 != pRspInfo->RspCode) {
		OnRspOrderAction(pOrderAction, pRspInfo);
	}	// 同委托应答：只处理错误信息
}
void SGEHandler::parseRspQrySeatFund(CSGERspInfoField* pRspInfo) {
	CSGERspAccountField fundInfo = { 0 };
	CSGERspAccountField* pFundInfo = &fundInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		COPY_CONTINUE("M20", pFundInfo->SeatID);
		COPY_CONTINUE("F42", pFundInfo->Available);
	}
	OnRspQrySeatFund(pFundInfo, pRspInfo);
}
void SGEHandler::parseRspQryPosition(CSGERspInfoField* pRspInfo) {
	CSGERspClientPositionField rspPosition = { 0 };
	CSGERspClientPositionField* pRspPosition = &rspPosition;

	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		if (pRspInfo->RspCode) {
			OnRspQryClientPosition(pRspPosition, pRspInfo, true);
			return;
		}	// 应答错误，直接返回

		TSGEMarketIDType MarketID;
		COPY_CONTINUE("I00", MarketID);
		if (MarketID[1] != '0'/*I00=00 现货*/ && MarketID[1] != '2'/*I00=02 延期*/) {
			return;
		}	// 非现货或延期持仓数据直接丢弃

		if (key == "I10") {
			if (strlen(pRspPosition->InstrumentID) > 0) {
				OnRspQryClientPosition(pRspPosition, pRspInfo, false);
			}	// 品种代码不为空，先发送上一条持仓数据
			assign(pRspPosition->InstrumentID, m_parser.GetStringValue());
			continue;
		}
		COPY_CONTINUE("M30", pRspPosition->ClientID);			// 交易编码
		COPY_CONTINUE("P01", pRspPosition->LongPosition);		// 多头持仓总量
		COPY_CONTINUE("P02", pRspPosition->ShortPosition);		// 空头持仓总量
		COPY_CONTINUE("P03", pRspPosition->YDLongPosition);		// 多头老仓量
		COPY_CONTINUE("P04", pRspPosition->YDShortPosition);	// 空头老仓量
		COPY_CONTINUE("P05", pRspPosition->TDLongOpen);			// 今日多头开仓（是开仓量不是持仓量）
		COPY_CONTINUE("P06", pRspPosition->TDShortOpen);		// 今日空头开仓（是开仓量不是持仓量）
		COPY_CONTINUE("P07", pRspPosition->LongOffset);			// 多头平仓量
		COPY_CONTINUE("P08", pRspPosition->ShortOffset);		// 空头平仓量
		COPY_CONTINUE("P09", pRspPosition->LongTurnover);		// 多头持仓额
		COPY_CONTINUE("P10", pRspPosition->ShortTurnover);		// 空头持仓额
		COPY_CONTINUE("P17", pRspPosition->LongFrozen);			// 多头平仓冻结
		COPY_CONTINUE("P18", pRspPosition->ShortFrozen);		// 空头平仓冻结
		if (key == "X06") {
			OnRspQryClientPosition(pRspPosition, pRspInfo, true);
		} 	//结束标志，发送最后一条数据
	}
}
void SGEHandler::parseRspQryStorage(CSGERspInfoField* pRspInfo) {
	CSGERspClientStorageField rspStorage = { 0 };
	CSGERspClientStorageField* pRspStorage = &rspStorage;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_RSPCODE("X39", pRspInfo->RspCode);
		COPY_CONTINUE("X40", pRspInfo->RspMsg);
		if (pRspInfo->RspCode) {
			OnRspQryClientStorage(pRspStorage, pRspInfo, true);
			return;
		}	// 应答错误，直接返回

		if (key == "M30") {
			if (strlen(pRspStorage->ClientID) > 0) {
				OnRspQryClientStorage(pRspStorage, pRspInfo, false);
			}	// 交易编码不为空，先发送上一条库存数据
			assign(pRspStorage->ClientID, m_parser.GetStringValue());
			continue;
		}
		COPY_CONTINUE("V00", pRspStorage->VarietyID);		// 交割品种代码
		COPY_CONTINUE("S01", pRspStorage->TotalStorage);	// 库存总量
		COPY_CONTINUE("S02", pRspStorage->Available);		// 可用库存
		COPY_CONTINUE("S03", pRspStorage->Frozen);			// 冻结库存
		COPY_CONTINUE("S04", pRspStorage->PendStorage);		// 待提库存
		if (key == "X06") {
			OnRspQryClientStorage(pRspStorage, pRspInfo, true);
		} 	//结束标志，发送最后一条数据
	}
}

// 私有流解析
void SGEHandler::parseOrderRtn(TSGEProductClassType productClass, bool isCancelRtn, int seqNo, bool isTrade) {
	CSGEOrderField orderRtn = { 0 };
	CSGEOrderField* pOrder = &orderRtn;

	TSGETraderIDType traderID;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();	
		COPY_CONTINUE("M30", pOrder->ClientID);
		COPY_CONTINUE("M60", traderID);
		COPY_CONTINUE("I00", pOrder->MarketID);
		COPY_CONTINUE("I10", pOrder->InstrumentID);
		COPY_CONTINUE("O00", pOrder->OrderSysID);
		COPY_CONTINUE("O01", pOrder->OrderLocalID);
		COPY_CONTINUE("O02", pOrder->Direction);
		COPY_CONTINUE("O03", pOrder->OffsetFlag);
		COPY_CONTINUE("O06", pOrder->OrderPrice);
		COPY_CONTINUE("O07", pOrder->OrderQuantity);
		COPY_CONTINUE("O08", pOrder->RemainQuantity);
		COPY_CONTINUE("O09", pOrder->OrderStatus);
		COPY_CONTINUE("O15", pOrder->OrderType);
		COPY_CONTINUE("T12", pOrder->ApplyTime);
		COPY_CONTINUE("T22", pOrder->CancelTime);
	}
	if (SGE_OC_Unknown == productClass) {		// 竞价报单类型根据市场代码区分
		pOrder->ProductClass = (pOrder->MarketID[1] == '2') ? SGE_OC_Defer : SGE_OC_Spot;
	}
	else {
		pOrder->ProductClass = productClass;
	}

	if (SGE_OC_Defer != pOrder->ProductClass) {
		pOrder->OffsetFlag = (SGE_D_Buy == pOrder->Direction) ? SGE_OP_Open : SGE_OP_Close;	
	}	// 只有延期有开平标志，现货、中立仓、交割都以按照：买->开；卖->平

// 	if (m_seatInfo.TraderID.compare(traderID)) {
// 		return;
// 	}	// 过滤非自己交易员的委托回报

	OnRtnOrder(pOrder, isTrade);
	m_privateSeqNo = seqNo;
}
void SGEHandler::parseTradeRtn(TSGEProductClassType productClass, int seqNo, bool isTrade) {
	CSGETradeField tradeRtn = { 0 };
	CSGETradeField* pTrade = &tradeRtn;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("M30", pTrade->ClientID);
		COPY_CONTINUE("I00", pTrade->MarketID);
		COPY_CONTINUE("I10", pTrade->InstrumentID);
		COPY_CONTINUE("O00", pTrade->OrderSysID);
		COPY_CONTINUE("O01", pTrade->OrderLocalID);
		COPY_CONTINUE("O02", pTrade->Direction);
		COPY_CONTINUE("O03", pTrade->OffsetFlag);
		COPY_CONTINUE("O62", pTrade->TradeTime);
		COPY_CONTINUE("O63", pTrade->TradePrice);
		COPY_CONTINUE("O64", pTrade->TradeVolume);
	}

	if (SGE_OC_Unknown == productClass) {		// 竞价报单类型根据市场代码区分
		pTrade->ProductClass = (pTrade->MarketID[1] == '2') ? SGE_OC_Defer : SGE_OC_Spot;
	}
	else {
		pTrade->ProductClass = productClass;
	}
	if (SGE_OC_Defer != pTrade->ProductClass) {
		pTrade->OffsetFlag = (SGE_D_Buy == pTrade->Direction) ? SGE_OP_Open : SGE_OP_Close;
	}	// 只有延期有开平标志，现货、中立仓、交割都以按照：买->开；卖->平

	OnRtnTrade(pTrade, isTrade);
	m_privateSeqNo = seqNo;
}

// 公共流解析
void SGEHandler::parseDeferFee() {
	CSGEDeferFee deferFee;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("I10", deferFee.InstrumentID);
		COPY_CONTINUE("I25", deferFee.FeeRate);
		COPY_CONTINUE("I26", deferFee.Direction);
	}
	Log::Info(fmt::format("SGEHandler OnRtnDeferFee: Instrument[{}] FeeDir[{}] Rate[{}]", deferFee.InstrumentID, deferFee.Direction, deferFee.FeeRate));
}
void SGEHandler::parseInstrumentStatus(bool isTrade) {
	TSGEInstrumentStatusType instStatus;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("I19" == m_parser.GetKey()) {
			instStatus = m_parser.GetCharValue();
			OnRtnInstrumentStatus(instStatus, isTrade);
		}
	}
}

// 行情流解析
void SGEHandler::parseDepthMarket() {
	CSGEDepthMarketDataField marketData = { 0 };	
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "I10") {
			if (strlen(marketData.InstrumentID) > 0) {
				OnRtnDepthMarketData(&marketData);	// 合约代码不为空，先发送上一条行情数据
			}
			assign(marketData.InstrumentID, m_parser.GetStringValue());
			continue;
		}
		if (key == "Q00") {
			parsePricesInfo(&marketData);
		}
		COPY_CONTINUE("Q11", marketData.QuoteDate);
		COPY_CONTINUE("Q12", marketData.QuoteTime);
		COPY_CONTINUE("Q15", marketData.OpenPrice);
		COPY_CONTINUE("Q16", marketData.HighestPrice);
		COPY_CONTINUE("Q17", marketData.LowestPrice);
		COPY_CONTINUE("Q18", marketData.LastPrice);
		COPY_CONTINUE("Q19", marketData.ClosePrice);
		COPY_CONTINUE("Q26", marketData.UpperLimitPrice);
		COPY_CONTINUE("Q27", marketData.OpenInterest);			// 持仓量
		COPY_CONTINUE("Q30", marketData.Turnover);
		COPY_CONTINUE("Q31", marketData.AvgPrice);
		COPY_CONTINUE("Q36", marketData.LowerLimitPrice);
		COPY_CONTINUE("O64", marketData.TradeVolume);			// 成交数量
		if (key == "X06") {
			OnRtnDepthMarketData(&marketData);		// 结束标志，发送最后一条行情数据
		}
	}
}
void SGEHandler::parseDeliveryMarket() {
	CSGEDepthMarketDataField marketData = { 0 };
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "I10") {
			if (strlen(marketData.InstrumentID) > 0) {
				OnRtnDepthMarketData(&marketData);		// 合约代码不为空，先发送上一条行情数据
			}
			assign(marketData.InstrumentID, m_parser.GetStringValue());
			continue;
		}
		COPY_CONTINUE("Q22", marketData.BidVolume1);	// 交割买量
		COPY_CONTINUE("Q24", marketData.AskVolume1);	// 交割卖量
		COPY_CONTINUE("Q33", marketData.BidVolume2);	// 中立仓买量
		COPY_CONTINUE("Q32", marketData.AskVolume2);	// 中立仓卖量
		if (key == "X06") {
			OnRtnDepthMarketData(&marketData);			// 结束标志，发送最后一条行情数据
		}
	}
}
void SGEHandler::parsePricesInfo(CSGEDepthMarketDataField* pMarketData) {
	int shiftTag = 1;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey, shiftTag < 10; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "Q21") {
			switch (shiftTag) {
			case 1:	pMarketData->BidPrice1 = m_parser.GetDoubleValue();
				break;
			case 2:	pMarketData->BidPrice2 = m_parser.GetDoubleValue();
				break;
			case 3:	pMarketData->BidPrice3 = m_parser.GetDoubleValue();
				break;
			case 4:	pMarketData->BidPrice4 = m_parser.GetDoubleValue();
				break;
			case 5:	pMarketData->BidPrice5 = m_parser.GetDoubleValue();
				break;
			default:break;
			}
		}
		else if (key == "Q22") {
			switch (shiftTag) {
			case 1:	pMarketData->BidVolume1 = m_parser.GetIntValue();
				break;
			case 2:	pMarketData->BidVolume2 = m_parser.GetIntValue();
				break;
			case 3:	pMarketData->BidVolume3 = m_parser.GetIntValue();
				break;
			case 4:	pMarketData->BidVolume4 = m_parser.GetIntValue();
				break;
			case 5:	pMarketData->BidVolume5 = m_parser.GetIntValue();
				break;
			default:break;
			}
		}
		else if (key == "Q23") {
			switch (shiftTag) {
			case 1:	pMarketData->AskPrice1 = m_parser.GetDoubleValue();
				break;
			case 2:	pMarketData->AskPrice2 = m_parser.GetDoubleValue();
				break;
			case 3:	pMarketData->AskPrice3 = m_parser.GetDoubleValue();
				break;
			case 4:	pMarketData->AskPrice4 = m_parser.GetDoubleValue();
				break;
			case 5:	pMarketData->AskPrice5 = m_parser.GetDoubleValue();
				break;
			default:break;
			}
		}
		else if (key == "Q24") {
			switch (shiftTag) {
			case 1:	pMarketData->AskVolume1 = m_parser.GetIntValue();
				break;
			case 2:	pMarketData->AskVolume2 = m_parser.GetIntValue();
				break;
			case 3:	pMarketData->AskVolume3 = m_parser.GetIntValue();
				break;
			case 4:	pMarketData->AskVolume4 = m_parser.GetIntValue();
				break;
			case 5:	pMarketData->AskVolume5 = m_parser.GetIntValue();
				break;
			default:break;
			}
			++shiftTag;
		}
	}
}

// 其他功能函数
bool SGEHandler::IsErrorRspInfo(CSGERspInfoField* pRspInfo) {
	return ((nullptr != pRspInfo) && (0 != pRspInfo->RspCode));
}
bool SGEHandler::IsInvalidQuote(CSGEDepthMarketDataField* pDepthMarketData) {
	return  (0 == strlen(pDepthMarketData->InstrumentID) || DoubleComparer::equal(DBL_MAX, pDepthMarketData->LastPrice)
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->AskPrice1) || 0 == pDepthMarketData->AskVolume1
		|| DoubleComparer::equal(DBL_MAX, pDepthMarketData->BidPrice1) || 0 == pDepthMarketData->BidVolume1);
}
bool SGEHandler::encryptKeyDomain(std::string tradeID, char* encryptedPwd, char* randomCode) {
	// 关键域拼接格式：pwd+userID
	std::string pwd = m_config.Credential.Password + tradeID;

	// SM3加密关键域
	unsigned char hashKey[32] = "";
	int procSocket = m_safeHandler.ApplySocket();
	int nRtn = m_safeHandler.m_pSYD_SM3_Hash(procSocket, (unsigned char*)pwd.c_str(), (const int)pwd.size(), hashKey);
	if (0 != nRtn) {
		m_safeHandler.ReleaseSocket(procSocket);
		auto const& reportMsg = fmt::format("SGEHandler: get hashKey failed, rtnCode[{}]", nRtn);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}

	// 获取二进制随机数(固定长度32)
	unsigned char randomBin[32] = "";
	nRtn = m_safeHandler.m_pSYD_GenRandom_Bin(procSocket, 32, randomBin);
	if (0 != nRtn) {
		m_safeHandler.ReleaseSocket(procSocket);
		auto const& reportMsg = fmt::format("SGEHandler: GenRandom failed, rtnCode[{}]", nRtn);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}
	m_safeHandler.ReleaseSocket(procSocket);

	//生成国密密码
	char encPwd[100] = "";
	size_t pwdLen = 100;
	//	-1: NULL pointer | -2：in_len is not required 32 bytes | -3：random_len is not required 32 bytes
	//	-4: The length of the out_cap is not enough	| -5：encoding failed
	nRtn = sge_safe_keyfld_enc_verification(hashKey, 32/*固定长度*/, randomBin, 32/*固定长度*/, encPwd, &pwdLen);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: safe_keyfld_enc failed, rtnCode[{}]", nRtn);
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}

	//去除转义字符、转换格式并返回
	FormatSafeInfo(encPwd, pwdLen, encryptedPwd);
	std::string randomBase64 = base64_encode(randomBin, 32);
	FormatSafeInfo(randomBase64.c_str(), randomBase64.size(), randomCode);
	return true;
}

void SGEHandler::FillingPositionNotification(CSGERspClientPositionField const& position, PositionNotification& notification) {
	double multiplier = m_instHelper.GetMultiplier(notification.InstID);
	if (position.LongPosition > 0) {
		notification.Direction = Direction::BUY;
		notification.Quantity = position.LongPosition;			// 总持仓量
		notification.TodayQty = 0;
		notification.Price = position.LongTurnover / (position.LongPosition * multiplier * 100/*成交额精度是分*/);
		notification.Frozen = position.LongFrozen;

		m_publisher.PushResponse(notification);
		m_publisher.NotifyUI(notification);
		char const* direction = (Direction::BUY == notification.Direction) ? "B" : "S";
		Log::Info(fmt::format("SGEHandler: QryPositionRsp, ClientID[{}] Inst[{}] Dir[{}] Total[{}] AvgPrice[{:.3f}] Frozen[{}]", position.ClientID
			, position.InstrumentID, direction, notification.Quantity, notification.Price.value(), notification.Frozen));
	}

	if (position.ShortPosition > 0) {
		notification.Direction = Direction::SELL;
		notification.Quantity = position.ShortPosition;			// 总持仓量
		notification.TodayQty = 0;
		notification.Price = position.ShortTurnover / (position.ShortPosition * multiplier * 100/*成交额精度是分*/);
		notification.Frozen = position.ShortFrozen;

		m_publisher.PushResponse(notification);
		m_publisher.NotifyUI(notification);
		char const* direction = (Direction::BUY == notification.Direction) ? "B" : "S";
		Log::Info(fmt::format("SGEHandler: QryPositionRsp, ClientID[{}] Inst[{}] Dir[{}] Total[{}] AvgPrice[{:.3f}] Frozen[{}]", position.ClientID
			, position.InstrumentID, direction, notification.Quantity, notification.Price.value(), notification.Frozen));
	}
}
Quote SGEHandler::CreateQuote(InstID instID, CSGEDepthMarketDataField* pQuote) {
	SimpleQuoteArray buyPriceArray;
	buyPriceArray[0] = SimpleQuote(pQuote->BidPrice1, pQuote->BidVolume1);
	buyPriceArray[1] = SimpleQuote(pQuote->BidPrice2, pQuote->BidVolume2);
	buyPriceArray[2] = SimpleQuote(pQuote->BidPrice3, pQuote->BidVolume3);
	buyPriceArray[3] = SimpleQuote(pQuote->BidPrice4, pQuote->BidVolume4);
	buyPriceArray[4] = SimpleQuote(pQuote->BidPrice5, pQuote->BidVolume5);
	SimpleQuoteArray sellPriceArray;
	sellPriceArray[0] = SimpleQuote(pQuote->AskPrice1, pQuote->AskVolume1);
	sellPriceArray[1] = SimpleQuote(pQuote->AskPrice2, pQuote->AskVolume2);
	sellPriceArray[2] = SimpleQuote(pQuote->AskPrice3, pQuote->AskVolume3);
	sellPriceArray[3] = SimpleQuote(pQuote->AskPrice4, pQuote->AskVolume4);
	sellPriceArray[4] = SimpleQuote(pQuote->AskPrice5, pQuote->AskVolume5);
	int openInterest = pQuote->OpenInterest;
	double lastPrice = pQuote->LastPrice;
	money turnover = pQuote->Turnover;

	Quote quote(instID, &m_instHelper.GetInstrument(instID), buyPriceArray, sellPriceArray, pQuote->OpenPrice
		, pQuote->HighestPrice, pQuote->LowestPrice, pQuote->UpperLimitPrice, pQuote->LowerLimitPrice, pQuote->OpenInterest
		, pQuote->LastPrice, 0, pQuote->TradeVolume, turnover, true, 0, SimpleQuote(), SimpleQuote());
	return quote;
}
