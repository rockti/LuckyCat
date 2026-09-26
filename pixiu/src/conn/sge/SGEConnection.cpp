#include "pch.h"
#include "SGEConnection.h"
#include "SGE/sge_api.h"
#include "SGE/sge_packet_struct.h"
#include "SGE/sge_safe_client_interface.h"
#include "utils/Log.h"
#include "utils/String.h"
#include "notify/ConnPublisher.h"


void SGEConnection::BeforeServiceRun(void* ptr) {
	s_pThis->m_sgeLogPtr = sge_mthread_log_create("./SGELog", "Service", LEVEL_ERROR);
	if (nullptr == s_pThis->m_sgeLogPtr) {
		Log::Error("SGEConnection: Create SGE Log Failed");
	}
}
void SGEConnection::AfterServiceStop(void* ptr) {
	if (nullptr != s_pThis->m_sgeLogPtr) {
		sge_mthread_log_destroy(s_pThis->m_sgeLogPtr);
	}
}

SGEConnection* SGEConnection::s_pThis = nullptr;
SGEConnection::SGEConnection(ConnConfig& config, Instruments const& instruments, ConnPublisher& publisher)
	: ExchangeConn(ConnPlatform::SGE), m_config(config), m_instHelper(instruments), m_publisher(publisher)
	, m_safeHandler(config, publisher), m_sgeHandler(config, m_instHelper, publisher, m_safeHandler)
	, m_sgeLogPtr(nullptr), m_sgeServicePtr(nullptr) {

	s_pThis = this;
	m_sgeServiceCBList.on_before_run_ = SGEConnection::BeforeServiceRun;
	m_sgeServiceCBList.on_after_stop_ = SGEConnection::AfterServiceStop;

	LoadConfig();
	InitContexts(m_tradeContext, true);
	InitContexts(m_quoteContext, false);
}

void SGEConnection::Init() {
	if (InitSafeLinkService()/*创建安全连接服务*/ && InitSGELinkService()/*创建交易连接服务*/) {
		if (GetPublicKey(false)) {
			CreateSession(false);
		}
		if (m_quoteContext.IsHandshaked && GetPublicKey()) {
			CreateSession();
		}
	}
}
void SGEConnection::Release() {
	if (m_tradeContext.SafeLinkPtr) {
		sge_safe_client_link_destroy(m_tradeContext.SafeLinkPtr);
		m_tradeContext.SafeLinkPtr = nullptr;
	}
	if (m_tradeContext.SafeApiPtr) {
		sge_safe_client_api_destroy(m_tradeContext.SafeApiPtr);
		m_tradeContext.SafeApiPtr = nullptr;
	}
	if (m_tradeContext.SGESessionPtr) {
		sge_session_disconnect(m_tradeContext.SGESessionPtr);
		m_tradeContext.SGESessionPtr = nullptr;
	}
	if (m_quoteContext.SGESessionPtr) {
		sge_session_disconnect(m_quoteContext.SGESessionPtr);
		m_quoteContext.SGESessionPtr = nullptr;
	}
	if (m_sgeLogPtr) {
		sge_mthread_log_destroy(m_sgeLogPtr);
	}
	
	Log::Info("SGEConnection Released");
}

void SGEConnection::InsertOrder(OrderInfo const& order) {
	m_sgeHandler.ReqOrderInsert(order);
}
void SGEConnection::InsertExOrder(OrderInfo const& order) {
	m_sgeHandler.ReqOrderInsert(order, false);	// 只发送FAK委托到行情前置，不提供撤单
}
void SGEConnection::CancelOrder(OrderCancel const& action) {
	m_sgeHandler.ReqOrderAction(action);
}
void SGEConnection::SubscribeQuote(char const* instrumentCode) {
	m_sgeHandler.SubscribeQuote(instrumentCode);
// 	Log::Info(fmt::format("SGEConnection: DoSubscribe[{}] market", instrumentCode));
}
void SGEConnection::QueryMblMarket(char const* instrumentCode) {
}
void SGEConnection::QueryPosition() {
	m_sgeHandler.QueryStorage();
	m_sgeHandler.QueryPosition();
}

void SGEConnection::QueryFund() {
	m_sgeHandler.QueryFund();
}

