
#include "standardapi.h"
#include "Syd_Api_WIN.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 1

#define LOG(format, ...) \
	if(DEBUG)	\
        printf("[DEBUG|%s@%s,%d] " format "\n", \
             __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__ )
             
#define SUCCESS 		0			//success
#define FAIL			-1			//fail
#define NETWORKERROR 	1588		//network error


DLL_DECLARE void* cb_connect_(void *ctx)
{
	int ret = 0;
	int sockfd = -1;
	socket_ctx_t *pSocketFd = NULL;
	encryptor_ctx_t cb_ctx;
	
	if(ctx == NULL)
	{
		LOG("ERR ARG CAN'T BE NULL");
		return NULL;
	}

	memset((char*)&cb_ctx,0,sizeof(encryptor_ctx_t));
	memcpy((char*)&cb_ctx,ctx,sizeof(encryptor_ctx_t));
	
	ret = SYD_Connect(cb_ctx.encryptor_ip,cb_ctx.port,NULL,cb_ctx.timeout,&sockfd);
	if(ret)
	{
		LOG("SYD_Connect ret=%02x",ret);
		return NULL;
	}
	
	pSocketFd = (socket_ctx_t*)malloc(sizeof(socket_ctx_t));
	memcpy((char*)pSocketFd,(char*)&cb_ctx,sizeof(encryptor_ctx_t));
	pSocketFd->sockfd = sockfd;

	return pSocketFd;
}

DLL_DECLARE int cb_disconnect_(void *handle)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd  = NULL;
	pSocketFd = (socket_ctx_t*)handle;
	
	ret = SYD_Disconnect(pSocketFd->sockfd);
	if(ret)
	{
		LOG("SYD_Disconnect ret=%02x",ret);
		return FAIL;
	}
	free(handle);
	handle = NULL;

	return SUCCESS;
}

DLL_DECLARE int cb_reconnect_(void *handle)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t*)handle;

	ret = SYD_ReConnect(pSocketFd->encryptor_ip,pSocketFd->port,pSocketFd->timeout,&pSocketFd->sockfd);	
	if(ret!=0&&ret!=1)
	{
		LOG("SYD_ReConnect ret=%02x",ret);
		return FAIL;
	}
	return SUCCESS;
}

DLL_DECLARE int cb_reset_connect_(void *handle)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t*)handle;

	ret = SYD_Reset(pSocketFd->encryptor_ip,pSocketFd->port,pSocketFd->timeout,&pSocketFd->sockfd);	
	if(ret)
	{
		LOG("SYD_Reset ret=%02x",ret);
		return FAIL;
	}
	return SUCCESS;
}

DLL_DECLARE int cb_is_connected_(void *handle)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t*)handle;

	ret = SYD_Connect_Status(pSocketFd->sockfd);

	return ret;
}

DLL_DECLARE int cb_get_skey_(void *handle , const char *idx, const unsigned char *cipher_key, const unsigned int cipher_key_len, unsigned char *skey, unsigned int *cap)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	unsigned char pKCV[33] = {0};
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;
	
	ret = SYD_SM2_ConfirmSessionKey_C(pSocketFd->sockfd,idx,cipher_key,cipher_key_len,skey,(int *)cap,pKCV);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM2_ConfirmSessionKey_C ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM2_ConfirmSessionKey_C ret=%02x",ret);
		return FAIL;
	}
	else
		return SUCCESS;

}

DLL_DECLARE int cb_encrypt_(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap)
{
	int ret = SUCCESS;	
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}

	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;
	ret = SYD_SM4_Encrypt_Data(pSocketFd->sockfd,skey,src,src_len,dst,(int *)cap);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM4_Encrypt_Data ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM4_Encrypt_Data ret=%02x",ret);
		return FAIL;
	}
	else
		return SUCCESS;

}

DLL_DECLARE int cb_decrypt_(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap)
{
	int ret = SUCCESS;	
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;

	ret = SYD_SM4_Decrypt_Data(pSocketFd->sockfd,skey,src,src_len,dst,(int *)cap);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM4_Decrypt_Data ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM4_Decrypt_Data ret=%02x",ret);
		return FAIL;
	}
	else
		return SUCCESS;

}
DLL_DECLARE int cb_mac_(void *handle, const unsigned char *skey, const unsigned int skey_len, const unsigned char *src, const unsigned int src_len, unsigned char *dst, unsigned int *cap)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;

	ret = SYD_SM4_Data_MAC(pSocketFd->sockfd,skey,src,src_len,dst,(int *)cap);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM4_Data_MAC ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM4_Data_MAC ret=%02x",ret);
		return FAIL;
	}
	else
		return SUCCESS;
}

DLL_DECLARE int cb_hash_ (void *handle, const unsigned char *data, const unsigned int data_len, unsigned char *hash, unsigned int *hash_cap)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;


	ret = SYD_SM3_Hash_Nopuk(pSocketFd->sockfd,data,data_len,hash);	
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM3_Hash_Nopuk ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM3_Hash_Nopuk ret=%02x",ret);
		return FAIL;
	}
	else
	{
		*hash_cap = 32;
		return SUCCESS;
	}
}
DLL_DECLARE int cb_gen_random_(void *handle, const unsigned int len, unsigned char *random)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR ARG CAN'T IS NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;

	ret = SYD_GenerateRandom(pSocketFd->sockfd,len/2,random);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_GenerateRandom ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_GenerateRandom ret=%02x",ret);
		return FAIL;
	}
	else
	{
		return SUCCESS;
	}
}

