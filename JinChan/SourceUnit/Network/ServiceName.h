#pragma once


/*CServiceName是表示一个网络服务名称的对象。其意义类似于URL。
*这里的网络服务名称由四部分组成，channel表示网络通讯的类型，host表示主机，port表示端口号，name表示内部的文件或者服务名。
*网络通讯类型是必须有的值，其他的值可以根据网络通讯的要求，设置其中某些部分。例如对于TCP，host和port是有意义的，但是name就没有意义。*/
class  CServiceName
{
public:
	/**构造函数，创建一个新的服务名称
	*@参数	location	服务名称，其表示方式为channel://host:port/name
	*			其简化方式包括
	*			channel://host/name
	*			channel://host:port
	*			channel://:port/name
	*			channel://host
	*			channel:///name
	*			channel://:port
	*/
	CServiceName(char const* location);
		
	//析构函数
	virtual ~CServiceName();
	
	//获取网络通讯类型
	char* GetChannel(void) const { return m_pChannel; }
	
	//获取主机名或地址
	char* GetHost(void) const { return m_pHost;}
		
	//获取文件名或服务名
	char* GetName(void) { return m_pServiceName; }

	//获取服务名称（tcp://127.0.0.1:13810）
	char* GetLocation(void) { return m_pLocation; }

	//获取端口号
	int GetPort(void) const { return m_nPort; }
private:
	char* m_pLocation;		//存放服务名称的串
	char* m_pBuffer;		//供分析用的服务名称
	char* m_pChannel;		//存储网络通讯类型
	char* m_pHost;			//存储主机名或地址	
	char* m_pServiceName;   //获取文件名或服务名
	int m_nPort;			//获取端口号
};
