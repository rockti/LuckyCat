#ifndef _STANDARDAPI_H_
#define _STANDARDAPI_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32 
#ifdef DLL_EXPORT
#define DLL_DECLARE __declspec(dllexport)
#else
#define DLL_DECLARE __declspec(dllimport)
#endif

#else 
#define DLL_DECLARE
#endif 


#pragma pack(4)
typedef struct encryptor_ctx_s {
	char encryptor_ip[16];
	int port;
	int timeout;
	int reserve0;
	char reserve1[64];
}encryptor_ctx_t;
#pragma pack()

#pragma pack(4)
typedef struct socket_ctx_s {
	char encryptor_ip[16];
	int port;
	int timeout;
	int reserve0;
	char reserve1[64];
	int sockfd;
}socket_ctx_t;
#pragma pack()

#pragma pack(4)
typedef struct socket_keypair_ctx_s {
	char encryptor_ip[32];
	int  pPubKeyLen;
	char pPubKey[140];
}keypair_ctx_t;
#pragma pack()


/************************************
功能说明
连接硬加密设备回调函数
函数原型
void* cb_connect_(void *ctx);
参数
ctx:		输入参数，用户自定义的和加密设备相关的上下文信息，如下结构体：
返回值
void*:		成功，返回加密设备连接句柄
NULL:		失败
************************************/
DLL_DECLARE void* cb_connect_(void *ctx);

/************************************
功能说明
断开硬加密设备连接回调函数
函数原型
int cb_disconnect_(void *handle);
参数
handle:		输入参数，加密设备连接句柄
返回值
0:			成功
-1:			失败
************************************/
DLL_DECLARE int cb_disconnect_(void *handle);

/************************************
功能说明
硬加密设备重新连接回调函数。若原连接可用，则使用原连接；否则断开原连接进行重连。
函数原型
int cb_reconnect_(void *handle);
参数
handle:		输入参数，加密设备连接句柄
返回值
0:			成功
非0:		重连不成功
************************************/
DLL_DECLARE int cb_reconnect_(void *handle);

/************************************
功能说明
重置硬加密设备的连接。无论原连接是否可用，断开原连接进行重连。
函数原型
int cb_reset_connect_(void *handle);
参数
handle:		输入参数，加密设备连接句柄
返回值
0:			成功
-1:			失败
************************************/
DLL_DECLARE int cb_reset_connect_(void *handle);

/************************************
功能说明
获取硬加密设备连接状态
函数原型
int cb_is_connected_(void *handle);
参数
handle:		输入参数，加密设备连接句柄
返回值
1:			加密设备处于连接状态
0:			加密设备处于断连状态
其他：		获取状态失败
************************************/
DLL_DECLARE int cb_is_connected_(void *handle);

/*
功能说明
加密原始数据的回调函数
函数原型
int cb_encrypt_ (void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
参数
handle:		输入参数，加密设备连接句柄
skey:		输入参数，会话密钥
skey_len:	输入参数，会话密钥长度
src:		输入参数，待加密的原始数据
src_len:	输入参数，待加密的原始数据长度
dst:		输出参数，密文，二进制编码
cap:		输出参数，密文长度
返回值
0:			成功
1588:		与加密机网络错误
-1:			失败
*/
DLL_DECLARE int cb_encrypt_ (void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);

/*
功能说明
解密密文数据的回调函数
函数原型
int cb_decrypt_(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
参数
handle:		输入参数，加密设备连接句柄
skey:		输入参数，会话密钥
skey_len:	输入参数，会话密钥长度
src:		输入参数，密文
src_len:	输入参数，密文长度
dst:		输出参数，明文
cap:		输出参数，明文长度
返回值
0:			成功
1588:		与加密机网络错误
-1:			失败
*/
DLL_DECLARE int cb_decrypt_(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);

/*
功能说明
计算数据MAC的回调函数
函数原型
int cb_mac_ (void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);
参数
handle:		输入参数，加密设备连接句柄
skey:		输入参数，会话密钥
skey_len:	输入参数，会话密钥长度
src:		输入参数，待加密的原始数据
src_len:	输入参数，待加密的原始数据长度
dst:		输出参数，计算出的MAC，二进制编码
cap:		输出参数，MAC长度
返回值
0:			成功
1588:		与加密机网络错误
-1:			失败
*/
DLL_DECLARE int cb_mac_ (void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap);

/*
功能说明
计算数据的散列值的回调函数
函数原型
int cb_hash_ (void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap);
参数
handle:		输入参数，加密设备连接句柄
data:		输入参数，待计算原始数据
data_len:	输入参数，待计算原始数据长度
hash:		输出参数，HASH值，二进制编码
hash_cap:	输出参数，HASH数据长度
返回值
0:			成功
1588:		与加密机网络错误
-1:			失败
*/
DLL_DECLARE int cb_hash_ (void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap);

