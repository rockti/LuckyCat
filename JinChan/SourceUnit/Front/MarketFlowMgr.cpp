#include "stdafx.h"
#include "MarketFlowMgr.h"
#include "CachedFlow.h"
#include "FileFlow.h"
#include "CTime.h"

CMarketFlowManager::CMarketFlowManager(char const* pszFlowPath, IFlow* marketFlow)
:m_pMarketFlow(marketFlow) {
	m_pTradingDayFlow = new CFileFlow("TradingDay", pszFlowPath, true);
}
CMarketFlowManager::~CMarketFlowManager() {
	for(iterator itor=begin(); itor != end(); itor++) {
		delete (*itor).second;
	}
	delete m_pTradingDayFlow;
}

IFlow* CMarketFlowManager::GetTopicFlow(DWORD nTopicID) {
	iterator itor = find(nTopicID);
	if (itor == end()) {
		return NULL;
	}
	return (*itor).second;
}
bool CMarketFlowManager::RegisterTopicFlow(DWORD nTopicID) {
	if (NULL != GetTopicFlow(nTopicID))
		return false;

	//这里使用内存流且不同步，因为重启后合约信息都没有了
	IFlow* pFlow  = new CCachedFlow(false, 0x7fffffff, 0x100000);
	pFlow->SetCommPhaseNo(m_pTradingDayFlow->GetCommPhaseNo());	//设置通讯阶段编号
	(*this)[nTopicID] = pFlow;
	return true;
}
void CMarketFlowManager::SetTradingDay(type::TradingDay& tradingDay, IFlow* topicflow) {
	m_tradingDay = tradingDay;
	m_pTradingDayFlow->SetCommPhaseNo((WORD)CDate::DateToLong(tradingDay.value()));
	topicflow->SetCommPhaseNo(m_pTradingDayFlow->GetCommPhaseNo());
}
int CMarketFlowManager::PulishToTopicFlow(CFTDPackage* pPackage, DWORD TopicID) {
	IFlow* pFlow = GetTopicFlow(TopicID);
	if (NULL == pFlow)	return -1;
	
	pPackage->MakePackage();		//把包转换成网络序
	if (NULL != m_pMarketFlow) {	//如果有行情流给行情流发送一份	
		m_pMarketFlow->Append(pPackage->GetAddress(), pPackage->GetLength());
	}
	return pFlow->Append(pPackage->GetAddress(), pPackage->GetLength());		//发给主题流
}
