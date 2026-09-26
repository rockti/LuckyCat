#include "stdafx.h"
#include "UserFlow.h"


CUserFlow::CUserFlow(int nFlowID, char const* pszPath, bool bReuse)
{
	char szFlowName[20];
	sprintf(szFlowName, "%06x", nFlowID);
	new (this)CUserFlow(szFlowName, pszPath, bReuse);
}
CUserFlow::CUserFlow(char const* pszFlowName, char const* pszPath, bool bReuse)
:m_fpContentFile(NULL),m_commPhaseNo(0),m_sequenceNo(0)
{
	OpenFile(pszFlowName, pszPath, bReuse);
}
CUserFlow::~CUserFlow()
{
	CloseFile();
}

void CUserFlow::SetCommPhaseNo(WORD nCommPhaseNo)
{
	if (nCommPhaseNo != m_commPhaseNo)
	{
		m_commPhaseNo = nCommPhaseNo;
		m_sequenceNo= 0;
		WriteFile();
	}
}

bool CUserFlow::Truncate(int nCount)
{
	return WriteFile();
}
int CUserFlow::Append(void* pObject, int length)
{
	++m_sequenceNo;
	WriteFile();
	return m_sequenceNo-1;		
}
void CUserFlow::SetCount(int nCount)
{
	m_sequenceNo = nCount;
	WriteFile();
}

bool CUserFlow::WriteFile()
{
	fseek(m_fpContentFile, 0, SEEK_SET);
	if(fprintf(m_fpContentFile, "%5d-%8d\n", m_commPhaseNo, m_sequenceNo) <= 0)
	{
		return false;
	}
	return true;
}
void CUserFlow::OpenFile(char const* pszFlowName, char const* pszPath, bool bReuse)
{
	assert(pszPath != NULL);
	m_sequenceNo = 0;
	CloseFile();

	char szContentFilename[512];
	sprintf(szContentFilename, "%s%s.tmp", pszPath, pszFlowName);	//以tmp为后缀命名UserFlow文件
	m_fpContentFile = mfopen(szContentFilename,"r+b");				//打开已存在文件
	if (NULL == m_fpContentFile)
	{
		m_fpContentFile = mfopen(szContentFilename,"w+b");			//文件不存在则创建文件
		if(NULL == m_fpContentFile)
		{
			CloseFile();
			RAISE_RUNTIME_ERROR("Can not open UserFlow file");
		}			
	}

	if (!bReuse || fscanf(m_fpContentFile,"%d,%d\n",&m_commPhaseNo,&m_sequenceNo)!=2)
	{
		if (!WriteFile())
		{
			CloseFile();
			RAISE_RUNTIME_ERROR("Can not init UserFlow file");
		}
	}
}
void CUserFlow::CloseFile()
{
	if(NULL != m_fpContentFile)
	{
		fclose(m_fpContentFile);
		m_fpContentFile = NULL;
	}
}

