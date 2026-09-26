#include "stdafx.h"
#include "FieldSet.h"

CFieldIteratorBase::CFieldIteratorBase(char* pHead, char* pEnd, CFieldDescribe* pFieldDescribe)
:m_pData(NULL),m_pCurr(pHead),m_pEnd(pEnd),m_pFieldDescribe(pFieldDescribe)
{
	Next();
}
void CFieldIteratorBase::Next()
{
	m_pData = NULL;
	while (NULL == m_pData)
	{
		if(DataRemain() < sizeof(TFieldHeader))
		{
			break;
		}
		GetHeader();
		m_pCurr += sizeof(TFieldHeader);

		if(DataRemain() < m_fieldHeader.Size)
		{
			break;
		} 
		if (m_pFieldDescribe==NULL || m_fieldHeader.FieldID == m_pFieldDescribe->m_FieldID) 
		{
			m_pData = m_pCurr;
		}
		m_pCurr += m_fieldHeader.Size;
	}
}

//////////////////////////////////////////////////////////////////////
CFieldTypeIterator::CFieldTypeIterator(char* pHead, char* pEnd, bool bInSys)
:CFieldIteratorBase(pHead, pEnd, NULL)
{
	m_bInSys = bInSys;
}
int CFieldTypeIterator::Retrieve(CFieldDescribe* pFieldDescribe, void* pStruct)
{
	if(NULL == m_pData)
	{
		return 0;
	}

	pFieldDescribe->StreamToStruct((char*)pStruct, m_pData, m_bInSys);

	if(m_bInSys)
	{
		return pFieldDescribe->GetStructSize();
	}
	else
	{
		return pFieldDescribe->GetStreamSize();
	}
}

//////////////////////////////////////////////////////////////////////
CNamedFieldIterator::CNamedFieldIterator(char* pHead, char* pEnd, CFieldDescribe* pFieldDescribe, bool bInSys)
:CFieldIteratorBase(pHead, pEnd, pFieldDescribe)
{
	m_bInSys = bInSys;
}
int CNamedFieldIterator::Retrieve(void* pStruct)
{
	if(NULL == m_pData)
	{
		return 0;
	}

	m_pFieldDescribe->StreamToStruct((char*)pStruct, m_pData,m_bInSys);
	if(m_bInSys)
	{
		return m_pFieldDescribe->GetStructSize();
	}
	else
	{
		return m_pFieldDescribe->GetStreamSize();
	}
}

CFieldSet::CFieldSet()
{
	m_bInSys = false;
}
CFieldSet::~CFieldSet()
{
}

int CFieldSet::AddField(CFieldDescribe* pFieldDescribe, void* pField)
{
	char* buf = NULL;
	if(m_bInSys)
	{
		buf = AllocField(pFieldDescribe->m_FieldID, pFieldDescribe->m_nStructSize);
	}
	else
	{
		buf = AllocField(pFieldDescribe->m_FieldID, pFieldDescribe->m_nStreamSize);
	}

	if(NULL == buf)
	{
		return -1;
	}

	pFieldDescribe->StructToStream((char*)pField, buf, m_bInSys);
	return 0;
}

int CFieldSet::GetSingleField(CFieldDescribe* pFieldDescribe, void* pField)
{
	CNamedFieldIterator itor = GetNamedFieldIterator(pFieldDescribe);
	if (itor.IsEnd())
	{
		return 0;
	}
	return itor.Retrieve(pField);
}

//只分配了空间
char* CFieldSet::AllocField(WORD wFieldID, WORD wSize)
{
	//新域加入后，包长不能超过缓冲区的长度
	if (m_pTail+wSize+sizeof(TFieldHeader) >= m_pPackageBuffer->GetData()+m_pPackageBuffer->GetLength()) 
	{
		return NULL;
	}

	TFieldHeader fh;
	fh.FieldID = wFieldID;
	fh.Size = wSize;
	CHANGE_ENDIAN(fh.FieldID);
	CHANGE_ENDIAN(fh.Size);

	memcpy(m_pTail, &fh, sizeof(fh));

	char *pBuf = m_pTail + sizeof(TFieldHeader);
	m_pTail += sizeof(TFieldHeader) + wSize;
	return pBuf;
}
void CFieldSet::SetType(bool bInSys)
{
	m_bInSys = bInSys;
}
