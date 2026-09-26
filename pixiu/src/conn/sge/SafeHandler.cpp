#include "pch.h"
#include "SafeHandler.h"
#include "SGE/Syd_Api_WIN.h"
#include "notify/ConnPublisher.h"
#include "utils/Log.h"

void* SafeHandler::cb_connect(void* ctx) {
	SafeContext* pContext = (SafeContext*)ctx;
	if (nullptr == pContext)	return NULL;

	int socketID = -1;
	for (auto i = 0; i < MAX_SAFE_HANDLER_COUNT; ++i) {
		int nRtn = m_pThis->m_pSYD_Connect((char const*)pContext->Address, pContext->Port, NULL, 20, &socketID);
		if (0 != nRtn) {
			Log::Error(fmt::format("SafeHandler: Connect to safeDevice failed, rtnCode[{}] Remote[{}:{}]", nRtn, pContext->Address, pContext->Port));
			return nullptr;
		}
		m_pThis->m_socketPool.Init(socketID);
		Log::Info(fmt::format("SafeHandler: Connect to safeDevice success, Remote[{}:{}] socketID[{}]", pContext->Address, pContext->Port, socketID));
	}
	return ctx;
}
int SafeHandler::cb_disconnect(void* handle) {
	for (auto socket : m_pThis->m_socketPool.All()) {
		m_pThis->m_pSYD_Disconnect(socket);
	}
	Log::Error("SafeHandler: safeDevice disconnected");
	return 0;
}
int SafeHandler::cb_reconnect(void* handle) {
	Log::Info("SafeHandler: do not support 'reconnect' interface now!");
	return -1;
}
int SafeHandler::cb_resetConnect(void* handle) {
	Log::Info("SafeHandler: do not support 'reset' interface now!");
	return -1;
}
int SafeHandler::cb_isConnected(void* handle) {
	int procSocket = m_pThis->m_socketPool.Apply();
	int isConnected = m_pThis->m_pSYD_ConnectStatus(procSocket);
	m_pThis->m_socketPool.Release(procSocket);
	return isConnected;
}

int SafeHandler::cb_encrypt_data(void* handle, const unsigned char* skey, const unsigned int skey_len, const unsigned char* src, const unsigned int src_len, unsigned char* dst, unsigned int* cap) {
	if (NULL == handle || NULL == skey || NULL == src || NULL == dst || NULL == cap)
		return -1;

	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_SM4_EncryptData(procSocket, skey, src, src_len, dst, (int*)cap);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: encrypt data[{}] failed, rtnCode[{}]", src, nRtn));
		return -1;
	}
	return 0;
}
int SafeHandler::cb_decrypt_data(void* handle, const unsigned char* skey, const unsigned int skey_len, const unsigned char* src, const unsigned int src_len, unsigned char* dst, unsigned int* cap) {
	if (NULL == handle || NULL == skey || NULL == src || NULL == dst || NULL == cap)
		return -1;

	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_SM4_DecryptData(procSocket, skey, src, src_len, dst, (int*)cap);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: decrypt data failed, rtnCode[{}]", nRtn));
		return -1;
	}
	return 0;
}
int SafeHandler::cb_mac_data(void* handle, const unsigned char* skey, const unsigned int skey_len, const unsigned char* src, const unsigned int src_len, unsigned char* dst, unsigned int* cap) {
	Log::Info("SafeHandler: do not support 'mac_data' interface now!");
	return -1;
}
int SafeHandler::cb_hash_data(void* handle, const unsigned char* data, const unsigned int data_len, unsigned char* hash, unsigned int* hash_cap) {
	if (NULL == handle || NULL == data)
		return -1;

	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_SM3_Hash(procSocket, data, data_len, hash);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: hash data failed, rtnCode[{}]", nRtn));
		return -1;
	}
	*hash_cap = 32;	// hash长度固定32
	return 0;
}
int SafeHandler::cb_sign_data(void* handle, const void* sign_keypair, const unsigned char* data, const unsigned int data_len, unsigned char* signed_data, unsigned int* cap) {
	if (NULL == handle || NULL == data || signed_data == NULL || NULL == cap)
		return -1;

	int procSocket = m_pThis->m_socketPool.Apply();
	KeyPairContext* pKeyContext = (KeyPairContext*)sign_keypair;
	int nRtn = m_pThis->m_pSYD_SM2_Sign(procSocket, pKeyContext->ID, 1, (const unsigned char*)pKeyContext->Key, pKeyContext->Len, data, data_len, signed_data, (int*)cap);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: sign data[{}] failed, rtnCode[{}]", data, nRtn));
		return -1;
	}
	return 0;
}
int SafeHandler::cb_verify_signed_data(void* handle, const unsigned char* data, const unsigned int data_len, const unsigned char* signed_data,
	unsigned int signed_data_len, const unsigned char* pub_key, const unsigned int pub_key_len) {
	if (NULL == handle || NULL == data || NULL == signed_data || NULL == pub_key)
		return -1;

	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_SM2_Verify(procSocket, m_pThis->m_exchPubKey, 136, 1, data, data_len, signed_data, signed_data_len);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: verify signed data failed, rtnCode[{}]", nRtn));
		return -1;
	}
	return 0;
}
int SafeHandler::cb_get_skey(void* handle, const char* idx, const unsigned char* cipher_key, const unsigned int cipher_key_len, unsigned char* skey, unsigned int* cap) {
	if (NULL == handle || NULL == idx || NULL == cipher_key || NULL == skey || NULL == cap)
		return -1;

	unsigned char pKCV[33] = "";
	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_SM2_ConfirmSessionKey(procSocket, idx, cipher_key, cipher_key_len, skey, (int*)cap, pKCV);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: confirm sessionKey failed, rtnCode[{}]", nRtn));
		return -1;
	}
	memcpy(m_pThis->m_sessionKey, skey, *cap);
