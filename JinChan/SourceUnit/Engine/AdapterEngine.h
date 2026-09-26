#include "stdafx.h"
#include "FrontEngine.h"
#include "FTDSession.h"
#include <unordered_map>
#include <optional>
#include "CachedFlow.h"
#include "ZTPType.h"
using namespace ztp;

class InvestorSessionManager {
	using FlowPtr = IFlow* ;
public:
	~InvestorSessionManager() {
		for (auto& kv : m_investorFlows) {
			delete kv.second;
		}
		for (auto& kv : m_operFlows) {
			delete kv.second;
		}
	}
	void InvestorLogined(type::InvestorID const& InvestorID, TFrontSessionInfo const& sessionInfo);
	void InvestorLogouted(type::InvestorID const& InvestorID, TFrontSessionInfo const& sessionInfo);

	void SessionDisconnected(DWORD SessionID);

	void OperatorLogined(type::OperatorID const& OperatorID, TFrontSessionInfo const& sessionInfo);
	void OperatorLogouted(type::OperatorID const& OperatorID, TFrontSessionInfo const& sessionInfo);
	void Notify(type::InvestorID InvestorID, CFTDPackage* pPackage);
	void NotifyAllInvestor(CFTDPackage* pPackage);

	std::optional<TFrontSessionInfo> GetSessionInfo(DWORD sessionID) const;

private:
	std::map<DWORD, TFrontSessionInfo> m_sessions;

	std::map<type::InvestorID, FlowPtr> m_investorFlows;
	std::map<type::OperatorID, FlowPtr> m_operFlows;
};

//采用单前置模式，不用考虑多个前置之间的数据同步
class CAdapteEngine : public CFrontEngine, public CFTDSessionCallback 
{
public:
	CAdapteEngine(CReactorThread* pReactor, CFrontForwarder* pFrontForwarder);
	virtual ~CAdapteEngine();
	virtual void OnTimer(int nIDEvent);
	virtual int HandlePackage(CFTDPackage* pFTDCPackage, CFTDSession* pSession);

	//处理从总线收到的ZTP报文
	void HandleMessage(CZTPPackage* pMessage);
	void InitOperatorInvestor();

protected:
	virtual CSession* CreateSession(CChannel* pChannel, DWORD dwMark);
	virtual void OnSessionDisconnected(CSession *pSession, int nReason);

	bool HandleZTPNotify(CZTPPackage* pMessage);		//处理ZTP通知数据
	void HandleZTPResponse(CZTPPackage* pMessage);		//处理ZTP应答数据

	void rspErrorToDialog(CZTPPackage* pMessage, CFTDSession* pSession);

	void SendToDialogFlow(CFTDSession* pSession, CFTDPackage* pPackage);
	void SendErrorRsp(CFTDPackage* pRequest, CFTDSession* pSession, int const nErrorCode, char const* pszErrorMsg);

private:
	int ReqUserLogin(CFTDPackage* pPackage, CFTDSession* pSession);

	void NotifyOrder(CZTPPackage* pPackage);
	void NotifyTrade(CZTPPackage* pPackage);
	void NotifyOrderError(CZTPPackage* pPackage);
	void NotifyActionError(CZTPPackage* pPackage);
	void NotifyDeferFee(CZTPPackage* pPackage);

	void OnRspUserLogin(CZTPPackage* pMessage, CFTDSession* pSession);
	void OnRspUserLogout(CZTPPackage* pMessage, CFTDSession* pSession);
	void OnDelUserSession(CZTPPackage* pMessage);		//删除会话通知（属于操作员权限，目前该功能未启用）

//	void OnRspOrderInsert(CZTPPackage* pMessage, CFTDSession* pSession);


	template<class RspHelper>
	void RspToDialog(CZTPPackage* pMessage, CFTDSession* pSession);

	template<class ForwardHelper>
	void ForwardFtdRequest(CFTDPackage* pFTDCPackage, CFTDSession* pSession);

private:
	CFTDPackage m_rspPackage;
	InvestorSessionManager m_sessionManager;
};
