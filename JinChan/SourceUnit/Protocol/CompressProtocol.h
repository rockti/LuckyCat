#pragma once

#include "Protocol.h"
#include "CompressPackage.h"

struct TCRPCompressMethod
{
	BYTE Method;
	BYTE ActiveID;
};

#define CRPCM_NONE	0x00		//不压缩
#define CRPCM_ZERO	0x01		//只压缩0x00的行程压缩

class CCompressProtocol : public CProtocol  
{
public:
	CCompressProtocol(CReactorThread* pReactor);
	virtual ~CCompressProtocol();

	virtual int Push(CPackage* pPackage, CProtocol* pUpper);
	void SetCompressMethod(BYTE chActiveID, BYTE chCompressMethod);	

protected:	
	virtual int Pop(CPackage* pPackage);
	BYTE GetCompressMethod(BYTE chActiveID);

protected:
	typedef vector<TCRPCompressMethod> CCompressMethodArray;
	CCompressMethodArray m_CompressMethods;
	CCompressPackage m_pkgCommpress;
};