// 	Log::Info(fmt::format("SafeHandler: confirm sessionKey success, sessionKey[{}]", skey));
	return 0;
}
int SafeHandler::cb_gen_random(void* handle, const unsigned int len, unsigned char* random) {
	if (handle == NULL)
		return -1;

	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_GenRandom(procSocket, len / 2, random);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: generate random failed, rtnCode[{}]", nRtn));
	}
	return nRtn;
}
int SafeHandler::cb_get_sign_keypair(void* handle, const char* idx, void** sign_keypair) {
	if (NULL == handle || NULL == idx || NULL == sign_keypair)
		return -1;

	if (strlen(idx) > 16) {
		Log::Error(fmt::format("SafeHandler: get_sign_keyPair failed, idx too long[{}]", strlen(idx)));
		return -1;
	}
	
	KeyPairContext* keyPair = (KeyPairContext*)malloc(sizeof(KeyPairContext));
	memset(keyPair, 0, sizeof(KeyPairContext));
	memcpy(keyPair->ID, idx, strlen(idx));
	keyPair->Len = 140;
	int procSocket = m_pThis->m_socketPool.Apply();
	int nRtn = m_pThis->m_pSYD_SM2_GetPublicKey(procSocket, idx, 0, (unsigned char*)keyPair->Key, &keyPair->Len);
	m_pThis->m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		Log::Error(fmt::format("SafeHandler: get keyPair failed, rtnCode[{}]", nRtn));
		free(keyPair);
		return -1;
	}
	*sign_keypair = keyPair;
	return 0;
}
int SafeHandler::cb_free_sign_keypair(void* handle, void* sign_keypair) {
	if (NULL == sign_keypair)
		return -1;

	free(sign_keypair);
	sign_keypair = NULL;
	return 0;
}
int SafeHandler::cb_get_enc_keypair(void* handle, const char* idx, void** enc_keypair) {
	Log::Info("SafeHandler: do not support 'get_enc_keypair' interface now!");
	return -1;	//暂未使用接口
}
int SafeHandler::cb_free_enc_keypair(void* handle, void* enc_keypair) {
	Log::Info("SafeHandler: do not support 'free_enc_keypair' interface now!");
	return -1;	//暂未使用接口
}

