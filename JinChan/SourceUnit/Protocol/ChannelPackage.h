#pragma once
//定义从Channel获取的原始的Package
#include "Channel.h"
#include "Package.h"

class  CChannelPackage : public CPackage  
{
public:
	CChannelPackage(int maxPackageSize);
	virtual ~CChannelPackage();
	
	//返回上层协议的编号 本框架上层协议只有XMPProtocol并且编号为0，所以永远返回0
	virtual DWORD GetActiveID();
	
	//1:完整包	0:不完整包	-1:错误包
	virtual int ValidPackage();

	int ReadFromChannel(CChannel* pChannel);
};
