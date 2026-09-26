#include "stdafx.h"
#include "SGEHandler.h"
#include "BaseExchApi.h"
#include "SGEConnection.h"
#include <fmt/format.h>
#include "SGESerializer.h"
#include "ZtpType.h"
#include "ServiceDef.h"


#pragma warning(disable:4100)
using namespace std;

static char ConvertDirectionToSGE(type::Direction direction) {
	return type::IsBuy(direction) ? 'b' : 's';
}
static char ConvertOperationToSGE(type::OffsetFlag operation) {
	switch (operation) {
	case type::OffsetFlag::Open: return '0';
	case type::OffsetFlag::Close: return '1';
	case type::OffsetFlag::CloseToday: return '1';
	default: return '2';		//其他全是强平
	}
}


static type::Direction ConvertDirectionFromSGE(char direction) {
	return ('b' == direction) ? type::Direction::Buy : type::Direction::Sell;
}

static type::DeferFeeDirection ConvertDeferFeeDirectionFromSGE(char direction) {
	switch (direction) {
	case '1':
		return type::DeferFeeDirection::LongPay;
	case '2':
		return type::DeferFeeDirection::ShortPay;
	case '3':
		return type::DeferFeeDirection::Equal;
	default:
		return type::DeferFeeDirection::Unknown;
		break;
	}
}
static type::OffsetFlag ConvertOperationFromSGE(char operation) {
	switch (operation) {
	case '0':	return type::OffsetFlag::Open;
	case '1':	return type::OffsetFlag::Close;
	default:	return type::OffsetFlag::ForceClose;
	}
}
static type::OrderStatus ConvertOrderStatusFromSGE(char status, bool isOrder) {
	switch (status) {
	case 'o':	return type::OrderStatus::NoTradeQueueing;		//委托成功，返回了系统号
	case 'p':	return type::OrderStatus::PartTradedQueueing;	//部分成交
	case 'c':	return type::OrderStatus::AllTraded;			//完全成交
	case '1':	return	isOrder ? type::OrderStatus::InsertQueueing : type::OrderStatus::CancelQueueing;	//正报未应答|正撤未应答
	case '2':	return  isOrder ? type::OrderStatus::InsertFail : type::OrderStatus::CancelFail;			//报单失败|撤单失败
	case 'a':	//自动撤单
	case 'd':	//用户撤单
	case 's':	//系统撤单
	case 'u':	//应急撤单
	case 'r':	//报价商双边撤单
	case '3':	//部成部撤(测试FAK部分成交	)
		return type::OrderStatus::Canceled;						//已经撤单
	default:
		Log::Warn(fmt::format("SGEHandler: unknown orderStatus[{}]", status));
		return type::OrderStatus::NoTradeNotQueueing;
	}
}

void SGETradeHandler::OnFrontConnected(void* ctx) {
	Log::Info("SGEHandler: FrontConnected");
	m_pThis->m_isConnected = true;
	if (m_pThis->GetPublicKey()) {
		auto const& msg = fmt::format("SGEHandler: handshake {}", m_pThis->Handshake() ? "success!" : "failed!");
		RUNTIME_INFO_REPORT(msg.c_str());
	}
}
void SGETradeHandler::OnFrontDisconnected(int error_no, void* ctx) {
	string reportMsg = fmt::format("SGEHandler: FrontDisconnect, errorNo[{}]", error_no);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
	m_pThis->m_isConnected = false;
	m_pThis->m_isLogined = false;
	//TODO: reconnect operation
	if (m_pThis->m_pTradeContext->SafeSessionPtr) {
		sge_safe_free_client_session(m_pThis->m_pTradeContext->SafeSessionPtr);
		m_pThis->m_pTradeContext->SafeSessionPtr = NULL;
	}
}
void SGETradeHandler::OnReceiveMsg(int type, const char* gtp_msg, unsigned int msgLen, void* ctx) {	
	int praseBufLens = msgLen + 1;	//留一位给'/0'
	char receivedMsg[SGE_MSG_LEN * 2] = "";
	if (MSG_TYPE_PLAIN == type) {
		memcpy(receivedMsg, gtp_msg, msgLen);
		string_view gtpMsg(receivedMsg, msgLen);
  		m_pThis->DispatchPlainMsg(gtpMsg);
// 		Log::Info(fmt::format("Market[{}]", receivedMsg));
	} else if (MSG_TYPE_CIPHER == type) {
		if (m_pThis->m_safeHandler.DecryptMsg(gtp_msg, msgLen, receivedMsg, praseBufLens, m_pThis->m_pTradeContext->SessionKey)) {
			string_view praseMsg(receivedMsg, msgLen);
			m_pThis->DispatchCipherMsg(praseMsg);
		}
	} else {
		string reportMsg = fmt::format("SGEHandler: received unknown msgType[{}], isTrade[1]", type);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
	}
}

void SGETradeHandler::OnSTFrontConnected(void* ctx) {
	Log::Info("SGEHandler: STFrontConnected");
	if (m_pThis->GetPublicKey(false)) {
		auto const& msg = fmt::format("SGEHandler: handshake {}", m_pThis->Handshake(false) ? "success!" : "failed!");
		RUNTIME_INFO_REPORT(msg.c_str());
	}
}
void SGETradeHandler::OnSTFrontDisconnected(int error_no, void* ctx) {
	string reportMsg = fmt::format("SGEHandler: STFrontDisconnect, errorNo[{}]", error_no);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
	//仓储非必要接口，只记日志，不释放加密机连接
}
void SGETradeHandler::OnSTReceiveMsg(int type, const char* gtp_msg, unsigned int msgLen, void* ctx) {
	int praseBufLens = msgLen + 1;	//留一位给'/0'
	char receivedMsg[SGE_MSG_LEN * 2] = "";
	if (MSG_TYPE_PLAIN == type) {
		memcpy(receivedMsg, gtp_msg, msgLen);
		string_view gtpMsg(receivedMsg, msgLen);
		m_pThis->DispatchPlainMsg(gtpMsg, false);
		Log::Info(fmt::format("Storage[{}]", receivedMsg));
	}
	else if (MSG_TYPE_CIPHER == type) {
		if (m_pThis->m_safeHandler.DecryptMsg(gtp_msg, msgLen, receivedMsg, praseBufLens, m_pThis->m_pStorageContext->SessionKey)) {
			string_view praseMsg(receivedMsg, msgLen);
			m_pThis->DispatchCipherMsg(praseMsg);
			Log::Info(fmt::format("Storage[{}]", receivedMsg));
		}
	}
	else {
		string reportMsg = fmt::format("SGEHandler: received unknown msgType[{}] isTrade[0]", type);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
	}
}


void LogOrderInsert(int rtn, data::COrder const& inputField) {
	char const* direction = type::IsBuy(inputField) ? "B" : "S";
	char const* operation = type::IsOpen(inputField) ? "OPEN" : "OFFSET";
	Log::Info(fmt::format("OrderInsert: rtn[{}] SeatID[{}] ClientID[{}] LocalID[{}] InstrumentID[{}] Flag[{}|{}] Price[{:.2f},{}] Condition[{}]",
		rtn, inputField.SeatID, inputField.ClientID, inputField.OrderLocalID, inputField.InstrumentID,
		direction, operation, inputField.LimitPrice, inputField.Volume, inputField.OrderPriceType));
}
void LogOrderAction(int rtn, data::COrderAction const& actionField) {
	Log::Info(fmt::format("OrderAction: rtn[{}] SeatID[{}] ClientID[{}] InstrumentID[{}] LocalID[{}] ActionID[{}] SystemID[{}]",
		rtn, actionField.SeatID, actionField.ClientID, actionField.InstrumentID,
		actionField.OrderLocalID, actionField.ActionLocalID, actionField.OrderSysID));
}

SGETradeHandler* SGETradeHandler::m_pThis = NULL;
SGETradeHandler::SGETradeHandler(InstrumentRepository& instRepo, string const& sgeAddress, int sgePort, SafeContext& safeContext)
:m_instRepo(instRepo),m_address(sgeAddress),m_port(sgePort)
,m_safeHandler(safeContext.Address, safeContext.Port)
,m_pTradeContext(nullptr),m_encryptedMsgLen(0),m_seriesSequence(0),m_instrumentStatus(type::InstrumentStatus::Initting) {
	m_pThis = this;

	m_pMarketPackage = new CFTDPackage();
	m_pMarketPackage->ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 2000);
	m_pTradePackage = new CZTPPackage();
	m_pTradePackage->ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);

	m_pSeriesFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x100000);
	m_seriesReader.AttachFlow(m_pSeriesFlow, m_seriesSequence);
	getLocalIp();
}
SGETradeHandler::~SGETradeHandler() {
}