void SGEConnection::LoadConfig() {
	memset(&m_safeContext, 0, sizeof(SafeContext));
	auto [safeAddress, safePort] = SeparatePort(m_config.LocalEndpoint);
	strcpy_s(m_safeContext.Address, safeAddress.c_str());
	m_safeContext.Port = safePort;

	// 两个前置需要有不同的交易员登录，所以这里做个特殊处理：一个配置节用‘:’区分两个交易员号，但是使用同一个密码
	auto panelVec = split(m_config.Credential.UserID, ":");
	if (panelVec.size() < 2) {
		std::string reportMsg = fmt::format("SGEConnection: Do not have enough traderID[{}], check config", panelVec.size());
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg);
		return;
	}

	memset(&m_tradeContext.SGESessionInfo, 0, sizeof(session_client_info_t));
	strcpy_s(m_tradeContext.SGESessionInfo.sge_id_, Desc(Exchange::SGE));
	strcpy_s(m_tradeContext.SGESessionInfo.trader_id_, panelVec[0].c_str());
	m_tradeContext.SGESessionInfo.enc_mode_ = 0;

	memset(&m_quoteContext.SGESessionInfo, 0, sizeof(session_client_info_t));
	strcpy_s(m_quoteContext.SGESessionInfo.sge_id_, Desc(Exchange::SGE));
	strcpy_s(m_quoteContext.SGESessionInfo.trader_id_, panelVec[1].c_str());
	m_quoteContext.SGESessionInfo.enc_mode_ = 0;
	Log::Info(fmt::format("SGEConnection: LoadConfig, traderID[{}]:[{}]", m_tradeContext.SGESessionInfo.trader_id_, m_quoteContext.SGESessionInfo.trader_id_));
}
void SGEConnection::InitContexts(SGEContext& context, bool isTrade) {
	context.SafeCBList.cb_connect_ = SafeHandler::cb_connect;						// 连接
	context.SafeCBList.cb_disconnect_ = SafeHandler::cb_disconnect;					// 断线
	context.SafeCBList.cb_reconnect_ = SafeHandler::cb_reconnect;					// 断线重连
	context.SafeCBList.cb_reset_connect_ = SafeHandler::cb_resetConnect;			// 重置连接
	context.SafeCBList.cb_is_connected_ = SafeHandler::cb_isConnected;				// 连接状态
	context.SafeCBList.cb_encrypt_ = SafeHandler::cb_encrypt_data;					// 加密数据
	context.SafeCBList.cb_decrypt_ = SafeHandler::cb_decrypt_data;					// 解密数据
	context.SafeCBList.cb_mac_ = SafeHandler::cb_mac_data;							// 计算mac
	context.SafeCBList.cb_hash_ = SafeHandler::cb_hash_data;						// 计算hash
	context.SafeCBList.cb_sign_ = SafeHandler::cb_sign_data;						// 数字签名
	context.SafeCBList.cb_verify_sign_ = SafeHandler::cb_verify_signed_data;		// 签名验证
	context.SafeCBList.cb_get_skey_ = SafeHandler::cb_get_skey;						// 获取会话密钥
	context.SafeCBList.cb_gen_random_ = SafeHandler::cb_gen_random;					// 生成随机数
	context.SafeCBList.cb_get_enc_keypair_ = SafeHandler::cb_get_enc_keypair;		// 获取加密密钥信息
	context.SafeCBList.cb_free_enc_keypair_ = SafeHandler::cb_free_enc_keypair;		// 释放加密密钥信息
	context.SafeCBList.cb_get_sign_keypair_ = SafeHandler::cb_get_sign_keypair;		// 获取签名密钥信息
	context.SafeCBList.cb_free_sign_keypair_ = SafeHandler::cb_free_sign_keypair;	// 签名验证
	context.SafeCBList.ctx_ = &m_safeContext;	// 在此之前必须先生成SafeContext

	context.SGECBList.on_connected_ = isTrade ? SGEHandler::OnTradeFrontConnected : SGEHandler::OnQuoteFrontConnected;
	context.SGECBList.on_disconnected_ = isTrade ? SGEHandler::OnTradeFrontDisconnected : SGEHandler::OnQuoteFrontDisconnected;
	context.SGECBList.on_read_ = isTrade ? SGEHandler::OnTradeReceiveMsg : SGEHandler::OnQuoteReceiveMsg;
	isTrade ? m_sgeHandler.RegistTradeContext(&m_tradeContext) : m_sgeHandler.RegistQuoteContext(&m_quoteContext);
}

bool SGEConnection::InitSafeLinkService() {
	std::string reportMsg = "SGEConnection: InitSafeLinkService ";
	m_tradeContext.SafeApiPtr = sge_safe_client_api_create(&m_tradeContext.SafeCBList, 0/*默认硬加密*/);
	if (nullptr != m_tradeContext.SafeApiPtr) {
		m_quoteContext.SafeApiPtr = m_tradeContext.SafeApiPtr;		// 行情连接与交易连接公用一套SafeApi

		m_tradeContext.SafeLinkPtr = sge_safe_client_link_create(m_tradeContext.SafeApiPtr);
		if (nullptr == m_tradeContext.SafeLinkPtr) {
			reportMsg += fmt::format("failed, can not connect to SGE_SafeDevice[{}:{}]", m_safeContext.Address, m_safeContext.Port);
			m_publisher.NotifyUI(reportMsg);
			Log::Error(reportMsg.c_str());
			return false;
		}
		m_quoteContext.SafeLinkPtr = m_tradeContext.SafeLinkPtr;	// 行情连接与交易连接公用一个加密设备Handle
	}
	else {
		reportMsg += "failed, SGE_API can not create";
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}

	reportMsg += "success";
	Log::Info(reportMsg.c_str());
	return true;
}
bool SGEConnection::InitSGELinkService() {
	std::string reportMsg = "SGEConnection: InitSGELinkService ";
	m_sgeServicePtr = sge_service_create(&m_sgeServiceCBList, &m_tradeContext);
	if (nullptr != m_sgeServicePtr) {
		if (0 != sge_service_run(m_sgeServicePtr)) {
			reportMsg += "failed, SGE_Link_Service can not run";
			m_publisher.NotifyUI(reportMsg);
			Log::Error(reportMsg.c_str());
			return false;
		}
	}
	else {
		reportMsg += "failed, SGE_Link_Service can not created";
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}

	reportMsg += "success";	
	Log::Info(reportMsg.c_str());
	return true;
}

