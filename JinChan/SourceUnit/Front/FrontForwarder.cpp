#include "stdafx.h"
#include "FrontForwarder.h"

CFrontForwarder::CFrontForwarder(IFlow* pPublishFlow)
:m_pPublishFlow(pPublishFlow) {
}
CFrontForwarder::~CFrontForwarder() {
}

//准备ZTP数据包包头信息
void CFrontForwarder::ForwardHead(CZTPPackage* pZTPPackage, DWORD TID, DWORD nSessionID, DWORD nRequestID) {
	pZTPPackage->PreparePublish(TID);
	pZTPPackage->SetSessionID(nSessionID);
	pZTPPackage->SetRequestID(nRequestID);
}

//组装ZTP格式的请求包，并发布到SeriesFlow中
void CFrontForwarder::ForwardTransaction(CZTPPackage* pZTPPackage) {
	pZTPPackage->MakePackage();
	m_pPublishFlow->Append(pZTPPackage->GetAddress(), pZTPPackage->GetLength());
}
