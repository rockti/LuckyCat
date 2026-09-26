#include "stdafx.h"
#include "Protocol.h"


CProtocol::CProtocol(CReactorThread* pReactor, int nHdrReserveLen, CPackage* pDemuxPackage, CPackage* pSendPackage)
:CEventHandler(pReactor),m_nActiveID(0),m_nHdrReserveLen(nHdrReserveLen)
,m_pDemuxPackage(pDemuxPackage),m_pSendPackage(pSendPackage)
,m_pUpper(NULL),m_pBrother(NULL),m_pUpperHandler(NULL),m_pErrorHandler(NULL)
{	
}
CProtocol::~CProtocol()
{
	while (m_lowers.size() > 0)
	{
		DetachLower(m_lowers[0]);
	}

	if(NULL != m_pDemuxPackage)
	{
		m_pDemuxPackage->Release();
	}

	if(NULL != m_pSendPackage)
	{
		m_pSendPackage->Release();
	}
}

void CProtocol::AttachLower(CProtocol* pLower, DWORD nActiveID)
{
	m_nActiveID = nActiveID;
	m_nHdrReserveLen += pLower->GetHdrReserveLen();
	CProtocolArray::iterator i = find(m_lowers.begin(), m_lowers.end(), pLower);
	if (i == m_lowers.end())
	{
		m_lowers.push_back(pLower);
		pLower->AddUpper(this);
	}
}
void CProtocol::DetachLower(CProtocol* pLower)
{
	CProtocolArray::iterator i = find(m_lowers.begin(), m_lowers.end(), pLower);
	if (i != m_lowers.end())
	{
		m_lowers.erase(i);
		pLower->RemoveUpper(GetActiveID());
	}
}

int CProtocol::Send(CPackage* pPackage, CProtocol* pUpper)
{
	int nRtn = MakePackage(pPackage, pUpper);
	if (nRtn < 0)
	{
		return nRtn;
	}

	CProtocolArray::iterator i = m_lowers.begin();
	for (; i!=m_lowers.end(); i++)
	{
		(*i)->Push(pPackage, this);
	}
	return 0;
}
int CProtocol::HandlePackage(CPackage* pPackage, CProtocol* pProtocol)
{
	int nRtn = 0;
	while(pPackage->GetLength() > 0)
	{
		m_pDemuxPackage->BufAddRef(pPackage);
		int len=m_pDemuxPackage->ValidPackage();
		if(len>=0)
		{
			//完整数据包				
			nRtn = Pop(m_pDemuxPackage);
			if(nRtn < 0)		
			{
				break;
			}
			pPackage->Pop(len);
		}
		else if(len == -1)	
		{
			//不完整数据包
			break;
		}
		else
		{				
			//错误数据包
			OnRecvErrorPackage(pPackage);
			nRtn = len;
			break;
		}
		m_pDemuxPackage->BufRelease();
	}
	return nRtn;
}

void CProtocol::AddUpper(CProtocol *pUpper)
{
	if (NULL == m_pUpper)
	{
		m_pUpper = pUpper;
	}
	else
	{
		CProtocol* pCurrent = m_pUpper;
		for (; NULL!=pCurrent; pCurrent=pCurrent->m_pBrother)
		{
			if (NULL == pCurrent->m_pBrother)
			{
				pCurrent->m_pBrother = pUpper;
				break;
			}
		}
	}
}
CProtocol* CProtocol::GetUpper(DWORD nActiveID)	
{
	CProtocol* pCurrent = m_pUpper;
	while (NULL != pCurrent)
	{
		if (pCurrent->GetActiveID() == nActiveID)
		{
			break;
		}
		pCurrent = pCurrent->m_pBrother;
	}
	return pCurrent;
}
CProtocol* CProtocol::RemoveUpper(DWORD nActiveID)
{
	CProtocol* pPrev = NULL;
	CProtocol* pCurrent = m_pUpper;
	while (NULL != pCurrent)
	{
		if (pCurrent->GetActiveID() == nActiveID)
		{
			if (NULL == pPrev)
			{
				m_pUpper = pCurrent->m_pBrother;
			}
			else
			{
				pPrev->m_pBrother = pCurrent->m_pBrother;
			}
			break;
		}
		pPrev = pCurrent;
		pCurrent = pCurrent->m_pBrother;
	}
	return pCurrent;
}

int CProtocol::Pop(CPackage* pPackage)
{
	DWORD nActiveID = pPackage->GetActiveID();
	CProtocolCallback* pUpper = GetUpper(nActiveID);

	if (NULL != pUpper)
	{
		return pUpper->HandlePackage(pPackage, this);
	}

	if (NULL != m_pUpperHandler) 
	{
		return m_pUpperHandler->HandlePackage(pPackage, this);
	}
	return 0;
}
int CProtocol::Push(CPackage *pPackage, CProtocol *pUpper)
{
	m_pSendPackage->BufAddRef(pPackage);
	int ret = Send(m_pSendPackage, pUpper);
	m_pSendPackage->BufRelease();
	return ret;
}

void CProtocol::NotifyError(int message, DWORD dwParam, void* pParam)
{
	if(NULL != m_pErrorHandler)
	{
		m_pErrorHandler->SendEvent(message, dwParam, pParam);
	}
}