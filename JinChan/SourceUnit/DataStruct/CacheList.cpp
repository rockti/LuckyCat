#include "stdafx.h"
#include "CacheList.h"

CCacheList::CCacheList(int nDataBlockSize)
:m_nDataBlockSize(nDataBlockSize),m_pCachePoolHead(NULL)
{
	m_pCacheHead = new CCacheNode(nDataBlockSize);
	m_pCacheTail = m_pCacheHead;
}
CCacheList::~CCacheList()
{
	CCacheNode* pNode = m_pCacheHead;
	CCacheNode* pNext = NULL;
	while (NULL != pNode)
	{
		pNext = pNode->GetNext();
		delete pNode;
		pNode = pNext;
	}

	pNode = m_pCachePoolHead;	
	while (pNode != NULL)
	{
		pNext = pNode->GetNext();
		delete pNode;
		pNode = pNext;
	}
}

void CCacheList::Clear()
{
	while(m_pCacheHead->GetLength() > 0)
	{
		PopFront(m_pCacheHead->GetLength());
	}
}
bool CCacheList::IsEmpty()
{
	int len = 1;
	return (NULL == GetData(len));
}

void* CCacheList::PushBack(void* pData, int nDataLen)
{
	void* pTarget = m_pCacheTail->PushBack(pData, nDataLen);
	if(NULL != pTarget)
	{
		return pTarget;
	}

	//先检查内存节点池是否有空，否则分配一个新的节点
	CCacheNode* pNode = m_pCachePoolHead;
	if(NULL != pNode)
	{
		m_pCachePoolHead = pNode->GetNext();	
		pNode->ResetNode();	
	}
	else
	{
		//最后的结点空间已经用完，分配一个新的结点
		pNode = new CCacheNode(m_nDataBlockSize);
	}	
	m_pCacheTail->SetNext(pNode);
	m_pCacheTail = pNode;

	//直接将数据复制进最后的结点
	pTarget = m_pCacheTail->PushBack(pData, nDataLen);
	return pTarget;
}

int CCacheList::PopFront(int len)
{
	int poplen = m_pCacheHead->PopFront(len);	
	if(m_pCacheHead->GetLength() <= 0)
	{
		CCacheNode *pNext = m_pCacheHead->GetNext();
		if(NULL != pNext)
		{
			//把要删除的节点放入内存节点池头部			
			m_pCacheHead->ResetNode();//恢复要被删除的结点状态
			m_pCacheHead->SetNext(m_pCachePoolHead);
			m_pCachePoolHead = m_pCacheHead;	
			m_pCacheHead = pNext;
		}
		else
		{
			m_pCacheTail = m_pCacheHead;
		}
	}
	return poplen;
}

void *CCacheList::GetData(int &nLength)
{
	return (char *)m_pCacheHead->GetData(nLength);
}


