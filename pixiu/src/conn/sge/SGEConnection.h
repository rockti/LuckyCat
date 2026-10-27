#pragma once
#include "config/Config.h"
#include "conn/ExchangeConn.h"
#include "conn/SGE/SafeHandler.h"
#include "conn/SGE/SGEHandler.h"

class ConnPublisher;
class SGEConnection : public ExchangeConn {	
public:
	static SGEConnection* s_pThis;
	static void BeforeServiceRun(void* ptr);
	static void AfterServiceStop(void* ptr);

	SGEConnection(ConnConfig& config, Instruments const& instruments, ConnPublisher& publisher);
	virtual void Init();
	virtual void Release();
	virtual void Login(char const* password) {};
	virtual void ResetOrderSet() {}
	virtual void InsertOrder(OrderInfo const& order);
	virtual void InsertExOrder(OrderInfo const& order);
	virtual int InsertMultiOrders(OrderInfo const* orders[]) { return 0; }
	virtual void CancelOrder(OrderCancel const& action);
	virtual void SubscribeQuote(char const* instrumentCode);
	virtual void UnSubscribeQuote(char const* instrumentCode) {}
	virtual void QueryMblMarket(char const* instrumentCode);
	virtual void QueryAccount() {}
	virtual void QueryOrder() {}
	virtual void QueryTrade() {}
	virtual void QueryPosition();
	virtual void QueryFund();

	virtual void SetVertificateCode(char const* password) { throw std::logic_error("SGE can not set VeritificateCode"); }
	virtual void InsertMarketingOrder(MarketingOrderInfo const& dealing) { throw std::logic_error("SGE not support marketing order"); }
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel) { throw std::logic_error("SGE not support marketing order"); }

private:
	void LoadConfig();
	void InitContexts(SGEContext& context, bool isTrade);
	bool InitSafeLinkService();
	bool InitSGELinkService();
	bool GetPublicKey(bool isTrade = true);		// SGE获取公钥
	bool CreateSession(bool isTrade = true);

private:
	ConnConfig& m_config;
	ExchangeConnInstrumentHelper m_instHelper;
	ConnPublisher& m_publisher;

	SafeHandler m_safeHandler;					// 安全接口
	SGEHandler m_sgeHandler;					// 回调接口

	SafeContext m_safeContext;					// 安全上下文
	SGEContext m_tradeContext;					// 交易连接上下文
	SGEContext m_quoteContext;					// 行情连接上下文

	sge_service_cb_t m_sgeServiceCBList;		// SGE连接服务回调
	sge_service_t* m_sgeServicePtr;				// SGE连接服务
	sge_mthread_log_t* m_sgeLogPtr;				// SGE接口日志
	int m_retryTimes = 6;
};
