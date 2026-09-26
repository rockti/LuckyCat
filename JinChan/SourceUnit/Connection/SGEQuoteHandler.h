#pragma once

#include "SGEHandlerBase.h"

#include "FTDPackage.h"
#include "FtdDataStruct.h"
#include "ZTPType.h"
#include "InstrumentManager.h"
class CFTDPackage;
class CZTPPackage;
class SafeHandler;
class CMarketFlowManager;
class CMarketRepository;
class SGEQuoteHandler : public MarketPublisher
{
public:
	static SGEQuoteHandler* m_pThis;
	static void OnFrontConnected(void* ctx);
	static void OnFrontDisconnected(int error_no, void* ctx);
	static void OnReceiveMsg(int type, const char* gtp_msg, unsigned int msgLen, void* ctx);

	SGEQuoteHandler(IFlow* marketFlow,  std::string const& sgeAddress, int sgePort, SafeContext& safeContext);
	~SGEQuoteHandler();

	int GetPort() const { return m_port; }
	char const* GetAddress() const { return m_address.c_str(); }
	char const* GetLocalIP() { return m_localIp.c_str(); }

	virtual void InitInstrumentSet(CInstrumentIDType instrumentID, data::CInstrument* pContract) {};

	virtual void Login();
	virtual void StartWork();	//金交所启动方式和期货交易所不同，这里做单元测试用

	void OnRspError(data::CRspInfo* pRspInfo);
	void OnRspLogin(data::CSeatLogin* pRspLogin, data::CRspInfo* pRspInfo);

	void OnRtnInstrumentInfo(data::CInstrument* pInstrumentField);		//合约信息通知
	void OnRtnInstrumentStatus(data::CInstrumentStatus* pStatusField);	//交易状态通知
	virtual void OnRtnDepthMarketData(CFTDDepthMarketDataField* pMarketDataField) override;

	void InitSeatInfo(data::CSeatInfo const& seatField);
	void RegiesterContext(SGEContext& context) { m_pQuoteContext = &context; }
private:
	bool GetPublicKey();	//获取公钥	
	bool Handshake();		//SGE安全握手

	void PraseSafeMessage();	//解析认证流
	bool PraseHandshakeMsg(data::CRspInfo* pRspInfo, fld_server_hello_t& svrHello);			//握手应答
	void doRspHandshake(fld_server_hello_t const& svrHello);	
	bool PraseCertificationMsg(data::CRspInfo* pRspInfo, fld_sessionkey_info_t& sessionKey);	//认证应答
	void doRspCertification(fld_sessionkey_info_t const& sessionKey);

	//明文消息分发
	void DispatchPlainMsg(std::string_view const& plainMsg);
	//密文消息分发
	void DispatchCipherMsg(std::string_view const praseMsg);

	void ParseErrorMessage();
	//PUBLIC_STREAM = 4;	解析公共流	广播模式 可靠（采用QUICK模式，不收过期的市场信息）
	void ParseTradeMessage();
	void ParsePublicMessage();
	//QUERY_STREAM = 5;		解析查询流	对话模式 非可靠	使用Timeout机制
	void ParseQueryMessage();
	//MARKET_STREAM = 6;	解析行情流	广播模式 可靠（直接-1，采用QUICK模式）
	void ParseMarketMessage(string_view const& plainMsg);

	void parseRspError(data::CRspInfo* pRspInfo);
	void parseRspLogin(data::CRspInfo* pRspInfo);

																																						

	void parseInstrumentInfo();
	void parseInstrumentStatus();

	void notifySeatLoginSuccess();


	void getLocalIp();
	bool encryptKeyDomain(char* encryptedPwd, char* randomCode);		//加密关键域

private:	//SGE用
	std::string m_address;
	std::string m_localIp;

	int m_port;
	int m_topicID;								//行情主题号
	int m_encryptedMsgLen;						//安全信息长度
	unsigned char m_encryptedMsg[SGE_MSG_LEN];	//发送的加密串

	SafeHandler m_safeHandler;					//安全回调接口
	SGEContext* m_pQuoteContext;				//交易接口上下文
	session_client_info_t m_sessionInfo;		//SGE连接参数
	Parser m_parser;							//SGE报文解析器

private:
	IFlow* m_marketFlow;
	CFTDPackage* m_pMarketPackage;				//行情流数据包：行情直接以FTD协议返回，不用转换ZTP了

	int m_seriesSequence;						//序列流包序号
	type::InstrumentStatus m_instrumentStatus;	//交易状态

	bool m_syncFinished = false;
	data::CSeatInfo m_seatInfo;
	bool m_isLogined = false;
	bool m_isConnected = false;
	int m_lastSocket;
};

