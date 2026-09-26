#pragma once
#include "IService.h"
#include "IFlow.h"
#include "FIPServerApi.h"


//流应答对象
class CZTPResponser : public IServiceRsponser
{
public:
	CZTPResponser(IFlow* pFlow):m_pFlow(pFlow)	{}
	virtual ~CZTPResponser(){};
	virtual void SendResponse(CZTPPackage* pPackage, DWORD dwSessionID=0) {
		m_pFlow->Append(pPackage->GetAddress(), pPackage->GetLength());
	}
private:
	IFlow* m_pFlow;
};
