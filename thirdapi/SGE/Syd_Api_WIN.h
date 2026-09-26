/*==================================================================
 *Copyright(C),2017,Sunyard Tech Co.,Ltd,All rights reversed
 *File name:		sydapi.h
 *Author:			Ren Qiu'an 
 *Version:			1.0
 *Date:				2017.5.22
 *Description:		cmd api Í·ÎÄ¼þ
 *Function List:	
 *OS Platform:	    Windows
 *==================================================================
 */

#ifndef __SYDAPI_FILE__
#define __SYDAPI_FILE__

#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32 
#ifdef SB_EXPORT
#define DLL_DECLARE __declspec(dllexport)
#else
#define DLL_DECLARE __declspec(dllimport)
#endif/*DLL_EXPORT*/

#else 
#define DLL_DECLARE
#endif

DLL_DECLARE int SYD_BIN2HEX(
	const unsigned char* pBin,
	const int nBinLen,
	unsigned char* pHex);

DLL_DECLARE int SYD_HEX2BIN(
	const unsigned char* pHex,
	const int nHexLen,
	unsigned char* pBin);

DLL_DECLARE int SYD_Connect(const char* sIp, const int nPort, const char* sPwdStr, const int nTimeOut, int* pSocketFd);

DLL_DECLARE int SYD_Disconnect(const int nSocketFd);

DLL_DECLARE int SYD_Heart_Beat(const int nSocketFd);



DLL_DECLARE int SYD_Connect_Status(const int nSocketFd);

DLL_DECLARE int SYD_ReConnect(const char* sIp,
	const int nPort,
	const int nTimeOut,
	int* pSocketFd);

DLL_DECLARE	int SYD_Reset(const char* sIp,
	const int nPort,
	const int nTimeOut,
	int* pSocketFd);

DLL_DECLARE int SYD_SM2_GenKeyPair(
	const int nSocketFd,
	unsigned char *pPriKey,
	int *pPriKeyLen,
	unsigned char *pPubKey,
	int *pPubKeyLen
	);

