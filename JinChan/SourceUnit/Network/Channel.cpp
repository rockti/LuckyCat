#include "stdafx.h"
#include "Channel.h"
#include "IFlow.h"


CChannel::CChannel(TChannelType type, int id)
:m_nType(type),m_nID(id),m_bConnected(true)
,m_fpLog(NULL),m_pFlowLog(NULL)
{
}
CChannel::~CChannel()
{
}

int CChannel::Read(int number, char* buffer)
{
	int readNumber;
	if (!m_bConnected)
	{
		WriteLog(CLRT_DISCONNECTED_READ, 0, NULL);
		return -1;
	}
	readNumber = ReadImp(number, buffer);
	if (readNumber >= 0)
	{
		WriteLog(CLRT_READ, readNumber, buffer);
	}
	else
	{
		WriteLog(CLRT_READ_ERROR, 0, NULL);
	}
	return readNumber;
}

int CChannel::Write(int number, char* buffer)
{
	int writeNumber;
	if (!m_bConnected)
	{
		WriteLog(CLRT_DISCONNECTED_WRITE, 0, NULL);
		return -1;
	}
	writeNumber = WriteImp(number, buffer);
	if (writeNumber >= 0)
	{
		WriteLog(CLRT_WRITE, writeNumber, buffer);
	}
	else
	{
		WriteLog(CLRT_WRITE_ERROR, 0, NULL);
	}
	return writeNumber;
}

bool CChannel::Available(void)
{
	if (!m_bConnected)
		return 0;
	return AvailableImp();
}

bool CChannel::Disconnect()
{
	if (!m_bConnected)
		return 0;
	m_bConnected = DisconnectImp();
	WriteLog(CLRT_DISCONNECT, 0, NULL);
	return m_bConnected;
}

bool CChannel::CheckConnection()
{
	if (!m_bConnected)
	{
		return false;
	}

	m_bConnected = CheckConnectionImp();
	return m_bConnected;
}

int CChannel::GetSelectId(void)
{
	return m_nID;
}

char* CChannel::GetRemoteName(void)
{
	return "";
}

char* CChannel::GetLocalName(void)
{
	return "";
}
char* CChannel::GetRemoteCommonName(void)
{
	return NULL;
}

void CChannel::SetLogFile(FILE* fp)
{
	m_fpLog = fp;
	char* pszRemoteName = GetRemoteName();
	WriteLog(CLRT_CONNECTED, strlen(pszRemoteName)+1, pszRemoteName);
}
void CChannel::SetLogFlow(IFlow* fp)
{
	m_pFlowLog = fp;
	char* pszRemoteName = GetRemoteName();
	WriteLog(CLRT_CONNECTED, strlen(pszRemoteName)+1, pszRemoteName);
}
void CChannel::WriteLog(WORD wType, WORD wDataLength, char const* pData)
{
	if (NULL != m_fpLog)
	{
		TChannelLogHeader header;
		header.dwId = (DWORD)GetId();

		struct _timeb timeout;
		_ftime( &timeout );
		header.dwTime = (DWORD)timeout.time;	
		header.dwMsTime = timeout.millitm;
		header.wType = wType;
		if (wDataLength > 0)
		{
			header.wLength = wDataLength;
		}
		else
		{
			header.wLength = 0;
		}
		header.ChangeEndian();
		fwrite(&header, sizeof(header), 1, m_fpLog);
		if (wDataLength > 0)
		{
			fwrite(pData, 1, wDataLength, m_fpLog);
		}
		fflush(m_fpLog);
	}

	if (NULL != m_pFlowLog)
	{
		TChannelLogHeader header;
		header.dwId = (DWORD)GetId();

		struct _timeb timeout;
		_ftime( &timeout );
		header.dwTime = (DWORD)timeout.time;	
		header.dwMsTime = timeout.millitm;
		header.wType = wType;
		if (wDataLength > 0)
		{
			header.wLength = wDataLength;
		}
		else
		{
			header.wLength = 0;
		}
		header.ChangeEndian();
		m_pFlowLog->Append(&header, sizeof(header));
		if (wDataLength > 0)
		{
			m_pFlowLog->Append((char*)pData, wDataLength);
		}
	}
}
