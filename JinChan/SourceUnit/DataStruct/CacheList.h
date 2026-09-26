#pragma once

//发送链表的结点,每个结点可以缓存多个数据块
class  CCacheNode
{
public:
	CCacheNode(int nBufSize):m_nBufSize(nBufSize),m_nLength(0)
	{
		m_pBuffer = new char [m_nBufSize];
		m_pData = m_pBuffer;
		m_pNext = NULL;
	}

	~CCacheNode()	{ delete m_pBuffer;	}
	//由于要把不用结点放入内存节点池复用，放入池前需要恢复结点状态
	void ResetNode()
	{
		m_nLength = 0;
		m_pData = m_pBuffer;
		m_pNext = NULL;
	}
	/*取出缓冲区头上的一块数据
	* @param len 输入：希望取出的数据长度；输出：实际取到的长度，不超过输入参数
	* @return 取到的数据。若未取到数据则为空
	*/
	void* GetData(int& len)
	{
		if(0 == m_nLength)
		{
			return NULL;
		}
		len = (m_nLength<len)?m_nLength:len;
		return m_pData;
	}

	/**将缓冲区最前面的数据移丢弃
	* @param len 丢弃数据的长度
	* @return 实际丢弃数据的长度
	*/
	int PopFront(int len)
	{
		if(len >m_nLength) 
			len = m_nLength;

		m_pData += len;
		m_nLength -= len;
		return len;
	}
	
	/**向缓冲区尾部加入一块数据
	* @param pData 加入数据的指针
	* @param nDataLen 加入数据的长度
	* @return 加入缓冲区的地址
	*/
	void* PushBack(void* pData, int nDataLen)
	{
		char* pTarget = m_pData+m_nLength;
		if(pTarget+nDataLen > m_pBuffer+m_nBufSize)
		{			
			if (0 == m_nLength) 
			{
				m_pData = m_pBuffer;
				pTarget = m_pData;
				if(nDataLen > m_nBufSize)
				{
					return NULL;
				}
			}
			else
			{
				return NULL;
			}
		}
		memcpy(pTarget, pData, nDataLen);
		m_nLength += nDataLen;
		return pTarget;
	}

	void SetNext(CCacheNode *pNext)	{ m_pNext = pNext; }
	CCacheNode* GetNext()	{ return m_pNext; }
	int GetLength()	{ return m_nLength; }
private:
	char* m_pBuffer;		//缓冲区指针
	int m_nBufSize;			//缓冲区长度
	char* m_pData;			//针向实际的数据块
	int m_nLength;			//本结点的数据长度
	CCacheNode* m_pNext;	//下一个结点的指针
};


//定义了一个数据块队列
class  CCacheList  
{
public:
	CCacheList(int nDataBlockSize = 4096);	
	virtual ~CCacheList();

	void Clear();
	bool IsEmpty();

	//取出队列头上的一块数据
	void* GetData(int& len);

	//将队列最前面的数据移丢弃
	int PopFront(int len);
	
	//向队列尾部加入一块数据
	void* PushBack(void* pData, int nDataLen);
		
protected:
	int m_nDataBlockSize;			//每个结点可存贮数据的长度
	CCacheNode* m_pCacheHead;		//头结点
	CCacheNode* m_pCacheTail;		//尾结点	
	CCacheNode* m_pCachePoolHead;	//内存结点池头
};
