#pragma once

#include "IFlow.h"
#include "CacheList.h"
#include "NotifyHandler.h"

//缓存中索引的节点
struct TCachedFlowNode
{
	void* address;				//对象在缓冲区中的偏移量(当包在缓存中，此项有意义)
	int size;					//对象的长度(当包在缓存中，此项有意义)
};

#define FLOW_MAX_NODE		0x7fffffff	//放入的消息个数32亿
#define FLOW_BLOCK_SIZE		0x00010000
#define FLOW_BUCKET_SIZE	(FLOW_MAX_NODE/FLOW_BLOCK_SIZE)

//CCachedFlowNodeVector是用于管理包的索引的队列。它分为两级存贮。
//一级存贮为FLOW_BUCKET_SIZE个,二级存贮的大小为FLOW_BLOCK_SIZE,最多可存贮的包索引节点为FLOW_MAX_NODE个
class CCachedFlowNodeVector
{
public:
	CCachedFlowNodeVector():m_nNodeCount(0)	{ memset(m_pBuckets, 0, FLOW_BUCKET_SIZE*sizeof(TCachedFlowNode*)); }	
	~CCachedFlowNodeVector()	{ clear(); }

	TCachedFlowNode& operator[](size_t n)	{ return m_pBuckets[n/FLOW_BLOCK_SIZE][n&(FLOW_BLOCK_SIZE-1)]; }

	//获取队列中节点的个数
	size_t size()	{ return m_nNodeCount; }

	void push_back(TCachedFlowNode& node)
	{
		if (0 == m_nNodeCount%FLOW_BLOCK_SIZE)
		{
			m_pBuckets[m_nNodeCount/FLOW_BLOCK_SIZE] = new TCachedFlowNode[FLOW_BLOCK_SIZE];
		}
		(*this)[m_nNodeCount] = node;
		m_nNodeCount++;	
	}
	void clear()
	{
		m_nNodeCount = 0;
		for (int i=0; i<FLOW_BUCKET_SIZE; i++)
		{
			if (m_pBuckets[i] == NULL)
			{
				break;
			}
			delete[] m_pBuckets[i];
		}
		memset(m_pBuckets, 0, FLOW_BUCKET_SIZE*sizeof(TCachedFlowNode *));
	}
	
private:
	volatile int m_nNodeCount;							//队列中节点的个数
	TCachedFlowNode* m_pBuckets[FLOW_BUCKET_SIZE];		//一级缓存
};

//CCachedFlow是带固定大小缓存的流，用于提高读写速度。
//它是部分线程安全的流，在缓冲区未用完之前，可以有一个写入者，同时有多个读取者。它可以挂接另外一个流，用于持久化或存贮缓冲区无法容纳的包。
//它在构造时要指定缓存的大小。缓存使用先进先出的方法管理。在读取包时，如果缓存中没有，则试着从挂接的流中读取。
class CCachedFlow : public IFlow
{
public:
	//构造函数
	//@param bSyncFlag 在加入一个包时，是否立刻同步到挂接的流。	true立刻同步到挂接的流，false不同步到挂接的流
	//@param nMaxObjects 缓存包的最大数量,如果为0则不限制包的个数
	//@param nDataBlockSize CacheList使用的数据区块大小，不要小于一个数据包的长度
	CCachedFlow(bool bSyncFlag, int nMaxObjects, int nDataBlockSize);	
	virtual ~CCachedFlow();

	void Clear();

	//获取流中已经有的包的个数
	virtual int GetCount(void);
	
	//获取流的通讯阶段序号
	virtual WORD GetCommPhaseNo();
	virtual void SetCommPhaseNo(WORD nCommPhaseNo);
	
	//挂接另一个流
	virtual int AttachUnderFlow(IFlow* pFlow);
	virtual IFlow* DeatchUnderFlow();

	//取出挂接的流存贮的包数量
	int GetUnderCount();

	//将缓存中的包同步到挂接的流中 0 成功 <0 失败
	int SyncUnderFlow(int id);

	//将流尾部nCount位置后的包截掉
	virtual bool Truncate(int nCount); 

	//向流中增加一个对象
	virtual int Append(void* pObject, int length);

	//获取指定编号的对象
 	virtual int Get(int id, void* pObject, int length);
 	
	//将缓冲的第一个包从缓冲区中移去
	bool PopFront();
	
	//增加一个观察者
	virtual void AttachObserver(CNotifyHandler* pEventHandler);	
	//删除一个观察者
	virtual void DeatchObserver();
	
protected:
	typedef CCachedFlowNodeVector CNodeQueue;

	bool m_bSyncFlag;					//在加入一个包时，是否立刻同步到挂接的流
	int m_nMaxObjects;					//缓存包的最大数量
	int m_nDataBlockSize;				//CacheList使用的数据区块大小
	CCacheList m_cacheList;				//包中数据的缓存（自定义数据队列）
	CNodeQueue m_nodeQueue;				//包的索引

	WORD m_nCommPhaseNo;				//通讯阶段序号
	IFlow* m_pUnderFlow;				//挂接的流
	CNotifyHandler* m_pNotifyHandler;	//通知器

	volatile int m_nFirstID;			//缓存中第一个包的序号
};