void SGETradeHandler::Login() {
	if (m_isLogined) {
		RUNTIME_INFO_REPORT("SGELogin: failed, is already logined!");
		return;
	}

	char randomCode[100] = "";
	char encryptedPwd[100] = "";
	if (encryptKeyDomain(encryptedPwd, randomCode))	{
		SGEFormatter sgeSerial(m_seatInfo);
		auto result = sgeSerial.ToLoginRequest(encryptedPwd, randomCode, m_localIp.c_str(), sge_get_version());
		Log::Info(fmt::format("SGELogin: {}", result.Message));

		m_encryptedMsgLen = result.Size * 2;
		if (!m_safeHandler.EncryptMsg("Login", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
			char const* reportMsg = "SGEHandler: encrypt LoginMsg failed, isTrade[1]";
			RUNTIME_INFO_REPORT(reportMsg);
			Log::Error(reportMsg);
			return;
		}

		int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
		if (0 != nRtn) {
			auto const& reportMsg = fmt::format("SGEHandler: send helloInfo failed, rtnCode[{}] isTrade[1]", nRtn);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return;
		}

		result = sgeSerial.ToStorageLoginRequest(encryptedPwd, randomCode, m_localIp.c_str(), sge_get_version());
		m_encryptedMsgLen = result.Size * 2;
		if (!m_safeHandler.EncryptMsg("STLogin", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pStorageContext->SessionKey)) {
			char const* reportMsg = "SGEHandler Storage: encrypt LoginMsg failed, isTrade[0]";
			RUNTIME_INFO_REPORT(reportMsg);
			Log::Error(reportMsg);
		}
		nRtn = sge_session_write(m_pStorageContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
		if (0 != nRtn) {
			auto const& reportMsg = fmt::format("SGEHandler Storage: send helloInfo failed, rtnCode[{}] isTrade[0]", nRtn);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return;
		}
	}
}

void SGETradeHandler::QueryPosition() {
	auto iter = m_clientInvestorMap.begin();
	SGEFormatter sgeSerial(m_seatInfo);
	auto const& result = sgeSerial.ToQueryPosition(iter->first.value());
	Log::Info(fmt::format("SGEQryPosition: {}", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("QueryPosition", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		char const* reportMsg = "SGEHandler: encrypt QueryPosition failed";
		RUNTIME_INFO_REPORT(reportMsg);
		Log::Error(reportMsg);
		return;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format(" SGEHandler: send QueryPosition failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}
}
bool SGETradeHandler::ReqOrderInsert(data::COrder& orderField) {
	uint32_t orderLocalId = orderField.OrderLocalID.value();
	char caoOrderLocalId[15] = "";
	sprintf(caoOrderLocalId, "%d", orderLocalId);
	FormatResult result;
	SGEFormatter formatter(m_seatInfo);

	switch (orderField.ProductClass) {
	case type::ProductClass::Defer: {		//延期
		result = formatter.ToDeferInsertRequest(orderField);
	}
		break;
	case type::ProductClass::Spot: {		//现货
		result = formatter.ToSpotInsertRequest(orderField);
	}
		break;
	default:
		return false;
	}

	Log::Info(fmt::format("SGEHandler: OrderInsert [{}]", result.Message));
	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("OrderInsert", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		const char* reportMsg = "SGEHandler: encrypt orderMsg failed";
		RUNTIME_INFO_REPORT(reportMsg);
		Log::Error(reportMsg);
		return false;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: send orderInsert failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	LogOrderInsert(nRtn, orderField);
	return true;
}
bool SGETradeHandler::ReqOrderAction(data::COrderAction& actionField) {

	SGEFormatter formatter(m_seatInfo);
	auto result = formatter.ToCancelRequest(actionField);

	Log::Info(fmt::format("SGEHandler: OrderAction [{}]", result.Message));

	m_encryptedMsgLen = result.Size * 2;
	if (!m_safeHandler.EncryptMsg("OrderAction", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pTradeContext->SessionKey)) {
		char const* reportMsg = "SGEHandler: encrypt actionMsg failed";
		RUNTIME_INFO_REPORT(reportMsg);
		Log::Error(reportMsg);
		return false;
	}

	int nRtn = sge_session_write(m_pTradeContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("send orderAction failed, rtnCode[%d]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}
	LogOrderAction(nRtn, actionField);
	return true;
}
void SGETradeHandler::StartWork() {
	/****************************************************************测试信息****************************************************************/
	//错误信息
// 	string_view praseMsg = "X01=GTP1.0,X03=M991,X04=5,X05=3,X06=L,X07=7429,X39=RSP013025,X40=GTP报文缺少必填字段";
	//握手应答
//  	string_view praseMsg = "X01=GTP1.0,X04=0,X05=0,X07=1,X03=AA01,X06=L,X39=RSP000000,X40=操作成功,M60=bjh3074,G04=SGE,G05=B7C4218AA328B47194091B285FEE8FA0,X83=0ssdfer";
	//验证应答
// 	std::string_view praseMsg = "X01=GTP1.0,X04=0,X05=0,X07=2,X03=AA11,X06=L,X39=RSP000000,X40=操作成功,M60=bjh3074,G04=SGE,X84=3079022100D6B8BD0BE8E2C505D98FCFE19DDE743E4A8B6FCD2F084840E0DBA82E8D7C1F4D02202D524F770BD4140E749B3E1D1E814EFBC0423F280AD37B68583DD6D9210482BB0420A06EE637FA6DFF3B2B94308848B8DFCF34E3E55ADFA3BE668D984B83A6EB2A900410C9470F8DDAE28D54409B398511A127BC,X86=D6BF1445F7DA80E69F7FE9085E0AE7A00F5E39C0981EC1B6246FF5A586A8F4FD3A6B98E3A6B1E8295E75A7E7F77B431F,X88=30440220662BE914F8BCDFBD1200259C513D457FB02652FD5BF426AC95775563CB0B58530220221D580F1351A826FD4447A0C5CD5B919C4D8F023A292BEB554DC30C5CAFF642?";
	//登录应答
//	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=3,X03=A101,X39=RSP012005,X40=密码不正确,M60=bys2074,X50=192.168.2.19,X55=2.0.3,M20=007421,X25=TRA,X20=[{X04=3,X05=0},{X04=4,X05=0},{X04=6,X05=-1}],X06=L";
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=3,X03=A101,X39=RSP000000,X40=操作成功,M60=bys2074,O14=32,T13=20191118,X50=192.168.2.19,X55=2.0.3,M20=007421,X25=TRA,X20=[{X04=3,X05=3389},{X04=4,X05=0},{X04=6,X05=-1}],X06=L";
	//交割申报应答
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=57785795,X03=T091,X06=L,X39=RSP014005,X40=合约未处于交割申报状态,M20=007421,M30=0100045189,I10=Ag(T+D),O01=83445025167891,O02=b,O07=1,M60=bys2074,O08=0,T82=c";
	//中立仓申报应答
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=34595315,X03=T121,X06=L,X39=RSP014008,X40=合约未处于中立仓申报状态,M20=007421,M30=0100045189,I10=Ag(T+D),O01=71545275819202,O02=s,O07=1,M60=bys2074,O08=0,T82=c";
	//委托应答
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=7,X03=T201,X06=L,X39=RSP015004,X40=价格不在涨跌停板内,M20=007422,M30=0100146798,I10=Au100g,I00=00,O01=1000,O02=b,O07=10,T11=20180823,T12=10:57:56,M60=blj09265,O08=0,O65=1,O06=10.00,O15=0";
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=9,X03=T201,X06=L,X39=RSP030104,X40=策略资金不足。,M20=007422,M30=0100146798,I10=Au100g,I00=00,O01=3000,O02=b,O07=10,T11=20180823,T12=10:58:39,O09=o,M60=blj09265,O08=10,O65=1,O06=333.00,O15=0";
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=22175875,X03=T201,X06=L,X39=RSP000000,X40=操作成功,M20=007421,M30=0000000074,I10=mAu(T+D),I00=02,O00=02279067,O01=01405105477973,O02=b,O07=1,T11=20170504,T12=14:09:57,O09=o,M60=bys2074,O08=1,O03=0,O65=1,O06=265.00,O15=0,T82=c";
	//撤单应答
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=56675185,X03=T211,X06=L,X39=RSP015008,X40=操作成功,M20=007421,M30=0000000074,I10=mAu(T+D),I00=02,O00=02279067,M60=bys2074,T82=c";
	//持仓查询
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=2,X03=T461,X39=RSP030048,X40=客户未绑定当前席位。,X06=L";
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=02585925,X03=T461,X39=RSP000000,X40=操作成功,I00=02,P80=[{I10=Ag(T+D),M30=0100035649,P01=0,P02=712,P03=0,P04=0,P05=0,P06=712,P07=0,P08=0,P09=0,P10=259880000,P11=0,P12=0,P13=0,P14=0,P15=0,P16=0,P17=0,P18=0},{I10=Au(T+D),M30=0100035649,P01=11,P02=0,P03=0,P04=0,P05=11,P06=0,P07=0,P08=0,P09=293759000,P10=0,P11=0,P12=0,P13=0,P14=0,P15=0,P16=0,P17=0,P18=0},{I10=mAu(T+D),M30=0100045189,P01=5,P02=10,P03=0,P04=0,P05=0,P06=0,P07=0,P08=0,P09=0,P10=0,P11=0,P12=0,P13=0,P14=0,P15=1,P16=0,P17=0,P18=0}],X06=L";
	//库存查询
// 	string_view praseMsg = "X01=GTP1.0,X04=1,X05=0,X07=36095775,X03=T471,X39=RSP000000,X40=操作成功,S91=[{M30=0100045189,M20=007421,V00=207,S01=20,S02=20,S03=0.000000,S04=0.000000,S13=0.000000,S15=0.000000,S14=0.000000,S19=0.000000},{M30=0100045189,M20=007421,V00=201,S01=0.000000,S02=0.000000,S03=0.000000,S04=0.000000,S13=0.000000,S15=0.000000,S14=0.000000,S19=0.000000},{M30=0100045189,M20=007421,V00=202,S01=0.000000,S02=0.000000,S03=0.000000,S04=0.000000,S13=0.000000,S15=0.000000,S14=0.000000,S19=0.000000}],X06=L";
	//竞价委托回报
// 	string_view praseMsg = "X01=GTP1.0,X04=3,X05=138,X03=T202,X06=L,M20=007421,M30=0000000074,I10=Ag(T+D),I00=02,O00=02046829,O01=60000021,O02=s,O07=30,T11=20170504,T12=21:15:12,O09=p,M60=bzh06074,O08=14,O03=1,O65=1,O06=4004.00,O15=0";
	//交割申报回报
// 	string_view praseMsg = "X01=GTP1.0,X04=3,X05=4068,X03=T102,M20=007422,M30=0100179828,I10=Au(T+D),O00=02118998,O01=2000020007,O02=b,O07=1,T11=20180528,T12=15:31:00,O09=o,M60=bxt11265,O08=1,X06=L";
	//中立仓申报回报
// 	string_view praseMsg = "X01=GTP1.0,X04=3,X05=1,X03=T122,M20=203111,M30=0000002031,I10=Ag(T+D),O00=02080673,O01=2000020001,O02=b,O07=435,T11=20180709,T12=15:31:00,O09=o,M60=pjw1031,O08=435,X06=L";
	//成交回报
// 	string_view praseMsg = "X01=GTP1.0,X04=3,X05=8,X03=T222,X06=L,M20=007421,M30=0000000074,I10=Au99.99,I00=00,O00=00000347,O01=3,O02=b,O64=1,O61=20170504,O62=20:32:33,O60=1705040000000050,O65=1,O63=280.00";
// 	string_view praseMsg = "X01=GTP1.0,X04=3,X05=24,X03=T222,X06=L,M20=007421,M30=0000000074,I10=Ag(T+D),I00=02,O00=02041102,O01=60000004,O02=s,O64=30,O61=20170504,O62=21:00:05,O60=1705040200017543,O03=1,O65=1,O63=4005.00";
	//竞价行情信息
// 	string_view praseMsg = "X01=GTP1.0,X04=6,X05=108324,X03=Q012,I00=00,Q52=[{I10=iAu99.99,I11=iAu9999,Q14=279.42,Q13=0.00,Q15=275.11,Q16=277.50,Q17=275.11,Q18=277.50,Q19=0.00,Q20=0.00,Q27=0,Q26=363.24,Q36=195.59,Q28=-1.92,Q25=40.060000,O64=4006,T80=53,Q29=0.006871,Q30=1110406200,Q31=277.18,Q11=20170504,Q12=13:35:25,Q00=[{Q21=275.20,Q22=1,Q23=277.25,Q24=500},{Q21=275.11,Q22=4,Q23=0.00,Q24=0},{Q21=275.00,Q22=3,Q23=0.00,Q24=0},{Q21=272.00,Q22=15,Q23=0.00,Q24=0},{Q21=270.00,Q22=1,Q23=0.00,Q24=0},{Q21=250.00,Q22=1,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]},{I10=iAu100g,I11=iAu100g,Q14=276.00,Q13=0.00,Q15=0.00,Q16=0.00,Q17=0.00,Q18=0.00,Q19=0.00,Q20=0.00,Q27=0,Q26=358.80,Q36=193.20,Q28=0.00,Q25=0.000000,O64=0,T80=12,Q29=0.000000,Q30=0,Q31=0.00,Q11=20170504,Q12=13:35:25,Q00=[{Q21=255.00,Q22=6,Q23=280.00,Q24=1},{Q21=250.00,Q22=1,Q23=350.00,Q24=1},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]},{I10=iAu99.5,I11=iAu995,Q14=237.80,Q13=0.00,Q15=0.00,Q16=0.00,Q17=0.00,Q18=0.00,Q19=0.00,Q20=0.00,Q27=0,Q26=309.14,Q36=166.46,Q28=0.00,Q25=0.000000,O64=0,T80=1,Q29=0.000000,Q30=0,Q31=0.00,Q11=20170504,Q12=13:35:25,Q00=[{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]},{I10=Au99.99,I11=Au9999,Q14=280.37,Q13=0.00,Q15=289.99,Q16=289.99,Q17=277.49,Q18=277.49,Q19=0.00,Q20=0.00,Q27=0,Q26=364.48,Q36=196.25,Q28=-2.88,Q25=8210.780000,O64=821078,T80=3953,Q29=0.010272,Q30=228266119040,Q31=278.00,Q11=20170504,Q12=13:35:25,Q00=[{Q21=277.47,Q22=1,Q23=277.49,Q24=9},{Q21=277.41,Q22=4,Q23=277.50,Q24=109},{Q21=277.40,Q22=1022,Q23=277.60,Q24=480},{Q21=277.33,Q22=7,Q23=277.75,Q24=5000},{Q21=277.31,Q22=1,Q23=277.76,Q24=200},{Q21=277.30,Q22=542,Q23=277.80,Q24=27},{Q21=277.23,Q22=25,Q23=277.95,Q24=1373},{Q21=277.20,Q22=2,Q23=277.98,Q24=5000},{Q21=277.18,Q22=40,Q23=278.00,Q24=1035},{Q21=277.10,Q22=202,Q23=278.10,Q24=4472}]},{I10=Au99.95,I11=Au9995,Q14=280.11,Q13=0.00,Q15=279.47,Q16=279.47,Q17=277.30,Q18=277.30,Q19=0.00,Q20=0.00,Q27=0,Q26=364.14,Q36=196.07,Q28=-2.81,Q25=362.000000,O64=362,T80=286,Q29=0.010032,Q30=10060772000,Q31=277.92,Q11=20170504,Q12=13:35:25,Q00=[{Q21=277.30,Q22=18,Q23=277.40,Q24=15},{Q21=277.05,Q22=1,Q23=277.60,Q24=10},{Q21=276.70,Q22=5,Q23=277.80,Q24=4},{Q21=267.99,Q22=3,Q23=277.90,Q24=7},{Q21=0.00,Q22=0,Q23=278.00,Q24=10},{Q21=0.00,Q22=0,Q23=278.45,Q24=5},{Q21=0.00,Q22=0,Q23=278.50,Q24=18},{Q21=0.00,Q22=0,Q23=279.00,Q24=5},{Q21=0.00,Q22=0,Q23=279.10,Q24=35},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]},{I10=Pt99.95,I11=Pt9995,Q14=220.27,Q13=0.00,Q15=218.00,Q16=218.00,Q17=215.50,Q18=215.50,Q19=0.00,Q20=0.00,Q27=0,Q26=286.35,Q36=154.18,Q28=-4.77,Q25=312.000000,O64=312,T80=109,Q29=0.021655,Q30=6746600000,Q31=216.23,Q11=20170504,Q12=13:35:25,Q00=[{Q21=0.00,Q22=0,Q23=215.50,Q24=8},{Q21=0.00,Q22=0,Q23=216.00,Q24=10},{Q21=0.00,Q22=0,Q23=217.00,Q24=10},{Q21=0.00,Q22=0,Q23=218.00,Q24=10},{Q21=0.00,Q22=0,Q23=219.00,Q24=10},{Q21=0.00,Q22=0,Q23=220.00,Q24=10},{Q21=0.00,Q22=0,Q23=221.00,Q24=10},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]},{I10=Au50g,I11=小金条,Q14=255.00,Q13=0.00,Q15=0.00,Q16=0.00,Q17=0.00,Q18=0.00,Q19=0.00,Q20=0.00,Q27=0,Q26=331.50,Q36=178.50,Q28=0.00,Q25=0.000000,O64=0,T80=1,Q29=0.000000,Q30=0,Q31=0.00,Q11=20170504,Q12=13:35:25,Q00=[{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]},{I10=Au100g,I11=100克金条,Q14=280.58,Q13=0.00,Q15=280.50,Q16=280.50,Q17=278.00,Q18=278.00,Q19=0.00,Q20=0.00,Q27=0,Q26=364.75,Q36=196.40,Q28=-2.58,Q25=45.200000,O64=452,T80=398,Q29=0.009195,Q30=1260456000,Q31=278.86,Q11=20170504,Q12=13:35:25,Q00=[{Q21=277.83,Q22=1,Q23=278.00,Q24=9},{Q21=277.82,Q22=2,Q23=278.30,Q24=9},{Q21=277.81,Q22=1,Q23=278.50,Q24=3},{Q21=277.80,Q22=1,Q23=279.88,Q24=1},{Q21=277.60,Q22=2,Q23=279.99,Q24=1},{Q21=277.50,Q22=1,Q23=280.00,Q24=2},{Q21=277.00,Q22=3,Q23=280.42,Q24=10},{Q21=276.68,Q22=3,Q23=280.47,Q24=10},{Q21=276.50,Q22=1,Q23=280.48,Q24=2},{Q21=276.00,Q22=3,Q23=280.50,Q24=5}]},{I10=Au99.5,I11=Au995,Q14=244.15,Q13=0.00,Q15=0.00,Q16=0.00,Q17=0.00,Q18=0.00,Q19=0.00,Q20=0.00,Q27=0,Q26=317.39,Q36=170.90,Q28=0.00,Q25=0.000000,O64=0,T80=1,Q29=0.000000,Q30=0,Q31=0.00,Q11=20170504,Q12=13:35:25,Q00=[{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]}],X06=L";
// 	string_view praseMsg = "X01=GTP1.0,X04=6,X05=13572,X03=Q012,I00=02,Q52=[{I10=Ag(T+D),I11=白银延期,Q14=4287.00,Q13=4302.00,Q15=4043.00,Q16=4100.00,Q17=4043.00,Q18=4100.00,Q19=0.00,Q20=0.00,Q27=11713768,Q26=4560.00,Q36=4043.00,Q28=-202.00,Q25=24.000000,O64=24,T80=20,Q29=0.046955,Q30=9828600,Q31=4095.00,Q11=20191115,Q12=14:38:27,Q00=[{Q21=4088.00,Q22=2,Q23=0.00,Q24=0},{Q21=4067.00,Q22=1,Q23=0.00,Q24=0},{Q21=4064.00,Q22=1,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0},{Q21=0.00,Q22=0,Q23=0.00,Q24=0}]}],X06=L";
	//交割行情信息
// 	string_view praseMsg = "X01=GTP1.0,X04=6,X05=87234,X03=Q202,Q69=[{I10=mAu(T+D),Q22=3479,Q24=0,Q33=0,Q32=0,T80=18},{I10=Au(T+D),Q22=2410,Q24=765,Q33=0,Q32=0,T80=61},{I10=Ag(T+D),Q22=3600,Q24=15915,Q33=0,Q32=0,T80=3},{I10=Au(T+N1),Q22=2920,Q24=0,Q33=0,Q32=0,T80=4},{I10=Au(T+N2),Q22=0,Q24=10,Q33=0,Q32=0,T80=5}],X06=L";
	//合约信息
// 	string testBuf = "X01=GTP1.0,X04=4,X05=21,X03=B102,X06=L,I10=Au(T+D),I11=黄金延期,I00=02,V03=1,I23=0,V00=202,I12=1000,I13=0.01,I14=200,I15=1,I16=0.090000,I17=0.090000,I18=1,I19=1,I20=272.28,I21=0.000000";
	//合约状态
// 	string_view praseMsg = "X01=GTP1.0,X04=4,X05=52,X03=B182,X06=L,I10=Ag(T+D),I19=2";
	//递延费率推送
// 	string_view praseMsg = "X01=GTP1.0,X04=4,X05=179,X03=B302,X06=L,T13=20180604,I10=mAu(T+D),I26=1,I25=0.000175";
	//录出应答
// 	string testBuf = "X01=GTP1.0,X04=1,X05=2,X07=2,X03=A111,X06=L,X39=RSP000000,X40=操作成功,M60=pjw1031,M20=203111";
	//库存推送
// 	string testBuf = "X01=GTP1.0,X04=3,X05=8,X03=S802,X06=L,M00=0074,M20=007421,T13=20200410,T80=2471,K04=RK20200410000006,M30=0100041882,K01=1,V00=201,W00=8201,T52=16.000000,T50=15.998400,S16=16.000000,T16=20200410,T17=09:29:46";


// 	OnReceiveMsg(MSG_TYPE_PLAIN, test.c_str(), test.size(), NULL);
//  	DispatchPlainMsg(praseMsg);
//	DispatchCipherMsg(praseMsg);
}

using namespace ztp;

namespace {
	template<int l>
	void assign(CStringType<l>& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(),sizeof(tmp)-1));
		target = tmp;
	}

	void assign(CIntType& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atoi(tmp);
	}

	void assign(CLongType& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atoi(tmp);
	}

	void assign(CCharType& target, string_view const& str) {
		target = str[0];
	}

	template<int l, int p>
	void assign(CDoubleType<l, p>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}
	void assign(char* target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		strcpy_s(target, str.length() + 1, tmp);
	}

	void assign(char& target, string_view const& str) {
		target = str[0];
	}

	void assign(long long& target, string_view const& str) {
		target = _atoi64(str.data());
	}

	void assign(int& target, string_view const& str) {
		target = atoi(str.data());
	}

	template<class tag>
	void assign(DoubleType<tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}

	template<class tag>
	void assign(RatioType<tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}
	void assign(double& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atof(tmp);
	}

	template<uint8_t size, class tag>
	void assign(StrongCharArray<size, tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = tmp;
	}

	template<class BaseT, class tag>
	void assign(NumberType<BaseT, tag>& target, string_view const& str) {
		char tmp[256] = "";
		memcpy(tmp, str.data(), min(str.length(), sizeof(tmp) - 1));
		target = atoi(tmp);
	}
}

template<class Target>
bool copy_if_key(char const* expectedKey, string_view const& key, Target& target, Parser& parser) {
	if (expectedKey == key) {
		assign(target, parser.GetStringValue());
		return true;
	}
	return false;
}
inline bool CopyErrorID(char const* ekey, string_view const& key, type::ErrorID& errorID, Parser& parser) {
	if (key == ekey) {
		errorID = parser.GetIntValue(3);
		return true;
	}
	return false;
}
#define COPY_CONTINUE(ekey,target) if(copy_if_key(ekey, key, target, m_parser)) continue;
#define COPY_ERRORID(ekey,target) if (CopyErrorID(ekey, key, target, m_parser)) continue;


void SGETradeHandler::OnRspError(data::CRspInfo* pRspInfo) {	
	string reportMsg = fmt::format("SGEHandler: OnRspError, errCode[{}] errMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
}
void SGETradeHandler::OnRspLogin(data::CSeatLogin* pRspLogin, data::CRspInfo* pRspInfo) {
	string reportMsg = "SGEHandler: SeatLogin ";
	if (0 != pRspInfo->ErrorID) {		
		reportMsg += fmt::format("failed, errorCode[{}] errorMsg[{}] seatID[{}] traderID[{}] pwd[{}]", 
			pRspInfo->ErrorID, pRspInfo->ErrorMsg,
			pRspLogin->SeatID, pRspLogin->TraderID, m_seatInfo.Password);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		m_isLogined = false;
	} else {
		m_isLogined = true;
		reportMsg += fmt::format("success, maxOrderLocalID[{}]", pRspLogin->OrderLocalID);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Info(reportMsg.c_str());

		m_seatInfo.OrderLocalID = pRspLogin->OrderLocalID;
		m_tradingDay = pRspLogin->TradingDay;

		if (m_tradeSeqNo <= 0) {
			m_syncFinished = true;
			Log::Info(fmt::format("No last private seq no ,no order need to sync, notify seat login success"));
			notifySeatLoginSuccess();
		}
	}
}
void SGETradeHandler::OnRspOrderInsert(data::COrder* pRspOrder, data::CRspInfo* pRspInfo) {
	//根据ClientID找到对应的InvestorID
	auto iter = m_clientInvestorMap.find(pRspOrder->ClientID);
	if (iter == m_clientInvestorMap.end()) {
		Log::Warn(fmt::format("SGEHandler: OnRspOrder received unsupported clientID[{}] data", pRspOrder->ClientID));
		return;
	}
	pRspOrder->InvestorID = iter->second;
	pRspOrder->BrokerID = type::BrokerID(m_seatInfo.ParticipantID.value());

	Log::Debug(fmt::format("SGEHandler: OnRspOrder, ErrorCode[{}] ErrorMsg[{}], InvestorID[{}] InstrumentID[{}] Direction[{}] Operation[{}] OrderPrice[{:.3f}] Quantity[{}] OrderSysID[{}] LocalID[{}] OrderStatus[{}]",
		pRspInfo->ErrorID, pRspInfo->ErrorMsg, pRspOrder->InvestorID, pRspOrder->InstrumentID, pRspOrder->Direction, pRspOrder->OffsetFlag,
		pRspOrder->LimitPrice, pRspOrder->Volume, pRspOrder->OrderSysID, pRspOrder->OrderLocalID, pRspOrder->OrderStatus));

	pRspOrder->OrderStatus = type::OrderStatus::InsertFail;
	m_pTradePackage->PreparePublish(TID_RspOrderInsert);
	m_pTradePackage->SetSubjectID(DIALOG_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(*pRspInfo);
	m_pTradePackage->AddField(*pRspOrder);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}
void SGETradeHandler::OnRspOrderAction(data::COrderAction* pRspAction, data::CRspInfo* pRspInfo) {
	//根据ClientID找到对应的InvestorID
	auto iter = m_clientInvestorMap.find(pRspAction->ClientID);
	if (iter == m_clientInvestorMap.end()) {
		Log::Warn(fmt::format("SGEHandler: OnRspAction received unsupported clientID[{}] data", pRspAction->ClientID));
		return;
	}
	pRspAction->InvestorID = iter->second;
	pRspAction->BrokerID = type::BrokerID(m_seatInfo.ParticipantID.value());

	Log::Debug(fmt::format("SGEHandler OnRspAction: ErrorCode[{}] ErrorMsg[{}], InvestorID[{}] InstrumentID[{}] OrderSysID[{}] LocalID[{}]", pRspInfo->ErrorID,
		pRspInfo->ErrorMsg, pRspAction->InvestorID, pRspAction->InstrumentID, pRspAction->OrderSysID, pRspAction->OrderLocalID));

	m_pTradePackage->PreparePublish(TID_RspOrderAction);
	m_pTradePackage->SetSubjectID(DIALOG_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(*pRspInfo);
	m_pTradePackage->AddField(*pRspAction);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}

void SGETradeHandler::OnRspQryClientPosition(data::CInvestorPosition* pRspPosition, data::CRspInfo* pRspInfo, bool bIsLast) {
// 	CSeatLoginField field;
// 	field.ExchangeID = "SGE";
// 	field.SeatID = m_seatInfo.SeatID;
// 	field.TraderID = m_seatInfo.TraderID;
// 	field.OrderLocalID = m_seatInfo.OrderLocalID;
// 	CSeatLoginField* pRspLogin = &field;
// 	if (SgeLinkType::TRADE == m_linkType) {
// 		Log::Info("QueryPosition End");
// 		m_pTradePackage->PreparePublish(TID_NotifySeatLoginSuccess);
// 		m_pTradePackage->SetSubjectID(PUBLIC_FLOW);
// 		m_pTradePackage->SetSequenceNo(m_seriesSequence);
// 		ZTP_ADD_FIELD(m_pTradePackage, pRspLogin);
// 		m_pTradePackage->MakePackage();
// 		m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
// 	}
// 	if (0 != pRspInfo->ErrorID)
// 	{
// 		FT formatter("SGE QueryPosition Failed: RequestId[%d] ErrorCode[%d] ErrorMsg[%s]",pRspInfo->RequestID,pRspInfo->ErrorID,pRspInfo->ErrorMsg);
// 		PostNotifyInfo(formatter, m_queueContext.GetInterfaceQueue());
// 		Log::Error(formatter);
// 		return;
// 	}
// 	else
// 	{
// 		PositionNotification notification;
// 		notification.contractIndex = GetContractIndex(pRspPosition->InstrumentID);
// 		if (notification.contractIndex < 0)
// 		{
// 			Log::Warn(FT("SGE OnRspQryClientPosition received not exist Contract[%s], RequestId[%d] ClientId[%s]"
// 				,pRspPosition->InstrumentID,pRspInfo->RequestID,pRspPosition->ClientID));
// 			return;
// 		}
// 
// 		if (0 != (pRspPosition->LongPosition+pRspPosition->ShortPosition))
// 		{
// 			InitUserPosition(*pRspPosition, notification);
// 			PostChipMessage(&notification, sizeof(PositionNotification), ChipMessage::NOTIFY_POSITION, m_queueContext.GetInterfaceQueue());
// 			char const* direction = (0==notification.direction) ? "B" : "S";
// 			Log::Info(FT("SGE OnRspQryClientPosition: ClientiId[%s] Contract[%s] Direction[%s] QuantityTotal[%d] QuantityNew[%d] QuantityOld[%d] AvgPrice[%.2f]",
// 				pRspPosition->ClientID,pRspPosition->InstrumentID,direction,notification.totalQuantity,notification.newPosition,notification.oldPosition,notification.price));
// 		}
// 
// 		if (bIsLast)
// 		{
// 			Log::Info("SGE QueryPosition Success");
// 		}
// 	}
}
//void SGEHandler::OnRspQryClientStorage(CRspClientStorageField* pRspStorage, CRspInfoField* pRspInfo, bool bIsLast)
//{
// 	if (0 != pRspInfo->ErrorID)
// 	{
// 		FT formatter("SGE QueryStorage Failed: RequestId[%d] ErrorCode[%d] ErrorMsg[%s]",pRspInfo->RequestID,pRspInfo->ErrorID,pRspInfo->ErrorMsg);
// 		PostNotifyInfo(formatter, m_queueContext.GetInterfaceQueue());
// 		Log::Error(formatter);
// 		return;
// 	}
// 	else
// 	{
// 		TGtpInstrumentIDType InstrumentID;
// 		memset(InstrumentID, 0, sizeof(TGtpInstrumentIDType));
// 		strcpy_s(InstrumentID, ConvertSpotVarietyFromSGE(pRspStorage->VarietyID));
// 
// 		PositionNotification notification;
// 		notification.contractIndex = GetContractIndex(InstrumentID);
// 		if (notification.contractIndex < 0)
// 		{
// 			Log::Warn(FT("SGE OnRspQryClientStorage received not exist Contract, VarietyID[%d] RequestId[%d] ClientId[%s]"
// 				,pRspStorage->VarietyID,pRspInfo->RequestID,pRspStorage->ClientID));
// 			return;
// 		}
// 
// 		if (0 != pRspStorage->AvailableStorage)
// 		{
// 			strcpy_s(notification.clientId, pRspStorage->ClientID);
// 			strcpy_s(notification.contract, InstrumentID);
// 			notification.direction = Direction::BUY;
// 			//单位统一转换成手
// 			switch (pRspStorage->VarietyID)
// 			{
// 			case 201:	//Au99.99 报价10克/手，最大报单50000手
// 				notification.totalQuantity = pRspStorage->AvailableStorage*1000/10;
// 				break;
// 			case 202:	//Au99.95 报价1000克/手，最大报单500手
// 				notification.totalQuantity = pRspStorage->AvailableStorage*1000/1000;
// 				break;
// 			case 207:	//Au100g 报价100克/手，最大报单1000手
// 				notification.totalQuantity = pRspStorage->AvailableStorage*1000/100;
// 				break;
// 			}
// 			notification.oldPosition = notification.totalQuantity;
// 
// 			PositionKey key(notification.contractIndex, notification.direction, Operation::OFFSET);
// 			PositionValue value(notification.price, notification.totalQuantity);
// 			m_user.InitPosition(key, value);
// 
// 			PostChipMessage(&notification, sizeof(PositionNotification), ChipMessage::NOTIFY_POSITION, m_queueContext.GetInterfaceQueue());
// 			Log::Info(FT("SGE OnRspQryClientPosition: ClientiId[%s] Contract[%s] QuantityTotal[%d]",pRspStorage->ClientID,InstrumentID,notification.totalQuantity));
// 		}
// 
// 		if (bIsLast)
// 		{
// 			Log::Info("SGE QueryStorage Success");
// 		}
// 	}
//}

void SGETradeHandler::OnRtnOrder(data::COrder* pOrderField) {
	//根据ClientID找到对应的InvestorID
	auto iter = m_clientInvestorMap.find(pOrderField->ClientID);
	if (iter == m_clientInvestorMap.end()) {
		Log::Warn(fmt::format("SGEHandler: OnRtnOrder received unsupported clientID[{}] data", pOrderField->ClientID));
		return;
	}
	pOrderField->InvestorID = iter->second;
	pOrderField->BrokerID = type::BrokerID(m_seatInfo.ParticipantID.value());
	pOrderField->VolumeTrade = pOrderField->Volume - pOrderField->VolumeRemain;

	Log::Info(fmt::format("SGEHandler OnRtnOrder: InvestorID[{}] InstrumentID[{}] Direction[{}] Operation[{}] OrderPrice[{:.3f}] Quantity[{}] Deal[{}] Remain[{}] SystemID[{}] LocalID[{}] OrderStatus[{}]",
		pOrderField->InvestorID, pOrderField->InstrumentID, pOrderField->Direction, pOrderField->OffsetFlag, pOrderField->LimitPrice,
		pOrderField->Volume, pOrderField->VolumeTrade, pOrderField->VolumeRemain, pOrderField->OrderSysID, pOrderField->OrderLocalID, pOrderField->OrderStatus));

	if (type::OrderStatus::InsertFail== pOrderField->OrderStatus || type::OrderStatus::CancelFail == pOrderField->OrderStatus) {
		return;	//错误信息统一在应答处理
	}

	m_pTradePackage->PreparePublish(TID_NotifyOrder);
	m_pTradePackage->SetSubjectID(PRIVATE_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(*pOrderField);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}
void SGETradeHandler::OnRtnTrade(data::CTrade* pTradeField) {
	//根据ClientID找到对应的InvestorID
	auto iter = m_clientInvestorMap.find(pTradeField->ClientID);
	if (iter == m_clientInvestorMap.end()) {
		Log::Warn(fmt::format("SGEHandler: OnRtnTrade received unsupported clientID[{}] data", pTradeField->ClientID));
		return;
	}
	pTradeField->InvestorID = iter->second;
	pTradeField->BrokerID = type::BrokerID(m_seatInfo.ParticipantID.value());

	Log::Info(fmt::format("SGEHandler OnRtnTrade: InvestorID[{}] InstrumentID[{}] Direction[{}] Operation[{}] DealPrice[{:.3f}] Quantity[{}] SystemID[{}] LocalID[{}]",
		pTradeField->InvestorID, pTradeField->InstrumentID, pTradeField->Direction, pTradeField->OffsetFlag,
		pTradeField->TradePrice, pTradeField->TradeVolume, pTradeField->OrderSysID, pTradeField->OrderLocalID));

	m_pTradePackage->PreparePublish(TID_NotifyTrade);
	m_pTradePackage->SetSubjectID(PRIVATE_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(*pTradeField);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}

void SGETradeHandler::OnRtnDeferFee(data::CDeferFee* pDeferFee) {
	Log::Info(fmt::format("SGEHandler OnRtnDeferFee: InstrumentID[{}] Direction[{}] Rate[{}]",
		pDeferFee->InstrumentID, pDeferFee->Direction, pDeferFee->FeeRate));

	m_pTradePackage->PreparePublish(TID_RtnDeferFee);
	m_pTradePackage->SetSubjectID(PRIVATE_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(*pDeferFee);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}
void SGETradeHandler::OnRtnInstrumentInfo(data::CInstrument* pInstrumentField) {	
// 		m_pTradePackage->PreparePublish(TID_NotifyInstrumentInfo);
// 		m_pTradePackage->SetSubjectID(PUBLIC_FLOW);
// 		m_pTradePackage->SetSequenceNo(m_seriesSequence); 
// 		ZTP_ADD_FIELD(m_pTradePackage, pInstrumentField);
// 		m_pTradePackage->MakePackage();
// 		m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}
void SGETradeHandler::OnRtnInstrumentStatus(data::CInstrumentStatus* pStatusField) {
	//只有交易流才发布合约状态
	if (pStatusField->InstrumentStatus != m_instrumentStatus) {
		m_instrumentStatus = pStatusField->InstrumentStatus;
		m_pTradePackage->PreparePublish(TID_NotifyInstrumentStatus);
		m_pTradePackage->SetSubjectID(PUBLIC_FLOW);
		m_pTradePackage->SetSequenceNo(m_seriesSequence);
		m_pTradePackage->AddField(*pStatusField);
		m_pTradePackage->MakePackage();
		m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());

		string reportMsg = fmt::format("SGEHandler: ExchangeStatus[{}]", SGEStatusDescribe(pStatusField->InstrumentStatus));
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Info(reportMsg.c_str());
	}
}
void SGETradeHandler::OnRtnDepthMarketData(CFTDDepthMarketDataField* pMarketDataField) {	//注意：行情处理用FTD格式，不用ZTP
// 	DWORD instrmentIDCount = m_pMarketRepository->InsertMaraketData(m_topicID, pMarketDataField);
// 	m_pMarketPackage->PreparePackage(FTD_TID_IntlRtnDepthMarketData, FTD_CHAIN_LAST);
// 	m_pMarketPackage->SetInstrumentIDCount(instrmentIDCount);
// 	FTD_ADD_FIELD(m_pMarketPackage, pMarketDataField);
// 
// 	m_pMarketFlowManager->GetLock().Lock();
// 	m_pMarketFlowManager->PulishToTopicFlow(m_pMarketPackage, m_topicID);
// 	m_pMarketFlowManager->GetLock().UnLock();
}

void SGETradeHandler::InitSeatInfo(data::CSeatInfo const& seatField) {
	CBaseExchApi::InitSeatInfo(seatField);

	strcpy_s(m_sessionInfo.sge_id_, m_seatInfo.ExchangeID.value());
	strcpy_s(m_sessionInfo.trader_id_, m_seatInfo.TraderID.value());
	m_sessionInfo.enc_mode_ = 0;
	memset(m_sessionInfo.svr_enc_pubkey_, 0, sizeof(m_sessionInfo.svr_enc_pubkey_));
	memset(m_sessionInfo.svr_sign_pubkey_, 0, sizeof(m_sessionInfo.svr_sign_pubkey_));
	m_sessionInfo.svr_enc_pubkey_len_ = 0;
	m_sessionInfo.svr_sign_pubkey_len_ = 0;
}

void SGETradeHandler::DispatchPlainMsg(string_view const& plainMsg, bool isTrade) {
	m_parser.Init(plainMsg);
	if (MARKET_STREAM == plainMsg[15]) {
		ParseMarketMessage(plainMsg);
	} else {
		Log::Info(fmt::format("SGEHandler: PlainMsg[{}][{}] isTrade[{}]", plainMsg, plainMsg.length(), isTrade));
		PraseSafeMessage(isTrade);
	}
}
void SGETradeHandler::DispatchCipherMsg(string_view const msg) {	
	m_parser.Init(msg);
	switch (msg[15]) {
	case TRADE_STREAM:		//交易流:应答消息(委托应答，撤单应答，内含通用报错X39，X40字段)
		++m_seriesSequence;
		ParseTradeMessage();
		break;
	case PRIVATE_STREAM:	//私有流(只有OnRtnOrder，OnRtnTrade)
		++m_seriesSequence;
		ParsePrivateMessage();
		break;
	case PUBLIC_STREAM:		//公共流(只处理合约状态变化)
		++m_seriesSequence;
		ParsePublicMessage();
		break;
	case QUERY_STREAM:		//查询流(从未出现-.-!!)
		++m_seriesSequence;
		ParseQueryMessage();
		break;
	case MARKET_STREAM:		//行情流
		ParseMarketMessage(msg);
		break;
	default:				//其他情况当作错误处理
		ParseErrorMessage();
		break;
	}
}

bool SGETradeHandler::GetPublicKey(bool isTrade) {
	string reportMsg = "SGEHandler: ";
	int pubKeyLen = 140;
	int procSocket = m_safeHandler.ApplySocket();
	int nRtn = m_safeHandler.m_pSYD_SM2_GetPublicKey(procSocket, m_sessionInfo.trader_id_, 0x00, m_sessionInfo.svr_sign_pubkey_, &pubKeyLen);
 	Log::Info(fmt::format("GetPublicKey, traderID[{}] svr_sign_public[{}], isTrade[{}]", m_sessionInfo.trader_id_, m_sessionInfo.svr_sign_pubkey_, isTrade));
	if (0 == nRtn) {
		m_sessionInfo.svr_sign_pubkey_len_ = pubKeyLen;
		nRtn = m_safeHandler.m_pSYD_SM2_GetPublicKey(procSocket, m_sessionInfo.trader_id_, 0x01, m_sessionInfo.svr_enc_pubkey_, &pubKeyLen);
		if (0 == nRtn) {
			m_sessionInfo.svr_enc_pubkey_len_ = pubKeyLen;
			m_safeHandler.ReleaseSocket(procSocket);
			return true;
		}
	}
	
	m_safeHandler.ReleaseSocket(procSocket);
	reportMsg += fmt::format("get publicKey failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
	return false;
}
bool SGETradeHandler::Handshake(bool isTrade) {
	string reportMsg = "SGEHandler: ";

	if (isTrade) {
		m_pTradeContext->SafeSessionPtr = sge_safe_create_client_session(m_pTradeContext->SafeApiPtr, &m_sessionInfo);
		if (NULL == m_pTradeContext->SafeSessionPtr) {
			reportMsg += "create SafeSession failed, isTrade[1]";
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	}
	else {
		m_pStorageContext->SafeSessionPtr = sge_safe_create_client_session(m_pStorageContext->SafeApiPtr, &m_sessionInfo);
		if (NULL == m_pTradeContext->SafeSessionPtr) {
			reportMsg += "create SafeSession failed, isTrade[0]";
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	}
		
	fld_client_hello_t helloInfo;
	strcpy_s(helloInfo.trader_id_, m_sessionInfo.trader_id_);
	strcpy_s(helloInfo.sge_id_, m_sessionInfo.sge_id_);
	helloInfo.enc_mode_ = 0;
	strcpy_s(helloInfo.safe_api_trader_version_, sge_safe_client_get_api_version());

	api_safe_client_session_t* pTheSafeSessionPtr = isTrade ? m_pTradeContext->SafeSessionPtr : m_pStorageContext->SafeSessionPtr;
	int nRtn = sge_safe_client_create_hello(pTheSafeSessionPtr, &helloInfo);	//创建安全信息
	if (0 == nRtn) {	//发送握手报文	
		stringstream ssHello("");
		int requestId = RequestIdACC::Generate();
		auto const& helloBody = fmt::format("M60={},G04={},X83=0,X56={}", helloInfo.trader_id_, helloInfo.sge_id_, helloInfo.safe_api_trader_version_);
		ssHello << fmt::format("X01=GTP1.0,X03=AA00,X04=1,X05={},X06=L,X07={},", requestId, requestId) << helloBody;
		Log::Info(fmt::format("SGEHello: [{}] isTrade[{}]", ssHello.str().c_str(), isTrade));

		//不同的前置对应不同的Session握手
		sge_session_t* theSessionPtr = isTrade ? m_pTradeContext->SGESessionPtr : m_pStorageContext->SGESessionPtr;
		nRtn = sge_session_write(theSessionPtr, MSG_TYPE_PLAIN, ssHello.str().c_str(), (unsigned int)ssHello.str().size());
		if (0 != nRtn) {
			reportMsg += fmt::format("send helloInfo failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	} else {
		reportMsg += fmt::format("create helloInfo failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}
	return true;
}

//安全认证部分
void SGETradeHandler::PraseSafeMessage(bool isTrade) {
	data::CRspInfo rspInfo;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "AA01") {		//握手应答
				fld_server_hello_t helloRtn = { 0 };
				if (PraseHandshakeMsg(pRspInfo, helloRtn)) {
					doRspHandshake(helloRtn, isTrade);
				}
			}
			else if (value == "AA11") {	//验证应答
				fld_sessionkey_info_t keyRtn = { 0 };
				if (PraseCertificationMsg(pRspInfo, keyRtn)) {
					doRspCertification(keyRtn, isTrade);
				}
			}
		}
		else if (key == "M991") {
			ParseErrorMessage();
		}
	}
}
bool SGETradeHandler::PraseHandshakeMsg(data::CRspInfo* pRspInfo, fld_server_hello_t& svrHello) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
		COPY_CONTINUE("M60", svrHello.trader_id_);
		COPY_CONTINUE("G04", svrHello.sge_id_);
		COPY_CONTINUE("G05", svrHello.sge_random_);
		COPY_CONTINUE("X83", svrHello.enc_mode_);
	}
	if (0 != pRspInfo->ErrorID) {
		OnRspError(pRspInfo);
		return false;
	}
	return true;
}
void SGETradeHandler::doRspHandshake(fld_server_hello_t const& svrHello, bool isTrade) {
	api_safe_client_link_t* theSafeLinkPtr = isTrade ? m_pTradeContext->SafeLinkPtr : m_pStorageContext->SafeLinkPtr;
	api_safe_client_session_t* theSafeSessionPtr = isTrade ? m_pTradeContext->SafeSessionPtr : m_pStorageContext->SafeSessionPtr;
	
	//处理Hello应答
	int nRtn = sge_safe_client_handle_hello_rsp(theSafeSessionPtr, theSafeLinkPtr, &svrHello);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: safeHello respond failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}

	//生成认证信息并发送
	fld_authentication_info_t auth_info = { 0 };
	nRtn = sge_safe_client_create_authentication(theSafeSessionPtr, theSafeLinkPtr, &auth_info);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: create authentication failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}

	stringstream ssAuthenticate("");
	int requestId = RequestIdACC::Generate();
	auto const& authenticateBody = fmt::format("M60={},G04={},M63={},G05={},M64={}", 
		auth_info.trader_id_, auth_info.sge_id_, auth_info.trader_random_, auth_info.sge_random_, auth_info.authen_signed_data_);
	ssAuthenticate << fmt::format("X01=GTP1.0,X03=AA10,X04=1,X05={},X06=L,X07={},", requestId, requestId) << authenticateBody;
	Log::Debug(fmt::format("Authenticate: [{}] isTrade[{}]", ssAuthenticate.str().c_str(), isTrade));

	sge_session_t* theSessionPtr = isTrade ? m_pTradeContext->SGESessionPtr : m_pStorageContext->SGESessionPtr;
	nRtn = sge_session_write(theSessionPtr, MSG_TYPE_PLAIN, ssAuthenticate.str().c_str(), (unsigned int)ssAuthenticate.str().size());
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: send authenticate request failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}
}
bool SGETradeHandler::PraseCertificationMsg(data::CRspInfo* pRspInfo, fld_sessionkey_info_t & sessionKey) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
		COPY_CONTINUE("M60", sessionKey.trader_id_);
		COPY_CONTINUE("G04", sessionKey.sge_id_);
		COPY_CONTINUE("X84", sessionKey.cipher_key_);
		COPY_CONTINUE("X86", sessionKey.enc_random_);
		COPY_CONTINUE("X88", sessionKey.skey_signed_data_);
	}

	if (0 != pRspInfo->ErrorID) {
		OnRspError(pRspInfo);
		return false;
	}
	return true;
}
void SGETradeHandler::doRspCertification(fld_sessionkey_info_t const& sessionKey, bool isTrade) {

	api_safe_client_link_t* theSafeLinkPtr = isTrade ? m_pTradeContext->SafeLinkPtr : m_pStorageContext->SafeLinkPtr;
	api_safe_client_session_t* theSafeSessionPtr = isTrade ? m_pTradeContext->SafeSessionPtr : m_pStorageContext->SafeSessionPtr;

	//验证SGE签名
	int nRtn = sge_safe_client_accept_sessionkey(theSafeSessionPtr, theSafeLinkPtr, &sessionKey);
	if (0 != nRtn) {
		string reportMsg = fmt::format("SGEHandler: DoCertification failed, rtnCode[{}] isTrade[{}]", nRtn, isTrade);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}
	Log::Debug(FT("SGEHandler: sessionKey[{}] isTrade[{}]", m_pThis->m_safeHandler.GetSessionKey(), isTrade));

	if (isTrade) {
		memcpy(m_pThis->m_pTradeContext->SessionKey, m_pThis->m_safeHandler.GetSessionKey(), 33);
		m_pThis->m_pTradeContext->IsHandshake = true;
	} 
	else {
		memcpy(m_pThis->m_pStorageContext->SessionKey, m_pThis->m_safeHandler.GetSessionKey(), 33);
		m_pThis->m_pStorageContext->IsHandshake = true;
	}
}
void SGETradeHandler::ParseErrorMessage() {
	data::CRspInfo rspInfo;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("X03"==m_parser.GetKey() && "M991"== m_parser.GetStringValue()) {	//M991通用非业务报错
			parseRspError(pRspInfo);
			OnRspError(pRspInfo);
		}
	}
}
void SGETradeHandler::parseRspError(data::CRspInfo* pRspInfo) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
	}
}

//应答部分
void SGETradeHandler::ParseTradeMessage() {
	data::CRspInfo rspInfo;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "A101") {		//登录应答
				parseRspLogin(pRspInfo);
			}
			else if (value == "T091") {	//交割申报应答
				parseRspOrderInsert(pRspInfo, type::ProductClass::Delivery);
			}
			else if (value == "T121") {	//中立仓应答
				parseRspOrderInsert(pRspInfo, type::ProductClass::Neutral);
			}
			else if (value == "T201") {	//委托应答
				parseRspOrderInsert(pRspInfo, type::ProductClass::Unknown);
			}
			else if (value == "T101") { //交割撤单
				parseRspOrderAction(pRspInfo, type::ProductClass::Delivery);
			}
			else if (value == "T131") {	//中立仓撤单
				parseRspOrderAction(pRspInfo, type::ProductClass::Neutral);
			}
			else if (value == "T211") {	//委托撤单
				parseRspOrderAction(pRspInfo, type::ProductClass::Unknown);
			}
			else if (value == "T461") {	//持仓查询
				parseRspQryPosition(pRspInfo);
			}
			else if (value == "T471") {	//库存查询
//				parseRspQryStorage(pRspInfo);
			}
		}
	}
}
void SGETradeHandler::parseRspLogin(data::CRspInfo* pRspInfo) {
	data::CSeatLogin seatLogin;
	auto* pRspLogin = &seatLogin;
	seatLogin.ExchangeID = "SGE";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
		COPY_CONTINUE("M20", pRspLogin->SeatID);
		COPY_CONTINUE("M60", pRspLogin->TraderID);
		COPY_CONTINUE("T13", pRspLogin->TradingDay);
		COPY_CONTINUE("O14", pRspLogin->OrderLocalID);
	}
	OnRspLogin(pRspLogin, pRspInfo);
}
void SGETradeHandler::parseRspOrderInsert(data::CRspInfo* pRspInfo, type::ProductClass productClass) {
	data::COrder rspOrder;
	data::COrder* pRspOrder = &rspOrder;
	memset(pRspOrder, 0, sizeof(data::COrder));
	rspOrder.ExchangeID = "SGE";
	char marketID[3] = "";
	char direction;
	char operation = char(type::OffsetFlag::Unknown);
	char orderPriceType = char(type::OrderPriceType::LimitPrice);
	COrderStatusType orderStatus;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
		COPY_CONTINUE("M20", pRspOrder->SeatID);
		COPY_CONTINUE("M30", pRspOrder->ClientID);
//		COPY_CONTINUE("M60", pRspOrder->OrderOperatorID);
		COPY_CONTINUE("I00", marketID);
		COPY_CONTINUE("I10", pRspOrder->InstrumentID);
		COPY_CONTINUE("O00", pRspOrder->OrderSysID);
		COPY_CONTINUE("O01", pRspOrder->OrderLocalID);	
		COPY_CONTINUE("O02", direction);
		COPY_CONTINUE("O03", operation);
		COPY_CONTINUE("O06", pRspOrder->LimitPrice);
		COPY_CONTINUE("O07", pRspOrder->Volume);
		COPY_CONTINUE("O08", pRspOrder->VolumeRemain);
		COPY_CONTINUE("O09", orderStatus);
		COPY_CONTINUE("O15", orderPriceType);
		COPY_CONTINUE("T12", pRspOrder->InsertTime);
	}

	pRspOrder->OrderPriceType = type::OrderPriceType(orderPriceType);
	if (productClass == type::ProductClass::Unknown) {		//竞价报单类型根据市场代码区分
		pRspOrder->ProductClass = (marketID[1] == '0') ?type::ProductClass::Spot: type::ProductClass::Defer;
	} else {
		pRspOrder->ProductClass = productClass;
	}
	pRspOrder->Direction = ConvertDirectionFromSGE(direction);
	if (operation == char(type::OffsetFlag::Unknown)) {
		pRspOrder->OffsetFlag = type::IsBuy(pRspOrder) ? type::OffsetFlag::Open : type::OffsetFlag::Close;
	} else {
		pRspOrder->OffsetFlag = ConvertOperationFromSGE(operation);
	}
	if (pRspInfo->ErrorID) {	//SGE竞价报单会先收到回报信息，因此OnRspOrderInsert只处理错误应答
		OnRspOrderInsert(pRspOrder, pRspInfo);
	}
}
void SGETradeHandler::parseRspOrderAction(data::CRspInfo* pRspInfo, type::ProductClass productClass) {
	data::COrderAction rspAction;
	auto* pRspAction = &rspAction;
	rspAction.ExchangeID = "SGE";
	char marketID[3] = "";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
		COPY_CONTINUE("M20", pRspAction->SeatID);
		COPY_CONTINUE("M30", pRspAction->ClientID);
//		COPY_CONTINUE("M60", pRspAction->CancelOperatorID);
		COPY_CONTINUE("I00", marketID);
		COPY_CONTINUE("I10", pRspAction->InstrumentID);
		COPY_CONTINUE("O00", pRspAction->OrderSysID);
	}

	if (productClass == type::ProductClass::Unknown) {		//竞价报单类型根据市场代码区分
		pRspAction->ProductClass = (marketID[1] == '0') ? type::ProductClass::Spot : type::ProductClass::Defer;
	} else {
		pRspAction->ProductClass = productClass;
	}
	if (pRspInfo->ErrorID) {	//同委托应答：只处理错误信息
		OnRspOrderAction(pRspAction, pRspInfo);
	}
}
void SGETradeHandler::parseRspQryPosition(data::CRspInfo* pRspInfo) {
	data::CInvestorPosition rspPosition;
	auto* pRspPosition = &rspPosition;
	rspPosition.ExchangeID = "SGE";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X39") {
			pRspInfo->ErrorID = m_parser.GetIntValue(4);
		}
		else if (key == "X40") {
			assign(pRspInfo->ErrorMsg, m_parser.GetStringValue());
			if (pRspInfo->ErrorID) {
				OnRspQryClientPosition(pRspPosition, pRspInfo);
				return;
			}
		}
		else if (key == "I00") {
			//OWEN
// 			TGtpMarketIdType MarketId; 
// 			copy(MarketId, m_parser.GetStringValue());
// 			if (MarketId[1] != '2') {	
// 				return;	//只处理延期持仓:I00=02，其他数据直接丢弃
// 			}
		}
		else if (key == "I10") {		//品种代码
			if (strlen(pRspPosition->InstrumentID.value()) > 0 ) {
				//品种代码不为空，先发送上一条持仓数据
				OnRspQryClientPosition(pRspPosition, pRspInfo, false);
			}
			assign(pRspPosition->InstrumentID, m_parser.GetStringValue());
		}
		else if (key == "M30") {		//交易编码
//			assign(pRspPosition->ClientID, m_parser.GetStringValue());
		}
// 		else if (key == "P01") {		//多头持仓
// 			pRspPosition->LongPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P02") {		//空头持仓
// 			pRspPosition->ShortPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P03") {		//上日多头持仓
// 			pRspPosition->YdLongPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P04") {		//上日空头持仓
// 			pRspPosition->YdShortPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P05") {		//今日多头开仓
// 			pRspPosition->TodayLongPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P06") {		//今日空头开仓
// 			pRspPosition->TodayShortPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P07") {		//今日多头平仓量
// 			pRspPosition->TodayOffsetLongPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P08") {		//今日空头平仓量
// 			pRspPosition->TodayOffsetShortPosition = m_parser.GetIntValue();
// 		}
// 		else if (key == "P09") {		//多头持仓金额
// 			pRspPosition->LongTurnover = m_parser.GetInt64Value();
// 		}
// 		else if (key == "P10") {		//空头持仓金额
// 			pRspPosition->ShortTurnover = m_parser.GetInt64Value();
// 		}
// 		else if (key == "P11") {		//交割申报多头冻结
// 			pRspPosition->LongFrozen = m_parser.GetIntValue();
// 		}
// 		else if (key == "P12") {		//交割申报空头冻结
// 			pRspPosition->ShortFrozen = m_parser.GetIntValue();
// 		}
// 		else if (key == "P13") {		//交割申报使用多头持仓
// 			pRspPosition->DeliveryApplyLong = m_parser.GetIntValue();
// 		}
// 		else if (key == "P14") {		//交割申报使用空头持仓
// 			pRspPosition->DeliveryApplyShort = m_parser.GetIntValue();
// 		}
// 		else if (key == "P15") {		//多头开仓冻结限额
// 			pRspPosition->LongLimitFrozen = m_parser.GetIntValue();
// 		}
// 		else if (key == "P16") {		//空头开仓冻结限额
// 			pRspPosition->ShortLimitFrozen = m_parser.GetIntValue();
// 		}
// 		else if (key == "P17") {		//多头平仓冻结
// 			pRspPosition->OffsetLongFrozen = m_parser.GetIntValue();
// 		}
// 		else if (key == "P18") {		//空头平仓冻结
// 			pRspPosition->OffsetShortFrozen = m_parser.GetIntValue();
// 		}
		else if (key == "X06") {
			//结束标志，发送最后一条数据
			OnRspQryClientPosition(pRspPosition, pRspInfo, true);
		}
	}
}
void SGETradeHandler::parseRspQryStorage(data::CRspInfo* pRspInfo) {
// 	CRspClientStorageField* pRspStorage = new CRspClientStorageField;
// 	memset(pRspStorage, 0, sizeof(CRspClientStorageField));
// 	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
// 		auto key = m_parser.GetKey();
// 		if (key == "X39") {
// 			pRspInfo->ErrorID = m_parser.GetIntValue(4);
// 		}
// 		else if (key == "X40") {
// 			copy(pRspInfo->ErrorMsg, m_parser.GetStringValue());
// 			if (pRspInfo->ErrorID) {
// 				OnRspQryClientStorage(pRspStorage, pRspInfo);
// 				return;
// 			}
// 		}
// 		else if (key == "M30") {		//交易编码
// 			if (strlen(pRspStorage->ClientID) > 0) {
// 				//交易编码不为空，先发送上一条持仓数据
// 				OnRspQryClientStorage(pRspStorage, pRspInfo, false);
// 			}
// 			copy(pRspStorage->ClientID, m_parser.GetStringValue());
// 		}
// 		else if (key == "M20") {		//席位代码
// 			copy(pRspStorage->SeatID, m_parser.GetStringValue());
// 		}
// 		else if (key == "V00") {		//交割品种代码
// 			pRspStorage->VarietyID = m_parser.GetIntValue();
// 		}
// 		else if (key == "S01") {		//库存总量
// 			pRspStorage->TotalStorage = m_parser.GetDoubleValue();
// 		}
// 		else if (key == "S02") {		//可用库存
// 			pRspStorage->AvailableStorage = m_parser.GetDoubleValue();
// 		}
// 		else if (key == "S03") {		//冻结库存
// 			pRspStorage->FrozenStorage = m_parser.GetDoubleValue();
// 		}
// 		else if (key == "S04") {		//待提库存
// 			pRspStorage->PendStorage = m_parser.GetDoubleValue();
// 		}
// 		else if (key == "X06") {
// 			//结束标志，发送最后一条数据
// 			OnRspQryClientStorage(pRspStorage, pRspInfo, true);
// 		}
// 	}
}

