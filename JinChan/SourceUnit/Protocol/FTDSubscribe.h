#pragma once
#include "FlowReader.h"
#include "FTDPackage.h"
#include "Protocol.h"
#include "IReadOnlyFlow.h"
#include "InstrumentManager.h"

//订阅者接口，需要订阅的应用继承该类，一个订阅者只能订阅一个主题(CommPhaseNo)。
//订阅者必须指明主题的通讯阶段序号、已接收消息个数，用于可靠传输
//当指定主题的消息抵达时，订阅驱动器将调用HandleMessage方法，用户可以重载该方法，对消息进行处理。
class CFTDSubscriber
{
public:
	virtual ~CFTDSubscriber(){};
	virtual void SetCommPhaseNo(WORD wCommPhaseNo) = 0;
	virtual WORD GetSequenceSeries() = 0;	//获取流类型
	virtual DWORD GetReceivedCount() = 0;	//已接收消息个数
	virtual void HandleMessage(CFTDPackage* pMessage) = 0;
};

DWORD const NAK_TIMEOUT	= 100;				//每一次NAK在NAK_TIMEOUT毫秒后发出，以后超时加倍
DWORD const RECV_HEARTBEAT_TIMEOUT	= 2000;	//接收心跳超时时间，单位为毫秒
class CFTDSubEndPoint
{
public:
	CFTDSubEndPoint(CFTDSubscriber* pSubscriber):m_pSubscriber(pSubscriber)	{}
	virtual ~CFTDSubEndPoint()	{}

	int HandlePackage(CFTDPackage* pFTDPackage)
	{
		if (pFTDPackage->GetFTDHeader()->SequenceNumber < m_pSubscriber->GetReceivedCount()+1)
			return -1;

		m_pSubscriber->HandleMessage(pFTDPackage);
		return 0;

	}
	inline WORD GetSequenceSeries()	{ return m_pSubscriber->GetSequenceSeries(); }
	inline void SetCommPhaseNo(WORD wCommPhaseNo)	{ m_pSubscriber->SetCommPhaseNo(wCommPhaseNo); }
private:
	CFTDSubscriber* m_pSubscriber;
};

class CFTDPubEndPoint 
{
public:
	CFTDPubEndPoint(IReadOnlyFlow* pFlow, WORD nSequenceSeries, int nStartId, CProtocol* pSendProtocol, InstIndexStore* subStore)
		:m_pFlow(pFlow),m_nSequenceSeries(nSequenceSeries),m_pSendProtocol(pSendProtocol),m_pSubStore(subStore)
	{
		m_SendPackage.ConstructAllocate(FTD_PACKAGE_MAX_SIZE+FTD_HEAD_LEN, 1000);
		m_flowReader.AttachFlow(pFlow, nStartId);
	}
	virtual ~CFTDPubEndPoint()	{}
	int PublishSend()
	{
		int nCount = 0;
		for(; nCount<40; nCount++)
		{
			CPackage* pPackage = GetNextDataPackage();	
			if (NULL == pPackage)
				break;
			if (0 != m_pSendProtocol->Send(pPackage))
				break;
		}
		return nCount;
	}
	void MoveTo(int nOffset)
	{
		if (m_flowReader.GetId() == nOffset)
		{
			return;
		}
		m_flowReader.SetId(nOffset);
	}
	inline WORD GetSequenceSeries()	{ return m_nSequenceSeries; }
private:
	CPackage* GetNextDataPackage()
	{
		m_SendPackage.AllocateMax();
		if(m_flowReader.GetNext(&m_SendPackage))
		{
			FTDHeader* pHeader = m_SendPackage.GetFTDHeader();
			if (NULL != m_pSubStore)
			{
				//如果该合约号没有订阅则不发送数据
				;
				if (!m_pSubStore->Has(m_SendPackage.GetInstrumentIDCount())) {
					Log::Info("has failed");
					return NULL;
				}
			}	
			pHeader->SequenceNumber = m_flowReader.GetId();
			pHeader->SequenceSeries = m_nSequenceSeries;
			return &m_SendPackage;
		}
		return NULL;
	}
private:
	WORD m_nSequenceSeries;
	IReadOnlyFlow* m_pFlow;
	CProtocol* m_pSendProtocol; 
	InstIndexStore* m_pSubStore = nullptr;
	CFTDPackage m_SendPackage;
	CFlowReader m_flowReader;
};
