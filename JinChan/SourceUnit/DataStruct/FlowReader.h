#pragma once
#include "IReadOnlyFlow.h"
#include "Package.h"

//CFlowReader是用于读取流的迭代器
class CFlowReader  
{
public:
	//SetID时用的计算相对位置的标识，类似于fseek中用的SEEK_XXX宏
	enum SeekFlag{ FROM_HEAD, FROM_HERE, FROM_END };
public:
	CFlowReader();
	virtual ~CFlowReader();

	int AttachFlow(IReadOnlyFlow* pFlow, int nStartId);
	void DetachFlow();

	//获取下一个要读取的编号
	int GetId(void) { return m_nNextId; }
	int* GetIdPtr(void) { return &m_nNextId; }

	//设置下一个要读取的编号 这个编号可以大于目前pFlow中的最大编号
	void SetId(int id, SeekFlag from = FROM_HEAD)
	{
		switch (from)
		{
		case FROM_HEAD:
			m_nNextId = id;
			break;
		case FROM_HERE:
			m_nNextId += id;
			break;
		default:
			m_nNextId = m_pFlow->GetCount() + id;
		}
	}
	
	bool Available(void) { return GetCount() > m_nNextId; }
	IReadOnlyFlow* GetFlow(void) { return m_pFlow; }
	int GetCount(void) { return m_pFlow->GetCount(); }

	//获取通讯阶段编号
	WORD GetCommPhaseNo() { return m_nCommPhaseNo; }

	//将挂接的流中的下一个对象取到指定的缓冲区中
	int GetNext(void* pObject, int length);

	//将挂接的流中的下一个包取到指定的包对象中
	bool GetNext(CPackage* pPackage);
	bool GetNextUseNoBuf(CPackage* pPackage);
	
protected:
	IReadOnlyFlow* m_pFlow;		//存储所关联的流
	WORD m_nCommPhaseNo;		//当前通讯阶段序号
	int m_nNextId;				//下一个要读的对象的编号
};