//回报部分
void SGETradeHandler::ParsePrivateMessage() {
	int privateSeqNo = 0;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X05") {
			privateSeqNo = m_parser.GetIntValue();
			Log::Info(fmt::format("PrivateSeqNo = {}", privateSeqNo));
			continue;
		} 
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "T202") {		//竞价委托回报
				parseOrderRtn(type::ProductClass::Unknown, true, privateSeqNo);
			}
			else if (value == "T212") {	//竞价撤单回报
				parseOrderRtn(type::ProductClass::Unknown, false, privateSeqNo);
			}
			else if (value == "T092") {	//交割申报回报
				parseOrderRtn(type::ProductClass::Delivery, true, privateSeqNo);
			}
			else if (value == "T102") {	//交割撤单回报
				parseOrderRtn(type::ProductClass::Delivery, false, privateSeqNo);
			}
			else if (value == "T122") {	//中立仓申报回报
				parseOrderRtn(type::ProductClass::Neutral, true, privateSeqNo);
			}
			else if (value == "T132") {	//中立仓撤单回报
				parseOrderRtn(type::ProductClass::Neutral, false, privateSeqNo);
			}
			else if (value == "T112") {	//交割申报成交
				parseTradeRtn(type::ProductClass::Delivery, privateSeqNo);
			}
			else if (value == "T222") {	//竞价成交回报
				parseTradeRtn(type::ProductClass::Unknown, privateSeqNo);
			}
			else if (value == "S802") { //库存流水推送
				parseStorageRtn();
				//因库存变化的SeqNo是独立的，不需要进行后面的SeatLoginSucess处理，故此处continue;
				continue;
			}
			if (privateSeqNo >= m_tradeSeqNo && !m_syncFinished) {
				m_syncFinished = true;
				Log::Info(fmt::format("Sync Order Finished, CurrentNo{}, Last Stop No{}", privateSeqNo, m_tradeSeqNo));
				notifySeatLoginSuccess();
			}

		}
	}
}
void SGETradeHandler::notifySeatLoginSuccess() {

	data::CSeatLogin field;
	field.ExchangeID = "SGE";
	field.SeatID = m_seatInfo.SeatID;
	field.TraderID = m_seatInfo.TraderID;
	field.OrderLocalID = m_seatInfo.OrderLocalID;
	field.TradingDay = m_tradingDay;
	m_pTradePackage->PreparePublish(TID_NotifySeatLoginSuccess);
	m_pTradePackage->SetSubjectID(PUBLIC_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(field);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}
void SGETradeHandler::parseOrderRtn(type::ProductClass productClass, bool isOrder, int sequenceNo) {
	data::COrder order;
	auto* pOrderField = &order;
	order.ExchangeID = "SGE";
	char marketID[3] = "";
	char direction;
	char operation = char(type::OffsetFlag::Unknown);
	char orderPriceType = char(type::OrderPriceType::LimitPrice);
	char orderStatus;
	order.SequenceNo = sequenceNo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("M20", pOrderField->SeatID);
		COPY_CONTINUE("M30", pOrderField->ClientID);
		COPY_CONTINUE("I00", marketID);
		COPY_CONTINUE("I10", pOrderField->InstrumentID);
		COPY_CONTINUE("O00", pOrderField->OrderSysID);
		COPY_CONTINUE("O01", pOrderField->OrderLocalID);
		COPY_CONTINUE("O02", direction);
		COPY_CONTINUE("O03", operation);
		COPY_CONTINUE("O06", pOrderField->LimitPrice);
		COPY_CONTINUE("O07", pOrderField->Volume);
		COPY_CONTINUE("O08", pOrderField->VolumeRemain);
		COPY_CONTINUE("O09", orderStatus);
		COPY_CONTINUE("O15", orderPriceType);
		COPY_CONTINUE("T12", pOrderField->InsertTime);
		COPY_CONTINUE("T22", pOrderField->CancelTime);
		COPY_CONTINUE("T11", pOrderField->TradingDay);
	}
	pOrderField->OrderPriceType = type::OrderPriceType(orderPriceType);
	if (productClass == type::ProductClass::Unknown) {		//竞价报单类型根据市场代码区分
		pOrderField->ProductClass = (marketID[1] == '0') ? type::ProductClass::Spot : type::ProductClass::Defer;
	} else {
		pOrderField->ProductClass = productClass;
	}
	pOrderField->Direction = ConvertDirectionFromSGE(direction);
	if (operation == ZTP_OF_Unknown) {
		pOrderField->OffsetFlag = type::IsBuy(pOrderField) ? type::OffsetFlag::Open : type::OffsetFlag::Close;
	} else {
		pOrderField->OffsetFlag = ConvertOperationFromSGE(operation);
	}
	pOrderField->OrderStatus = ConvertOrderStatusFromSGE(orderStatus, isOrder);
	OnRtnOrder(pOrderField);
}
void SGETradeHandler::parseTradeRtn(type::ProductClass productClass, int sequenceNo) {
	data::CTrade trade;
	auto* pTradeField = &trade;
	trade.ExchangeID = "SGE";
	char direction;
	char operation = char(type::OffsetFlag::Unknown);
	trade.SequenceNo = sequenceNo;

	char marketID[3] = "";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("M30", pTradeField->ClientID);
		COPY_CONTINUE("I00", marketID);
		COPY_CONTINUE("I10", pTradeField->InstrumentID);
		COPY_CONTINUE("O00", pTradeField->OrderSysID);
		COPY_CONTINUE("O01", pTradeField->OrderLocalID);
		COPY_CONTINUE("O02", direction);
		COPY_CONTINUE("O03", operation);
		COPY_CONTINUE("O60", pTradeField->TradeID);
		COPY_CONTINUE("O61", pTradeField->TradeDate);
		COPY_CONTINUE("O62", pTradeField->TradeTime);
		COPY_CONTINUE("O63", pTradeField->TradePrice);
		COPY_CONTINUE("O64", pTradeField->TradeVolume);
	}

	if (productClass == type::ProductClass::Unknown) {		//竞价报单类型根据市场代码区分
		pTradeField->ProductClass = (marketID[1] == '0') ? type::ProductClass::Spot : type::ProductClass::Defer;
	} else {
		pTradeField->ProductClass = productClass;
	}
	pTradeField->Direction = ConvertDirectionFromSGE(direction);
	if (operation == ZTP_OF_Unknown) {
		pTradeField->OffsetFlag = type::IsBuy(pTradeField) ? type::OffsetFlag::Open : type::OffsetFlag::Close;
	} else {
		pTradeField->OffsetFlag = ConvertOperationFromSGE(operation);
	}
	OnRtnTrade(pTradeField);
}

