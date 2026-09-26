#include "stdafx.h"
#include "OrderQueue.h"

COrderQueue::COrderQueue(int nMaxSlot, int nDataBlockSize)
:m_nMaxSlot(nMaxSlot),m_nSlotHead(0)
,m_nMaxDataItem(m_nMaxSlot*2),m_nDataItemHead(0),m_nDataItemTail(0)
,m_nStartIndex(1)
{
	m_pSlot = new CDataItem* [m_nMaxSlot];
	m_pDataItem = new CDataItem[m_nMaxDataItem];
	Clear();
}
COrderQueue::~COrderQueue()
{
	delete[] m_pSlot;
	delete[] m_pDataItem;
}

void COrderQueue::Clear()
{
	memset(m_pSlot, 0, sizeof(CDataItem *)*m_nMaxSlot);
	memset(m_pDataItem, 0, sizeof(CDataItem)*m_nMaxDataItem);
	m_nSlotHead = 0;
	m_nDataItemHead = 0;
	m_nDataItemTail= 0;
	m_nStartIndex = 1;
}

bool COrderQueue::EnQueue(int nIndex, void *pData, int nDataLen)
{
	//如果新加入的序号不在slot范围内，则返回失败
	if (nIndex<m_nStartIndex || nIndex>=m_nStartIndex+m_nMaxSlot)
	{
		return false;
	}

	CDataItem** pSlot = &m_pSlot[(m_nSlotHead+nIndex-m_nStartIndex)%m_nMaxSlot];
	//丢弃重复的数据
	if (NULL != *pSlot)
	{
		return false;
	}

	//将数据块加入cacheList,并加入dataitem
	CDataItem *pDataItem = &m_pDataItem[m_nDataItemTail];
	pDataItem->bDelivered = false;
	pDataItem->pData = m_cacheList.PushBack(pData, nDataLen);
	pDataItem->nDataLen = nDataLen;

	m_nDataItemTail ++;
	if (m_nDataItemTail >= m_nMaxDataItem)
	{
		m_nDataItemTail = 0;
	}

	//将dataitem加入slot
	*pSlot = pDataItem;

	return true;
}

void COrderQueue::DeQueue()
{
	CDataItem** pSlot = &m_pSlot[m_nSlotHead];
	if (*pSlot != NULL)
	{
		(*pSlot)->bDelivered = true;
		*pSlot = NULL;
	}

	m_nSlotHead ++;
	if (m_nSlotHead >= m_nMaxSlot)
	{
		m_nSlotHead = 0;
	}

	while (m_nDataItemHead != m_nDataItemTail)
	{
		CDataItem *pDataItem = &m_pDataItem[m_nDataItemHead];
		if (!pDataItem->bDelivered)
		{
			break;
		}
		m_cacheList.PopFront(pDataItem->nDataLen);

		m_nDataItemHead++;
		if (m_nDataItemHead >= m_nMaxDataItem)
		{
			m_nDataItemHead = 0;
		}
	}

	m_nStartIndex++;
}

void *COrderQueue::QueueFront(int &nDataLen)
{
	CDataItem** pSlot = &m_pSlot[m_nSlotHead];
	if (NULL == *pSlot)
	{
		return NULL;
	}

	nDataLen = (*pSlot)->nDataLen;
	return (*pSlot)->pData;
}

