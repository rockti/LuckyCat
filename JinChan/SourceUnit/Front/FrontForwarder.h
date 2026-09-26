#pragma once
#include "IFlow.h"
#include "ZTPPackage.h"

//ZTP数据包预处理器
class CFrontForwarder  
{
public:
	CFrontForwarder(IFlow* pPublishFlow);
	virtual ~CFrontForwarder();

	void SetCommPhaseNo(WORD nCommPhaseNo)	{ m_pPublishFlow->SetCommPhaseNo(nCommPhaseNo); }
	void ForwardHead(CZTPPackage* pZTPPackage, DWORD TID, DWORD nSessionID, DWORD nRequestID);
	void ForwardTransaction(CZTPPackage* pZTPPackage);
private:
	IFlow* m_pPublishFlow;
};
