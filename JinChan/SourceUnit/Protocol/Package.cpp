#include "stdafx.h"
#include "Package.h"

CPackage::CPackage()
:m_pPackageBuffer(NULL),m_nReserve(0),m_nRefCount(1)
{
	Clear();
}
CPackage::~CPackage()
{
	BufRelease();
}

void CPackage::Clear()
{
	if (NULL == m_pPackageBuffer) 
	{
		m_pHead = NULL;
		m_pTail = NULL;
	}
	else
	{
		m_pHead = m_pPackageBuffer->GetData() + m_pPackageBuffer->GetLength();
		m_pTail = m_pHead;
	}
}

char* CPackage::Push(int size)
{
	if(size <= 0)	return NULL;
	if((m_pHead-size) >= m_pPackageBuffer->GetData())
	{
		m_pHead -= size;
		return m_pHead;
	}
	return NULL;
}
char* CPackage::Pop(int size)
{
	if(GetLength() < size)	return NULL;
	char* tmp = m_pHead;
	m_pHead += size;
	return tmp;
}

void CPackage::BufAddRef(CPackage* pPackage)
{
	BufRelease();
	m_pPackageBuffer = pPackage->m_pPackageBuffer;
	if(NULL != m_pPackageBuffer)
	{
		m_pPackageBuffer->AddRef();
		m_pHead = pPackage->m_pHead;
		m_pTail = pPackage->m_pTail;
	}
}
CPackageBuffer* CPackage::BufRelease()
{
	CPackageBuffer* old = m_pPackageBuffer;
	if(NULL != m_pPackageBuffer)
	{
		m_pPackageBuffer->Release();
		m_pPackageBuffer = NULL;
		m_pHead=m_pTail = NULL;
	}
	return old;
}

void CPackage::ConstructAllocate(int nCapacity, int nReserve)
{
	delete m_pPackageBuffer;
	m_pPackageBuffer = new CPackageBuffer(nCapacity + nReserve);
	m_nReserve = nReserve;
	Clear();
}
char* CPackage::Allocate(int length)
{
	if(NULL == m_pPackageBuffer)
	{
		return NULL;
	}
	if(length > m_pPackageBuffer->GetLength())
	{
		return NULL;
	}

	m_pTail = m_pPackageBuffer->GetData() + m_pPackageBuffer->GetLength();
	m_pHead = m_pTail - length;
	return m_pHead;
}
char* CPackage::AllocateMax()
{
	if(NULL == m_pPackageBuffer)
	{
		return NULL;
	}
	m_pHead = m_pPackageBuffer->GetData() + m_nReserve;
	m_pTail = m_pPackageBuffer->GetData() + m_pPackageBuffer->GetLength();
	return m_pHead;
}

void CPackage::Truncate(int newLength)
{
	if(GetLength() > newLength)
	{
		m_pTail= m_pHead + newLength;
	}
}

void CPackage::Release()
{
	m_nRefCount--;
	if(m_nRefCount <= 0)
		delete this;
}

void CPackage::AddRef()
{
	m_nRefCount++;
}

void CPackage::AttachBuffer(CPackageBuffer* pPackageBuffer)
{
	BufRelease();
	m_pPackageBuffer = pPackageBuffer;
	m_pPackageBuffer->AddRef();
	m_pHead = pPackageBuffer->GetData();
	m_pTail = m_pHead + pPackageBuffer->GetLength();
}

void CPackage::SetData(char* pData, int nDataLen)
{
	BufRelease();
	m_pHead = pData;
	m_pTail = pData+nDataLen;
}
