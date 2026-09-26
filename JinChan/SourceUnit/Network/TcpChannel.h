#pragma once

#include "Channel.h"

//CTcpChannel是一个基于TCP的网络连接对象
class  CTcpChannel : public CChannel
{
public:
	//构造函数，使用指定的socket，创建此对象
	CTcpChannel(int socketId);
	virtual ~CTcpChannel(void);
	
	//获取对方的地址，例如其IP地址等
	virtual char* GetRemoteName(void);

    //获取本地的地址IP
    virtual char* GetLocalName(void);

protected:
	/**基于TCP的读取实现函数
	* @param number	buffer缓冲区的长度
	* @param buffer	存放缓冲区
	* @return >0 实际读取得长度
	* @return 0  读取时遭中断返回
	* @return -1 连结断开
	*/
	virtual int ReadImp(int number, char* buffer);

	/**基于TCP的写出实现函数
	* @param number	准备发送的长度
	* @param buffer	存放缓冲区
	* @return >0 实际发送得长度
	* @return 0  读取时遭中断返回
	* @return -1 发送失败
	*/
	virtual int WriteImp(int number, char* buffer);
	
	/**基于TCP的判断可读性实现函数
	* @return true  可读
	* @return false 不可读
	*/
	virtual bool AvailableImp(void);
	
	/**基于TCP的断开连接实现函数
	* @return true  断开成功
	* @return false 断开失败
	*/
	virtual bool DisconnectImp(void);
	
	/**基于TCP的检查连接性实现函数
	* @return true  连接正常
	* @return false 连接断开
	*/
	virtual bool CheckConnectionImp(void);

protected:
	DWORD m_dwInAddr;	 // 对端IP
    DWORD m_dwLocalAddr; // 本地IP
};
