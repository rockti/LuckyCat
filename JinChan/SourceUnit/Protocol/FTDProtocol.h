#pragma once
#include "Protocol.h"
#include "FTDPackage.h"
#include "FTDSubscribe.h"
#include "InstrumentManager.h"

#define MSG_FTDERR_BADPACKAGE		0xA001
#define MSG_INVALID_USERID			0xB001

//协议栈容器的第第三层协议，主要对FTD包的处理
class  CFTDProtocol : public CProtocol  
{
public:
	CFTDProtocol(CReactorThread* pReactor);
	virtual ~CFTDProtocol();

	virtual int Pop(CPackage* pPackage);
	virtual int OnRecvErrorPackage(CPackage *pPackage);

	void PublishSend();
	void Publish(IReadOnlyFlow* pFlow, WORD nSequenceSeries, int nStartId);
	void UnPublish(WORD nSequenceSeries);
	void RegisterSubscriber(CFTDSubscriber* pSubscriber);
	void UnRegisterSubscriber(CFTDSubscriber* pSubscriber);
	void SetCommPhaseNo(WORD wCommPhaseNo);
	void SetSubStore(InstIndexStore* subStore);
	void Clear();
	
private:
	CFTDSubEndPoint* GetSubEndPoint(WORD nSequenceSeries);
	CFTDPubEndPoint* GetPubEndPoint(WORD nSequenceSeries);

private:
	typedef std::map<WORD, CFTDSubEndPoint*> CSubEndPointMap;
	CSubEndPointMap m_subEndPointMap;

	typedef std::map<WORD, CFTDPubEndPoint*> CPubEndPointMap;
	CPubEndPointMap m_pubEndPointMap;

	InstIndexStore* m_pSubStore = nullptr;
};
// 	typedef CHashMap<DWORD,CInstrumentIDType,HashInt> CSubInstrumentMap;	//合约对应的ID
