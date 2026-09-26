#pragma once
#include "IFlow.h"

//CUserFlow是各种用户流的基类，保存通讯号和包的SequenceNo
class CUserFlow : public IFlow
{
public:
	CUserFlow(int nFlowID, char const *pszPath, bool bReuse);
	CUserFlow(char const* pszFlowName, char const* pszPath, bool bReuse);
	virtual ~CUserFlow();
	virtual int Get(int id, void* pObject, int length)	{ return 0; }
	virtual int GetCount(void)	{ return m_sequenceNo; }
	virtual int const* GetCountPtr(void)	{ return reinterpret_cast<int const*>(&m_sequenceNo); }
	virtual WORD GetCommPhaseNo()	{ return m_commPhaseNo; }
	
	virtual void SetCommPhaseNo(WORD nCommPhaseNo);
	virtual bool Truncate(int nCount);
	virtual int Append(void* pObject, int length);

	void SetCount(int nCount);
private:	
	bool WriteFile();
	void OpenFile(char const* pszFlowName, char const* pszPath, bool bReuse);
	void CloseFile();

private:
	FILE* m_fpContentFile;		//存储内容的文件
	uint32_t m_commPhaseNo;			//通讯阶段编号
	DWORD m_sequenceNo;			//通讯包的序号
};