SafeHandler* SafeHandler::m_pThis = NULL;
SafeHandler::SafeHandler(ConnConfig& config, ConnPublisher& publisher)
	:m_config(config), m_publisher(publisher) {
	m_pThis = this;
#ifdef _WIN64
	m_libInstance = LoadLibrary(_T("./SydApi_64.dll"));
#else 
	m_libInstance = LoadLibrary(_T("./SydApi_32.dll"));
#endif

	if (m_libInstance) {
		m_pSYD_Connect = (Connect)GetProcAddress(m_libInstance, "SYD_Connect");
		m_pSYD_Disconnect = (Disconnect)GetProcAddress(m_libInstance, "SYD_Disconnect");
		m_pSYD_Reconnect = (Reconnect)GetProcAddress(m_libInstance, "SYD_ReConnect");
		m_pSYD_Reset = (Reset)GetProcAddress(m_libInstance, "SYD_Reset");
		m_pSYD_ConnectStatus = (ConnectStatus)GetProcAddress(m_libInstance, "SYD_Connect_Status");

		m_pSYD_SM2_GetPublicKey = (GetPublicKey)GetProcAddress(m_libInstance, "SYD_SM2_Get_PublicKey_C");
		m_pSYD_GenRandom = (GenRandom)GetProcAddress(m_libInstance, "SYD_GenerateRandom");
		m_pSYD_GenRandom_Bin = (GenRandom_Bin)GetProcAddress(m_libInstance, "SYD_GenerateRandom_Bin");
		m_pSYD_GenSessionKey = (GenSessionKey)GetProcAddress(m_libInstance, "SYD_SM2_GenSessionKey");
		m_pSYD_SM2_Sign = (SM2Sign)GetProcAddress(m_libInstance, "SYD_SM2_Sign_C");
		m_pSYD_SM2_Verify = (SM2_Verify)GetProcAddress(m_libInstance, "SYD_SM2_Verify");
		m_pSYD_SM3_Hash = (SM3Hash)GetProcAddress(m_libInstance, "SYD_SM3_Hash_Nopuk");
		m_pSYD_SM2_ConfirmSessionKey = (ConfirmSessionKey)GetProcAddress(m_libInstance, "SYD_SM2_ConfirmSessionKey_C");

		m_pSYD_SM4_EncryptData = (EncryptData)GetProcAddress(m_libInstance, "SYD_SM4_Encrypt_Data");
		m_pSYD_SM4_DecryptData = (DecryptData)GetProcAddress(m_libInstance, "SYD_SM4_Decrypt_Data");
	}
	else {
		std::string errMsg = "SafeHandler: Load SydApi Failed";
		m_publisher.NotifyUI(errMsg);
		Log::Info(errMsg);
	}

	// 获取交易所公钥
	std::string pubKey = m_config.Credential.TradeAuthCode;
	strcpy_s((char*)m_exchPubKey, sizeof(m_exchPubKey), pubKey.c_str());
	Log::Info(fmt::format("SafeHandler: Load SydApi Success, public key[{}]", pubKey));
}
SafeHandler::~SafeHandler() {
	if (m_libInstance) {
		FreeLibrary(m_libInstance);
		m_libInstance = NULL;
	}
}
bool SafeHandler::EncryptMsg(char const* infoType, char const* oriMsg, int msgLen, unsigned char* encryptMsg, int& encryptLen, unsigned char const* sessionKey) {
	int procSocket = m_socketPool.Apply();
	int nRtn = m_pSYD_SM4_EncryptData(procSocket, sessionKey, (unsigned char*)oriMsg, msgLen, encryptMsg, &encryptLen);
	m_socketPool.Release(procSocket);
	if (0 != nRtn) {		
		std::string errMsg = fmt::format("SafeHandler: encrypt msg failed, rtnCode[{}] infoType[{}] sessionKey[{}] oriMsg[{}]", nRtn, infoType, sessionKey, oriMsg);
		m_publisher.NotifyUI(errMsg);
		Log::Error(errMsg.c_str());
		return false;
	}
	return true;
}
bool SafeHandler::DecryptMsg(char const* encryptMsgs, unsigned int msgLen, char* decryptMsg, int& decryptLen, unsigned char const* sessionKey) {
	int procSocket = m_socketPool.Apply();
	int nRtn = m_pSYD_SM4_DecryptData(procSocket, sessionKey, (unsigned char const*)encryptMsgs, msgLen, (unsigned char*)decryptMsg, &decryptLen);
	m_socketPool.Release(procSocket);
	if (0 != nRtn) {
		std::string errMsg = fmt::format("SafeHandler: decrypt msg failed, rtnCode[{}] sessionKey[{}]", nRtn, sessionKey);
		m_publisher.NotifyUI(errMsg);
		Log::Error(errMsg.c_str());
		return false;
	}
	return true;
}
