#pragma once
#include "Session.h"
#include "ReactorThread.h"
#include "SelectThread.h"
#include "ListenHandler.h"
#include "NetworkFactory.h"

const int TIMER_ID_CONNECT = 1;
const int UM_LISTEN_RESULT	= 0x40901 + SM_USER;	//监听结果
const int UM_TRY_CONNECT	= 0x40903 + SM_USER;	//尝试连接
const int UM_CONNECT_RESULT	= 0x40902 + SM_USER;	//连接结果
const int UM_CONNECT_LOOP	= 0x40904 + SM_USER;	//重复连接

class CSessionConnecter {
public:
	CSessionConnecter(char const* nameService, DWORD dwMark);
	CChannel* GetChannel()	{ return m_pChannel; }
	CServiceName* GetServiceName()	{ return &m_nameService; }
	DWORD GetMark()	{ return m_dwMark; }

	CChannel* Connect();
	void OnConnected(CChannel* pChannel);
	void CheckChannel(CChannel* pChannel);
private:
	CServiceName m_nameService;
	CChannel* m_pChannel;
	DWORD m_dwMark;
};

class CConnecterManager : public CEventHandler
{
public:
	CConnecterManager(CReactorThread* pReactor, CEventHandler* pSessionFactory);
	~CConnecterManager();
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);

	void Clear();
	void AppendConnecter(CSessionConnecter* pConnecter);
	void OnConnected(CChannel* pChannel);
	void OnDiscconnected(CChannel* pChannel);

	int Connect();
	int ConnectNext();

	void CancelConnect()	{ m_nConnecterPos = -1; }
	bool IsConnecting()	{ return -1 != m_nConnecterPos; }

private:

private:
	vector<CSessionConnecter*> m_connecters;
	CEventHandler* m_pSessionFactory;
	int m_nConnecterPos;
};

class CSessionListener : public CListenHandler
{
public:
	CSessionListener(CReactorThread* pReactor, CEventHandler* pSessionFactory, CServerBase* pServer, DWORD dwMark);
	int OnConnected(CChannel* pChannel);
private:
	CEventHandler* m_pSessionFactory;
	DWORD m_dwMark;
};

class CBaseSessionFactory : public CEventHandler, public CSessionCallback {
public:
	CBaseSessionFactory(CReactorThread* pReactor, int nMaxSession);
	CSession* GetSession(type::SessionID nSessionID);
	void DisconnectAll(int nReason);

protected:
	virtual CSession* CreateSession(CChannel* pChannel, DWORD dwMark) = 0;
	virtual void OnSessionConnected(CSession* pSession);
	virtual void OnSessionDisconnected(CSession* pSession, int nReason);
	virtual void OnSessionWarning(CSession* pSession, int nReason, int nParam);

	void OnChannelCreated(CChannel* pChannel, DWORD dwMark);

	size_t m_nMaxSession;

	using CSessionMap = std::map<type::SessionID, CSession*>;
	CSessionMap m_sessionMap;

};

class CClientSessionFactory : public CBaseSessionFactory {
public:
	CClientSessionFactory(CReactorThread* pReactor, int nMaxSession, CSelectThread* pConnReactor = nullptr);
	~CClientSessionFactory();
	virtual void Start();
	void Stop();
	virtual void OnTimer(int nIDEvent);
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);
	void RegisterConnecter(const char* nameService, DWORD dwMark = 0);

protected:
	virtual void OnSessionDisconnected(CSession* pSession, int nReason);
private:

	CReactorThread* m_pConnReactor;
	CConnecterManager* m_pConnecterManager;
	bool m_bSessionCreateBySelf;

	using CSessionMap = std::map<type::SessionID, CSession*>;
	CSessionMap m_sessionMap;

};


class CServerSessionFactory : public CBaseSessionFactory {
public:
	CServerSessionFactory(CReactorThread* pReactor, int nMaxSession);
	~CServerSessionFactory();
	void RegisterListener(const char* nameService, DWORD dwMark = 1);
	virtual void Start();
//	virtual void OnTimer(int nIDEvent);
	virtual int HandleEvent(int nEventID, DWORD dwParam, void* pParam);
	void Stop();

private:
	std::vector<CSessionListener*> m_listenerSet;

};

/*
class CSessionFactory : public CEventHandler, public CSessionCallback
{
public:
	CSessionFactory(CReactorThread* pReactor, int nMaxSession, CSelectThread* pConnReactor=NULL);
	virtual ~CSessionFactory();
	virtual void Start();
	virtual void OnTimer(int nIDEvent);
	virtual int HandleEvent(int nEventID, DWORD dwParam, void *pParam);

	void Stop();
	void DisconnectAll(int nReason);
	void RegisterConnecter(const char* nameService, DWORD dwMark = 0);
	void RegisterListener(const char* nameService, DWORD dwMark = 1);
	void EnableConnecter(bool bEnable);
	void EnableListener(bool bEnable);
	void SetConnectMode(bool bRandomConnect);
	CSession* GetSession(type::SessionID nSessionID);

	inline bool IsConnecterEnable()	 { return m_bConnecterEnable; }
	inline bool IsListenerEnable()	{ return m_bListenerEnable; }

protected:
	virtual CSession* CreateSession(CChannel* pChannel, DWORD dwMark) = 0;
	virtual void OnSessionConnected(CSession* pSession);
	virtual void OnSessionDisconnected(CSession* pSession, int nReason);
	virtual void OnSessionWarning(CSession* pSession, int nReason, int nParam);	

	void OnChannelCreated(CChannel* pChannel, DWORD dwMark);

protected:
	CReactorThread* m_pConnReactor;
	CConnecterManager* m_pConnecterManager;
	vector<CSessionListener*> m_listenerSet;

	size_t m_nMaxSession;
	bool m_bRandomConnect;
	bool m_bConnecterEnable;
	bool m_bListenerEnable;
	bool m_bSessionCreateBySelf;

	using CSessionMap = std::map<type::SessionID, CSession*>;
	CSessionMap m_sessionMap;	
};
*/