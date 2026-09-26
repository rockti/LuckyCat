#pragma once
#include "ZTPPackage.h"
#include "UserDataType.h"
/*
typedef struct
{
	CSessionIDType	SessionID;		//会话编号
	CRequestIDType  RequestID;		//请求编号
}	TSessionRef;

//复制requestID等信息，便于回报给客户
#define Copy_Request_Info(target,source) (target)->RequestID=(source)->RequestID;

class IZTPServiceLink
{
public:
	virtual ~IZTPServiceLink()	{};
	virtual bool HandleMessage(CZTPPackage* pPackage, TSessionRef const& sessionRef) = 0;
};

//IServiceRsponser是一个ZTP应答发送接口
class IServiceRsponser
{
public:
	virtual ~IServiceRsponser(){};
	virtual void SendResponse(CZTPPackage* pPackage, DWORD dwSessionID=0) = 0;
};
*/