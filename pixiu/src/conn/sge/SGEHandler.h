#pragma once

#include <string_view>

#include "SGE/sge_api.h"
#include "SGE/sge_packet_struct.h"
#include "SGE/sge_safe_client_interface.h"
#include "SGE/keyfld_enc.h"
#include "SGE/SGEDataType.h"
#include "SGE/SGEStruct.h"
#include "conn/SGE/SafeHandler.h"
#include "conn/SGE/SGESerializer.h"
#include "conn/ExchangeConn.h"
#include "model/Quote.h"

//国密接口连接上下文
typedef struct context {
	api_safe_client_t* SafeApiPtr = nullptr;				// 国密Api
	api_safe_client_link_t* SafeLinkPtr = nullptr;			// 设备Handle
	api_safe_client_session_t* SafeSessionPtr = nullptr;	// 国密会话
	sge_session_t* SGESessionPtr = nullptr;					// SGE会话

	session_client_info_t SGESessionInfo;					// SGE会话参数
	sge_session_callbacks_t SGECBList;						// 连接回调接口
	client_dev_cb_list_t SafeCBList;						// 安全回调接口
	unsigned char SessionKey[34] = "";						// 加密密钥
	bool IsHandshaked = false;								// 握手状态
	bool IsLogined = false;									// 登录状态
	uint32_t LastLocalID = 0;								// 最后本地委托号
} SGEContext;

struct PositionNotification;
class SGEHandler {
public:
	static SGEHandler* m_pThis;
	static void OnTradeFrontConnected(void* ctx);
	static void OnTradeFrontDisconnected(int error_no, void* ctx);
	static void OnTradeReceiveMsg(int type, const char* sge_msg, unsigned int msgLen, void* ctx);

	static void OnQuoteFrontConnected(void* ctx);
	static void OnQuoteFrontDisconnected(int error_no, void* ctx);
	static void OnQuoteReceiveMsg(int type, const char* ssge_msg, unsigned int msgLen, void* ctx);

	SGEHandler(ConnConfig& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher, SafeHandler& safeHandler);
	~SGEHandler();

	void RegistTradeContext(SGEContext* pContext) { m_pTradeContext = pContext; }
	void RegistQuoteContext(SGEContext* pContext) { m_pQutoeContext = pContext; }

	char const* GetLocalIP() { return m_localIp.c_str(); }

	void Login(bool isTrade);
	void SubscribeQuote(char const* instrumentCode);
	void QueryFund();
	void QueryStorage();
	void QueryPosition();	// SGE的是查询持仓明细
	void ReqOrderInsert(OrderInfo const& orderInfo, bool isTrade=true);
	void ReqOrderAction(OrderCancel const& actionInfo);

	void OnRspError(CSGERspInfoField* pRspInfo);
	void OnRspLogin(CSGERspInfoField* pRspInfo, CSGERspLoginField* pRspLogin);					// 登录应答
	void OnRspOrderInsert(CSGEInputOrderField* pInputOrder, CSGERspInfoField* pRspInfo);		// 报单错误应答
	void OnRspOrderAction(CSGEOrderActionField* pInputOrderAction, CSGERspInfoField* pRspInfo);	// 撤单错误应答
	void OnRspQrySeatFund(CSGERspAccountField* pRspFund, CSGERspInfoField* pRspInfo);			// 席位资金查询
	void OnRspQryClientPosition(CSGERspClientPositionField* pRspPosition, CSGERspInfoField* pRspInfo, bool bIsLast = true);	// 客户持仓
	void OnRspQryClientStorage(CSGERspClientStorageField* pRspStorage, CSGERspInfoField* pRspInfo, bool bIsLast=true);		// 客户库存

	void OnRtnOrder(CSGEOrderField* pOrder, bool isTrade);
	void OnRtnTrade(CSGETradeField* pTrade, bool isTrade);
	void OnRtnInstrumentStatus(TSGEInstrumentStatusType instStatus, bool isTrade);	// 交易状态通知
	void OnRtnDepthMarketData(CSGEDepthMarketDataField* pMarketDataField);

private:
	void GetLocalIp();
	bool Handshake(bool isTrade = true);		// SGE安全握手

	void DispatchPlainMsg(std::string_view const& plainMsg, bool isTrade = true);	// 明文消息分发	
	void DispatchCipherMsg(std::string_view const praseMsg, bool isTrade = true);	// 密文消息分发

