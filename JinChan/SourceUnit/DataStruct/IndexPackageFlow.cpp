#include "stdafx.h"
#include "IndexPackageFlow.h"

CIndexPackageFlow::CIndexPackageFlow()
:m_pUnderFlow(NULL),m_nCommPhaseNo(0)
{
	m_PackageIndex.clear();
}
CIndexPackageFlow::~CIndexPackageFlow()
{
}

int CIndexPackageFlow::GetCount(void)
{
	return m_PackageIndex.size();
}
void CIndexPackageFlow::AttachUnderFlow(IReadOnlyFlow *pFlow)
{
	m_pUnderFlow = pFlow;
	m_nCommPhaseNo = pFlow->GetCommPhaseNo();
}
void CIndexPackageFlow::DeatchUnderFlow()
{
	IReadOnlyFlow* pFlow = m_pUnderFlow;
	m_pUnderFlow = NULL;
}

bool CIndexPackageFlow::Truncate(int nCount)
{
	if (nCount <= GetCount())
	{
		return true;
	}
	m_PackageIndex.resize(nCount);
	return true;
}
int CIndexPackageFlow::Append(int nIndex)
{
	m_PackageIndex.push_back(nIndex);
	return m_PackageIndex.size()-1;
}

WORD CIndexPackageFlow::GetCommPhaseNo()
{
	return m_nCommPhaseNo;
}
void CIndexPackageFlow::SetCommPhaseNo(WORD nCommPhaseNo)
{
	if (m_nCommPhaseNo != nCommPhaseNo)
	{
		m_PackageIndex.clear();
		m_nCommPhaseNo = nCommPhaseNo;
	}
}

int CIndexPackageFlow::Get(int id, void* pObject, int length)
{
	return m_pUnderFlow->Get(m_PackageIndex[id], pObject, length);
}
int CIndexPackageFlow::GetPackageIndex(int id)
{
	return m_PackageIndex[id];
}