bool SGEConnection::GetPublicKey(bool isTrade) {
	std::string reportMsg = "SGEHandler: GetPublicKey ";
	int pubKeyLen = 140;
	int procSocket = m_safeHandler.ApplySocket();

	SGEContext& theContext = isTrade ? m_tradeContext : m_quoteContext;
	int nRtn = m_safeHandler.m_pSYD_SM2_GetPublicKey(procSocket, theContext.SGESessionInfo.trader_id_, 0x00, theContext.SGESessionInfo.svr_sign_pubkey_, &pubKeyLen);
// 	Log::Info(fmt::format("SGEHandler: GetPublicKey, traderID[{}] svr_sign_public[{}] rtn[{}] isTrade[{}]", theContext.SGESessionInfo.trader_id_, theContext.SGESessionInfo.svr_sign_pubkey_, nRtn, isTrade));
	if (0 == nRtn) {
		theContext.SGESessionInfo.svr_sign_pubkey_len_ = pubKeyLen;		// 签名公钥长度
		nRtn = m_safeHandler.m_pSYD_SM2_GetPublicKey(procSocket, theContext.SGESessionInfo.trader_id_, 0x01, theContext.SGESessionInfo.svr_enc_pubkey_, &pubKeyLen);
// 		Log::Info(fmt::format("SGEHandler: GetPublicKey, traderID[{}] svr_enc_public[{}] rtn[{}] isTrade[{}]", theContext.SGESessionInfo.trader_id_, theContext.SGESessionInfo.svr_enc_pubkey_, nRtn, isTrade));
		if (0 == nRtn) {
			theContext.SGESessionInfo.svr_enc_pubkey_len_ = pubKeyLen;	// 加密公钥长度
			m_safeHandler.ReleaseSocket(procSocket);
			return true;
		}
	}

	m_safeHandler.ReleaseSocket(procSocket);
	reportMsg += fmt::format("failed, rtnCode[{}]", nRtn);
	Log::Error(reportMsg.c_str());
	return false;
}
bool SGEConnection::CreateSession(bool isTrade) {
	std::string reportMsg = "SGEConnection: CreateSession ";

	auto [theAddress, thePort] = isTrade ? SeparatePort(m_config.TradeEndpoint) : SeparatePort(m_config.QuoteEndpoint);
	SGEContext& theContext = isTrade ? m_tradeContext : m_quoteContext;
	theContext.SGESessionPtr = sge_session_create(m_sgeServicePtr, theAddress.c_str(), thePort, &theContext.SGECBList, 15, 120, &theContext);
	if (nullptr != theContext.SGESessionPtr) {
		int nRtn = sge_session_connect(theContext.SGESessionPtr);
		if (0 != nRtn) {
			reportMsg += fmt::format("failed, rtnCode[{}] can not connect to SGERemote[{}:{}] isTrade[{}]", nRtn, theAddress.c_str(), thePort, isTrade);
			m_publisher.NotifyUI(reportMsg);
			Log::Error(reportMsg.c_str());
			return false;
		}
	}
	else {
		reportMsg += "failed, can not create session";
		m_publisher.NotifyUI(reportMsg);
		Log::Error(reportMsg.c_str());
		return false;
	}

	//等待安全握手完成
	while (false == theContext.IsHandshaked && m_retryTimes-- > 0) {
		Sleep(150);
	}

	if (m_retryTimes <= 0) {
		reportMsg += fmt::format("failed, SafeRemote[{}:{}] SGERemote[{}:{}] APIVersion[{}] isTrade[{}] retryTimes[{}]", m_safeContext.Address, m_safeContext.Port
			, theAddress.c_str(), thePort, sge_get_version(), isTrade, m_retryTimes);
		m_publisher.NotifyUI(reportMsg);
		Log::Info(reportMsg.c_str());
		return false;
	}
	
	m_retryTimes = 6;
	int nRtn = sge_session_set_heartbeat_timeout(theContext.SGESessionPtr, 120);
	reportMsg += fmt::format("success, SafeRemote[{}:{}] SGERemote[{}:{}] APIVersion[{}] SessionKey[{}] isTrade[{}]", m_safeContext.Address, m_safeContext.Port
		, theAddress.c_str(), thePort, sge_get_version(), theContext.SessionKey, isTrade);	
	Log::Info(reportMsg.c_str());
	m_sgeHandler.Login(isTrade);
	return true;
}
