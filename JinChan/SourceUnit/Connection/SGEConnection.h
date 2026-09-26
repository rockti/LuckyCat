#pragma once
#include "MarketFlowMgr.h"
#include "MarketRepository.h"
#include "SGEHandler.h"
#include "SGEQuoteHandler.h"


class SGEConnectionBase
{
public:
	static SGEConnectionBase* s_pThis;
	static void BeforeServiceRun(void* ptr);
	static void AfterServiceStop(void* ptr);

	SGEConnectionBase(std::string const& sgeAddress, int sgePort, std::string const& safeAddress, int safePort);
	void initSGEContext(SGEContext& sgeContext);
	virtual bool InitInstance(CSelectThread* pFrontThread = nullptr);
	virtual void ExitInstance();


protected:
	bool InitSafeDevice();
	bool InitLinkService();
	bool CreateSGESession();

protected:
	SafeContext m_safeContext;					//安全上下文
	SGEContext m_sgeContext;					//交易连接上下文
	sge_mthread_log_t* m_sgeLogPtr;				//SGE接口日志
	sge_service_t* m_sgeServicePtr;				//SGE连接服务
	sge_service_cb_t m_sgeServiceCBList;		//SGE连接服务回调

	std::string m_sgeAddress;
	int m_sgePort; 
	std::string m_safeAddress;
	int m_safePort;

};

class SGETradeConnection : public SGEConnectionBase {
public:
	SGETradeConnection(InstrumentRepository& instRepo, std::string const& sgeAddress, int sgePort, std::string const& safeAddress, int safePort);
	virtual bool InitInstance(CSelectThread* pFrontThread = nullptr);
	virtual void ExitInstance();

	bool CreateStorageSession();
	SGETradeHandler& GetHandler() { return m_sgeHandler; }

private:
	SGETradeHandler m_sgeHandler;					//连接回调接口
	SGEContext m_storageContext;				//仓储连接上下文
};