bool SpotVarityCodeToInstrumentID(int varityCode, ztp::type::InstrumentID& instID) {
	switch (varityCode) {
	case 201:
		instID = "Au99.99";
		break;
	case 202:
		instID = "Au99.95";
		break;
	case 203:
		instID = "Pt99.95";
		break;
	case 204:
		instID = "Au50g";
		break;
	case 207:
		instID = "Au100g";
		break;
	case 210:
		instID = "Au99.5";
		break;
	case 301:
		instID = "iAu99.99";
		break;
	case 302:
		instID = "iAu99.5";
		break;
	case 303:
		instID = "iAu100g";
		break;
	default:
		return false;
		break;
	}
	return true;
}

type::Volume SGETradeHandler::SpotWeightToVolume(type::InstrumentID instID, double weight) {
	auto* pInst = m_instRepo.Get(instID);
	if (pInst == nullptr) {
		Log::Error(fmt::format("SGEHandler spotWeightToVolume: can not find spot instruemnt[{}]", instID));
		return 0;
	}
	auto volume = weight * 1000 / pInst->VolumeMultiple;
	if (volume < 1) {
		Log::Error(fmt::format("SGEHandler spotWeightToVolume: can not convert InstrumentID[{}] Weight[{}] Multiple[{}]", 
			instID, weight, pInst->VolumeMultiple));
		return 0;
	}
	return int(volume);
}

