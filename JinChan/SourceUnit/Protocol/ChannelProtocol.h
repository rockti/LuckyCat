#pragma once

#include "Protocol.h"
#include "ChannelPackage.h"
#include "CacheList.h"

#define MSG_CHANNEL_READ_ERR		0x1001		//读失败
#define MSG_CHANNEL_WRITE_ERR		0x1002		//写失败
#define MSG_CHANNDL_DRSTROY			0x1003		//销毁

//协议栈容器的最底层协议，负责从网络中发送和接受网络数据包，并传递到上层协议
class  CChannelProtocol : public CProtocol  
{
public:
	// @param pReactor 指向运行引擎的指针
	// @param pChannel 捆绑的Channel指针
	// @param maxPackageSize 接收缓冲区的最大长度
	CChannelProtocol(CReactorThread* pReactor, CChannel* pChannel, int maxPackageSize);	
	virtual ~CChannelProtocol();

	virtual int Push(CPackage* pPackage, CProtocol* pUpper);
	
	//获取Channel的Id，并检查是否有发送任务
	//param pReadId　 Channel的Id，用于返回　
	//param pWriteId  检查是否有发送任务，如有，返回Channel的Id，否则，返回NULL
	virtual void GetIds(int* pReadId, int* pWriteId);
	
	//由容器调用，负责从网络读取数据包，并传至上层协议，成功返回0
	virtual int HandleInput();
	
	//由容器调用，负责向网络发送数据包，成功返回0
	virtual int HandleOutput();

	//处理定时操作，由CReactor调用
	virtual void OnTimer(int nIDEvent);

	//负责向网络发送数据包，成功返回0
	int Flush();
	
protected:
	CChannel* m_pChannel;		//指向Channel的指针
	CCacheList m_CacheList;		//发送缓冲区链
	CChannelPackage m_ChannelPackage; //本层协议的对应Package
};
