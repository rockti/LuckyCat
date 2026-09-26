//报盘管理线程
#pragma once
#include "EventHandler.h"
#include "ZtpDataStruct.h"
#include "BaseExchApi.h"
#include "Repository.h"

//read request from pTReq,send response to pTSeries,read seat info from pConfig
class CExchangeLinkManager // : public CSelectThread
{
public:
	CExchangeLinkManager();
	virtual ~CExchangeLinkManager()	{}

	CBaseExchApi* GetSGEHandler(InstrumentRepository* instRepo = nullptr, data::CSeatInfo const* seatInfo = nullptr);
	void StartWork();


private:
	void initExchLink(InstrumentRepository& instRepo, data::CSeatInfo const* seatInfo);
	CBaseExchApi* m_pTradeLink = nullptr;
};