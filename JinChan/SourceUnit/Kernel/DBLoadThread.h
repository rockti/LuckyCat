#pragma once
/*
#include "EventThread.h"
#include "FieldDescribe.h"
#include "CachedFlow.h"
#include "DBConnection.h"
#include "Mutex.h"
#include "UserDataType.h"
#include "ZTPPackage.h"
#include "SQL/SQLAPI.h"

const int UM_INIT_RESULT		= 0x1004;		//初始化数据结果
const int UM_INIT_APPEND_DATA	= 0x1005;		//初始化数据发送
const int UM_INIT_APPEND_END	= 0x1006;		//初始化发送完成

struct TInitTableInfo {
	char* pszTable;
	CFieldDescribe* pFieldDescribe;
	char* pszSQL;
	DWORD tid;
	int nParaNumber;
	bool bCanceled;
};

class CInitDataLoadThread;
class IInitDataLoadAssigner {
public:
	virtual TInitTableInfo* GetTinitTableInfo() = 0;
	virtual void OnDataLoaded(CInitDataLoadThread* pInitDataLoadThread, char const* pszTableName, int nErrorCode, char const* pszErrorMsg) = 0;
};

//数据库读取对象
class CInitDataLoader : public IInitDataLoadAssigner
{
public:
	CInitDataLoader(CEventHandler* pLoadHandler);
	virtual ~CInitDataLoader();
	virtual TInitTableInfo* GetTinitTableInfo();
	virtual void OnDataLoaded(CInitDataLoadThread* pTinitDataLoadThread, char const* pszTableName, int nErrorCode, char const* pszErrorMsg);

	bool ConnectDB(char const* databaseName, char const* userName, char const* password);
	bool StartLoadData(CDateType& WorkDay);

	bool IsBusy()	{ return m_tableCount > 0; }
	int GetErrorCode()	{ return m_errorCode; }
	char const* GetErrorMsg()	{ return m_strErrorMsg.c_str(); }

private:
	void StartWorkThread(TInitTableInfo* pTableInfo);
	void SetError(char const* pszTableName, int nErrorCode, char const* pszErrorMsg);
	void AppendData(CInitDataLoadThread* pDataLoadThread);
	void Cancel();

private:
	CEventHandler* m_pLoderHandler;
	CInitDataLoadThread* m_pInitLoadThread;
	int m_tableCount;
	int m_errorCode;
	int m_startPos;

	std::string m_strErrorMsg;

	TInitTableInfo* m_pCurrTask;
	TInitTableInfo* m_pCurrTableInfo;
	CMutex m_lock;
};

//数据库Load线程
class CInitDataLoadThread : public CThreadBase  
{
public:
	CInitDataLoadThread(IInitDataLoadAssigner* pAssigner);
	virtual ~CInitDataLoadThread();

	virtual void Run();
	bool ConnectDB(char const* databaseName, char const* userName, char const* password)	{ return m_pDbConnection->ConnectDB(databaseName, userName, password); }

	IFlow* GetDataFlow()	{ return m_pDataFlow; }
private:
	void DataLoading(TInitTableInfo* pTableInfo);
	bool LoadTable(TInitTableInfo* pTableInfo);
	void FlushPackage(DWORD TID);

private:
	IInitDataLoadAssigner* m_pAssigner;
	CDbConnection* m_pDbConnection;
	SACommand m_saCommand;
	CZTPPackage* m_pLoadPackage;
	CCachedFlow* m_pDataFlow;
};
*/