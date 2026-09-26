#include "stdafx.h"
#include "CompressPackage.h"


CCompressPackage::CCompressPackage()
{
}
CCompressPackage::~CCompressPackage()
{
}

DWORD CCompressPackage::GetActiveID()
{
	return (DWORD)m_header.Type;
}

int CCompressPackage::ValidPackage()
{
	int pkglen = GetLength();
	if(pkglen < CRPHLEN)
		return -1;

	m_header.FromStream(m_pHead);
	Pop(CRPHLEN);

	return pkglen;
}

int CCompressPackage::MakePackage()
{
	char* buf = Push(CRPHLEN);
	if(NULL == buf)
		return -1;

	m_header.ToStream(buf);

	return 0;
}
