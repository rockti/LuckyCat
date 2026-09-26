#pragma once
#include "MarketFlowMgr.h"
#include "MarketRepository.h"
#include "MockSGEHandler.h"


class MockSGEConnection //: public CSelectThread
{
public:
	static MockSGEConnection* s_pThis;
	static void BeforeServiceRun(void* ptr);
	static void AfterServiceStop(void* ptr);

	MockSGEConnection(SgeLinkType::TYPE type, CMarketFlowManager* pFlowManager,
		int topicID, std::string const& sgeAddress, int sgePort, std::string const& safeAddress, int safePort);
	virtual bool InitInstance(CSelectThread* pFrontThread=NULL);
	virtual void ExitInstance();

	SGEMockHandler* GetHandler() { return &m_sgeHandler; }

private:
	bool InitSafeDevice();
	bool InitLinkService();
	bool CreateLinkSession();

private:

	SgeLinkType::TYPE m_linkType;
	CMarketFlowManager* m_pFlowManager;		//流管理器
	SGEContext m_sgeContext;				//连接上下文
	SafeContext m_safeContext;				//安全上下文
	SGEMockHandler m_sgeHandler;			//回调接口
	int m_topicID;							//主题号

	sge_mthread_log_t* m_sgeLogPtr;				//SGE接口日志
	sge_service_t* m_sgeServicePtr;				//SGE连接服务
	sge_service_cb_t m_sgeServiceCBList;		//SGE连接服务回调
};
