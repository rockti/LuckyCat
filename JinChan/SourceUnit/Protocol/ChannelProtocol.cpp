#include "stdafx.h"
#include "ChannelProtocol.h"
#include "SocketInit.h"
CChannelProtocol::CChannelProtocol(CReactorThread* pReactor, CChannel* pChannel, int maxPackageSize)
:CProtocol(pReactor,0,NULL,NULL), m_CacheList(20000),m_ChannelPackage(maxPackageSize)
{
	m_pChannel = pChannel;
	if (CT_STREAM == pChannel->GetType())
	{
		SetTimer(1, 1000);		//打开定时强制刷新定时器
	}
}
CChannelProtocol::~CChannelProtocol()
{
}

int CChannelProtocol::Push(CPackage* pPackage, CProtocol* pUpper)
{
	if (CT_STREAM == m_pChannel->GetType())
	{
		m_CacheList.PushBack(pPackage->GetAddress(),pPackage->GetLength());
		return 0;
	}
	else
	{
		int len = m_pChannel->Write(pPackage->GetLength(),pPackage->GetAddress());
		if (len != pPackage->GetLength())
		{
			return -1;
		}
	}
	return 0;
}
void CChannelProtocol::GetIds(int* pReadId, int* pWriteId)
{
	if (!m_pChannel->Available())
	{
		*pReadId = 0;
		*pWriteId = 0;
		return;
	}

	*pReadId = (NULL!=m_pChannel) ? m_pChannel->GetId() : 0;
	*pWriteId = (!m_CacheList.IsEmpty()) ? m_pChannel->GetId() : 0;
}

int CChannelProtocol::HandleInput()
{
	int rtn = 0;
	for (int i=0; i<8; i++)
	{
		int nReadLen = m_ChannelPackage.ReadFromChannel(m_pChannel);
		if(nReadLen<0)
		{
			auto error = GET_LAST_SOCK_ERROR();
			NotifyError(MSG_CHANNEL_READ_ERR, error, this);
			return -1;
		}
		if (nReadLen == 0)
		{
			break;
		}
		rtn = Pop(&m_ChannelPackage);
		if (rtn != 0)
		{
			return rtn;
		} 
	}
	return rtn;
}

int CChannelProtocol::HandleOutput()
{
 	return Flush();
}
void CChannelProtocol::OnTimer(int nIDEvent)
{
	Flush();	//定时写，防止select对write的检查失效
}
int CChannelProtocol::Flush()
{
	if (!m_pChannel->CheckConnection())
	{
		return 0;
	}

	for(int i=0; i<8; i++)
	{
		int nDataLen = 8192;		//--可能需要调整	
		char* pData = (char*)m_CacheList.GetData(nDataLen);		
		if(NULL == pData)
			break;
		
		int len = m_pChannel->Write(nDataLen, pData);
		if(len < 0)
		{
			auto error = GET_LAST_SOCK_ERROR();
			NotifyError(MSG_CHANNEL_WRITE_ERR, error, this);
			return len;
		}
		m_CacheList.PopFront(len);
				
		if(len != nDataLen)
		{
			break;
		}
	}
	return 0;
}