DLL_DECLARE int cb_get_sign_keypair_(void *handle, const char *idx, void **sign_keypair)
{
	int ret = SUCCESS;
	if(handle == NULL || strlen(idx) > 16)
	{
		LOG("ERR -  handle can't is NULL, or idx lenth greater than 16");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;
	keypair_ctx_t *keypair = (keypair_ctx_t *)malloc(sizeof(keypair_ctx_t));
	memset(keypair, 0, sizeof(keypair_ctx_t));
	keypair->pPubKeyLen=140;
	memcpy(keypair->encryptor_ip,idx,strlen(idx));
	ret = SYD_SM2_Get_PublicKey_C(pSocketFd->sockfd,idx,0,(unsigned char *)keypair->pPubKey,&keypair->pPubKeyLen);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM2_Get_PublicKey_C ret=%02x",ret);
		free(keypair);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM2_Get_PublicKey_C ret=%02x",ret);
		free(keypair);
		return FAIL;
	}
	else
	{	
		*sign_keypair = keypair;
		return SUCCESS;
	}
}

DLL_DECLARE int cb_free_sign_keypair_(void *handle, void *sign_keypair)
{
	
	if(sign_keypair == NULL)
	{
		LOG("ERR -  handle can't is NULL");
		return FAIL;
	}

	free(sign_keypair);
	sign_keypair = NULL;

	return SUCCESS;
}

DLL_DECLARE int cb_sign_ (void *handle, const void *sign_keypair, const unsigned char *data, const unsigned int data_len, unsigned char *signed_data, unsigned int *cap)
{
	int ret = SUCCESS;
	if(handle == NULL || sign_keypair == NULL)
	{
		LOG("ERR -  handle can't is NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;
	keypair_ctx_t *keypair = (keypair_ctx_t *)sign_keypair;

	ret = SYD_SM2_Sign_C(pSocketFd->sockfd,keypair->encryptor_ip,1,(const unsigned char*)keypair->pPubKey,keypair->pPubKeyLen,data,data_len,signed_data,(int *)cap);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM2_Sign_C ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM2_Sign_C ret=%02x",ret);
		return FAIL;
	}
	else
	{
		return SUCCESS;
	}
}

DLL_DECLARE int cb_verify_sign_ (void *handle, const unsigned char *data, const unsigned int data_len, const unsigned char *signed_data, const unsigned int signed_data_len, const unsigned char *pub_key, const unsigned int pub_key_len)
{
	int ret = SUCCESS;
	if(handle == NULL)
	{
		LOG("ERR -  handle can't is NULL");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;
	unsigned char pub_der[140] = "03420004";
	memcpy(pub_der+8, pub_key, pub_key_len);//金仕达公钥未DER编码，需要手动添加03420004

	ret = SYD_SM2_Verify(pSocketFd->sockfd,pub_der,pub_key_len+8,1,data,data_len,signed_data,signed_data_len);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM2_Verify ret=%02x",ret);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM2_Verify ret=%02x",ret);
		return FAIL;
	}
	else
	{
		return SUCCESS;
	}
}

DLL_DECLARE int cb_get_enc_keypair_(void *handle , const char *idx, void **enc_keypair)
{
	int ret = SUCCESS;
	if(handle == NULL || strlen(idx) > 16)
	{
		LOG("ERR -	handle can't is NULL, or idx lenth greater than 16");
		return FAIL;
	}
	socket_ctx_t *pSocketFd = (socket_ctx_t *)handle;
	keypair_ctx_t *keypair = (keypair_ctx_t *)malloc(sizeof(keypair_ctx_t));
	memset(keypair, 0, sizeof(keypair_ctx_t));
	keypair->pPubKeyLen=140;
	memcpy(keypair->encryptor_ip,idx,strlen(idx));

	ret = SYD_SM2_Get_PublicKey_C(pSocketFd->sockfd,idx,1,(unsigned char *)keypair->pPubKey,&keypair->pPubKeyLen);
	if(ret == -1 || ret == -2 || ret == -3)
	{
		LOG("SYD_SM2_Get_PublicKey_C ret=%02x",ret);
		free(keypair);
		return NETWORKERROR;
	}
	else if(ret != 0)
	{
		LOG("SYD_SM2_Get_PublicKey_C ret=%02x",ret);
		free(keypair);
		return FAIL;
	}
	else
	{	
		*enc_keypair = keypair;
		return SUCCESS;
	}
}
DLL_DECLARE int cb_free_enc_keypair_(void *handle, void *enc_keypair)
{
	
	if(enc_keypair == NULL)
	{
		LOG("ERR -	handle can't is NULL");
		return FAIL;
	}

	free(enc_keypair);
	enc_keypair = NULL;

	return SUCCESS;
}


