#include "stdafx.h"
#include "FTDProtocol.h"


CFTDProtocol::CFTDProtocol(CReactorThread* pReactor)
:CProtocol(pReactor, FTD_HEAD_LEN, new CFTDPackage(), new CFTDPackage())
{
}
CFTDProtocol::~CFTDProtocol()
{
	Clear();
}

void CFTDProtocol::Clear() {
	for (auto& kv : m_subEndPointMap) {
		delete kv.second;
	}
	m_subEndPointMap.clear();

	for (auto& kv : m_pubEndPointMap) {
		delete kv.second;
	}
	m_pubEndPointMap.clear();
}

int CFTDProtocol::Pop(CPackage* pPackage)
{
	CFTDPackage* pFTDPackage = (CFTDPackage*)pPackage;
	CFTDSubEndPoint* pSubEndPoint = GetSubEndPoint(pFTDPackage->GetFTDHeader()->SequenceSeries);
	if (NULL != pSubEndPoint)
	{
		return pSubEndPoint->HandlePackage(pFTDPackage);
	}
	return CProtocol::Pop(pPackage);
}
void CFTDProtocol::PublishSend() {

	for (auto& kv : m_pubEndPointMap) {
		if (kv.second != nullptr) {
			kv.second->PublishSend();
		}
	}
}
int CFTDProtocol::OnRecvErrorPackage(CPackage* pPackage)
{
	NotifyError(MSG_FTDERR_BADPACKAGE, 0, this);
	return 0;
}

void CFTDProtocol::Publish(IReadOnlyFlow* pFlow, WORD nSequenceSeries, int nStartId)
{
	CFTDPubEndPoint* pPubEndPoint = GetPubEndPoint(nSequenceSeries);
	if (NULL == pPubEndPoint) {
		pPubEndPoint = new CFTDPubEndPoint(pFlow, nSequenceSeries, nStartId, this, m_pSubStore);
		m_pubEndPointMap.emplace(nSequenceSeries, pPubEndPoint);
		pPubEndPoint->MoveTo(nStartId); //discuss TODO
	}
}
void CFTDProtocol::UnPublish(WORD nSequenceSeries)
{
	CFTDPubEndPoint* pPubEndPoint = GetPubEndPoint(nSequenceSeries);
	if (NULL != pPubEndPoint)
	{
		delete pPubEndPoint;
		m_pubEndPointMap.erase(m_pubEndPointMap.find(nSequenceSeries));
	}
}

void CFTDProtocol::RegisterSubscriber(CFTDSubscriber* pSubscriber)
{
	WORD nSequenceSeries = pSubscriber->GetSequenceSeries();
	CFTDSubEndPoint* pSubEndPoint = GetSubEndPoint(nSequenceSeries);
	if (NULL == pSubEndPoint)
	{
		pSubEndPoint = new CFTDSubEndPoint(pSubscriber);
		m_subEndPointMap.emplace(nSequenceSeries, pSubEndPoint);
	}
}
void CFTDProtocol::UnRegisterSubscriber(CFTDSubscriber* pSubscriber)
{
	WORD nSequenceSeries = pSubscriber->GetSequenceSeries();
	CFTDSubEndPoint* pSubEndPoint = GetSubEndPoint(nSequenceSeries);
	if (NULL != pSubEndPoint)
	{
		delete pSubEndPoint;
		m_subEndPointMap.erase(m_subEndPointMap.find(nSequenceSeries));
	}
}

CFTDSubEndPoint* CFTDProtocol::GetSubEndPoint(WORD nSequenceSeries)
{
	auto iter = m_subEndPointMap.find(nSequenceSeries);
	if (iter != m_subEndPointMap.end()) {
		return iter->second;
	}
	return nullptr;
}

CFTDPubEndPoint* CFTDProtocol::GetPubEndPoint(WORD nSequenceSeries) {
	auto iter = m_pubEndPointMap.find(nSequenceSeries);
	if (iter != m_pubEndPointMap.end()) {
		return iter->second;
	}
	return nullptr;
}

void CFTDProtocol::SetCommPhaseNo(WORD wCommPhaseNo) {
	for (auto& kv : m_subEndPointMap) {
		kv.second->SetCommPhaseNo(wCommPhaseNo);
	}
}
void CFTDProtocol::SetSubStore(InstIndexStore* subStore)
{
	m_pSubStore = subStore;
}
