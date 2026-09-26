#pragma once

#include "FieldSet.h"
//#include "HashMap.h"
#include "UserDataType.h"
#include "PackageDefine.h"

#define FTD_PACKAGE_MAX_SIZE	4000

BYTE const FTD_CHAIN_CONTINUE	= 'C';		//报文链的中间FTD报文。
BYTE const FTD_CHAIN_LAST		= 'L';		//报文链的最后一个FTD报文。

//FTD报文头结构
struct	FTDHeader
{
	FTDHeader()	{ memset(this, 0 ,sizeof(FTDHeader)); }
	BYTE	Chain;				//报文链	1字节	ASCII码字符。
	WORD	SequenceSeries;		//序列类别号	2字节	二进制无符号短整数。
	WORD	FieldCount;			//数据域数量	2字节	二进制无符号短整数。
	WORD	FTDContentLength;	//信息正文长度	2字节	二进制无符号短整数。
	DWORD	TransactionID;		//信息正文类型	4字节	二进制无符号整数。(TID)
	DWORD	SequenceNumber;		//序列号	4字节	二进制无符号整数。
	DWORD	RequestID;			//请求编号  4字节   二进制无符号整数。(由发送请求者维护，应答中会带回) 
	DWORD	InstrumentIDCount;	//合约号的整数值，不用解包就知道是哪个合约  4 二进制无符号整数。
	
	inline void ChangeEndian()	
	{
		CHANGE_ENDIAN(SequenceSeries);
		CHANGE_ENDIAN(FieldCount);
		CHANGE_ENDIAN(FTDContentLength);
		CHANGE_ENDIAN(TransactionID);
		CHANGE_ENDIAN(SequenceNumber);
		CHANGE_ENDIAN(RequestID);
		CHANGE_ENDIAN(InstrumentIDCount);
	}
	inline void ToStream(char* pStream)
	{
		memcpy(pStream, this, sizeof(FTDHeader));
		((FTDHeader*)pStream)->ChangeEndian();
	}
	inline void FromStream(char* pStream)
	{
		memcpy(this, pStream, sizeof(FTDHeader));
		ChangeEndian();
	}
};
int const FTD_HEAD_LEN	= sizeof(FTDHeader);

typedef std::map<DWORD,CInstrumentIDType> CSubInstrumentMap;	//合约号订阅

using namespace FTD_TID_DEFINE;
//定义了FTDProtocol协议层所识别的Package
class  CFTDPackage : public CFieldSet
{
public:
	CFTDPackage();
	virtual ~CFTDPackage();
	
	virtual DWORD GetActiveID();
	virtual int ValidPackage();	
	virtual int MakePackage();

	inline FTDHeader* GetFTDHeader()	{ return &m_FTDHeader; }	
	inline DWORD GetTID()	{ return m_FTDHeader.TransactionID; }
	inline void SetChain(BYTE chain)	{ m_FTDHeader.Chain = chain; }
	inline BYTE GetChain()	{ return m_FTDHeader.Chain; }	

	inline void SetRequestID(DWORD requestID)	{ m_FTDHeader.RequestID = requestID; }
	inline DWORD GetRequestID()	{ return m_FTDHeader.RequestID; }	

	inline void SetInstrumentIDCount(DWORD instrumentIDCount)	{ m_FTDHeader.InstrumentIDCount = instrumentIDCount; }
	inline DWORD GetInstrumentIDCount()	{ return m_FTDHeader.InstrumentIDCount; }

	void PreparePackage(DWORD tid, BYTE chain);
	void PrepareResponse(CFTDPackage* pRequest, DWORD tid, BYTE chain);

	static CFTDPackage* CreatePackage(int nHeaderReserve);
protected:
	FTDHeader m_FTDHeader;	  //FTD报头
};

#define FTD_ADD_FIELD(pPackage, pField) ((pPackage)->AddField(&((pField)->m_Describe), (pField)))
#define FTD_GET_SINGLE_FIELD(pPackage, pField)	((pPackage)->GetSingleField(&((pField)->m_Describe), (pField)))