DLL_DECLARE int SYD_SM2_Sign(
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

DLL_DECLARE int SYD_SM2_Verify(
	const int nSocketFd,
	const unsigned char *pPubKey,
	const int nPubKeyLen,
	const int nOrgDataType,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	const unsigned char* pSignData,
	const int nSignDataSize
	);

DLL_DECLARE int SYD_SM2_Encrypt(
	const int nSocketFd,
	const unsigned char *pPubKey,
	const int nPubKeyLen,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	unsigned char* pOutData,
	int *pOutDataSize
	);

DLL_DECLARE int SYD_SM2_Decrypt(
	const int nSocketFd,
	const unsigned char *pPriKey,
	const int nPriKeyLen,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	unsigned char* pOutData,
	int *pOutDataSize
	);

DLL_DECLARE int SYD_SM3_Hash(
	const int nSocketFd,
	const unsigned char *pPubKey,
	const int nPubKeyLen,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	unsigned char* pHash
	);

DLL_DECLARE int SYD_SM3_Hash_Nopuk(
	const int nSocketFd,
	const unsigned char * pOrgData,
	const int nOrgDataSize,
	unsigned char * pHash);

DLL_DECLARE int SYD_SM2_GenSessionKey(
	const int nSocketFd,
	const unsigned char *pPubKey,
	const int nPubKeyLen,
	unsigned char  *pCipherKey,
	int *pCipherKeyLen,
	unsigned char  *pSessionKey,
	int *pSessionKeyLen,
	unsigned char *pKCV
	);

DLL_DECLARE int SYD_SM2_ConfirmSessionKey(
	const int nSocketFd,
	const unsigned char *pPriKey,
	const int nPriKeyLen,
	const unsigned  char *pCipherKey,
	const int nCipherKeyLen,
	unsigned char  *pSessionKey,
	int *pSessionKeyLen,
	unsigned char *pKCV
	);

DLL_DECLARE int SYD_GenerateRandom(
	const int nSocketFd,
	const int nRandomSize,
	unsigned char *pRandom);

DLL_DECLARE int SYD_SM4_Encrypt_Data(
	const int nSocketFd,
	const unsigned char *pSessionKey,
	const unsigned char* pInData,
	const int nInDataSize,
	unsigned char* pOutData,
	int *pOutDataSize
	);

DLL_DECLARE int SYD_SM4_Decrypt_Data(
	const int nSocketFd,
	const unsigned char *pSessionKey,
	const unsigned char* pInData,
	const int nInDataSize,
	unsigned char* pOutData,
	int *pOutDataSize
	);

DLL_DECLARE int SYD_SM4_Data_MAC(
	const int nSocketFd,
	const unsigned char *pSessionKey,
	const unsigned char* pInData,
	const int nInDataSize,
	unsigned char* pOutMac,
	int *pOutMacSize
	);

DLL_DECLARE int SYD_SM2_DetachedSign(
	const int nSocketFd,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	const char* sCertDN,
	unsigned char* pSignData,
	int* pSignDataSize);

DLL_DECLARE int SYD_SM2_DetachedVerify(
	const int nSocketFd,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	const unsigned char* pSignData,
	const int nSignDataSize);

DLL_DECLARE int SYD_SM2_Sign_C(
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

DLL_DECLARE int SYD_SM2_Decrypt_C(
	const int nSocketFd,
	const char *pUserID,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	unsigned char* pOutData,
	int *pOutDataSize);

DLL_DECLARE int SYD_SM2_ConfirmSessionKey_C(
	const int nSocketFd,
	const char *pUserID,
	const unsigned  char *pCipherKey,
	const int nCipherKeyLen,
	unsigned char  *pSessionKey,
	int *pSessionKeyLen,
	unsigned char *pKCV
	);

DLL_DECLARE int SYD_Get_X509_CertInfo(
	const int nSocketFd,
	const unsigned char *pCert,
	const int nCertLen,
	unsigned char *CertInfo,
	int* pCertInfoLen);

DLL_DECLARE int SYD_SM2_Get_PublicKey_C(
	const int nSocketFd,
	const char *pUserID,
	const int  nKeyType,
	unsigned char *pPubKey,
	int *pPubKeyLen);

DLL_DECLARE int SYD_Get_X509_CertFile_C(
	const int nSocketFd,
	const char *pUserID,
	const int  nCertType,
	unsigned char *pCert,
	int *pCertLen);

DLL_DECLARE int SYD_SM2_Sign_SGE(
	const int nSocketFd,
	const int nOrgDataType,
	const unsigned char *pPubKey,
	const int nPubKeyLen,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	unsigned char* pSignData,
	int* pSignDataSize);

DLL_DECLARE int SYD_SM2_Decrypt_SGE(
	const int nSocketFd,
	const unsigned char* pOrgData,
	const int nOrgDataSize,
	unsigned char* pOutData,
	int *pOutDataSize);

DLL_DECLARE int SYD_SM2_ConfirmSessionKey_SGE(
	const int nSocketFd,
	const unsigned  char *pCipherKey,
	const int nCipherKeyLen,
	unsigned char  *pSessionKey,
	int *pSessionKeyLen,
	unsigned char *pKCV);

DLL_DECLARE int SYD_SM2_Get_PublicKey_SGE(
	const int nSocketFd,
	const int nKeyType,
	unsigned char *pPubKey,
	int *pPubKeyLen);

DLL_DECLARE int SYD_Get_X509_CertFile_SGE(
	const int nSocketFd,
	const int  nCertType,
	unsigned char *pCert,
	int *pCertLen);

DLL_DECLARE int SYD_Get_DevInfo(
	int nSocketFd,
	unsigned char *pDevInfo,
	int *pDevInfoLen);

DLL_DECLARE int SYD_Get_SysInfo(
	const int nSocketFd,
	unsigned char *pSysInfo,
	int *pSysInfoLen);

DLL_DECLARE int SYD_Get_ErrMsg(int ErrNo, char *ErrMsg);

DLL_DECLARE int SYD_SM2_PRI2DEK_C(
	const int nSocketFd,
	const char *pUserID,
	const unsigned  char *pCipherKey,
	const int nCipherKeyLen,
	const unsigned char  *pDEK,
	const int nDEKLen,
	unsigned char  *pEncData,
	int *pEncDataLen
	);

DLL_DECLARE int SYD_SM2_PRI2DEK_SGE(
	const int nSocketFd,
	const unsigned  char *pCipherKey,
	const int nCipherKeyLen,
	const unsigned char  *pDEK,
	const int nDEKLen,
	unsigned char  *pEncData,
	int *pEncDataLen
	);

DLL_DECLARE int SYD_SM2_DEK2PUK(
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

DLL_DECLARE int SYD_GenerateRandom_Bin(
	const int nSocketFd,
	const int nRandomSize,
	unsigned char *pRandom);

#ifdef __cplusplus
}
#endif
	
#endif