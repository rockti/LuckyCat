#include "stdafx.h"
#include "MarketRepository.h"
#include "BaseThread.h"
#include "Profile.h"
#include "UdpClient.h"
#include "FtdDataStruct.h"
/*

CMarketRepository::CMarketRepository(CReactorThread* pReactor) //CEventHandler(pReactor),
:m_pFTDPackage(new CFTDPackage()),m_instrumentCount(0) {
	m_udpFlag = theProfile().GetPrivateProfile("Module", "UdpFlag", 0);
	if (1 == m_udpFlag) {
		string broadcastUrl = theProfile().GetPrivateProfile("Module", "MarketDataUrl", "udp://127.0.0.1:30001");
		CServiceName svrName(broadcastUrl.c_str());
		CUdpClient* pClient = new CUdpClient;
		m_udpChannel = pClient->Connect(&svrName);

// 		int interval = theProfile().GetPrivateProfile("Module", "PublishInterval", 1000);
// 		SetTimer(1, interval);	//广播给交易核心进行风控
	}
	m_pFTDPackage->ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 1000);
}
CMarketRepository::~CMarketRepository() {
	delete m_pFTDPackage;
}

void CMarketRepository::OnTimer(int nIDEvent) {
// 	if (m_marketTopicMap.empty())	//没有数据直接返回
// 		return;
// 
// 	m_lock.Lock();
// 	m_pFTDPackage->PreparePackage(TID_NotifyDynamicRiskCalculate, CHAIN_LAST);	//通知动态计算浮动盈亏
// 	for (MarketTopicMap::iterator topicIter=m_marketTopicMap.begin(); topicIter!=m_marketTopicMap.end(); ++topicIter) {
// 		MarketDataMap* pMarketDataMap = topicIter->second;
// 		for(MarketDataMap::iterator marketIter=pMarketDataMap->begin(); marketIter!=pMarketDataMap->end(); ++marketIter) {		
// 			CFTDDepthMarketDataField* pFTDMarket = marketIter->second;
// 			CFTDDepthMarketDataField ftdDepthMarketData;
// 			memcpy(&ftdDepthMarketData, pFTDMarket, sizeof(CFTDDepthMarketDataField));
// 			if (FTD_ADD_FIELD(m_pFTDPackage, &ftdDepthMarketData) < 0) {	//数据已经加满，先发掉然后重新组包
// 				m_pFTDPackage->MakePackage();
// 				m_udpChannel->Write(m_pFTDPackage->GetLength(), m_pFTDPackage->GetAddress());
// 
// 				m_pFTDPackage->PreparePackage(TID_NotifyDynamicRiskCalculate, CHAIN_LAST);
// 				FTD_ADD_FIELD(m_pFTDPackage, &ftdDepthMarketData);
// 			}
// 		}
// 	}
// 
// 	//发送最后一个批次
// 	m_pFTDPackage->MakePackage();
// 	m_udpChannel->Write(m_pFTDPackage->GetLength(), m_pFTDPackage->GetAddress());
// 
// 	m_lock.UnLock();
}
DWORD CMarketRepository::GetInstrumentIndex(CInstrumentIDType instrmentID) {
	auto iter = m_instrumentIndexMap.find(instrmentID);
	return (iter == m_instrumentIndexMap.end()) ? -1 : iter->second;
}
DWORD CMarketRepository::InsertMaraketData(int nTopicID, CFTDDepthMarketDataField* pMarketData) {
	m_lock.Lock();

	//先更新主题map
	MarketDataMap* pMarketDataMap = NULL;
	MarketTopicMap::iterator topicIter = m_marketTopicMap.find(nTopicID);
	if(topicIter == m_marketTopicMap.end()) {	
		pMarketDataMap = new MarketDataMap();	
		m_marketTopicMap.insert(MarketTopicMap::value_type(nTopicID, pMarketDataMap));
	} else {
		pMarketDataMap = topicIter->second;
	}

	//再更新行情数据map
	CFTDDepthMarketDataField* pData = NULL;
	MarketDataMap::iterator iter = pMarketDataMap->find(pMarketData->InstrumentID);
	if(iter == pMarketDataMap->end()) {
		pData = new CFTDDepthMarketDataField();		
		memcpy(pData, pMarketData, sizeof(CFTDDepthMarketDataField));
		pMarketDataMap->insert(MarketDataMap::value_type(pData->InstrumentID, pData));
	} else {
		pData = iter->second;
		memcpy(pData, pMarketData, sizeof(CFTDDepthMarketDataField));
	}

	//最后更新IndexMap
	DWORD instrumentCount = m_instrumentCount;
	auto indexIter = m_instrumentIndexMap.find(pMarketData->InstrumentID);
	if(indexIter == m_instrumentIndexMap.end()) {
		m_instrumentIndexMap.emplace(pMarketData->InstrumentID, m_instrumentCount++);
	} else {
		instrumentCount = indexIter->second;
	}
	m_lock.UnLock();

	return instrumentCount;
}
CFTDDepthMarketDataField* CMarketRepository::GetMaraketData(CInstrumentIDType InstrumentID) {
	CFTDDepthMarketDataField* pFTDDepthMarketDataField = NULL;
	for(MarketTopicMap::iterator topicIter=m_marketTopicMap.begin(); topicIter!=m_marketTopicMap.end(); ++topicIter) {
		MarketDataMap* pMarketDataMap = topicIter->second;
		MarketDataMap::iterator iter = pMarketDataMap->find(InstrumentID);
		if(iter != pMarketDataMap->end()) {
			pFTDDepthMarketDataField = iter->second;
			return pFTDDepthMarketDataField;
		}
	}
	return pFTDDepthMarketDataField;
}
*/