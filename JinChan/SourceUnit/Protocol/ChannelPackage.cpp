#include "stdafx.h"
#include "ChannelPackage.h"

CChannelPackage::CChannelPackage(int maxPackageSize)
{
	ConstructAllocate(maxPackageSize, 0);
}
CChannelPackage::~CChannelPackage()
{
}
DWORD CChannelPackage::GetActiveID()
{
	return 0;
}
int CChannelPackage::ValidPackage()
{
	return GetLength();
}
int CChannelPackage::ReadFromChannel(CChannel* pChannel)
{
	int nReadLen = 0;
	if (CT_STREAM == pChannel->GetType())
	{
		if (NULL == m_pPackageBuffer)
		{
			return 0;
		}

		int n = m_pHead - m_pPackageBuffer->GetData();	
		memmove(m_pPackageBuffer->GetData(), m_pHead, GetLength());
		m_pHead -= n;
		m_pTail -= n;

		int nCount = m_pPackageBuffer->GetLength() - GetLength();
		nReadLen = pChannel->Read(nCount,m_pTail);
		if(nReadLen>0)
		{
			m_pTail += nReadLen;
		}
	}
	else
	{
		m_pTail = m_pHead = m_pPackageBuffer->GetData();
		nReadLen = pChannel->Read(m_pPackageBuffer->GetLength(), m_pTail);
		if(nReadLen>0)
		{
			m_pTail += nReadLen;
		}
	}

	return nReadLen;
}
