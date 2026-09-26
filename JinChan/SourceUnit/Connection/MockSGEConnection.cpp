#include "stdafx.h"
#include "MockSGEConnection.h"
#include "CachedFlow.h"
#include "Profile.h"

void MockSGEConnection::BeforeServiceRun(void* ptr) {
	s_pThis->m_sgeLogPtr = sge_mthread_log_create("./SGELog", "Service", LEVEL_DEBUG);
	if (NULL == s_pThis->m_sgeLogPtr) {
		Log::Error("SGEConnection: Create SGE Log Failed");
	}
}
void MockSGEConnection::AfterServiceStop(void* ptr) {
	if (s_pThis->m_sgeLogPtr) {
		sge_mthread_log_destroy(s_pThis->m_sgeLogPtr);
	}
}

MockSGEConnection* MockSGEConnection::s_pThis = NULL;
MockSGEConnection::MockSGEConnection(SgeLinkType::TYPE type, CMarketFlowManager* pFlowManager, int topicID, string const& sgeAddress, int sgePort, string const& safeAddress, int safePort)
:m_linkType(type), m_pFlowManager(pFlowManager), m_topicID(topicID)
,m_sgeHandler(type, pFlowManager, topicID, sgeAddress, sgePort, m_safeContext)
,m_sgeLogPtr(NULL),m_sgeServicePtr(NULL)
{
	s_pThis = this;
	m_sgeServiceCBList.on_before_run_ = MockSGEConnection::BeforeServiceRun;
	m_sgeServiceCBList.on_after_stop_ = MockSGEConnection::AfterServiceStop;

	memset(&m_safeContext, 0, sizeof(SafeContext));
	strcpy_s(m_safeContext.Address, safeAddress.c_str());
	m_safeContext.Port = safePort;

	memset(&m_sgeContext, 0, sizeof(SGEContext));
	m_sgeContext.SafeCBList.cb_connect_ = SafeHandler::cb_connect;						//连接
	m_sgeContext.SafeCBList.cb_disconnect_ = SafeHandler::cb_disconnect;				//断线
	m_sgeContext.SafeCBList.cb_reconnect_ = SafeHandler::cb_reconnect;					//断线重连
	m_sgeContext.SafeCBList.cb_reset_connect_ = SafeHandler::cb_resetConnect;			//重置连接
	m_sgeContext.SafeCBList.cb_is_connected_ = SafeHandler::cb_isConnected;				//连接状态
	m_sgeContext.SafeCBList.cb_encrypt_ = SafeHandler::cb_encrypt_data;					//加密数据
	m_sgeContext.SafeCBList.cb_decrypt_ = SafeHandler::cb_decrypt_data;					//解密数据
	m_sgeContext.SafeCBList.cb_mac_ = SafeHandler::cb_mac_data;							//计算mac
	m_sgeContext.SafeCBList.cb_hash_ = SafeHandler::cb_hash_data;						//计算hash
	m_sgeContext.SafeCBList.cb_sign_ = SafeHandler::cb_sign_data;						//数字签名
	m_sgeContext.SafeCBList.cb_verify_sign_ = SafeHandler::cb_verify_signed_data;		//签名验证
	m_sgeContext.SafeCBList.cb_get_skey_ = SafeHandler::cb_get_skey;					//获取会话密钥
	m_sgeContext.SafeCBList.cb_gen_random_ = SafeHandler::cb_gen_random;				//生成随机数
	m_sgeContext.SafeCBList.cb_get_enc_keypair_ = SafeHandler::cb_get_enc_keypair;		//获取加密密钥信息
	m_sgeContext.SafeCBList.cb_free_enc_keypair_ = SafeHandler::cb_free_enc_keypair;	//释放加密密钥信息
	m_sgeContext.SafeCBList.cb_get_sign_keypair_ = SafeHandler::cb_get_sign_keypair;	//获取签名密钥信息
	m_sgeContext.SafeCBList.cb_free_sign_keypair_ = SafeHandler::cb_free_sign_keypair;	//签名验证
	m_sgeContext.SafeCBList.ctx_ = &m_safeContext;
	m_sgeContext.SGECBList.on_connected_ = SGETradeHandler::OnFrontConnected;
	m_sgeContext.SGECBList.on_disconnected_ = SGETradeHandler::OnFrontDisconnected;
	m_sgeContext.SGECBList.on_read_ = SGETradeHandler::OnReceiveMsg;

	m_sgeHandler.RegistContext(&m_sgeContext);
}

bool MockSGEConnection::InitInstance(CSelectThread* pFrontThread)
{
	if (InitSafeDevice() && InitLinkService()) {
		if (SgeLinkType::QUOTE == m_sgeHandler.GetLinkType() && NULL != pFrontThread) {
			m_pFlowManager->RegisterTopicFlow(m_topicID);								//在这里生成Topic对应的内存流
			CCachedFlow* pTopicFlow = (CCachedFlow*)m_pFlowManager->GetTopicFlow(m_topicID);
			CNotifyHandler* notifyier = new CNotifyHandler(pFrontThread, m_topicID);	//利用前置线程创建通知器:CreatePipe
			pTopicFlow->AttachObserver(notifyier);		//将TopicFlow注册到通知器，当TopicFlow调用Append时进行通知
		}

		return CreateLinkSession();
	}
	return false;
}

void MockSGEConnection::ExitInstance() {

}

bool MockSGEConnection::InitSafeDevice() {
	return true;
}
bool MockSGEConnection::InitLinkService() {
	return true;
}
bool MockSGEConnection::CreateLinkSession() {
	return true;
}
