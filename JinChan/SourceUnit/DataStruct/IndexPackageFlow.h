#pragma once
#include "IReadOnlyFlow.h"

class CIndexPackageFlow : public IReadOnlyFlow  
{
public:
	CIndexPackageFlow();
	virtual ~CIndexPackageFlow();

	virtual int GetCount(void);	
	virtual void AttachUnderFlow(IReadOnlyFlow* pFlow);
	virtual void DeatchUnderFlow();
	
	virtual bool Truncate(int nCount); 
	virtual int Append(int nIndex);
	
	virtual WORD GetCommPhaseNo();	
	virtual void SetCommPhaseNo(WORD nCommPhaseNo);

	//获取指定编号的对象
 	virtual int Get(int id, void* pObject, int length);

	//获取指定编号的对象在下层流中的序号
	int GetPackageIndex(int id);

protected:
	typedef vector<DWORD> CPackageIndex;
	CPackageIndex m_PackageIndex;
	IReadOnlyFlow* m_pUnderFlow; 
	WORD m_nCommPhaseNo;
};
