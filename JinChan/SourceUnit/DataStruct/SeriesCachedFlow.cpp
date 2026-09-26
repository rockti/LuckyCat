#include "stdafx.h"
#include "SeriesCachedFlow.h"

CSeriesCachedFlow::CSeriesCachedFlow(bool bSyncFlag, int nMaxObjects, int nDataBlockSize)
:CCachedFlow(bSyncFlag,nMaxObjects,nDataBlockSize)
,m_iHaveGetID(-1)
{
}
CSeriesCachedFlow::~CSeriesCachedFlow()
{
}

int CSeriesCachedFlow::Append(void* pObject, int length)
{
	int nCount = (int)m_nodeQueue.size();
	while(PopFront());

	TCachedFlowNode node;
	node.address = m_cacheList.PushBack(pObject, length);
	node.size = length;
	m_nodeQueue.push_back(node);
	if (m_bSyncFlag) 
	{
		SyncUnderFlow(nCount);
	}	

	//产生通知 m_pEventHandler 在这里的唯一作用，就是通知流上有新的数据。
	if (m_pNotifyHandler)	
		m_pNotifyHandler->Notify();

	return nCount;
}

int CSeriesCachedFlow::Get(int id, void** pObject)
{
	if(id >= m_nFirstID)
	{	
		TCachedFlowNode node = m_nodeQueue[id];
		*pObject = node.address;
		m_iHaveGetID = id;
		return node.size;
	}
	return -1;
};

bool CSeriesCachedFlow::PopFront()
{
	if (m_nFirstID >= GetCount())
	{
		return false;
	}
	//要pop的数据大于等于已经取走数，不允许pop出去
	if (m_nFirstID >= m_iHaveGetID)
	{
		return false;
	}	
	m_cacheList.PopFront(m_nodeQueue[m_nFirstID].size);
	m_nodeQueue[m_nFirstID].address = NULL;
	m_nFirstID++;
	return true;
}
