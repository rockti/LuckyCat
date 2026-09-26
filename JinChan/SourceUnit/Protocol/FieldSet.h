#pragma once

#include "FieldDescribe.h"
#include "Package.h"

struct TFieldHeader
{
	WORD FieldID;		//域ID
	WORD Size;
};

//域迭代器基类
class CFieldIteratorBase
{
public:
	CFieldIteratorBase(char* pHead, char* pEnd, CFieldDescribe* pFieldDescribe);

	void Next();
	bool IsEnd()	{ return NULL==m_pData; }
	void RetrieveHeader(TFieldHeader& fieldHeader)	{ fieldHeader = m_fieldHeader; }
public:
	bool m_bInSys;

private:
	int DataRemain()	{ return int(m_pEnd - m_pCurr); }
	void GetHeader()
	{
		memcpy(&m_fieldHeader, m_pCurr, sizeof(TFieldHeader));
		CHANGE_ENDIAN(m_fieldHeader.FieldID);
		CHANGE_ENDIAN(m_fieldHeader.Size);
	}
	
protected:
	TFieldHeader m_fieldHeader;
	CFieldDescribe* m_pFieldDescribe;
	char* m_pEnd;
	char* m_pCurr;
	char* m_pData;
};
//整个包的迭代器
class CFieldTypeIterator : public CFieldIteratorBase
{
public:
	CFieldTypeIterator(char* pHead, char* pEnd, bool bInSys);
	int Retrieve(CFieldDescribe* pFieldDescribe, void* pStruct);
	template<class Field>
	int Retrieve(Field& field) {
		if (nullptr == m_pData) {
			return 0;
		}

		if (m_fieldHeader.FieldID != Field::ID || m_fieldHeader.Size != sizeof(Field)) {
			Log::Warn(fmt::format("Retrieve Filed failed ExpectedID[{}] ActualID[{}] ExpectedSize[{}] ActualSize[{}]", 
				m_fieldHeader.FieldID,Field::ID,m_fieldHeader.Size, sizeof(Field)));
			return 0;
		}

		memcpy(&field, m_pData, sizeof(Field));
		return sizeof(Field);
	}
};
//指定域的的迭代器
class  CNamedFieldIterator : public CFieldIteratorBase {
public:
	CNamedFieldIterator(char* pHead, char* pEnd, CFieldDescribe* pFieldDescribe, bool bInSys);
	int Retrieve(void* pStruct);
};

class CFieldSet : public CPackage
{
public:
	CFieldSet();
	virtual ~CFieldSet();

	void SetType(bool bInSys);
	int AddField(CFieldDescribe* pFieldDescribe, void* pField);

	int GetSingleField(CFieldDescribe* pFieldDescribe, void* pField);
	char* AllocField(WORD wFieldID, WORD wSize);
		
	template<typename Field>
	int AddField(Field const& field) {
		char* buf = NULL;
		m_bInSys = true;
		buf = AllocField(Field::ID, sizeof(field));

		if (NULL == buf) {
			return -1;
		}
		memcpy(buf, &field, sizeof(field));
		return 0;
	}

	template<typename Field>
	int GetSingleField(Field& field) {
		CFieldTypeIterator iter(m_pHead, m_pTail, true);
		if (iter.IsEnd()) {
			return 0;
		}
		return iter.Retrieve(field);
	}
	//获取整个包的迭代器
	CFieldTypeIterator GetFieldTypeIterator()	{ return CFieldTypeIterator(m_pHead,m_pTail,m_bInSys); }
	//获取指定域的迭代器
	CNamedFieldIterator GetNamedFieldIterator(CFieldDescribe* pFieldDescribe)	{ return CNamedFieldIterator(m_pHead,m_pTail,pFieldDescribe,m_bInSys); }
private:
	bool m_bInSys;
};

