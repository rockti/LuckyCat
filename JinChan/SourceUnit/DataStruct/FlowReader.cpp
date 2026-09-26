#include "stdafx.h"
#include "FlowReader.h"

CFlowReader::CFlowReader()
:m_pFlow(NULL),m_nCommPhaseNo(0),m_nNextId(0)
{
}
CFlowReader::~CFlowReader()
{
	DetachFlow();
}

int CFlowReader::AttachFlow(IReadOnlyFlow* pFlow, int nStartId)
{
	m_pFlow = pFlow;
	m_nCommPhaseNo = pFlow->GetCommPhaseNo();	
	SetId(nStartId);
	return 0;
}
void CFlowReader::DetachFlow()
{
	IReadOnlyFlow* pFlow = m_pFlow;
	m_pFlow = NULL;
}

int CFlowReader::GetNext(void* pObject, int length)
{
	if(NULL == m_pFlow)
		return -1;

	WORD nCommPhaseNo = m_pFlow->GetCommPhaseNo();
	if (nCommPhaseNo != m_nCommPhaseNo)
	{
		SetId(0);
		m_nCommPhaseNo = nCommPhaseNo;
	}

	if (m_nNextId >= m_pFlow->GetCount())
	{
		return -1;
	}

	int len = m_pFlow->Get(m_nNextId, pObject, length);
	if(len >= 0)
	{
		SetId(1, FROM_HERE);
	}

	return len;
}

bool CFlowReader::GetNext(CPackage* pPackage)
{
	if(NULL == m_pFlow)
		return false;

	int len = GetNext(pPackage->GetAddress(), pPackage->GetLength());
	if(len < 0)
		return false;

	pPackage->Truncate(len);
	pPackage->ValidPackage();

	return true;
}
bool CFlowReader::GetNextUseNoBuf(CPackage* pPackage)
{
	if(NULL == m_pFlow)
		return false;

	WORD nCommPhaseNo = m_pFlow->GetCommPhaseNo();
	if (nCommPhaseNo != m_nCommPhaseNo)
	{
		SetId(0);
		m_nCommPhaseNo = nCommPhaseNo;
	}

	if (m_nNextId >= m_pFlow->GetCount())
	{
		return false;
	}

	void* pData = NULL;
	int iLen = m_pFlow->Get(m_nNextId, &pData);	
	if(iLen >= 0)
	{
		SetId(1, FROM_HERE);
	}
	else
	{
		return false;
	}
	pPackage->SetData((char *)pData, iLen);
	pPackage->ValidPackage();

	return true;
}
