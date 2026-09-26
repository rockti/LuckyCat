#pragma once

#include "Channel.h"
#include "ClientBase.h"
#include "ServerBase.h"
#include "ServiceName.h"


//CNetworkFactory是所有网络对象的对象工厂，负责这些对象的生产。
//开发者不需要显式地调用任何网络对象的构造函数，只需要创建一个CNetworkFactory，然后由它来负责生产所有其它的网络对象。
class  CNetworkFactory
{
public:
	CNetworkFactory();
	virtual ~CNetworkFactory();

	static CNetworkFactory* GetInstance();

	/**创建一个服务器端。本方法只是依次调用链表中的每个类厂来创建服务器端。
	* 具体的创建工作要在派生类中通过重载实现
	* @param pName	服务名称
	* @return  得到的服务器端
	*/
	virtual CServerBase* CreateServer(CServiceName* pName);
	
	/**通过创建客户端，创建一个连接。
	* @param	pName	要连接的服务名称
	* @return       得到的网络连接
	*/
	virtual CChannel* CreateChannel(CServiceName* pName);

protected:
	/**创建一个客户端。本方法只是依次调用链表中的每个类厂来创建客户端。
	* 具体的创建工作要在派生类中通过重载实现
	* @param	pName	要连接的服务名称
	* @return   得到的客户端
	*/
	virtual CClientBase* CreateClient(CServiceName *pName);
	void RegisterFactory();

private:
	static CNetworkFactory* m_pInstance;	
	CNetworkFactory* m_pNext;
};
