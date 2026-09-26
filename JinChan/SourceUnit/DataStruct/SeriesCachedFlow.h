#pragma once
#include "CachedFlow.h"

//有顺序的流,最大对象都是0x7fffffff
class CSeriesCachedFlow : public CCachedFlow
{
public:
	CSeriesCachedFlow(bool bSyncFlag, int nMaxObjects, int nDataBlockSize);	
	virtual ~CSeriesCachedFlow();

	virtual int Append(void* pObject, int length);
	virtual int Get(int id, void** pObject);

	bool PopFront();

protected:
	volatile int m_iHaveGetID;
};
