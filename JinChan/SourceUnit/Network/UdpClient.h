#pragma once

#include "ClientBase.h"

class CUdpClient : public CClientBase 
{
public:
	CUdpClient();
	virtual ~CUdpClient();

	virtual CChannel* Connect(CServiceName* pName);
};
