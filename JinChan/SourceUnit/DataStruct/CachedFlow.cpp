#include "stdafx.h"
#include "CachedFlow.h"


CCachedFlow::CCachedFlow(bool bSyncFlag, int nMaxObjects, int nDataBlockSize)
:m_bSyncFlag(bSyncFlag),m_nMaxObjects(nMaxObjects),m_nDataBlockSize(nDataBlockSize)
,m_cacheList(nDataBlockSize),m_nCommPhaseNo(0),m_pUnderFlow(NULL),m_pNotifyHandler(NULL)
{
	Clear();
}
CCachedFlow::~CCachedFlow()
{
	delete m_pUnderFlow;
	m_pUnderFlow = NULL;
}

void CCachedFlow::Clear()
{
	m_nFirstID = 0;
	m_nodeQueue.clear();
	m_cacheList.Clear();	
}

int CCachedFlow::GetCount(void)
{
	return (int)m_nodeQueue.size();
}
WORD CCachedFlow::GetCommPhaseNo()
{
	return m_nCommPhaseNo;
}
void CCachedFlow::SetCommPhaseNo(WORD nCommPhaseNo)
{
	if (m_nCommPhaseNo != nCommPhaseNo)
	{
		Clear();
		m_nCommPhaseNo = nCommPhaseNo;
	}
	if (NULL != m_pUnderFlow)
	{
		m_pUnderFlow->SetCommPhaseNo(nCommPhaseNo);
	}
}

int CCachedFlow::AttachUnderFlow(IFlow* pFlow)
{
	m_pUnderFlow = pFlow;
	m_nCommPhaseNo = pFlow->GetCommPhaseNo();

	Clear();
	
	//将下层流的对象载入缓存
	char* buf = new char[m_nDataBlockSize];
	for(int i=0; i<m_pUnderFlow->GetCount(); i++)
	{
		int len = m_pUnderFlow->Get(i, buf, m_nDataBlockSize);
		Append(buf, len);
	}
	delete[] buf;

	return 0;
}
IFlow* CCachedFlow::DeatchUnderFlow()
{
	IFlow* pFlow = m_pUnderFlow;
	m_pUnderFlow = NULL;
	return pFlow;
}

int CCachedFlow::GetUnderCount()
{
	if(NULL == m_pUnderFlow)
	{
		return -1;
	}
	return m_pUnderFlow->GetCount();
}
int CCachedFlow::SyncUnderFlow(int id)
{
	int undercount = GetUnderCount();
	if(id != undercount)
	{
		return -1;
	}
	if(GetCount() == undercount)
	{
		return -1;
	}

	TCachedFlowNode node = m_nodeQueue[undercount];
	m_pUnderFlow->Append(node.address, node.size);	
	return 0;
}

bool CCachedFlow::Truncate(int nCount)
{
	if (NULL != m_pUnderFlow)
	{	
		if(!m_pUnderFlow->Truncate(nCount))
		{
			return false;
		}
		CCachedFlow::AttachUnderFlow(m_pUnderFlow);
	}
	else
	{
		if (0 == nCount)
		{
			CCachedFlow::Clear();
		}
		else
		{
			return false;
		}
	}	
	return true;
}

int CCachedFlow::Append(void* pObject, int length)
{
	int nCount = (int)m_nodeQueue.size();
	if(m_nMaxObjects>0 && nCount-m_nFirstID>=m_nMaxObjects)
	{
		if(NULL!=m_pUnderFlow && m_pUnderFlow->GetCount()<m_nFirstID)
		{
			//这时加入缓存会导致某个对象既不在缓存中，也不在下层流中，返回-1,表示Append失败
			return -1;
		}
		PopFront();
	}

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
	
int CCachedFlow::Get(int id, void *pObject, int length)
{
	if(id >= m_nFirstID)
	{
		TCachedFlowNode node = m_nodeQueue[id];
		if(node.size > length)
		{
			exit(0);	//获取对象的缓冲区太小
		}
		memcpy(pObject, node.address, node.size);
		return node.size;
	}
	else if (NULL != m_pUnderFlow)
	{
		return m_pUnderFlow->Get(id, pObject, length);
	}
	return -1;
}

bool CCachedFlow::PopFront()
{
	if (m_nFirstID >= GetCount())
	{
		return false;
	}
	m_cacheList.PopFront( m_nodeQueue[m_nFirstID].size );
	m_nodeQueue[m_nFirstID].address = NULL;
	m_nFirstID++;
	return true;
}

void CCachedFlow::AttachObserver(CNotifyHandler *pEventHandler)
{
	m_pNotifyHandler = pEventHandler;
}
void CCachedFlow::DeatchObserver()
{
	m_pNotifyHandler = NULL;
}
	
