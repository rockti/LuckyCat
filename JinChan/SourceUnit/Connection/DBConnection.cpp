#include "stdafx.h"
#include "DBConnection.h"
#include "Logger.h"
#include <fmt/format.h>

bool CDbConnection::ConnectDB(char const* databaseName, char const* userName, char const* password)
{
	m_databaseName = databaseName;
	m_userName = userName;
	m_password = password;
	return ConnectDB();
}
bool CDbConnection::ConnectDB()
{
	try 
	{
		//mysql需要设置编码
		setOption("CharacterSet") = "gbk";

		//建立数据库连接
		Connect(m_databaseName.c_str(), m_userName.c_str(), m_password.c_str(), SA_MySQL_Client);
	}	
	catch(SAException& x)
	{
		string reportMsg = fmt::format("CDbConnection: connect DB[{}] failed, errorId[{}] errorMsg[{}]", m_databaseName.c_str(), x.ErrNativeCode(), (char const*)x.ErrText());
		RUNTIME_INFO_REPORT(reportMsg.c_str());	
		Log::Error(reportMsg.c_str());

		try
		{
			CheckConntectionError(x.ErrNativeCode());
			Rollback();
		}
		catch(SAException &)
		{
		}
		return false;
	}
	return true;
}

void CDbConnection::CheckConntectionError(int nErrorNativeCode)
{
	int const ORACLE_CONNECTION_ERROR_CODE[] = {0, 12500, 1034, 3113, 3114, 12571, 1033, 1012, 28, 3135};
	for (int i=0; i<sizeof(ORACLE_CONNECTION_ERROR_CODE)/sizeof(int); i++)
	{
		if (nErrorNativeCode == ORACLE_CONNECTION_ERROR_CODE[i])
		{
			Disconnect();
			break;
		}
	}
}
void CDbConnection::KeepAlive()
{
	SACommand cmd(this, "select * from t_sync_systemstatus");
	try 
	{
		cmd.Execute();
		while(cmd.FetchNext());
	}	
	catch(SAException &x)
	{
		try
		{
			CheckConntectionError(x.ErrNativeCode());
			Rollback();
		}
		catch(SAException &)
		{
		}
	}
}
bool CDbConnection::GetRow(SACommand* pCommand, char* buf, CFieldDescribe* pFieldDescribe)
{
	for (int i=0; i<pFieldDescribe->GetMemberCount(); i++)
	{
		TMemberDesc* pMemberDesc = pFieldDescribe->GetMemberDesc(i);
		char* pMember = buf + pMemberDesc->nStructOffset;
		switch(pMemberDesc->nType)
		{
		case FT_WORD:
			*((WORD*)pMember) = pCommand->Field(pMemberDesc->szName).asShort();
			break;
		case FT_LONG:
		case FT_DWORD:			
			*((DWORD*)pMember) = pCommand->Field(pMemberDesc->szName).asLong();
			break;
		case FT_BYTE:
			if (SA_dtDateTime == pCommand->Field(pMemberDesc->szName).DataType())
			{
				SADateTime dt = pCommand->Field(pMemberDesc->szName).asDateTime();
				sprintf(pMember, "%04d%02d%02d", dt.GetYear(), dt.GetMonth(), dt.GetDay());
			}
			else
			{
				memcpy(pMember, (const char *)(pCommand->Field(pMemberDesc->szName).asString()), pMemberDesc->nSize);
			}

			if (pMemberDesc->nSize != 1)
			{
				pMember[pMemberDesc->nSize-1] = '\0';
				char *p = pMember + strlen(pMember);
				while(p > pMember)
				{
					p--;
					if (*p != ' ')
					{
						break;
					}
					*p = '\0';
				}
			}
			break;
		case FT_REAL8:
			if (pCommand->Field(pMemberDesc->szName).isNull())
			{
				*((REAL8 *)pMember) = DOUBLE_NaN;
			}
			else
			{
				*((REAL8 *)pMember) = pCommand->Field(pMemberDesc->szName).asDouble();
			}
			break;
		}		
	}

	return true;
}
