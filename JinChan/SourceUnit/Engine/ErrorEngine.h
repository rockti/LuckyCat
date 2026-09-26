#pragma once

#define MAX_ERROR_MSG 400
/*
typedef struct
{
	int errorID;
	char* errorMsg;
}	TErrorType;

//CErrorEngine是一个错误引擎的类，提供了错误管理和报告的机制
class CErrorEngine: public map<int, char*>
{
public:
	CErrorEngine(void);
	virtual ~CErrorEngine(void);

	//设置一个错误类型
	void registerErrorType(int errorID, char* errorMsg);
	//设置一组错误类型	pErrorType	错误类型数组，最后一个的errorID应当为0
	void registerErrorType(TErrorType pErrorType[]);

	void reportError(int errorID);
	char* getErrorMsg(int errorID);

	//获取最近的错误，同时清除错误
	virtual void getLastError(int* pLastErrorID=NULL, char** pLastErrorMsg=NULL);
	//获取最近的错误，但不清除错误
	virtual void getLastErrorWithoutClear(int* pLastErrorID=NULL, char** pLastErrorMsg=NULL);
private:	
	int m_LastErrorID;		//最近的错误编号
	char* m_LastErrorMsg;	//最近的错误信息
};
*/