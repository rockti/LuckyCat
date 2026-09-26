#pragma once
#include "SQL/SQLAPI.h"
#include "FieldDescribe.h"

class CDbConnection : public SAConnection 
{
public:
	CDbConnection()	{}
	virtual ~CDbConnection()	{}
	bool ConnectDB();
	bool ConnectDB(char const* databaseName, char const* userName, char const* password);
	void CheckConntectionError(int nErrorNativeCode);
	void KeepAlive();

	static bool GetRow(SACommand* pCommand, char* buf, CFieldDescribe* pFieldDescribe);
private:
	std::string m_databaseName;
	std::string m_userName;
	std::string m_password;
};
