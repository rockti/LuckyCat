#pragma once
#include "ZTPPackage.h"
#include "UserDataType.h"
#include "SessionFactory.h"
#include "FrontForwarder.h"
#include "IndexPackageFlow.h"

struct TFrontSessionInfo {
	CSession* pSession;					//会话号
	DWORD nConnectTime;					//连接建立时间
	int nDialogFlowSeqNo;				//对话流当前序号
	bool bIsLogin;						//是否已经登录
	CLoginIDType LoginID;				//会话所属用户代码
	CUserTypeType UserType;				//用户类型
	CBrokerIDType BrokerID;				//会话所属会员代码
	int nCommFlux;						//单位时间的通讯流量
	int nMaxCommFlux;					//单位时间的通讯流量上限
};

class CFrontEngine : public CServerSessionFactory
{
public:
	CFrontEngine(CReactorThread* pReactor, CFrontForwarder* pFrontForwarder);
	virtual ~CFrontEngine();
	
	virtual void OnTimer(int nIDEvent);	
	inline void SetMaxCommFlux(int nMaxCommFlux)	{ m_maxCommFlux = nMaxCommFlux; }

protected:
	//当会话连接建立的时候，创建前置会话信息对象。 一开始的时候，会话状态是没有登录。
	virtual void OnSessionConnected(CSession* pSession);
	//当会话断开的时候，删除前置会话信息对象。并主动向交易系统发出登出请求。
	virtual void OnSessionDisconnected(CSession* pSession, int nReason);

protected:
	void ClearAllMap();
	
	TFrontSessionInfo* GetSessionInfo(DWORD nSessionID);
	bool IsSessionLogin(DWORD nSessionID);
	void SetSessionOwner(DWORD const nSessionID, CReadOnlyLoginIDType& loginID, CUserTypeType const& userType, CReadOnlyBrokerIDType& brokerID);
	bool GetSessionOwner(DWORD nSessionID, CLoginIDType& loginID, CBrokerIDType& brokerID);
	//清除前置会话对象的用户信息。这个会话对象还是存在，但是用户信息清空了。 会话的状态为未登录。这个方法用于用户主动发出登出请求。
	void ClearSessionOwner(DWORD const nSessionID);
	bool SetSessionMaxCommFlux(DWORD nSessionID, int nMaxCommFlux);
	void CheckSession(CSession* pSession, TFrontSessionInfo* pSessionInfo);
	void CalcFlux(CSession* pSession, int const nFlux);

	void SendToPublicFlow(CPackage* pPackage);
	int AppendToReturnFlow(CPackage* pPackage);

protected:
	CFrontForwarder* m_pFrontForwarder;			//ZTP包预处理器（填充包头、组包发送）
	CZTPPackage m_reqPackage;
	IFlow* m_pPublicFlow;	//通知流
	IFlow* m_pReturnFlow;	//回报流
	int m_maxCommFlux;

	typedef std::map<type::SessionID, TFrontSessionInfo> CSessionInfoMap;
	CSessionInfoMap m_sessionInfoMap;
};
