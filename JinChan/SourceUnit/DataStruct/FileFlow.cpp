#include "stdafx.h"
#include "FileFlow.h"


#define BLOCK_SIZE	100
struct TFlowId
{
	DWORD dwSize;                       //对象长度
	void ChangeEndian()	{ CHANGE_ENDIAN(dwSize); }
};

CFileFlow::CFileFlow(char const* pszFlowName, char const* pszPath, bool bReuse)
:m_fpIdFile(NULL),m_fpContentFile(NULL),m_nCount(0),m_nReadId(-1),m_nCommPhaseNo(0)
{
	FPOS_SET(m_nContentSize, 0);
	FPOS_SET(m_nReadOffset, 0);

	OpenFile(pszFlowName, pszPath, bReuse);
	INIT_CRITICAL_VAR(m_criticalVar);
}
CFileFlow::CFileFlow(int nFlowID, char const* pszPath, bool bReuse)
{
	char szFlowName[20];
	sprintf(szFlowName, "%08x", nFlowID);
	new (this) CFileFlow(szFlowName, pszPath, bReuse);
}
CFileFlow::~CFileFlow()
{
	DELETE_CRITICAL_VAR(m_criticalVar);
	CloseFile();
}

int CFileFlow::GetCount(void)
{
	return m_nCount;
}
WORD CFileFlow::GetCommPhaseNo()
{
	return m_nCommPhaseNo;
}
void CFileFlow::SetCommPhaseNo(WORD nCommPhaseNo)
{
	if (nCommPhaseNo != m_nCommPhaseNo)
	{
		m_nCommPhaseNo = nCommPhaseNo;
		m_nCount= 0;
		InitFile();
	}
}

bool CFileFlow::Truncate(int nCount)
{
	if (m_nCount <= nCount)
	{
		return true;
	}
	m_nCount= nCount;
	return InitFile();
}
void CFileFlow::FlushToDisk(void)
{
	fflush(m_fpContentFile);
	fflush(m_fpIdFile);
	fflush(NULL);
}

int CFileFlow::Get(int id, void* pObject, int length)
{
	//减少临界区，把临界区的边界往下移动
	fpos_t offset = GetOffset(id); 
	TFlowId flowId;
	ENTER_CRITICAL(m_criticalVar);
	fsetpos(m_fpContentFile, &offset);
	if(fread(&flowId,sizeof(flowId),1,m_fpContentFile)!=1)
		Log::Error("CFileFlow::Can not read content file for CFlow");
	flowId.ChangeEndian();

	if (flowId.dwSize>(DWORD)length)
		Log::Error("CFileFlow::Insufficient buffer length while reading CFlow");

	if (fread(pObject,1,flowId.dwSize,m_fpContentFile)!=(size_t)flowId.dwSize)
		Log::Error("CFileFlow::Can not read content file for CFlow");

	m_nReadId = id;
	FPOS_SET(m_nReadOffset,FPOS_GET(offset) + flowId.dwSize + sizeof(flowId));
	LEAVE_CRITICAL(m_criticalVar);

	return flowId.dwSize;
}
int CFileFlow::Append(void* pObject, int length)
{
	//减少临界区，把临界区的边界往下移动
	ENTER_CRITICAL(m_criticalVar);
	TFlowId flowId;
	flowId.dwSize=length;	
	flowId.ChangeEndian();	

	fsetpos(m_fpContentFile, &m_nContentSize);

	if (fwrite(&flowId,sizeof(flowId),1,m_fpContentFile)!=1)
		Log::Error("CFileFlow::Can not write content file for CFlow");

	if (fwrite(pObject,1,length,m_fpContentFile) != (size_t)length)
		Log::Error("CFileFlow::Can not write content file for CFlow");

	fflush(m_fpContentFile);

	FPOS_SET(m_nContentSize,FPOS_GET(m_nContentSize)+length+sizeof(flowId));
	m_nCount++;

	if(m_nCount%BLOCK_SIZE==0)
	{
		m_BlockOffset.push_back(m_nContentSize);
		fseek(m_fpIdFile,0,SEEK_END);
		UF_INT8 dwOffset = FPOS_GET(m_nContentSize);
		CHANGE_ENDIAN(dwOffset);
		if (fwrite(&dwOffset,sizeof(dwOffset),1,m_fpIdFile)!=1)
			Log::Error("Can not write id file for CFlow");
		fflush(m_fpIdFile);
	}
	LEAVE_CRITICAL(m_criticalVar);
	return m_nCount-1;
}

