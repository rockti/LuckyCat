#pragma once

#include "IReadOnlyFlow.h"

//IFlow是一种对象容器（流）对象只能从尾部添加，可以按序号读出。
class  IFlow : public IReadOnlyFlow
{
public:
	IFlow() {}
	virtual ~IFlow() {}
		
	//设置流的通讯阶段序号
	virtual void SetCommPhaseNo(WORD nCommPhaseNo) = 0;

	//将流尾部的对象截掉nCount保留的对象的个数
	virtual bool Truncate(int nCount) = 0; 

	//向流中增加一个对象
	virtual int Append(void* pObject, int length) = 0;
};

