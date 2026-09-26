#pragma once

#include "CacheList.h"

struct CDataItem
{
	void* pData;
	int nDataLen;
	bool bDelivered;
};

class COrderQueue  
{
public:
	COrderQueue(int nMaxSlot, int nDataBlockSize = 40000);
	virtual ~COrderQueue();

	bool EnQueue(int nIndex, void* pData, int nDataLen);
	void DeQueue();

	void* QueueFront(int& nDataLen);

	void Clear();

	bool IsEmpty()	{ return m_nDataItemHead == m_nDataItemTail; }
	int GetStartIndex()	{ return m_nStartIndex; }
	void SetStartIndex(int nStartIndex)
	{
		Clear();
		m_nStartIndex = nStartIndex;
	}

protected:
	CDataItem** m_pSlot;
	int m_nMaxSlot;
	int m_nSlotHead;

	CDataItem* m_pDataItem;
	int m_nMaxDataItem;
	int m_nDataItemHead;
	int m_nDataItemTail;

	CCacheList m_cacheList;
	int m_nStartIndex;
};