/*
功能说明
对原始数据进行数字签名的回调函数
函数原型
int cb_sign_ (void *handle, const void *sign_keypair, const unsigned char *data, const unsigned int data_len, unsigned char *signed_data, unsigned int *cap);
参数
handle:				输入参数，加密设备连接句柄
sign_keypair:		输入参数，签名密钥对
data:				输入参数，待签名的原始数据
data_len:			输入参数，待签名的原始数据长度
signed_data:		输出参数，签名数据，二进制编码
cap:				输出参数，签名数据长度
返回值
0:					成功
1588:				与加密机网络错误
-1:					失败
*/
DLL_DECLARE int cb_sign_ (void *handle, const void *sign_keypair, const unsigned char *data, const unsigned int data_len, unsigned char *signed_data, unsigned int *cap);

/*
功能说明
用指定的签名公钥对原始数据进行数字签名验证的回调函数
函数原型
int cb_verify_sign_ (void *handle, const unsigned char *data, const unsigned int data_len, const unsigned char *signed_data, const unsigned int signed_data_len, const unsigned char *pub_key, const unsigned int pub_key_len);
参数
handle:				输入参数，加密设备连接句柄
data:				输入参数，待验证的原始数据
data_len:			输入参数，待验证的原始数据长度
signed_data:		输入参数，待验证的签名数据，二进制编码
signed_data_len:	输入参数，待验证的签名数据长度
pub_key:			输入参数，签名公钥
pubkey_len:			输入参数，签名公钥长度
返回值
0:					验签成功
1588:				与加密机网络错误
-1:					验签失败
*/
DLL_DECLARE int cb_verify_sign_ (void *handle, const unsigned char *data, const unsigned int data_len, const unsigned char *signed_data, const unsigned int signed_data_len, const unsigned char *pub_key, const unsigned int pub_key_len);

/*
功能说明
解密用加密公钥加密的会话密钥密文，生成转加密后的会话密钥，用于本地应用数据的加解密
函数原型
int cb_get_skey_(void *handle , const char *idx, const unsigned char *cipher_key, const unsigned int cipher_key_len, unsigned char *skey, unsigned int *cap);
参数
handle:			输入参数，加密设备连接句柄
idx:			输入参数，获取加密私钥索引
cipher_key:		输入参数，加密公钥加密的会话密钥密文，HEX编码
cipher_key_len:	输入参数，加密公钥加密的会话密钥密文长度
skey:			输出参数，转加密后的会话密钥，HEX编码
cap:			输出参数，转加密后的会话密钥长度，最大输出长度为63。
注：
如果skey存储在加密机内部，skey和cap参数可以不用输出。
返回值
0:				成功
1588:			与加密机网络错误
-1:				失败
*/
DLL_DECLARE int cb_get_skey_(void *handle , const char *idx, const unsigned char *cipher_key, const unsigned int cipher_key_len, unsigned char *skey, unsigned int *cap);

/*
功能说明
生成要求长度的随机数的回调函数
函数原型
int cb_gen_random_(void *handle, const unsigned int len, unsigned char *random);
参数
handle:		输入参数，加密设备连接句柄
len:		输入参数，要求生成的随机数长度
random:		输出参数，生成的随机 数， HEX编码
返回值
0:			成功
1588:		与加密机网络错误
-1:			失败
*/
DLL_DECLARE int cb_gen_random_(void *handle, const unsigned int len, unsigned char *random);

/*
功能说明
获取加密机中加密证书中的密钥对。
函数原型
int cb_get_enc_keypair_(void *handle , const char *idx, void **enc_keypair);
参数
handle:			输入参数，加密设备连接句柄.
idx:			输入参数，获取加密密钥对索引
enc_keypair:	输出参数，加密密钥对指针的地址
返回值
0:				成功
1588:			与加密机网络错误
-1:				失败
*/
DLL_DECLARE int cb_get_enc_keypair_(void *handle , const char *idx, void **enc_keypair);

/*
功能说明
释放签名密钥信息资源。
函数原型
int cb_free_enc_keypair_(void *handle, void *enc_keypair);
参数
handle:			输入参数，加密设备连接句柄.
sign_keypair:	输入参数，加密密钥信息
返回值
0:				成功
-1:				失败
*/
DLL_DECLARE int cb_free_enc_keypair_(void *handle, void *enc_keypair);

/*
功能说明
仅用于获取加密机中客户端签名证书中的密钥信息，用于客户端签名。
函数原型
int cb_get_sign_keypair_(void *handle, const char *idx, void **sign_keypair);
参数
handle:			输入参数，加密设备连接句柄.
idx:			输入参数，获取签名密钥对索引
sign_keypair:	输出参数，签名密钥对指针的地址，用作cb_sign_函数的输入
返回值
0:				成功
1588:			与加密机网络错误
-1:				失败
*/
DLL_DECLARE int cb_get_sign_keypair_(void *handle, const char *idx, void **sign_keypair);

/*
功能说明
仅用于释放客户端签名密钥信息资源。
函数原型
int cb_free_sign_keypair_(void *handle, void *sign_keypair);
参数
handle:			输入参数，加密设备连接句柄.
sign_keypair:	输入参数，签名密钥信息
返回值
0:				成功
-1:				失败
*/
DLL_DECLARE int cb_free_sign_keypair_(void *handle, void *sign_keypair);

#ifdef __cplusplus
}
#endif
#endif

