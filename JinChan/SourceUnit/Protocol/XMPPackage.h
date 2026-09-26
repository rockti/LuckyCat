#pragma once

#include "Package.h"
#include "FieldDescribe.h"

#define XMP_EXTHEAD_MAX_SIZE	127
#define XMP_PACKAGE_MAX_SIZE	4096

#define XMPTagNone				0x00
#define XMPTagKeepAlive			0x01
#define XMPTagHeartbeatTimeOut	0x02

struct TXMPHeader
{
	BYTE Type;			//标示上层协议的协议ID
	BYTE ExtensionLen;	//扩展报文长度
	WORD Length;		//包体长度
	
	inline void Init()	{ memset(this, 0 ,sizeof(TXMPHeader)); }
	inline void ChangeEndian()	{ CHANGE_ENDIAN(Length); }
	inline void ToStream(char* pStream)
	{
		memcpy(pStream, this, sizeof(TXMPHeader));
		((TXMPHeader*)pStream)->ChangeEndian();
	}
	inline void FromStream(char* pStream)
	{
		memcpy(this, pStream, sizeof(TXMPHeader));
		ChangeEndian();
	}
};

//扩展XMP报头
struct TXMPExtHeader
{
	BYTE Tag;		//状态标示
	BYTE TagLen;	//扩展包的长度
	BYTE Data[XMP_EXTHEAD_MAX_SIZE];	//扩展包包体
	inline void Init()	{ memset(this, 0 ,sizeof(TXMPExtHeader)); }
};

int const XMPHLEN = sizeof(TXMPHeader);
int const XMPEXTHLEN = 2;

//定义了XMPProtocol协议层所识别的Package
class  CXMPPackage : public CPackage  
{
public:
	CXMPPackage();
	virtual ~CXMPPackage();
	
	//获取上层协议的编号
	virtual DWORD GetActiveID();
	virtual int ValidPackage();
	virtual int MakePackage();

	inline TXMPHeader* GetXMPHeader()	{ return &m_XMPHeader; }
	inline int GetBodyLength()	{ return m_XMPHeader.Length; }
	//获得扩展报头的长度
	inline BYTE GetExtensionLen()	{ return m_XMPHeader.ExtensionLen; }	
	//获得扩展报头的标示
	inline BYTE GetExtTag()	{ return m_ExtHeader.Tag; }
	//获得扩展报头内容的长度
	inline BYTE GetExtTagLen()	{ return m_ExtHeader.TagLen; }
	//获得扩展报头的内容
	inline BYTE* GetExtData()	{ return m_ExtHeader.Data; }

	/**设置扩展报头内容
	*@param Tag 扩展报头的标示
	*@param TagLen 扩展报头内容的长度
	*@param pData 扩展报头的内容
	*/
	void SetExtHeader(BYTE Tag, BYTE TagLen, BYTE *pData );
protected:
	TXMPHeader m_XMPHeader;		//XMP报头
	TXMPExtHeader m_ExtHeader;	//XMP扩展报头
};

