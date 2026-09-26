#include "stdafx.h"
#include "AccomplishFlow.h"
#include <fmt/format.h>

CAccomplishFlow::CAccomplishFlow()
:m_count(0),m_commPhaseNo(0)
{
	m_pDbConnection = new CDbConnection;
	m_pSACommand = new SACommand(m_pDbConnection);
}
CAccomplishFlow::~CAccomplishFlow()
{
	delete m_pSACommand;
	delete m_pDbConnection;
}

bool CAccomplishFlow::ConnectDB(char const* databaseName, char const* userName, char const* password)
{
	if(m_pDbConnection->ConnectDB(databaseName, userName, password))
	{
		return Load();
	}
	return false;
}
void CAccomplishFlow::SetCommPhaseNo(WORD nCommPhaseNo)
{
	m_commPhaseNo = nCommPhaseNo;
	Load();
}
bool CAccomplishFlow::Truncate(int nCount)
{
	m_count = nCount;
	return Save();
}
int CAccomplishFlow::Append(void* pObject, int length)
{
	int nPos = ++m_count;
	if (!Save())
	{
		return -1;
	}
	return nPos;
}

bool CAccomplishFlow::Load()
{
	try 
	{
		SACommand cmd(m_pDbConnection);
		cmd.setCommandText("SELECT SequenceNo FROM t_Oper_MDBAccomplish WHERE CommPhaseNo=:1");
		cmd.Param(1).setAsShort() = m_commPhaseNo;
		cmd.Execute();
		if(!cmd.FetchNext())
			return Truncate(0);

		m_count = cmd.Field("SequenceNo").asLong();

		string reportMsg = fmt::format("AccomplishFlow: Load t_Oper_MDBAccomplish success, CommPhaseNo[{}] SequenceNo[{}]", m_commPhaseNo, m_count);
		RUNTIME_INFO_REPORT(reportMsg.c_str());	
		Log::Info(reportMsg.c_str());
	}	
	catch(SAException &x)
	{
		try
		{
			m_pDbConnection->CheckConntectionError(x.ErrNativeCode());
			m_pDbConnection->Rollback();
		}
		catch(SAException &)
		{
		}

		string reportMsg = fmt::format("AccomplishFlow: Load t_Oper_MDBAccomplish failed, CommPhaseNo[{}] Message[{}]", m_commPhaseNo, (char const*)x.ErrText());
		RUNTIME_INFO_REPORT(reportMsg.c_str());	
		Log::Fatal(reportMsg.c_str());
		return false;
	}
	return true;
}
bool CAccomplishFlow::Save()
{
	try 
	{
		if (!m_pDbConnection->isConnected())
		{
			m_pDbConnection->ConnectDB();
			if (!m_pDbConnection->isConnected())
			{
				return false;
			}
		}
		
		SACommand cmd(m_pDbConnection, "UPDATE t_Oper_MDBAccomplish SET SequenceNo=:1 WHERE CommPhaseNo=:2");
		cmd.Param(1).setAsLong() = m_count;
		cmd.Param(2).setAsShort() = m_commPhaseNo;	
		cmd.Execute();

		string reportMsg = fmt::format("AccomplishFlow: update t_Oper_MDBAccomplish success, CommPhaseNo[{}] SequenceNo[{}]", m_commPhaseNo, m_count);
		RUNTIME_INFO_REPORT(reportMsg.c_str());		
		Log::Info(reportMsg.c_str());

		int a = cmd.RowsAffected();
		if(1 != cmd.RowsAffected())
		{
			cmd.setCommandText("INSERT INTO t_Oper_MDBAccomplish(SequenceNo,CommPhaseNo) VALUES(:1, :2)");
			cmd.Param(1).setAsLong() = m_count;
			cmd.Param(2).setAsShort() = m_commPhaseNo;
			cmd.Execute();

			reportMsg = fmt::format("AccomplishFlow: insert t_Oper_MDBAccomplish success, CommPhaseNo[{}] SequenceNo[{}]", m_commPhaseNo, m_count);
			RUNTIME_INFO_REPORT(reportMsg.c_str());	
			Log::Info(reportMsg.c_str());
		}

		m_pDbConnection->Commit();
	}	
	catch(SAException &x)
	{
		try
		{
			m_pDbConnection->CheckConntectionError(x.ErrNativeCode());
			m_pDbConnection->Rollback();
		}
		catch(SAException &)
		{
		}

		string reportMsg = fmt::format("AccomplishFlow: Save t_Oper_MDBAccomplish failed, CommPhaseNo[{}] Message[{}]", m_commPhaseNo, (char const*)x.ErrText());
		RUNTIME_INFO_REPORT(reportMsg.c_str());	
		Log::Fatal(reportMsg.c_str());
		return false;
	}
	return true;
}

