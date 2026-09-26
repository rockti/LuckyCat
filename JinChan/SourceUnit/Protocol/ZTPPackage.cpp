#include "stdafx.h"
#include "ZTPPackage.h"

CZTPPackage::CZTPPackage()
{
	m_ZTPHeader.Chain = CHAIN_LAST;
	SetType(true);
}

CZTPPackage::~CZTPPackage()
{
}

DWORD CZTPPackage::GetActiveID()
{
	if (m_ZTPHeader.Type == ZTP_PUBLISH)
	{
		return m_ZTPHeader.SubjectID;
	}
	return 0;
}

int CZTPPackage::ValidPackage()
{
	if(GetLength() < ZTP_HEAD_LEN)
		return -1;

	m_ZTPHeader.FromStream(m_pHead);

	Pop(ZTP_HEAD_LEN);
	Truncate(m_ZTPHeader.ContentLength);

	return m_ZTPHeader.ContentLength+ZTP_HEAD_LEN;
}

int CZTPPackage::MakePackage() {
	m_ZTPHeader.ContentLength = GetLength();
	char* pBuffer = Push(ZTP_HEAD_LEN);
	if (NULL == pBuffer) {
		return -1;
	}
	m_ZTPHeader.ToStream(pBuffer);
	return 0;
}
int CZTPPackage::MakeNoBufPackage() {
	m_ZTPHeader.ContentLength = GetLength();
	 m_pHead -= ZTP_HEAD_LEN;
	char* pBuffer = m_pHead;
	if (NULL == pBuffer) {
		return -1;
	}
	m_ZTPHeader.ToStream(pBuffer);
	return 0;
}
void CZTPPackage::PrepareRequest(DWORD pId, BYTE chain)
{
	AllocateMax();
	Truncate(0);
	memset(&m_ZTPHeader, 0, sizeof(TZTPHeader));
	m_ZTPHeader.TransactionId = pId;
	m_ZTPHeader.Chain = chain;
	m_ZTPHeader.Type = ZTP_REQUEST;
}
void CZTPPackage::PrepareResponse(CZTPPackage* pRequest, DWORD pId, BYTE chain)
{
	AllocateMax();
	Truncate(0);
	TZTPHeader* pReqHeader = pRequest->GetZTPHeader();
	m_ZTPHeader.Type = ZTP_RESPONSE;
	m_ZTPHeader.Chain = chain;
	m_ZTPHeader.ContentLength = 0;
	m_ZTPHeader.SubjectID = 0;
	m_ZTPHeader.TransactionId = pId;
	m_ZTPHeader.CommPhaseNo = 0;
	m_ZTPHeader.SequenceNo = 0;
	m_ZTPHeader.RequestID = pReqHeader->RequestID;
	m_ZTPHeader.SessionID = pReqHeader->SessionID;
}
void CZTPPackage::PreparePublish(DWORD pId, BYTE chain)
{
	AllocateMax();
	Truncate(0);
	memset(&m_ZTPHeader, 0, sizeof(TZTPHeader));
	m_ZTPHeader.Chain = chain;
	m_ZTPHeader.Type = ZTP_PUBLISH;	
	m_ZTPHeader.TransactionId = pId;
}

CZTPPackage* CZTPPackage::CreatePackage(int nHeaderReserve)
{
	CZTPPackage* pPackage = new CZTPPackage;
	pPackage->ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, nHeaderReserve);
	return pPackage;
}
