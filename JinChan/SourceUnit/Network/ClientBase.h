#pragma once
#include "Channel.h"
#include "ServiceName.h"

//CClinet是一个抽象的未完成连接的通讯客户端，实现了一个基于有连接通讯的客户端基本功能。
class CClientBase
{
public:
	CClientBase()	{ SetErrorMsg(NULL); }
	virtual ~CClientBase()	{}
	
	/**连接指定的服务。如果连不上，也应当立即返回，不能在此长时间睡眠。	本方法将使用ConnectImpl完成真正的连接操作
	* @param   pName	要连接的服务名
	* @return  如果连接上，则返回由其产生的CChannel，如果连接不上，则返回NULL如果连接不上，则可以使用GetErrorEvent获取其错误信息
	*/
	virtual CChannel* Connect(CServiceName* pName) = 0;
	
	//获取最近的错误信息，如果最近的操作获得成功，则返回NULL
	char* GetErrorMsg(void) { return m_pErrorMsg; }

protected:
	//设置最近发生的错误信息
	void SetErrorMsg(char* msg) { m_pErrorMsg = msg; }

private:	
	char* m_pErrorMsg;		//存储最近发生的错误信息
};
