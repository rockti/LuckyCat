//行情源前置，管理api和行情源的交互
//对话流不再使用Flow对象，只管理序号，减少内存占用
//扩展CSessionFactory::CreateSession参数，便于创建会话时区分不同的连接

#pragma once
#include "UserDataType.h"
#include "EventHandler.h"
#include "Semaphore.h"
#include "Session.h"
#include "FTDSession.h"
#include "MarketRepository.h"
#include "MarketFlowMgr.h"
#include "SessionFactory.h"
#include "ErrorEngine.h"
#include "ErrorDefine.h"
#include "Repository.h"

const int MDF_MAX_SEQUENCE_SERIES = 100;

const int UM_MDFRONT_ADD_SESSION			= 0x41C01 + SM_USER;	//前置增加Session
const int UM_MDFRONT_DEL_SESSION			= 0x41C02 + SM_USER;	//前置删除Session

#define FOPEN 1		//开启flag
#define FCLOSE 1	//关闭flag
#define SHMCOUNT   1000000		//共享内存数量

struct TMarketFrontSessionInfo {
	DWORD nSessionID;
	CSession* pSession;
	bool bIsLogin;
	CLoginIDType LoginID;
	int nDialogFlowSeqNo;				//对话流当前序号
	TMarketFrontSessionInfo& operator =(TMarketFrontSessionInfo const& r) {
		memcpy(this, &r, sizeof(TMarketFrontSessionInfo));
		return *this;
	}
};

class CMarketFront : public CEventHandler, public CSessionCallback, public CFTDSessionCallback {
public:
	CMarketFront(InvestorInfoRepository& investorRepo, CReactorThread* pFrontReactor, CMarketFlowManager* pFlowManager, CEventHandler* pFrontManager);
	virtual ~CMarketFront();

	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);
	//处理从客户端发出的FTD请求
	virtual int HandlePackage(CFTDPackage* pFTDCPackage, CFTDSession* pSession);	

	int AddSession(CFTDSession* pSession);
	int DelSession(DWORD dwSessionID, int nErrorCode);
	CFTDSession* GetSession(DWORD dwSessionID);
protected:
	virtual void OnSessionConnected(CSession *pSession);
	virtual void OnSessionDisconnected(CSession *pSession, int nReason);
	virtual void OnSessionWarning(CSession *pSession, int nReason, int nParam);
	
private:
	void DisconnectAll(int nReason);
	bool IsSessionLogin(DWORD nSessionID);
	bool GetSessionOwner(DWORD nSessionID, CLoginIDType& LoginID);
	void SetSessionOwner(DWORD nSessionID, CReadOnlyLoginIDType& LoginID);
	void ClearSessionOwner(DWORD const nSessionID);
	void InitRspInfo(CFTDRspInfoField* pRspInfoField);
	void SendToDialogFlow(CFTDSession* pSession, CFTDPackage* pPackage);
	void RspNotLogin(CFTDSession* pSession);

	int OnReqUserLogin(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	void OnReqUserLogout(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	int OnReqSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	void sendSnapshot(CFTDDepthMarketDataField* snapshot, CFTDPackage* pFTDPackage, CFTDSession* pSession);
	int OnReqUnSubMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession);
	int OnRspQryMarketData(CFTDPackage* pFTDPackage, CFTDSession* pSession);

	void PublishTopicFlows(CFTDPackage* pSubscribePackage, CFTDSession* pSession);

	void CMarketFront::rspError(CFTDSession* pSession, Error error);
private:
	InvestorInfoRepository& m_investorRepo;
	typedef std::map<DWORD, TMarketFrontSessionInfo> CSessionInfoMap;
	CSessionInfoMap m_sessionInfoMap;

	CMarketFlowManager* m_pFlowManager;
	CEventHandler* m_pFrontManager;
	CFTDPackage m_rspPackage;
};

//管理多个前置数据同步，目前只有单前置
class CMarketFrontManager : public CServerSessionFactory {
public:
	CMarketFrontManager(CReactorThread* pReactor, CMarketFlowManager* pFlowManager);
	~CMarketFrontManager();

	//处理Session退出Event
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);
	//创建Session并发出Event
	virtual CSession* CreateSession(CChannel* pChannel, DWORD dwMark);
	
	//创建一个单独Front前置
	CSelectThread* CreateFrontThread(InvestorInfoRepository& investorRepo);

private:
	CMarketFlowManager* m_pFlowManager;
	CMarketFront* m_pMarketFront;
	CSelectThread* m_pSelectThread;
};