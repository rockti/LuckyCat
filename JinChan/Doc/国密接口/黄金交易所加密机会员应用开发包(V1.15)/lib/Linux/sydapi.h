/*==================================================================
 *Copyright(C),2016,Sunyard Tech Co.,Ltd,All rights reversed
 *File name:        sydapi.h
 *Author:           Ren Qiu'an
 *Version:          1.0
 *Date:             2017.5.22
 *Description:      cmd api 头文件
 *Function List:
 *History:          null
 *==================================================================
 */

#ifndef __SYDAPI_FILE__
#define __SYDAPI_FILE__

typedef struct
{
    char issuer[257];           /* 颁发者DN*/
    char serialNumber[65];      /* 证书序列号*/
    char subject[257];          /* 证书主题*/
    char notBefore[15];         /* 证书有效期的起始时间*/
    char notAfter[15];          /* 证书有效期的终止时间*/
    char pubkey[257];
} CERT_INFO;


int SYD_Connect(const char* sIp,
				const int nPort,
				const char* sPwdStr,
				const int nTimeOut,
				int* pSocketFd);
int SYD_Disconnect(const int nSocketFd);
int SYD_Heart_Beat(const int nSocketFd);
int SYD_Connect_Status(const int nSocketFd);
int SYD_ReConnect(const char* sIp,
				const int nPort,
				const int nTimeOut,
				int* pSocketFd);
int SYD_Reset(const char* sIp,
				const int nPort,
				const int nTimeOut,
				int* pSocketFd);
int SYD_SM2_GenKeyPair(
    const int nSocketFd,
    unsigned char *pPriKey,
    int *pPriKeyLen,
    unsigned char *pPubKey,
    int *pPubKeyLen
);
int SYD_SM2_Sign(
    const int nSocketFd,
    const unsigned char *pPriKey,
    const int nPriKeyLen,
    const int nOrgDataType,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pSignData,
    int* pSignDataSize
);

int SYD_SM2_Verify(
    const int nSocketFd,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    const int nOrgDataType,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    const unsigned char* pSignData,
    const int nSignDataSize
);

int SYD_SM2_Encrypt(
    const int nSocketFd,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pOutData,
    int *pOutDataSize
);
int SYD_SM2_Decrypt(
    const int nSocketFd,
    const unsigned char *pPriKey,
    const int nPriKeyLen,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pOutData,
    int *pOutDataSize
);

int SYD_SM3_Hash(
    const int nSocketFd,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pHash
);

int SYD_SM3_Hash_Nopuk(
	const int nSocketFd,
	const unsigned char * pOrgData,
	const int nOrgDataSize,
	unsigned char * pHash
);

int SYD_SM2_GenSessionKey(
    const int nSocketFd,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    unsigned char  *pCipherKey,
    int *pCipherKeyLen,
    unsigned char  *pSessionKey,
    int *pSessionKeyLen,
    unsigned char *pKCV
);

int SYD_SM2_ConfirmSessionKey(
    const int nSocketFd,
    const unsigned char *pPriKey,
    const int nPriKeyLen,
    const unsigned  char *pCipherKey,
    const int nCipherKeyLen,
    unsigned char  *pSessionKey,
    int *pSessionKeyLen,
    unsigned char *pKCV
);

int SYD_GenerateRandom(
    const int nSocketFd,
    const int nRandomSize,
    unsigned char *pRandom
);

int SYD_GenerateRandom_Bin(
    const int nSocketFd,
    const int nRandomSize,
    unsigned char *pRandom
);

int SYD_SM4_Encrypt_Data(
    const int nSocketFd,
    const unsigned char *pSessionKey,
    const unsigned char* pInData,
    const int nInDataSize,
    unsigned char* pOutData,
    int *pOutDataSize
);

int SYD_SM4_Decrypt_Data(
    const int nSocketFd,
    const unsigned char *pSessionKey,
    const unsigned char* pInData,
    const int nInDataSize,
    unsigned char* pOutData,
    int *pOutDataSize
);

int SYD_SM4_Data_MAC(
    const int nSocketFd,
    const unsigned char *pSessionKey,
    const unsigned char* pInData,
    const int nInDataSize,
    unsigned char* pOutMac,
    int *pOutMacSize
);

