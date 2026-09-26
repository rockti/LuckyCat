#pragma once

#include "SGEHandlerBase.h"

#include "BaseExchApi.h"
#include "ZTPType.h"

class CFTDPackage;
class CZTPPackage;
class SafeHandler;
class CMarketFlowManager;
class CMarketRepository;
class SGETradeHandler : public CBaseExchApi, public MarketPublisher
{
public:
	static SGETradeHandler* m_pThis;
	static void OnFrontConnected(void* ctx);
	static void OnFrontDisconnected(int error_no, void* ctx);
	static void OnReceiveMsg(int type, const char* gtp_msg, unsigned int msgLen, void* ctx);

	static void OnSTFrontConnected(void* ctx);
	static void OnSTFrontDisconnected(int error_no, void* ctx);
	static void OnSTReceiveMsg(int type, const char* gtp_msg, unsigned int msgLen, void* ctx);

	SGETradeHandler(InstrumentRepository& instRepo, std::string const& sgeAddress, int sgePort, SafeContext& safeContext);
	~SGETradeHandler();

// 	SgeLinkType::TYPE GetLinkType() const { return m_linkType; }
	int GetPort() const { return m_port; }
	char const* GetAddress() const { return m_address.c_str(); }
	char const* GetLocalIP() { return m_localIp.c_str(); }
// 	int GetPrivateSeqNo() const { return m_privateSeq; }

	void RegistTradeContext(SGEContext* pContext) { m_pTradeContext = pContext; }
	void RegistStorageContext(SGEContext* pContext) { m_pStorageContext = pContext; }
//	void SubscribeMarket(char const* contractCode);

//	virtual void InitInstrumentSet(CInstrumentIDType instrumentID, data::CInstrument* pContract) {};

	virtual void Login();
	virtual bool ReqOrderInsert(data::COrder& orderField);
	virtual bool ReqOrderAction(data::COrderAction& actionField);
	virtual void StartWork();	//金交所启动方式和期货交易所不同，这里做单元测试用

	void OnRspError(data::CRspInfo* pRspInfo);
	void OnRspLogin(data::CSeatLogin* pRspLogin, data::CRspInfo* pRspInfo);
	void OnRspOrderInsert(data::COrder* pRspOrder, data::CRspInfo* pRspInfo);
	void OnRspOrderAction(data::COrderAction* pRspAction, data::CRspInfo* pRspInfo);	
	void OnRspQryClientPosition(data::CInvestorPosition* pRspPosition, data::CRspInfo* pRspInfo, bool bIsLast=true);
// 	void OnRspQryClientStorage(CRspClientStorageField* pRspStorage, data::CRspInfo* pRspInfo, bool bIsLast=true);

	void OnRtnOrder(data::COrder* pOrderField);
	void OnRtnTrade(data::CTrade* pTradeField);
	void OnRtnDeferFee(data::CDeferFee* pDeferFee);						//递延费通知
	void OnRtnInstrumentInfo(data::CInstrument* pInstrumentField);		//合约信息通知
	void OnRtnInstrumentStatus(data::CInstrumentStatus* pStatusField);	//交易状态通知
	virtual void OnRtnDepthMarketData(CFTDDepthMarketDataField* pMarketDataField) override;

	virtual void InitSeatInfo(data::CSeatInfo const& seatField) override;
private:
	bool GetPublicKey(bool isTrade=true);	//获取公钥	
	bool Handshake(bool isTrade=true);		//SGE安全握手
	void QueryPosition();

	void PraseSafeMessage(bool isTrade=true);	//解析认证流
	bool PraseHandshakeMsg(data::CRspInfo* pRspInfo, fld_server_hello_t& svrHello);			//握手应答
	void doRspHandshake(fld_server_hello_t const& svrHello, bool isTrade=true);	
	bool PraseCertificationMsg(data::CRspInfo* pRspInfo, fld_sessionkey_info_t& sessionKey);	//认证应答
	void doRspCertification(fld_sessionkey_info_t const& sessionKey, bool isTrade=true);

	//明文消息分发
	void DispatchPlainMsg(std::string_view const& plainMsg, bool isTrade=true);
	//密文消息分发
	void DispatchCipherMsg(std::string_view const praseMsg);

	void ParseErrorMessage();
	//TRADE_STREAM = 1;		解析交易流	对话模式 非可靠（没有应答，重连后根据本地委托号再次发送|发起查询）
	void ParseTradeMessage();
	//PRIVATE_STREAM = 3;	解析私有流	私有模式 可靠（针对单个接入单元）
	void ParsePrivateMessage();
	//PUBLIC_STREAM = 4;	解析公共流	广播模式 可靠（采用QUICK模式，不收过期的市场信息）
	void ParsePublicMessage();
	//QUERY_STREAM = 5;		解析查询流	对话模式 非可靠	使用Timeout机制
	void ParseQueryMessage();
	//MARKET_STREAM = 6;	解析行情流	广播模式 可靠（直接-1，采用QUICK模式）
	void ParseMarketMessage(string_view const& plainMsg);

	void parseRspError(data::CRspInfo* pRspInfo);
	void parseRspLogin(data::CRspInfo* pRspInfo);

	void parseRspOrderInsert(data::CRspInfo* pRspInfo, type::ProductClass orderClassType);
	void parseRspOrderAction(data::CRspInfo* pRspInfo, type::ProductClass orderClassType);
	void parseRspQryPosition(data::CRspInfo* pRspInfo);
	void parseRspQryStorage(data::CRspInfo* pRspInfo);
																																						
	void parseOrderRtn(type::ProductClass orderClassType, bool isOrder, int sequenceNo);
	void parseTradeRtn(type::ProductClass orderClassType, int sequenceNo);
	void parseStorageRtn();

	void parseDeferFee();
	void parseInstrumentInfo();
	void parseInstrumentStatus();

	void notifySeatLoginSuccess();


	void getLocalIp();
	bool encryptKeyDomain(char* encryptedPwd, char* randomCode);		//加密关键域
	type::Volume SpotWeightToVolume(type::InstrumentID instID, double weight);

private:	//SGE用
	std::string m_address;
	std::string m_localIp;

	int m_port;
	int m_encryptedMsgLen;						//安全信息长度
	unsigned char m_encryptedMsg[SGE_MSG_LEN];	//发送的加密串

	SafeHandler m_safeHandler;					//安全回调接口
	SGEContext* m_pTradeContext;				//交易接口上下文
	SGEContext* m_pStorageContext;				//仓储接口上下文
	session_client_info_t m_sessionInfo;		//SGE连接参数
	Parser m_parser;							//SGE报文解析器
	InstrumentRepository& m_instRepo;

private:
	CFTDPackage* m_pMarketPackage;				//行情流数据包：行情直接以FTD协议返回，不用转换ZTP了
	CZTPPackage* m_pTradePackage;				//交易流数据包

	int m_seriesSequence;						//序列流包序号
	type::InstrumentStatus m_instrumentStatus;	//交易状态

	bool m_syncFinished = false;
};

