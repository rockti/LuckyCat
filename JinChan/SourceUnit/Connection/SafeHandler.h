#pragma once
#include "Mutex.h"

#include <atomic>
#include <condition_variable>
#define BOOST_NO_MT
#include <boost/pool/pool_alloc.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>

#include <tuple>

typedef boost::container::deque_options<boost::container::block_size<128u>>::type block_128_option_t;

template<class T>
using Deque = boost::container::deque<T, void, block_128_option_t>;



int const MAX_SAFE_HANDLER_COUNT = 5;
struct Socket {
	int socket;
};

template <typename T>
class blockingqueue {
private:
	std::mutex              d_mutex;
	std::condition_variable d_condition;
	Deque<T>           d_queue;
public:
	void push(T const& value) {
		{
			std::unique_lock<std::mutex> lock(this->d_mutex);
			d_queue.push_front(value);
		}
		this->d_condition.notify_one();
	}
	T pop() {
		std::unique_lock<std::mutex> lock(this->d_mutex);
		this->d_condition.wait(lock, [=] { return !this->d_queue.empty(); });
		T rc(std::move(this->d_queue.back()));
		this->d_queue.pop_back();
		return rc;
	}
	T last() {
		return this->d_queue.back();
	}
	size_t size() const {
		return this->d_queue.size();
	}
};

class SocketPool {
public:
	void Init(int socket) {
		if (sockets_.size() < MAX_SAFE_HANDLER_COUNT) {
			queue_.push(socket);
			sockets_.push_back(socket);
		}
	}

	int Apply() {
		return queue_.pop();
	}

	void Release(int socket) {
		queue_.push(socket);
	}

	int Last() {
		return queue_.last();
	}
	std::vector<int> const& All() const {
		return sockets_;
	}

	size_t remain_count() const {
		return queue_.size();
	}
private:
	std::vector<int> sockets_;
	blockingqueue<int> queue_;
	int count_ = 0;
};

class SafeSocketPool {
	struct HR {
		int PHandler;
		bool Used;
	};

public:
	SafeSocketPool() :m_socketCount(0), m_usedCount(0) {
	}

	void Init(int pSocket) {
		if (m_socketCount < MAX_SAFE_HANDLER_COUNT) {
			m_handlerArry[m_socketCount].PHandler = pSocket;
			m_handlerArry[m_socketCount].Used = false;
			++m_socketCount;
			sockets_.push_back(pSocket);
		}
	}

	int Apply() {
		if (m_usedCount == m_socketCount) {
			m_mutex.Lock();
		}
		for (int i = 0; i < m_socketCount; ++i) {
			if (!m_handlerArry[i].Used) {
				++m_usedCount;
				m_handlerArry[i].Used = true;
				return m_handlerArry[i].PHandler;
			}
		}
		return 0;
	}
	void Release(int socket) {
		for (int i = 0; i < m_socketCount; ++i) {
			if (socket == m_handlerArry[i].PHandler) {
				m_handlerArry[i].Used = false;
				--m_usedCount;
				break;
			}
		}
		if (m_socketCount - m_usedCount > 0) {
			m_mutex.UnLock();
		}
	}

	int GetSocket() {
		if (m_socketCount > 0) {
			return m_handlerArry[--m_socketCount].PHandler;
		}
		return 0;
	}

	std::vector<int> const& All() const {
		return sockets_;
	}

private:
	std::vector<int> sockets_;
	HR m_handlerArry[MAX_SAFE_HANDLER_COUNT];
	int m_socketCount;
	int m_usedCount;
	CMutex m_mutex;
};

typedef struct encryptor_context {
	char Address[16];
	int Port;
} SafeContext;

typedef struct keyPair_context {
	char ID[32];
	char Key[140];
	int Len;
} KeyPairContext;

class SafeHandler
{
public:
	static SafeHandler* m_pThis;
	static void* cb_connect(void* ctx);
	static int cb_disconnect(void* handle);
	static int cb_reconnect(void* handle);
	static int cb_resetConnect(void* handle);
	static int cb_isConnected(void* handle);

