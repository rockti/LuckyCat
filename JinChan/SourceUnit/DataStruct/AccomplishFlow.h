#pragma once
#include "IFlow.h"
#include "DBConnection.h"

class CAccomplishFlow : public IFlow  
{
public:
	CAccomplishFlow();
	virtual ~CAccomplishFlow();

	bool ConnectDB(char const* databaseName, char const* userName, char const* password);

	virtual int GetCount(void)	{ return m_count; }
	
	virtual int const* GetCountPtr(void)	{ return &m_count; }

 	virtual int Get(int id, void* pObject, int length)	{ return 0; }

	virtual WORD GetCommPhaseNo()	{ return m_commPhaseNo; }

	virtual void SetCommPhaseNo(WORD nCommPhaseNo);

	virtual bool Truncate(int nCount); 

	virtual int Append(void* pObject, int length);

private:
	bool Load();
	bool Save();

private:
	int m_count;
	WORD m_commPhaseNo;
	CDbConnection* m_pDbConnection;
	SACommand* m_pSACommand;
};