fpos_t CFileFlow::GetContentSize(void)
{
	return m_nContentSize;
}
fpos_t CFileFlow::GetOffset(int id)
{
	fpos_t offset;
	FPOS_SET(offset, 0);
	if (m_nReadId+1 == id)
	{
		offset = m_nReadOffset;
	}
	else
	{
		TFlowId flowId;
		int i=id/BLOCK_SIZE;
		offset=m_BlockOffset[i];
		//减少临界区，把临界区的边界往下移动
		ENTER_CRITICAL(m_criticalVar);
		for(i*=BLOCK_SIZE;i<id;i++){
			fsetpos(m_fpContentFile, &offset);
			if(fread(&flowId,sizeof(flowId),1,m_fpContentFile)!=1)
				Log::Error("CFileFlow::Can not read content file for CFlow");
			flowId.ChangeEndian();
			FPOS_SET(offset,FPOS_GET(offset)+flowId.dwSize+sizeof(flowId));
		}
		LEAVE_CRITICAL(m_criticalVar);
	}
	return offset;
}

void CFileFlow::OpenFile(char const* pszFlowName, char const* pszPath, bool bReuse)
{
	assert(pszPath != NULL);

	CloseFile();

	char szIdFilename[512];
	char szContentFilename[512];
	sprintf(szIdFilename, "%s%s.id",pszPath, pszFlowName);
	sprintf(szContentFilename, "%s%s.con", pszPath, pszFlowName);

	m_nCount = 0;
	FPOS_SET(m_nContentSize, 0);
	m_BlockOffset.clear();

	fpos_t firstOffset;
	FPOS_SET(firstOffset, 0);
	m_BlockOffset.push_back(firstOffset);

	m_fpIdFile = mfopen(szIdFilename,"a+b");
	m_fpContentFile = mfopen(szContentFilename,"a+b");
	if ((NULL==m_fpIdFile) || (NULL==m_fpContentFile))
	{
		CloseFile();
		Log::Error("CFileFlow: can not open CFlow file");
	}

	if (!bReuse)
	{
		if (!InitFile())
		{
			CloseFile();
			Log::Error("CFileFlow: can not init CFlow file");
		}
		return ;
	}

	struct _stati64 statbuf;
	int result = _stati64( szContentFilename, &statbuf );
	if(0 != result)	Log::Error("CFileFlow: something wrong when getting file information");

	FPOS_SET(m_nContentSize, statbuf.st_size);

	fpos_t size;
	FPOS_SET(size, 0);
	fseek(m_fpIdFile, 0, SEEK_SET);
	if(1 != fread(&m_nCommPhaseNo,sizeof(m_nCommPhaseNo),1,m_fpIdFile))
	{
		if (!InitFile())
		{
			CloseFile();
			Log::Error("CFileFlow: can not init CFlow file");
		}
		return ;
	}

	CHANGE_ENDIAN(m_nCommPhaseNo);
	for(;;)
	{
		UF_INT8 dwOffset = 0;
		if(1 != fread(&dwOffset,sizeof(dwOffset),1,m_fpIdFile))	break;
		
		CHANGE_ENDIAN(dwOffset);
		fpos_t fpOffset;
		FPOS_SET(fpOffset,dwOffset);
		m_BlockOffset.push_back(fpOffset);
		size = fpOffset;
		m_nCount += BLOCK_SIZE;
	}

	TFlowId flowId;
	int i = 0;
	for(i=0 ; ; i++)
	{
		if(0 != fsetpos(m_fpContentFile,&size))
		{
			Log::Error("CFileFlow: fsetpos failed");
		}
		if(1 != fread(&flowId,sizeof(flowId),1,m_fpContentFile))	break;

		flowId.ChangeEndian();
		FPOS_SET(size, FPOS_GET(size)+flowId.dwSize+sizeof(flowId));
	}

	m_nCount+=i;
}
bool CFileFlow::InitFile()
{
	m_nContentSize = GetOffset(m_nCount);
	int nIdCount = m_nCount/BLOCK_SIZE;
	m_BlockOffset.resize(nIdCount+1);

	if(_chsize(_fileno(m_fpIdFile), nIdCount) != 0)
	{
		return false;
	}

	if(_chsize(_fileno(m_fpContentFile), (long)m_nContentSize) != 0)
	{
		return false;
	}

	if (fseek(m_fpIdFile, 0, SEEK_SET) != 0)
	{
		return false;
	}

	WORD nCommPhaseNo = m_nCommPhaseNo;
	CHANGE_ENDIAN(nCommPhaseNo);
	if(fwrite(&nCommPhaseNo, sizeof(nCommPhaseNo), 1, m_fpIdFile) != 1)
	{
		return false;
	}
	fflush(m_fpIdFile);
	return true;
}
void CFileFlow::CloseFile()
{
	if(NULL != m_fpIdFile)
	{
		fclose(m_fpIdFile);
		m_fpIdFile = NULL;
	}
	if(NULL != m_fpContentFile)
	{
		fclose(m_fpContentFile);
		m_fpContentFile = NULL;
	}
}