using namespace ztp;

void SGETradeHandler::parseStorageRtn() {
	data::CSpotPositionNotify spotPos;
	spotPos.BrokerID = type::BrokerID(m_seatInfo.ParticipantID.value());
	spotPos.ExchangeID = "SGE";

	type::ClientID clientID;
	double weight;
	int varityCode;
	StrongCharArray <3, struct TranTypeTag> storageTransType;
	// "X01=GTP1.0,X04=3,X05=8,X03=S802,X06=L,M00=0074,M20=007421,T13=20200410,T80=2471,K04=RK20200410000006,M30=0100041882,
	// K01=1,V00=201,W00=8201,T52=16.000000,T50=15.998400,S16=16.000000,T16=20200410,T17=09:29:46";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("M30", clientID);
		COPY_CONTINUE("T52", weight);
		COPY_CONTINUE("V00", varityCode);
		COPY_CONTINUE("K01", storageTransType);
		COPY_CONTINUE("T17", spotPos.NotifyTime);
	}
	auto iter = m_clientInvestorMap.find(clientID);
	if (iter == m_clientInvestorMap.end()) {
		Log::Warn(fmt::format("SGEHandler: OnSpotRtn received unsupported clientID[{}] data", clientID));
		return;
	}
	spotPos.InvestorID = iter->second;

	bool result = SpotVarityCodeToInstrumentID(varityCode, spotPos.InstrumentID);
	if (!result) {
		Log::Error(fmt::format("SGEHandler pasreStorageRtn: can not find varityCode [{}]", varityCode));
		return;
	}
	if (storageTransType == "1") { //入库
		spotPos.OffsetFlag = type::OffsetFlag::Open;
	} else if (storageTransType == "K") { //出库
		spotPos.OffsetFlag = type::OffsetFlag::Close;
	} else {
		Log::Info(fmt::format("SGEHandler pasreStorageRtn: unsupport storage trans type varityCode[{}] type[{}]", varityCode, storageTransType));
		return;
	}
	spotPos.Volume = SpotWeightToVolume(spotPos.InstrumentID, weight);
	if (spotPos.Volume < 1) {
		return;
	}
	Log::Info(fmt::format("SGEHandler parseStorageRtn: InvestorID[{}] InstrumentID[{}] OffsetFlag[{}] Volume[{}] weight[{}]",
		spotPos.InvestorID,spotPos.InstrumentID, spotPos.OffsetFlag, spotPos.Volume,weight));
	m_pTradePackage->PreparePublish(TID_NotifySpotPosition);
	m_pTradePackage->SetSubjectID(PRIVATE_FLOW);
	m_pTradePackage->SetSequenceNo(m_seriesSequence);
	m_pTradePackage->AddField(spotPos);
	m_pTradePackage->MakePackage();
	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}

