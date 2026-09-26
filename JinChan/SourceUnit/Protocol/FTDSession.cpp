#include "stdafx.h"
#include "FTDSession.h"
#include "FtdDataStruct.h"
#include "InstrumentManager.h"

CFTDSession::CFTDSession(CReactorThread* pReactor, CChannel* pChannel)
:CXMPSession(pReactor, pChannel),m_pPackageHandler(NULL)
{
	m_pCompressProtocol = new CCompressProtocol(m_pIOThread);
	m_pCompressProtocol->AttachLower(m_pXMPProtocol, XMPTypeCompressed);
	m_pFTDProtocol = new CFTDProtocol(m_pIOThread);
	m_pFTDProtocol->AttachLower(m_pCompressProtocol, XMPTypeFTD);

	m_pFTDProtocol->RegisterErrorHandler(this);
	m_pFTDProtocol->RegisterUpperHandler(this);
}
CFTDSession::~CFTDSession()
{
	delete m_pFTDProtocol;
	delete m_pCompressProtocol;
}

void CFTDSession::GetIds(int* pReadId, int* pWriteId)
{
	CXMPSession::GetIds(pReadId, pWriteId);
	if (0 == *pWriteId)
	{
		m_pFTDProtocol->PublishSend();
		CXMPSession::GetIds(pReadId, pWriteId);
	}
}
int CFTDSession::HandleEvent(int nEventID, DWORD dwParam, void* pParam)
{
	switch(nEventID)
	{
	case MSG_XMPERR_RECVHEARTBEAT:
	case MSG_XMPERR_SENDHEARTBEAT:
	case MSG_XMPERR_BADPACKAGE:
	case MSG_FTDERR_BADPACKAGE:
		Disconnect(nEventID);
		return 0;
	case MSG_XMPWARNING_RECVHEARTBEAT:
		if (NULL != m_pSessionCallback)
		{
			m_pSessionCallback->OnSessionWarning(this, nEventID, (int)dwParam);
		}
		return 0;
	}

	return CSession::HandleEvent(nEventID, dwParam, pParam);
}	
int CFTDSession::HandlePackage(CPackage* pPackage, CProtocol* pProtocol)
{
	if (pProtocol == m_pFTDProtocol)
	{
		CFTDPackage* pFTDPacakge = (CFTDPackage*)pPackage;
		if (NULL == m_pPackageHandler)
			return 0;
		return m_pPackageHandler->HandlePackage(pFTDPacakge, this);
	}
	return 0;
}
void CFTDSession::Disconnect(int nErrorCode)
{
	if (0 == nErrorCode)
	{
		m_pFTDProtocol->PublishSend();	//如果是主动断开，则尝试发送流中未发完的包
	}
	m_pFTDProtocol->Clear();

	CXMPSession::Disconnect(nErrorCode);
}

void CFTDSession::CreatSubInstrument() {
	m_pFTDProtocol->SetSubStore(&m_SubInstruments);
}
bool CFTDSession::InsertInstrumentID(DWORD instrumentIDCount, CInstrumentIDType instrumentID) {
	if (m_SubInstruments.Has(instrumentIDCount)) {
		return false;
	} else {
		m_SubInstruments.Add(InstIndex::Of(instrumentIDCount));
		return true;
	}
}
bool CFTDSession::DeleteInstrumentID(DWORD instrumentIDCount)
{
	if(m_SubInstruments.Has(instrumentIDCount)) {
		m_SubInstruments.Remove(InstIndex::Of(instrumentIDCount));
		return true;
	} else {
		return false;
	}
}
