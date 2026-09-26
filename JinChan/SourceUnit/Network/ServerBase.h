#pragma once

#include "Channel.h"
#include "ServiceName.h"

//CServerBase是一个抽象的未完成连接的通讯服务器端，实现了一个基于有连接通讯的服务器端基本功能。
class  CServerBase 
{
public:
	CServerBase(CServiceName* pName):m_pServiceName(pName)	{ SetErrorMsg(NULL); }
	virtual ~CServerBase() {}
	
	/**等待一个客户端的连接，本方法将使用acceptImpl完成真正的操作
	* @param  wait	表示需要等待到连接才返回，0表示不等待
	* @return NULL   新Channel未建成功
	* @return 非NULL 新的Channel
	*/
	virtual CChannel* Accept(int wait) = 0;
	
	//获取最近的错误信息，如果最近的操作获得成功，则返回NULL
	char const* GetErrorMsg(void) { return m_pszErrorMsg; }

	//获取本服务器端的服务名称
	CServiceName* GetServiceName(void) { return m_pServiceName; }
	
	//获取本服务器端在进行select时，使用的文件号。 如果本网络实现使用的是查询式的，则应当返回-1
	virtual int GetId(void)=0;

protected:
	void SetErrorMsg(char* pszMsg) { m_pszErrorMsg = pszMsg; }
	
private:
	CServiceName* m_pServiceName;   //存储本服务器端的服务名称
	char* m_pszErrorMsg;			//存储最近发生的错误信息
};

