#include "stdafx.h"
#include "CachedFileFlow.h"
#include "FileFlow.h"

CCachedFileFlow::CCachedFileFlow(char const* pszFlowName, char const* pszFlowPath, bool bReuse, int nMaxObjects, int nDataBlockSize)
:CCachedFlow(true, nMaxObjects, nDataBlockSize)
{
	m_pUnderFlow = new CFileFlow(pszFlowName, pszFlowPath, bReuse);
	CCachedFlow::AttachUnderFlow(m_pUnderFlow);
}

CCachedFileFlow::CCachedFileFlow(int nFlowID, char const* pszFlowPath, bool bReuse, int nMaxObjects, int nDataBlockSize)
:CCachedFlow(true, nMaxObjects, nDataBlockSize)
{
	m_pUnderFlow = new CFileFlow(nFlowID, pszFlowPath, bReuse);
	CCachedFlow::AttachUnderFlow(m_pUnderFlow);
}

CCachedFileFlow::~CCachedFileFlow()
{
	delete m_pUnderFlow;
	m_pUnderFlow = NULL;
}

int CCachedFileFlow::AttachUnderFlow(IFlow* pFlow)
{
	RAISE_RUNTIME_ERROR("CCachedFileFlow can't attach under flow");
	return -1;
}

void CCachedFileFlow::FlushToDisk(void )
{
	CFileFlow* pFileflow = (CFileFlow*)m_pUnderFlow;
	pFileflow->FlushToDisk();
}

