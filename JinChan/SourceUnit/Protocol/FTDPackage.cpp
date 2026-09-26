#include "stdafx.h"
#include "FTDPackage.h"

CFTDPackage::CFTDPackage()
{
	m_FTDHeader.Chain = FTD_CHAIN_LAST;
	SetType(true);
}
CFTDPackage::~CFTDPackage()
{
}

DWORD CFTDPackage::GetActiveID()
{
	return m_FTDHeader.SequenceSeries;
}
int CFTDPackage::ValidPackage()
{
	if(GetLength() < FTD_HEAD_LEN)
		return -1;

	m_FTDHeader.FromStream(m_pHead);

	char* pData = m_pHead+FTD_HEAD_LEN;
	if(m_FTDHeader.FTDContentLength != GetLength()-FTD_HEAD_LEN)
	{
		//包体长度与包头不符
		Log::Fatal(FT("CFTDPackage: FTDPackage length is inconsistent, transactionId[%08x] packageLen[%d]!=headerLen[%d]+contentLen[%d]"
			,m_FTDHeader.TransactionID, GetLength(), FTD_HEAD_LEN, m_FTDHeader.FTDContentLength));
		return -2;
	}

	Pop(FTD_HEAD_LEN);
	return m_FTDHeader.FTDContentLength + FTD_HEAD_LEN;
}
int CFTDPackage::MakePackage()
{
	m_FTDHeader.FTDContentLength = GetLength();
	m_FTDHeader.FieldCount = 0;

	CFieldTypeIterator itor = GetFieldTypeIterator();
	while (!itor.IsEnd())
	{
		m_FTDHeader.FieldCount++;
		itor.Next();
	}

	char* pBuffer = Push(FTD_HEAD_LEN);
	if (NULL == pBuffer) 
		return -1;
	
	m_FTDHeader.ToStream(pBuffer);
	return 0;
}

void CFTDPackage::PreparePackage(DWORD tid, BYTE chain) {
	AllocateMax();
	Truncate(0);
	memset(&m_FTDHeader, 0, sizeof(FTDHeader));
	m_FTDHeader.Chain = chain;
	m_FTDHeader.TransactionID = tid;
}
void CFTDPackage::PrepareResponse(CFTDPackage* pRequest, DWORD tid, BYTE chain)
{
	PreparePackage(tid, chain);
	SetRequestID(pRequest->GetRequestID());
}
CFTDPackage* CFTDPackage::CreatePackage(int nHeaderReserve)
{
	CFTDPackage* pPackage = new CFTDPackage;
	pPackage->ConstructAllocate(FTD_PACKAGE_MAX_SIZE, nHeaderReserve);
	return pPackage;
}
