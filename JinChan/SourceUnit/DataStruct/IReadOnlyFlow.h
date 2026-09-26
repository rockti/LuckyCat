#pragma once
typedef unsigned short      WORD;
//IReadOnlyFlow是一种对象容器（流）,流中的对象可以按序号读出。
class IReadOnlyFlow  
{
public:
	IReadOnlyFlow() {}
	virtual ~IReadOnlyFlow() {}

	//获取流中已经有的对象的个数
	virtual int GetCount(void) = 0;
	
	//获取流的通讯阶段序号
	virtual WORD GetCommPhaseNo() = 0;

	//获取指定编号的对象
 	virtual int Get(int id, void* pObject, int length) = 0;
	virtual int Get(int id, void** pObject) { return 0; };
};
