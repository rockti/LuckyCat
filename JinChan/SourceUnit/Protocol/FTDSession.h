#pragma once

#include "FTDPackage.h"
#include "FTDProtocol.h"
#include "XMPSession.h"
#include "CompressProtocol.h"

#include "InstrumentManager.h"

class CFTDSession;
class CFTDSessionCallback {
public:
	virtual int HandlePackage(CFTDPackage* pFTDPackage, CFTDSession* pSession) = 0;
};

//是一个存放协议栈的容器，是基于IO处理的EventHandler类，一般是由ListenCtrl创建，它必须注册到一个Reactor。
//容器的底部捆绑在一个TcpChannel上，容器内的协议只有通过容器才能得到读写操作
class CFTDSession : public CProtocolCallback, public CXMPSession
{
public:
	//@param pReactor   运行引擎	@param pChannel   TcpChannel 	
	CFTDSession(CReactorThread* pReactor, CChannel* pChannel);
	
	virtual ~CFTDSession();
	
	virtual void GetIds(int* pReadId, int* pWriteId);
	virtual int HandleEvent(int nEventID, DWORD dwParam, void *pParam);
	virtual int HandlePackage(CPackage* pPackage, CProtocol* pProtocol);
	virtual void Disconnect(int nErrorCode);
	
	void RegisterPackageHandler(CFTDSessionCallback* pPackageHandler)	{ m_pPackageHandler = pPackageHandler; }
	void Publish(IReadOnlyFlow* pFlow, WORD wSequenceSeries, int nStartId)	{ m_pFTDProtocol->Publish(pFlow, wSequenceSeries, nStartId); }
	void UnPublish(WORD wSequenceSeries)	{ m_pFTDProtocol->UnPublish(wSequenceSeries); }

	void RegisterSubscriber(CFTDSubscriber* pSubscriber)	{ m_pFTDProtocol->RegisterSubscriber(pSubscriber); }
	void UnRegisterSubscriber(CFTDSubscriber* pSubscriber)	{ m_pFTDProtocol->UnRegisterSubscriber(pSubscriber); }
		
	void SendRequestPackage(CFTDPackage* pPackage)	{ m_pFTDProtocol->Send(pPackage); }
	void SetCommPhaseNo(WORD wCommPhaseNo)	{ m_pFTDProtocol->SetCommPhaseNo(wCommPhaseNo); }
	void SetCompressMethod(BYTE chCompressMethod)	{ m_pCompressProtocol->SetCompressMethod(XMPTypeFTD, chCompressMethod); }
	
	void CreatSubInstrument();
	bool InsertInstrumentID(DWORD instrumentIDCount, CInstrumentIDType instrumentID);
	bool DeleteInstrumentID(DWORD instrumentIDCount);

	InstIndexStore m_SubInstruments;
protected:
	CFTDProtocol* m_pFTDProtocol;
	CCompressProtocol* m_pCompressProtocol;
	CFTDSessionCallback* m_pPackageHandler;
};
