#pragma once

#include "serverBase.h"

//CTcpServer是一个基于TCP的网络服务器端
class  CTcpServer : public CServerBase
{
public:
	CTcpServer(CServiceName* pName);
	virtual ~CTcpServer();
	
	//基于TCP的获取select文件号方法
	virtual int GetId(void);

protected:
	/*基于TCP的等待客户端连接方法
	* @param  wait	表示需要等待到连接才返回，0表示不等待
	* @return NULL   新Channel未建成功
	* @return 非NULL 新的Channel
	*/
	virtual CChannel* Accept(int wait=0);
	virtual CChannel* CreateChannel(int id);
private:
	int m_nID;	//存储本服务器端的文件号
};