void SGETradeHandler::ParsePublicMessage() {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "B102") {		//合约信息推送（行情也要合约信息）
				parseInstrumentInfo();
			}
			else if (value == "B182") {	//合约状态推送
				parseInstrumentStatus();
			}
			else if (value == "B302") {	//递延费推送
				Log::Info(fmt::format("DeferFee Raw Message:[{}]", m_parser.GetRawMessage()));
				parseDeferFee();
			}
		}
	}
}
void SGETradeHandler::parseDeferFee() {
	data::CDeferFee deferFee;
	char direction = ZTP_DFD_Unknown;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("I10", deferFee.InstrumentID);
		COPY_CONTINUE("I25", deferFee.FeeRate);
		COPY_CONTINUE("I26", direction);
	}
	deferFee.Direction = ConvertDeferFeeDirectionFromSGE(direction);
	OnRtnDeferFee(&deferFee);
}
void SGETradeHandler::parseInstrumentInfo() {
	data::CInstrument inst;
	auto* pInstrument = &inst;
	inst.ExchangeID = "SGE";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_CONTINUE("I10", pInstrument->InstrumentID);
		COPY_CONTINUE("I11", pInstrument->InstrumentName);
		COPY_CONTINUE("I12", pInstrument->VolumeMultiple);
		COPY_CONTINUE("I13", pInstrument->PriceTick);
		COPY_CONTINUE("I14", pInstrument->MaxOrderVolume);
		COPY_CONTINUE("I15", pInstrument->MinOrderVolume);
		COPY_CONTINUE("V00", pInstrument->ProductID);
	}
	OnRtnInstrumentInfo(pInstrument);
}
void SGETradeHandler::parseInstrumentStatus() {
	data::CInstrumentStatus instStatus;
	auto* pStatusField = &instStatus;
	instStatus.ExchangeID = "SGE";
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("I19" == m_parser.GetKey()) {
			pStatusField->InstrumentStatus = type::InstrumentStatus(m_parser.GetCharValue());
			OnRtnInstrumentStatus(pStatusField);
		}
	}
}
void SGETradeHandler::ParseQueryMessage() {
// 	Log::Debug(FT("SGEHandler: QueryMessage[%s]", praseBuf));
}

