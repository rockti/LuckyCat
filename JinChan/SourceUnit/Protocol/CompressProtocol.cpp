#include "stdafx.h"
#include "CompressProtocol.h"
#include "CompressUtil.h"

CCompressProtocol::CCompressProtocol(CReactorThread* pReactor)
:CProtocol(pReactor,CRPHLEN,new CCompressPackage(),new CCompressPackage())
{
	m_pkgCommpress.ConstructAllocate(2*CRP_PACKAGE_MAX_SIZE, 256);
}
CCompressProtocol::~CCompressProtocol()
{
}

int CCompressProtocol::Pop(CPackage* pPackage)
{
	CCompressPackage* pCRPPackage = (CCompressPackage*)pPackage;

	switch(pCRPPackage->GetCompressMethod())
	{
	case CRPCM_ZERO:
		{
			m_pkgCommpress.AllocateMax();
			unsigned long nOutputNumBytes = m_pkgCommpress.GetLength();
			CompressUtil::Zerodecompress((BYTE*)pCRPPackage->GetAddress(),pCRPPackage->GetLength(), (BYTE*)m_pkgCommpress.GetAddress(),nOutputNumBytes);
			m_pkgCommpress.Truncate(nOutputNumBytes);
			m_pkgCommpress.GetCRPHeader()->Type = pCRPPackage->GetCRPHeader()->Type;
		}
		break;
	default:
		return CProtocol::Pop(pPackage);
	}

	return CProtocol::Pop(&m_pkgCommpress);
}
int CCompressProtocol::Push(CPackage* pPackage, CProtocol* pUpper)
{
	CCompressPackage* pCRPSendPackage = (CCompressPackage*)m_pSendPackage;

	BYTE chActiveID = (BYTE)pUpper->GetActiveID();
	pCRPSendPackage->GetCRPHeader()->Type = chActiveID;
	BYTE chCompressMethod = GetCompressMethod(chActiveID);
	pCRPSendPackage->GetCRPHeader()->Method = chCompressMethod;
	if (chCompressMethod == CRPCM_NONE)
	{
		return CProtocol::Push(pPackage, pUpper);
	}

	m_pkgCommpress.AllocateMax();
	unsigned long nOutputNumBytes = m_pkgCommpress.GetLength();
	switch (chCompressMethod)
	{
	case CRPCM_ZERO:
		{
			CompressUtil::Zerocompress((BYTE*)pPackage->GetAddress(), pPackage->GetLength(), (BYTE*)m_pkgCommpress.GetAddress(), nOutputNumBytes);
		}
		break;
	default:
		break;
	}

	if (nOutputNumBytes >= pPackage->GetLength())
	{
		pCRPSendPackage->GetCRPHeader()->Method = CRPCM_NONE;
		return CProtocol::Push(pPackage, pUpper);
	}

	m_pkgCommpress.Truncate(nOutputNumBytes);
	return CProtocol::Push(&m_pkgCommpress, pUpper);
}

void CCompressProtocol::SetCompressMethod(BYTE chActiveID, BYTE chCompressMethod)	
{
	TCRPCompressMethod cm;
	cm.Method = chCompressMethod;
	cm.ActiveID = chActiveID;
	m_CompressMethods.push_back(cm);
}

BYTE CCompressProtocol::GetCompressMethod(BYTE chActiveID)
{
	CCompressMethodArray::iterator i = m_CompressMethods.begin();
	for (; i != m_CompressMethods.end(); i++)
	{
		if ((*i).ActiveID == chActiveID)
		{
			return (*i).Method;
		}
	}
	return CRPCM_NONE;
}

