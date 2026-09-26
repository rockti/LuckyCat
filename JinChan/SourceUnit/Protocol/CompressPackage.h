#pragma once

#include "Package.h"

#define CRP_PACKAGE_MAX_SIZE	4096
struct TCRPHeader
{
	BYTE Type;			//标示上层协议的协议ID
	BYTE Method;		//压缩算法代码
	
	inline void Init()	{ memset(this, 0 ,sizeof(TCRPHeader)); }
	inline void ToStream(char* pStream)	{ memcpy(pStream, this, sizeof(TCRPHeader)); }
	inline void FromStream(char* pStream)	{ memcpy(this, pStream, sizeof(TCRPHeader)); }
};

int const CRPHLEN = sizeof(TCRPHeader);

class CCompressPackage : public CPackage  
{
public:
	CCompressPackage();
	virtual ~CCompressPackage();
	
	virtual DWORD GetActiveID();
	virtual int ValidPackage();

	virtual int MakePackage();

	inline BYTE GetCompressMethod()	{ return m_header.Method; }
	inline TCRPHeader* GetCRPHeader()	{ return &m_header; }

protected:
	TCRPHeader m_header;
};
