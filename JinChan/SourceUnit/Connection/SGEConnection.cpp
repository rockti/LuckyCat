#include "stdafx.h"
#include "SGEConnection.h"
#include "CachedFlow.h"
#include "Profile.h"
#include <fmt/format.h>


void SGEConnectionBase::BeforeServiceRun(void* ptr) {
	s_pThis->m_sgeLogPtr = sge_mthread_log_create("./SGELog", "Service", LEVEL_ERROR);
	if (NULL == s_pThis->m_sgeLogPtr) {
		Log::Error("SGEConnection: Create SGE Log Failed");
	}
}
void SGEConnectionBase::AfterServiceStop(void* ptr) {
	if (s_pThis->m_sgeLogPtr) {
		sge_mthread_log_destroy(s_pThis->m_sgeLogPtr);
	}
}

SGEConnectionBase* SGEConnectionBase::s_pThis = NULL;
SGEConnectionBase::SGEConnectionBase(std::string const& sgeAddress, int sgePort, std::string const& safeAddress, int safePort)
:m_sgeLogPtr(NULL),m_sgeServicePtr(NULL),m_sgeAddress(sgeAddress), m_sgePort(sgePort),
m_safeAddress(safeAddress), m_safePort(safePort) {
	s_pThis = this;
	m_sgeServiceCBList.on_before_run_ = SGEConnectionBase::BeforeServiceRun;
	m_sgeServiceCBList.on_after_stop_ = SGEConnectionBase::AfterServiceStop;

	memset(&m_safeContext, 0, sizeof(SafeContext));
	strcpy_s(m_safeContext.Address, safeAddress.c_str());
	m_safeContext.Port = safePort;
	initSGEContext(m_sgeContext);
}

void SGEConnectionBase::initSGEContext(SGEContext& sgeContext) {
	memset(&sgeContext, 0, sizeof(SGEContext));
	sgeContext.SafeCBList.cb_connect_ = SafeHandler::cb_connect;						//连接
	sgeContext.SafeCBList.cb_disconnect_ = SafeHandler::cb_disconnect;				//断线
	sgeContext.SafeCBList.cb_reconnect_ = SafeHandler::cb_reconnect;					//断线重连
	sgeContext.SafeCBList.cb_reset_connect_ = SafeHandler::cb_resetConnect;			//重置连接
	sgeContext.SafeCBList.cb_is_connected_ = SafeHandler::cb_isConnected;				//连接状态
	sgeContext.SafeCBList.cb_encrypt_ = SafeHandler::cb_encrypt_data;					//加密数据
	sgeContext.SafeCBList.cb_decrypt_ = SafeHandler::cb_decrypt_data;					//解密数据
	sgeContext.SafeCBList.cb_mac_ = SafeHandler::cb_mac_data;							//计算mac
	sgeContext.SafeCBList.cb_hash_ = SafeHandler::cb_hash_data;						//计算hash
	sgeContext.SafeCBList.cb_sign_ = SafeHandler::cb_sign_data;						//数字签名
	sgeContext.SafeCBList.cb_verify_sign_ = SafeHandler::cb_verify_signed_data;		//签名验证
	sgeContext.SafeCBList.cb_get_skey_ = SafeHandler::cb_get_skey;					//获取会话密钥
	sgeContext.SafeCBList.cb_gen_random_ = SafeHandler::cb_gen_random;				//生成随机数
	sgeContext.SafeCBList.cb_get_enc_keypair_ = SafeHandler::cb_get_enc_keypair;		//获取加密密钥信息
	sgeContext.SafeCBList.cb_free_enc_keypair_ = SafeHandler::cb_free_enc_keypair;	//释放加密密钥信息
	sgeContext.SafeCBList.cb_get_sign_keypair_ = SafeHandler::cb_get_sign_keypair;	//获取签名密钥信息
	sgeContext.SafeCBList.cb_free_sign_keypair_ = SafeHandler::cb_free_sign_keypair;	//签名验证
	sgeContext.SafeCBList.ctx_ = &m_safeContext;	//在此之前必须先生成SafeContext
}

bool SGEConnectionBase::InitInstance(CSelectThread* pFrontThread) {
	if (InitSafeDevice() && InitLinkService()) {
		return CreateSGESession();
	}
	return false;
}
void SGEConnectionBase::ExitInstance() {
	if (m_sgeContext.SafeLinkPtr) {
		sge_safe_client_link_destroy(m_sgeContext.SafeLinkPtr);
		m_sgeContext.SafeLinkPtr = NULL;
	}
	if (m_sgeContext.SafeApiPtr) {
		sge_safe_client_api_destroy(m_sgeContext.SafeApiPtr);
		m_sgeContext.SafeApiPtr = NULL;
	}
	if (m_sgeContext.SGESessionPtr) {
		sge_session_disconnect(m_sgeContext.SGESessionPtr);
		m_sgeContext.SGESessionPtr = NULL;
	}

	if (m_sgeServicePtr) {
		sge_service_stop(m_sgeServicePtr);
		sge_service_destroy(m_sgeServicePtr);
		m_sgeServicePtr = NULL;
	}
	if (m_sgeLogPtr) {
		sge_mthread_log_destroy(m_sgeLogPtr);
		m_sgeLogPtr = NULL;
	}
}