int SYD_SM2_DetachedSign(
	const int nSocketFd,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
    const char* sCertDN,
	unsigned char* pSignData,
	int* pSignDataSize
);
int SYD_SM2_DetachedVerify(
	const int nSocketFd,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
    const unsigned char* pSignData,
	const int nSignDataSize
);
int SYD_SM2_Sign_C(
    const int nSocketFd,
    const char *pUserID,
    const int nOrgDataType,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pSignData,
    int* pSignDataSize
);
int SYD_SM2_Decrypt_C(
    const int nSocketFd,
    const char *pUserID,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pOutData,
    int *pOutDataSize
);
int SYD_SM2_ConfirmSessionKey_C(
    const int nSocketFd,
    const char *pUserID,
    const unsigned  char *pCipherKey,
    const int nCipherKeyLen,
    unsigned char  *pSessionKey,
    int *pSessionKeyLen,
    unsigned char *pKCV
);
int  SYD_Get_X509_CertInfo (
    const int nSocketFd,
    const unsigned char *pCert,
    const int nCertLen,
    unsigned char *CertInfo,
    int* pCertInfoLen
);
int SYD_SM2_Get_PublicKey_C(
    const int nSocketFd,
    const char *pUserID,
    const int  nKeyType,
    unsigned char *pPubKey,
    int *pPubKeyLen
);
int SYD_Get_X509_CertFile_C(
    const int nSocketFd,
    const char *pUserID,
    const int  nCertType,
    unsigned char *pCert,
    int *pCertLen);

int SYD_BIN2HEX (
    const unsigned char* pBin,
    const int nBinLen,
    unsigned char* pHex);
int SYD_HEX2BIN (
    const unsigned char* pHex,
    const int nHexLen,
    unsigned char* pBin);

/*定制指令*/
int SYD_SM2_Sign_SGE(
    const int nSocketFd,
    const int nOrgDataType,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pSignData,
    int* pSignDataSize);
int SYD_SM2_Decrypt_SGE(
    const int nSocketFd,
    const unsigned char* pOrgData,
    const int nOrgDataSize,
    unsigned char* pOutData,
    int *pOutDataSize );
int SYD_SM2_ConfirmSessionKey_SGE(
    const int nSocketFd,
    const unsigned  char *pCipherKey,
    const int nCipherKeyLen,
    unsigned char  *pSessionKey,
    int *pSessionKeyLen,
    unsigned char *pKCV);
int SYD_SM2_Get_PublicKey_SGE(
    const int nSocketFd,
    const int nKeyType,
    unsigned char *pPubKey,
    int *pPubKeyLen);
int SYD_Get_X509_CertFile_SGE(
    const int nSocketFd,
    const int  nCertType,
    unsigned char *pCert,
    int *pCertLen);
int SYD_Get_DevInfo(
    const int nSocketFd,
    unsigned char *pDevInfo,
    int *pDevInfoLen);
int SYD_Get_SysInfo(
    const int nSocketFd,
    unsigned char *pSysInfo,
    int *pSysInfoLen);
//2017.11.24
int SYD_ComposeKeyByLmk(
    const int nSocketFd,
    const unsigned char *pKeyPlain,
    const int nKeyPlainLen,
    unsigned char  *pKey1Lmk,
    int *pKey1LmkLen,
    unsigned char *pKCV);
/*************************/
int SYD_SM2_PRI2DEK_C(
    const int nSocketFd,
    const char *pUserID,
    const unsigned  char *pCipherKey,
    const int nCipherKeyLen,
    const unsigned char  *pDEK,
    const int nDEKLen,
    unsigned char  *pEncData,
    int *pEncDataLen
);
int SYD_SM2_PRI2DEK_SGE(
    const int nSocketFd,
    const unsigned  char *pCipherKey,
    const int nCipherKeyLen,
    const unsigned char  *pDEK,
    const int nDEKLen,
    unsigned char  *pEncData,
    int *pEncDataLen
);
int SYD_SM2_DEK2PUK(
    const int nSocketFd,
    const unsigned char  *pDEK,
    const int nDEKLen,
    const unsigned char  *pEncData,
    const int nEncDataLen,
    const unsigned char *pPubKey,
    const int nPubKeyLen,
    unsigned  char *pCipherKey,
    int *pCipherKeyLen
);
void PrintStr(char *Title,const char *fmt,...);
int  PrintHex(char *Title,unsigned char *Data,int DataLen);
#endif
