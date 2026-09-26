#pragma once

#include "NetworkFactory.h"

class CTcpNetworkFactory : public CNetworkFactory  
{
public:
	CTcpNetworkFactory();
	virtual ~CTcpNetworkFactory();

	/**创建一个TCP服务器端。如果Service Name不匹配则调用链表后面的类厂来创建。
	* @param pName	服务名称
	* @return  得到的服务器端
	*/
	virtual CServerBase* CreateServer(CServiceName* pName);
	
	/**创建一个TCP客户端。如果Service Name不匹配则调用链表后面的类厂来创建。
	* @param	pName	要连接的服务名称
	* @return   得到的客户端
	*/
	virtual CClientBase* CreateClient(CServiceName* pName);

private:
	static CTcpNetworkFactory m_instance;
};
