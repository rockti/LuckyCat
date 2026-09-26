#include "stdafx.h"
#include "FieldDescribe.h"


CFieldDescribe::CFieldDescribe(WORD FieldID, int nStructSize, char const* pszFieldName, char const* pszComment, describeFunc func)
:m_FieldID(FieldID),m_nStructSize(nStructSize),m_nStreamSize(0),m_nTotalMember(0)
{
	strcpy(m_szFieldName, pszFieldName);
	strcpy(m_szComment, pszComment);
	func();
}
CFieldDescribe::~CFieldDescribe()
{
}

void CFieldDescribe::StructToStream(char* pStruct, char* pStream, bool bInSys)
{
	if(bInSys)
	{
		memcpy(pStream, pStruct, m_nStructSize);
		return;
	}
	for(int i=0; i<m_nTotalMember; i++)
	{
		TMemberDesc* p = &m_MemberDesc[i];
		memcpy(pStream+p->nStreamOffset, pStruct+p->nStructOffset, p->nSize);
	}
}
void CFieldDescribe::StreamToStruct(char* pStruct, char* pStream, bool bInSys)
{
	if(bInSys)
	{
		memcpy(pStruct, pStream, m_nStructSize);
		return;
	}
	for(int i=0;i<m_nTotalMember;i++)
	{
		TMemberDesc* p = &m_MemberDesc[i];
		memcpy(pStruct+p->nStructOffset, pStream+p->nStreamOffset, p->nSize);
	}
}