	void PraseSafeMessage(bool isTrade = true);
	bool PraseHandshakeMsg(CSGERspInfoField* pRspInfo, fld_server_hello_t& svrHello);			//握手应答
	void doRspHandshake(fld_server_hello_t const& svrHello, bool isTrade = true);
	bool PraseCertificationMsg(CSGERspInfoField* pRspInfo, fld_sessionkey_info_t& sessionKey);	//认证应答
	void doRspCertification(fld_sessionkey_info_t const& sessionKey, bool isTrade = true);
	
	bool IsErrorRspInfo(CSGERspInfoField* pRspInfo);
	bool IsInvalidQuote(CSGEDepthMarketDataField* pDepthMarketData);

private:
	void ParseErrorMessage(bool isTrade);
	void ParseTradeMessage(bool isTrade);						// TRADE_STREAM = 1;	解析交易流	对话模式 非可靠（没有应答，重连后根据本地委托号再次发送|发起查询）
	void ParsePrivateMessage(bool isTrade);						// PRIVATE_STREAM = 3;	解析私有流	私有模式 可靠（针对单个接入单元）	
	void ParsePublicMessage(bool isTrade);						// PUBLIC_STREAM = 4;	解析公共流	广播模式 可靠（采用QUICK模式，不收过期的市场信息）	
// 	void ParseQueryMessage();									// QUERY_STREAM = 5;	解析查询流	对话模式 非可靠	使用Timeout机制
	void ParseMarketMessage(std::string_view const& plainMsg);	// MARKET_STREAM = 6;	解析行情流	广播模式 可靠（直接-1，采用QUICK模式）

	// 对话流解析
	void parseRspError(CSGERspInfoField* pRspInfo);
	void parseRspLogin(CSGERspInfoField* pRspInfo);
	void parseRspOrderInsert(CSGERspInfoField* pRspInfo, TSGEProductClassType productClass);
	void parseRspOrderAction(CSGERspInfoField* pRspInfo, TSGEProductClassType productClass);
	void parseRspQrySeatFund(CSGERspInfoField* pRspInfo);
	void parseRspQryPosition(CSGERspInfoField* pRspInfo);
	void parseRspQryStorage(CSGERspInfoField* pRspInfo);

	// 私有流解析
	void parseOrderRtn(TSGEProductClassType productClass, bool isCancelRtn, int seqNo, bool isTrade);
	void parseTradeRtn(TSGEProductClassType productClass, int seqNo, bool isTrade);

	// 公共流解析
	void parseDeferFee();
	void parseInstrumentStatus(bool isTrade);

	// 行情流解析
	void parseDepthMarket();
	void parseDeliveryMarket();
	void parsePricesInfo(CSGEDepthMarketDataField* pMarketData);
 
	// 其他功能函数
	bool encryptKeyDomain(std::string tradeID, char* encryptedPwd, char* randomCode);		//加密关键域
	void FillingPositionNotification(CSGERspClientPositionField const& position, PositionNotification& notification);
	Quote CreateQuote(InstID instID, CSGEDepthMarketDataField* pQuote);

private:
	ConnConfig const& m_config;
	ExchangeConnInstrumentHelper const& m_instHelper;
	ConnPublisher& m_publisher;

	SafeHandler& m_safeHandler;					// 安全接口
	SGEContext* m_pTradeContext;				// 交易上下文
	SGEContext* m_pQutoeContext;				// 行情上下文

	int m_requestNo;							// 请求号（仅交易流维护）
	int m_privateSeqNo;							// 私有流序号(两个前置会有重复序列号，重复的不处理)
	int m_encryptedMsgLen;						// 安全信息长度	
	unsigned char m_encryptedMsg[SGE_MSG_LEN];	// 发送的加密串

	Parser m_parser;							// SGE报文解析器
	CSeatInfo m_seatInfo;
	CSeatInfo m_exSeatInfo;

	std::string m_localIp;
	std::set <InstID> m_subscribeList;			// 行情订阅列表
	std::map<std::string, int> m_systemIDMap;	// <localId, systemId>	撤单应答没有本地委托号，所以委托应答时候要记录对应关系
	TSGEInstrumentStatusType m_currentStatus;	// 当前交易状态
};