bool SGEConnectionBase::InitSafeDevice() {
	string reportMsg = "SGEConnection: InitSafeDevice ";
	m_sgeContext.SafeApiPtr = sge_safe_client_api_create(&m_sgeContext.SafeCBList, 0/*默认硬加密*/);
	if (NULL != m_sgeContext.SafeApiPtr) {
		m_sgeContext.SafeLinkPtr = sge_safe_client_link_create(m_sgeContext.SafeApiPtr);
		if (NULL == m_sgeContext.SafeLinkPtr) {
			reportMsg += fmt::format("failed, can not connect to SGE_SafeDevice[{}:{}]", m_safeContext.Address, m_safeContext.Port);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	} else {
		reportMsg += "failed, SGE_API can not create";
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	reportMsg += "success!";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());

	return true;
}
bool SGEConnectionBase::InitLinkService() {
	string reportMsg = "SGEConnection: InitLinkService ";
	m_sgeServicePtr = sge_service_create(&m_sgeServiceCBList, &m_sgeContext);
	if (NULL != m_sgeServicePtr) {
		if (0 != sge_service_run(m_sgeServicePtr)) {
			reportMsg += "failed, SGE_Link_Service can not run";
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	}
	else {
		reportMsg += "failed, SGE_Link_Service can not created";
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	reportMsg += "success!";
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());
	return true;
}
bool SGEConnectionBase::CreateSGESession() {
	string reportMsg = "SGEConnection: CreateTradeSession ";
	m_sgeContext.SGESessionPtr = sge_session_create(m_sgeServicePtr, m_sgeAddress.c_str(), m_sgePort, &m_sgeContext.SGECBList, 15, 120, &m_sgeContext);
	if (NULL != m_sgeContext.SGESessionPtr) {
		int nRtn = sge_session_connect(m_sgeContext.SGESessionPtr);
		if (0 != nRtn) {
			reportMsg += fmt::format("failed, rtnCode[{}] can not connect to SGERemote[{}:{}]", nRtn, m_sgeAddress.c_str(), m_sgePort);
			printNlog(reportMsg);
			return false;
		}
	} else {
		reportMsg += "failed, can not create SGE session";
		printNlog(reportMsg);
		return false;
	}

	//等待安全握手完成
	while (!m_sgeContext.IsHandshake) {
		Sleep(10);
	}
	int nRtn = sge_session_set_heartbeat_timeout(m_sgeContext.SGESessionPtr, 120);
	reportMsg += fmt::format("success, SafeRemote[{}:{}] SafeVersion[{}] SGERemote[{}:{}] ApiVersion[{}]", 
		m_safeContext.Address, m_safeContext.Port, sge_safe_client_get_api_version(), 
		m_sgeAddress.c_str(), m_sgePort, sge_get_version());
	printNlog(reportMsg);
	return true;
}
SGETradeConnection::SGETradeConnection(InstrumentRepository& instRepo, 
	std::string const& sgeAddress, int sgePort, 
	std::string const& safeAddress, int safePort):
	SGEConnectionBase(sgeAddress,sgePort,safeAddress,safePort),
	m_sgeHandler(instRepo, sgeAddress,sgePort, m_safeContext){
	m_sgeContext.SGECBList.on_connected_ = SGETradeHandler::OnFrontConnected;
	m_sgeContext.SGECBList.on_disconnected_ = SGETradeHandler::OnFrontDisconnected;
	m_sgeContext.SGECBList.on_read_ = SGETradeHandler::OnReceiveMsg;
	m_sgeHandler.RegistTradeContext(&m_sgeContext);

	initSGEContext(m_storageContext);
	m_storageContext.SGECBList.on_connected_ = SGETradeHandler::OnSTFrontConnected;
	m_storageContext.SGECBList.on_disconnected_ = SGETradeHandler::OnSTFrontDisconnected;
	m_storageContext.SGECBList.on_read_ = SGETradeHandler::OnSTReceiveMsg;
	m_sgeHandler.RegistStorageContext(&m_storageContext);

}
bool SGETradeConnection::InitInstance(CSelectThread* pFrontThread) {
	bool result = SGEConnectionBase::InitInstance(pFrontThread);
	if (result) {
		//仓储连接与交易连接共用安全连接
		m_storageContext.SafeApiPtr = m_sgeContext.SafeApiPtr;
		m_storageContext.SafeLinkPtr = m_sgeContext.SafeLinkPtr;

	}
	auto connectStorage = theProfile().GetPrivateProfile("Front", "ConnectStorage", "true");
	if (connectStorage == "true") {
		return result && CreateStorageSession();
	} else {
		return result;
	}
}
void SGETradeConnection::ExitInstance() {
	SGEConnectionBase::ExitInstance();
}
bool SGETradeConnection::CreateStorageSession() {
	string reportMsg = "SGEConnection: CreateStorageSession ";
	int storagePort = 5555;
	string storageAddr = "188.31.25.70";
	m_storageContext.SGESessionPtr = sge_session_create(m_sgeServicePtr, storageAddr.c_str(), storagePort, &m_storageContext.SGECBList, 15, 120, &m_storageContext);
	if (NULL != m_storageContext.SGESessionPtr) {
		int nRtn = sge_session_connect(m_storageContext.SGESessionPtr);
		if (0 != nRtn) {
			reportMsg += FT("failed, rtnCode[%d] can not connect to Remote[{}:{}]", nRtn, storageAddr.c_str(), storagePort);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	} else {
		reportMsg += "failed, can not create SGE session";
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	//等待安全握手完成
	while (!m_storageContext.IsHandshake) {
		Sleep(10);
	}

	reportMsg += fmt::format("success, SafeRemote[{}:{}] SafeVersion[{}] SGERemote[{}:{}]", m_safeContext.Address, m_safeContext.Port,
		sge_safe_client_get_api_version(), storageAddr.c_str(), storagePort);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Info(reportMsg.c_str());
	return true;
}