//行情部分
void SGETradeHandler::ParseMarketMessage(string_view const& plainMsg) {
// 	if (SgeLinkType::TRADE == m_linkType)
// 		return;	//交易连接不解析行情报文
// 
// 	SGEParser parser(*this);
// 	parser.ParseMarket(plainMsg);
}

void SGETradeHandler::getLocalIp() {
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (0 != WSAStartup(wVersionRequested, &wsaData)) {
		Log::Fatal("SGEConnection: get local IP failed");
	}

	char local[255] = { 0 };
	gethostname(local, sizeof(local));
	hostent* ph = gethostbyname(local);
	in_addr addr;
	memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr)); // 这里仅获取第一个ip
	m_localIp.assign(inet_ntoa(addr));
	WSACleanup();
}
bool SGETradeHandler::encryptKeyDomain(char* encryptedPwd, char* randomCode) {
	string pwd = m_seatInfo.Password.value();
	pwd += m_seatInfo.TraderID.value();

	//SM3加密关键域
	unsigned char hashKey[32] = "";
	int procSocket = m_safeHandler.ApplySocket();
 	int nRtn = m_safeHandler.m_pSYD_SM3_Hash(procSocket, (unsigned char*)pwd.c_str(), (const int)pwd.size(), hashKey);
	if (0 != nRtn) {
		m_safeHandler.ReleaseSocket(procSocket);
		auto const& reportMsg = fmt::format("SGEHandler: get hashKey failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	//获取二进制随机数(固定长度32)
	unsigned char randomBin[32] = "";
	nRtn = m_safeHandler.m_pSYD_GenRandom_Bin(procSocket, 32, randomBin);
	if (0 != nRtn) {
		m_safeHandler.ReleaseSocket(procSocket);
		auto const& reportMsg = fmt::format("SGEHandler: GenRandomBin failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}
	m_safeHandler.ReleaseSocket(procSocket);

	//生成国密密码
	char encPwd[100] = "";
	size_t pwdLen = 100;
	//	-1: NULL pointer | -2：in_len is not required 32 bytes | -3：random_len is not required 32 bytes
	//	-4: The length of the out_cap is not enough	| -5：encoding failed
	nRtn = sge_safe_keyfld_enc_verification(hashKey, 32/*固定长度*/, randomBin, 32/*固定长度*/, encPwd, &pwdLen);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEHandler: safe_keyfld_enc failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	//去除转义字符、转换格式并返回
	FormatSafeInfo(encPwd, pwdLen, encryptedPwd);
	string randomBase64 = Base64_encode(randomBin, 32);
	FormatSafeInfo(randomBase64.c_str(), randomBase64.size(), randomCode);
	return true;
}
