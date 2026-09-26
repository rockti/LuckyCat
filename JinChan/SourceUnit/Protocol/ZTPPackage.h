#pragma once
#include "FieldSet.h"
#include "ServiceDef.h"

#define ZTP_FIELDTYPE_SIZE		10
#define ZTP_PACKAGE_MAX_SIZE	2048

const BYTE ZTP_REQUEST		= 'R';		//请求
const BYTE ZTP_RESPONSE		= 'O';		//应答
const BYTE ZTP_PUBLISH		= 'P';		//发布

const BYTE CHAIN_CONTINUE	= 'C';		//还有后续的ZTP报文
const BYTE CHAIN_LAST		= 'L';		//最后一个ZTP报文

//ZTP报文头结构,只内部使用，不在网络传输，不需要change endian
const int FTD_GROUP = 0;
const int ZTP_GROUP = 1;
struct	TZTPHeader
{
	TZTPHeader()	{ memset(this, 0 ,sizeof(TZTPHeader)); }
	BYTE	Type;				//包类型（'P'发布，'R'请求，'O'应答）
	BYTE	Chain;				//ZTP报文的连续标志
	WORD	ContentLength;		//除报头之外，各field长度和(信息正文长度)
	DWORD	SubjectID;			//主题
	DWORD	TransactionId;		//信息正文类型(TID)
	DWORD	CommPhaseNo;		//通讯阶段序号,每次更换序号，SequenceNo都从1重新开始
	DWORD	SequenceNo;			//ZTP报文的序号
	DWORD	RequestID;			//请求ID
	DWORD	SessionID;			//请求发起者在前置的会话ID
	DWORD	GroupID;			//组标识，用来区分是FTD来的还是新的ZTP来的请求。0是FTD,1是ZTP

	inline void ToStream(char* pStream)	{
		memcpy(pStream, this, sizeof(TZTPHeader));
	}
	inline void FromStream(char* pStream) {
		memcpy(this, pStream, sizeof(TZTPHeader));
	}
};
int const ZTP_HEAD_LEN	= sizeof(TZTPHeader);

using namespace ztp;
//定义了ZTPProtocol协议层所识别的Package
class  CZTPPackage : public CFieldSet
{
public:
	CZTPPackage();
	virtual ~CZTPPackage();
	
	virtual DWORD GetActiveID();	
	virtual int ValidPackage();	
	virtual int MakePackage();	//转换成流格式

	TZTPHeader* GetZTPHeader()	{ return &m_ZTPHeader; }

	//组装没有自己buf的数据包
	int MakeNoBufPackage();

	//初始化成请求报文
	void PrepareRequest(DWORD pId, BYTE chain = CHAIN_LAST);	
	//根据请求报文初始化成应答报文
	void PrepareResponse(CZTPPackage* pRequest, DWORD pId, BYTE chain = CHAIN_LAST);	
	//初始化成发布报文
	void PreparePublish(DWORD pId, BYTE chain = CHAIN_LAST);

	void SetSubjectID(DWORD SubjectID)	{ m_ZTPHeader.SubjectID = SubjectID; }
	DWORD GetSubjectID()	{ return m_ZTPHeader.SubjectID; }
	void SetTID(DWORD TID)	{ m_ZTPHeader.TransactionId = TID; }
	DWORD GetTID()	{ return m_ZTPHeader.TransactionId; }
	void SetChain(BYTE chain)	{ m_ZTPHeader.Chain = chain; }
	BYTE GetChain()	{ return m_ZTPHeader.Chain; }
	void SetSequenceNo(DWORD SequenceNo)	{ m_ZTPHeader.SequenceNo = SequenceNo; }
	DWORD GetSequenceNo()	{ return m_ZTPHeader.SequenceNo; }
	void SetRequestID(DWORD RequestID)	{ m_ZTPHeader.RequestID = RequestID; }
	DWORD GetRequestID()	{ return m_ZTPHeader.RequestID; }
	void SetSessionID(DWORD SessionID)	{ m_ZTPHeader.SessionID = SessionID; }
	DWORD GetSessionID()	{ return m_ZTPHeader.SessionID; }
	void SetGroupID(DWORD GroupID)	{ m_ZTPHeader.GroupID = GroupID; }
	DWORD GetGroupID()	{ return m_ZTPHeader.GroupID; }

	static CZTPPackage* CreatePackage(int nHeaderReserve);

protected:
	TZTPHeader m_ZTPHeader;	//报文头
};

#define ZTP_ADD_FIELD(pPackage, pField) ((pPackage)->AddField(&((pField)->m_Describe), (pField)))
#define ZTP_GET_SINGLE_FIELD(pPackage, pField)	((pPackage)->GetSingleField(&((pField)->m_Describe), (pField)))
