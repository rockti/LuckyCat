#pragma once


const int MAXEXCHID = 15;//支持最多交易所个数，实际上不仅仅是交易所，应该定义为输入流
const int MAXGATEID = 5;

#define LINKTIMER_ID 0x0000002100
// 
// //报盘管理线程
// class CExchangeLinkManager : public CSelectReactor
// {
// public:
// 	///read request from pTReq,send response to pTSeries,read seat info from pConfig
// 	CExchangeLinkManager(char *pIniFileName,CConfig	*pConfig,bool isQueryBase);
// 	virtual ~CExchangeLinkManager();
// 
// 	virtual bool Init();
// 
// 	//从一个席位信息初始化一个交易所链接
// 	void initLink(CSeatField *pSeatField);
// 
// 	//初始化客户交易编码
// 	void initClientTradingID(CClientTradingIDField *pClientTradingIDField);
// 
// 	CBaseExchApi *getExchangeLink(const char *pAPIID);
// 	CBaseExchApi *getExchangeLink(int apiID);
// 	CBaseExchApi *getExchangeLink(CInvestorIDType InvestorID);
// 
// 	void setBaseExchAPI(CBaseExchApi *baseExAPI,int apiID);
// 
// 	int size(){return MAXEXCHID;};
// 	int gwsize(){return MAXGATEID;};
// 
// 	//启动各交易所报盘
// 	void StartWork();
// 
// 	//20150620 xuzh 根据字符串得到api的号，这里是较为简单的算法，后续会使用复杂算法
// 	int GetAPIID(const char *pAPIID);
// 	int GetAPIID(CInvestorIDType InvestorID);
// 	void initGateWLink(CInitMGateWayField *pGate = NULL);
// 	CBaseGateWApi *getGwLink(int apiID);
// 	int GetGwAPIID(const char *pExSvrType);
// 
// 	class CLinkTimer : public CEventHandler
// 	{
// 	public:
// 		CLinkTimer(CExchangeLinkManager *pLinkManger, int loopInterval):CEventHandler(pLinkManger)
// 		{
// 			m_pLinkManger = pLinkManger;
// 			SetTimer(LINKTIMER_ID, loopInterval);		
// 		}
// 
// 		~CLinkTimer()
// 		{
// 			KillTimer(LINKTIMER_ID);
// 		}
// 
// 		virtual void OnTimer(int nIDEvent)
// 		{
// 			switch(nIDEvent)
// 			{
// 			case LINKTIMER_ID:
// 				{
// 					Run(); 
// 					break;
// 				}
// 			default:
// 				break;
// 			}
// 		}
// 
// 		void Run()
// 		{
// 			int j;
// 			for (j=4; j<m_pLinkManger->size(); j++)
// 			{
// 				CBaseExchApi *pBaseApi = m_pLinkManger->getExchangeLink(j);
// 				if(pBaseApi == NULL || !pBaseApi->m_bCheckLinked || pBaseApi->GetStatus())
// 				{
// 					continue;
// 				}
// 				pBaseApi->StartWork();
// 			}
// 		}
// 
// 	private:
// 		CExchangeLinkManager *m_pLinkManger;
// 	};
// private:
// 	char m_pConfigFileName[128];
// 	CConfig *m_pConfig;
// 	vector<CSeatField *> m_InitSeatVec;//初始化席位的消息存储
// 	CBaseExchApi *m_ExchApiArryMap[MAXEXCHID];
// 	CBaseGateWApi *m_GateWApiArryMap[MAXGATEID];//初始化网关消息存储
// 	vector<CInitMGateWayField> m_InitGwVec;
// 	char tmpchar[512];
// 	bool m_bQueryBase;
// 	CLinkTimer *m_pLinkTimer;
// };