	//加密数据
	static int cb_encrypt_data(void* handle, const unsigned char* skey, const unsigned int skey_len, const unsigned char* src, const unsigned int src_len, unsigned char* dst, unsigned int* cap);
	//解密数据
	static int cb_decrypt_data(void* handle, const unsigned char* skey, const unsigned int skey_len, const unsigned char* src, const unsigned int src_len, unsigned char* dst, unsigned int* cap);
	//计算MAC
	static int cb_mac_data(void* handle, const unsigned char* skey, const unsigned int skey_len, const unsigned char* src, const unsigned int src_len, unsigned char* dst, unsigned int* cap);
	//计算Hash
	static int cb_hash_data (void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap);
	//数字签名
	static int cb_sign_data(void* handle, const void* sign_keypair, const unsigned char* data, const unsigned int data_len, unsigned char* signed_data, unsigned int* cap);
	//签名验证
	static int cb_verify_signed_data(void* handle, const unsigned char* data, const unsigned int data_len, const unsigned char* signed_data, unsigned int signed_data_len, const unsigned char* pub_key, const unsigned int pub_key_len);
	//获取会话密钥
	static int cb_get_skey(void* handle, const char* idx, const unsigned char* cipher_key, const unsigned int cipher_key_len, unsigned char* skey, unsigned int* cap);
	//生成随机数
	static int cb_gen_random(void* handle, const unsigned int len, unsigned char* random);
	//获取签名公钥信息
	static int cb_get_sign_keypair(void* handle, const char* idx, void** sign_keypair);
	//释放签名公钥信息
	static int cb_free_sign_keypair(void* handle, void* sign_keypair);
	//获取加密密钥信息(暂未使用)
	static int cb_get_enc_keypair(void* handle, const char* idx, void** enc_keypair);
	//释放加密密钥信息(暂未使用)
	static int cb_free_enc_keypair(void* handle, void* enc_keypair);

	SafeHandler(char const* address, int port);
	~SafeHandler();
	
	int GetPort() const { return m_port; }
	char const* GetAddress() { return m_address.c_str(); }
	int ApplySocket() { return m_socketPool.Apply(); }
	void ReleaseSocket(int socket) { return m_socketPool.Release(socket); }
	unsigned char const* GetSessionKey() { return m_sessionKey; }

	bool EncryptMsg(char const* infoType, char const* oriMsg, int msgLen, unsigned char* encryptMsg, int& encryptLen, unsigned char const* sessionKey);
	bool DecryptMsg(char const* encryptMsgs, unsigned int msgLen, char* decryptMsg, int& decryptLen, unsigned char const* sessionKey);
	void Log();
public:
	int (*m_pSYD_GenRandom_Bin)(const int nSocketFd, const int nRandomSize, unsigned char* pRandom);
	int (*m_pSYD_SM3_Hash)(const int nSocketFd, const unsigned char* pOrgData, const int nOrgDataSize, unsigned char* pHash);
	int (*m_pSYD_SM2_GetPublicKey)(const int nSocketFd, const char* pUserID, const int nKeyType, unsigned char* pPubKey, int* pPubKeyLen);
private:
	typedef int(*GenRandom_Bin)(const int nSocketFd, const int nRandomSize, unsigned char* pRandom);
	typedef int(*SM3Hash)(const int nSocketFd, const unsigned char* pOrgData, const int nOrgDataSize, unsigned char* pHash);
	typedef int(*GetPublicKey)(const int nSocketFd, const char* pUserID, const int nKeyType, unsigned char* pPubKey, int* pPubKeyLen);

	int (*m_pSYD_Connect)(const char* sIp, const int nPort, const char* sPwdStr, const int nTimeOut, int* pSocketFd);
	typedef int(*Connect)(const char* sIp, const int nPort, const char* sPwdStr, const int nTimeOut, int* pSocketFd);
	int (*m_pSYD_Disconnect)(const int nSocketFd);
	typedef int(*Disconnect)(const int nSocketFd);
	int (*m_pSYD_Reconnect)(const char* sIp, const int nPort, const int nTimeOut, int* pSocketFd);
	typedef int(*Reconnect)(const char* sIp, const int nPort, const int nTimeOut, int* pSocketFd);
	int (*m_pSYD_Reset)(const char* sIp, const int nPort, const int nTimeOut, int* pSocketFd);
	typedef int(*Reset)(const char* sIp, const int nPort, const int nTimeOut, int* pSocketFd);
	int (*m_pSYD_ConnectStatus)(const int nSocketFd);
	typedef int(*ConnectStatus)(const int nSocketFd);
	
	//生成随机数(随机数固定长度32，不包含\0)
	int (*m_pSYD_GenRandom)(const int nSocketFd, const int nRandomSize, unsigned char* pRandom);
	typedef int(*GenRandom)(const int nSocketFd, const int nRandomSize, unsigned char* pRandom);

	//生成身份认证信息
	int (*m_pSYD_GenSessionKey)(const int nSocketFd, const unsigned char* pPubKey, const int nPubKeyLen,
		unsigned char* pCipherKey, int* pCipherKeyLen, unsigned char* pSessionKey, int* pSessionKeyLen, unsigned char* pKCV);
	typedef int(*GenSessionKey)(const int nSocketFd, const unsigned char* pPubKey, const int nPubKeyLen,
		unsigned char* pCipherKey, int* pCipherKeyLen, unsigned char* pSessionKey, int* pSessionKeyLen, unsigned char* pKCV);

	//签名
	int (*m_pSYD_SM2_Sign)(const int nSocketFd, const char* pUserID, const int nOrgDataType, const unsigned char* pPubKey, const int nPubKeyLen,
		const unsigned char* pOrgData, const int nOrgDataSize, unsigned char* pSignData, int* pSignDataSize);
	typedef int(*SM2Sign)(const int nSocketFd, const char* pUserID, const int nOrgDataType, const unsigned char* pPubKey, const int nPubKeyLen,
		const unsigned char* pOrgData, const int nOrgDataSize, unsigned char* pSignData, int* pSignDataSize);
	//验签
	int (*m_pSYD_SM2_Verify)(const int nSocketFd, const unsigned char* pPubKey, const int nPubKeyLen, const int nOrgDataType, const unsigned char* pOrgData, const int nOrgDataSize, const unsigned char* pSignData, const int nSignDataSize);
	typedef int(*SM2_Verify)(const int nSocketFd, const unsigned char* pPubKey, const int nPubKeyLen, const int nOrgDataType, const unsigned char* pOrgData, const int nOrgDataSize, const unsigned char* pSignData, const int nSignDataSize);

	//解析会话密钥
	int (*m_pSYD_SM2_ConfirmSessionKey)(int const nSocketFd, char const* pUserID, const unsigned char* pCipherKey, const int nCipherKeyLen, unsigned char* pSessionKey, int* pSessionKeyLen, unsigned char* pKCV);
	typedef int(*ConfirmSessionKey)(int const nSocketFd, char const* pUserID, const unsigned char* pCipherKey, const int nCipherKeyLen, unsigned char* pSessionKey, int* pSessionKeyLen, unsigned char* pKCV);

	int (*m_pSYD_SM4_EncryptData)(const int nSocketFd, const unsigned char* pSessionKey, const unsigned char* pInData, const int nInDataSize, unsigned char* pOutData, int* pOutDataSize);
	typedef int(*EncryptData)(const int nSocketFd, const unsigned char* pSessionKey, const unsigned char* pInData, const int nInDataSize, unsigned char* pOutData, int* pOutDataSize);
	int (*m_pSYD_SM4_DecryptData)(const int nSocketFd, const unsigned char* pSessionKey, const unsigned char* pInData, const int nInDataSize, unsigned char* pOutData, int* pOutDataSize);
	typedef int(*DecryptData)(const int nSocketFd, const unsigned char* pSessionKey, const unsigned char* pInData, const int nInDataSize, unsigned char* pOutData, int* pOutDataSize);

	std::string m_address;
	int m_port;
	HINSTANCE m_libInstance;
	SocketPool m_socketPool;
	unsigned char m_exchPubKey[140];		//交易所公钥
	unsigned char m_sessionKey[34] = "";			//加密密钥(每个Session对应一个Key)
};
