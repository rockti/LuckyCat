#include "stdafx.h"
#include "SGEQuoteHandler.h"
#include "BaseExchApi.h"
#include "SGEConnection.h"
#include <fmt/format.h>
#include "SGESerializer.h"
#include "ZtpType.h"
#include "ServiceDef.h"

void SGEQuoteHandler::OnFrontConnected(void* ctx) {
	Log::Info("SGEQuoteHandler: FrontConnected");
	m_pThis->m_isConnected = true;
	if (m_pThis->GetPublicKey()) {
		auto const& msg = fmt::format("SGEQuoteHandler: handshake {}", m_pThis->Handshake() ? "success!" : "failed!");
		RUNTIME_INFO_REPORT(msg.c_str());
	}
}
void SGEQuoteHandler::OnFrontDisconnected(int error_no, void* ctx) {
	string reportMsg = fmt::format("SGEQuoteHandler: FrontDisconnect, errorNo[{}]", error_no);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
	m_pThis->m_isConnected = false;
	m_pThis->m_isLogined = false;
	//TODO: reconnect operation
	if (m_pThis->m_pQuoteContext->SafeSessionPtr) {
		sge_safe_free_client_session(m_pThis->m_pQuoteContext->SafeSessionPtr);
		m_pThis->m_pQuoteContext->SafeSessionPtr = NULL;
	}
}
void SGEQuoteHandler::OnReceiveMsg(int type, const char* gtp_msg, unsigned int msgLen, void* ctx) {
	int praseBufLens = SGE_MSG_LEN * 2;	//留一位给'/0'
	char receivedMsg[SGE_MSG_LEN * 2] = "";
	if (MSG_TYPE_PLAIN == type) {
		memcpy(receivedMsg, gtp_msg, msgLen);
		string_view gtpMsg(receivedMsg, msgLen);
  		m_pThis->DispatchPlainMsg(gtpMsg);
	} else if (MSG_TYPE_CIPHER == type) {
		if (m_pThis->m_safeHandler.DecryptMsg(gtp_msg, msgLen, receivedMsg, praseBufLens, m_pThis->m_pQuoteContext->SessionKey)) {
			string_view praseMsg(receivedMsg, msgLen);
			m_pThis->DispatchCipherMsg(praseMsg);
		}
	} else {
		string reportMsg = fmt::format("SGEQuoteHandler: received unknown msgType[{}]", type);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
	}
}


SGEQuoteHandler* SGEQuoteHandler::m_pThis = NULL;
SGEQuoteHandler::SGEQuoteHandler(IFlow* marketFlow, string const& sgeAddress, int sgePort, SafeContext& safeContext)
:m_marketFlow(marketFlow),m_address(sgeAddress),m_port(sgePort)
,m_safeHandler(safeContext.Address, safeContext.Port)
,m_pQuoteContext(nullptr),m_encryptedMsgLen(0),m_seriesSequence(0),m_instrumentStatus(type::InstrumentStatus::Initting) {
	m_pThis = this;

	m_pMarketPackage = new CFTDPackage();
	m_pMarketPackage->ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 2000);

	getLocalIp();
}
SGEQuoteHandler::~SGEQuoteHandler() {
}

void SGEQuoteHandler::Login() {
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
		if (!m_safeHandler.EncryptMsg("Login", result.Message, result.Size, m_encryptedMsg, m_encryptedMsgLen, m_pQuoteContext->SessionKey)) {
			char const* reportMsg = "SGEQuoteHandler: encrypt LoginMsg failed, isTrade[1]";
			RUNTIME_INFO_REPORT(reportMsg);
			Log::Error(reportMsg);
			return;
		}


		int nRtn = sge_session_write(m_pQuoteContext->SGESessionPtr, MSG_TYPE_CIPHER, (char const*)m_encryptedMsg, m_encryptedMsgLen);
		if (0 != nRtn) {
			auto const& reportMsg = fmt::format("SGEQuoteHandler: send helloInfo failed, rtnCode[{}] isTrade[1]", nRtn);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return;
		}
	}
}

void SGEQuoteHandler::StartWork() {
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
#define COPY_ERRORID(ekey,target) if (CopyErrorID(ekey, key, target,m_parser)) continue;


void SGEQuoteHandler::OnRspError(data::CRspInfo* pRspInfo) {	
	string reportMsg = fmt::format("SGEQuoteHandler: OnRspError, errCode[{}] errMsg[{}]", pRspInfo->ErrorID, pRspInfo->ErrorMsg);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
}
void SGEQuoteHandler::OnRspLogin(data::CSeatLogin* pRspLogin, data::CRspInfo* pRspInfo) {
	string reportMsg = "SGEQuoteHandler: SeatLogin ";
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
	}
}

void SGEQuoteHandler::OnRtnInstrumentInfo(data::CInstrument* pInstrumentField) {	
// 	CFTDDepthMarketDataField ftdMarketData;
// 	CFTDDepthMarketDataField* pMarketData = &ftdMarketData;
// 	pMarketData->InstrumentID = pInstrumentField->InstrumentID.value();
	if (pInstrumentField->InstrumentID == "") {
		return;
	}
	static std::set<type::InstrumentID> insts;
	if (insts.find(pInstrumentField->InstrumentID) == insts.end()) {
		insts.insert(pInstrumentField->InstrumentID);
		auto index = FTDInstance().Add(pInstrumentField->InstrumentID);	//根据InstrumentID建立MarketRepository的Map
	}
}
void SGEQuoteHandler::OnRtnInstrumentStatus(data::CInstrumentStatus* pStatusField) {

}
void SGEQuoteHandler::OnRtnDepthMarketData(CFTDDepthMarketDataField* pMarketDataField) {	//注意：行情处理用FTD格式，不用ZTP
	auto instIndex = FTDInstance().UpdateSnapshot(*pMarketDataField);
	m_pMarketPackage->PreparePackage(FTD_TID_IntlRtnDepthMarketData, FTD_CHAIN_LAST);
	m_pMarketPackage->SetInstrumentIDCount(instIndex.Index());

	FTD_ADD_FIELD(m_pMarketPackage, pMarketDataField);
	m_pMarketPackage->MakePackage();
	m_marketFlow->Append(m_pMarketPackage->GetAddress(), m_pMarketPackage->GetLength());
}

void SGEQuoteHandler::InitSeatInfo(data::CSeatInfo const& seatField) {
	CopySeatInfoEntity(&m_seatInfo, &seatField);

	strcpy_s(m_sessionInfo.sge_id_, m_seatInfo.ExchangeID.value());
	strcpy_s(m_sessionInfo.trader_id_, m_seatInfo.TraderID.value());
	m_sessionInfo.enc_mode_ = 0;
	memset(m_sessionInfo.svr_enc_pubkey_, 0, sizeof(m_sessionInfo.svr_enc_pubkey_));
	memset(m_sessionInfo.svr_sign_pubkey_, 0, sizeof(m_sessionInfo.svr_sign_pubkey_));
	m_sessionInfo.svr_enc_pubkey_len_ = 0;
	m_sessionInfo.svr_sign_pubkey_len_ = 0;
}

void SGEQuoteHandler::DispatchPlainMsg(string_view const& plainMsg) {
	m_parser.Init(plainMsg);
	if (MARKET_STREAM == plainMsg[15]) {
		ParseMarketMessage(plainMsg);
	} else {
		Log::Info(fmt::format("SGEQuoteHandler: PlainMsg[{}][{}]", plainMsg, plainMsg.length()));
		PraseSafeMessage();
	}
}
void SGEQuoteHandler::DispatchCipherMsg(string_view const msg) {	
	m_parser.Init(msg);
	switch (msg[15]) {
	case TRADE_STREAM:		//交易流:应答消息(委托应答，撤单应答，内含通用报错X39，X40字段)
		++m_seriesSequence;
		ParseTradeMessage();
		break;
	case PRIVATE_STREAM:	//私有流(只有OnRtnOrder，OnRtnTrade)
		++m_seriesSequence;
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

bool SGEQuoteHandler::GetPublicKey() {
	string reportMsg = "SGEQuoteHandler: ";
	int pubKeyLen = 140;
	int procSocket = m_safeHandler.ApplySocket();
	int nRtn = m_safeHandler.m_pSYD_SM2_GetPublicKey(procSocket, m_sessionInfo.trader_id_, 0x00, m_sessionInfo.svr_sign_pubkey_, &pubKeyLen);
 	Log::Info(fmt::format("GetPublicKey, traderID[{}] svr_sign_public[{}] rtn[{}]", m_sessionInfo.trader_id_, m_sessionInfo.svr_sign_pubkey_, nRtn));
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
	reportMsg += fmt::format("get publicKey failed, rtnCode[{}]", nRtn);
	RUNTIME_INFO_REPORT(reportMsg.c_str());
	Log::Error(reportMsg.c_str());
	return false;
}
bool SGEQuoteHandler::Handshake() {
	string reportMsg = "SGEQuoteHandler: ";

	m_pQuoteContext->SafeSessionPtr = sge_safe_create_client_session(m_pQuoteContext->SafeApiPtr, &m_sessionInfo);
	if (NULL == m_pQuoteContext->SafeSessionPtr) {
		reportMsg += "create SafeSession failed, isTrade[1]";
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}
	fld_client_hello_t helloInfo;
	strcpy_s(helloInfo.trader_id_, m_sessionInfo.trader_id_);
	strcpy_s(helloInfo.sge_id_, m_sessionInfo.sge_id_);
	helloInfo.enc_mode_ = 0;
	strcpy_s(helloInfo.safe_api_trader_version_, sge_safe_client_get_api_version());

	api_safe_client_session_t* pTheSafeSessionPtr = m_pQuoteContext->SafeSessionPtr;
	int nRtn = sge_safe_client_create_hello(pTheSafeSessionPtr, &helloInfo);	//创建安全信息
	if (0 == nRtn) {	//发送握手报文	
		stringstream ssHello("");
		int requestId = RequestIdACC::Generate();
		auto const& helloBody = fmt::format("M60={},G04={},X83=0,X56={}", helloInfo.trader_id_, helloInfo.sge_id_, helloInfo.safe_api_trader_version_);
		ssHello << fmt::format("X01=GTP1.0,X03=AA00,X04=1,X05={},X06=L,X07={},", requestId, requestId) << helloBody;
		Log::Info(fmt::format("SGEHello: [{}]", ssHello.str().c_str()));

		//不同的前置对应不同的Session握手
		sge_session_t* theSessionPtr = m_pQuoteContext->SGESessionPtr;
		nRtn = sge_session_write(theSessionPtr, MSG_TYPE_PLAIN, ssHello.str().c_str(), (unsigned int)ssHello.str().size());
		if (0 != nRtn) {
			reportMsg += fmt::format("send helloInfo failed, rtnCode[{}]", nRtn);
			RUNTIME_INFO_REPORT(reportMsg.c_str());
			Log::Error(reportMsg.c_str());
			return false;
		}
	} else {
		reportMsg += fmt::format("create helloInfo failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}
	return true;
}

//安全认证部分
void SGEQuoteHandler::PraseSafeMessage() {
	data::CRspInfo rspInfo;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "AA01") {		//握手应答
				fld_server_hello_t helloRtn = { 0 };
				if (PraseHandshakeMsg(pRspInfo, helloRtn)) {
					doRspHandshake(helloRtn);
				}
			}
			else if (value == "AA11") {	//验证应答
				fld_sessionkey_info_t keyRtn = { 0 };
				if (PraseCertificationMsg(pRspInfo, keyRtn)) {
					doRspCertification(keyRtn);
				}
			}
		}
		else if (key == "M991") {
			ParseErrorMessage();
		}
	}
}
bool SGEQuoteHandler::PraseHandshakeMsg(data::CRspInfo* pRspInfo, fld_server_hello_t& svrHello) {
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
void SGEQuoteHandler::doRspHandshake(fld_server_hello_t const& svrHello) {
	api_safe_client_link_t* theSafeLinkPtr = m_pQuoteContext->SafeLinkPtr;
	api_safe_client_session_t* theSafeSessionPtr = m_pQuoteContext->SafeSessionPtr;
	
	//处理Hello应答
	int nRtn = sge_safe_client_handle_hello_rsp(theSafeSessionPtr, theSafeLinkPtr, &svrHello);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEQuoteHandler: safeHello respond failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}

	//生成认证信息并发送
	fld_authentication_info_t auth_info = { 0 };
	nRtn = sge_safe_client_create_authentication(theSafeSessionPtr, theSafeLinkPtr, &auth_info);
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEQuoteHandler: create authentication failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}

	stringstream ssAuthenticate("");
	int requestId = RequestIdACC::Generate();
	auto const& authenticateBody = fmt::format("M60={},G04={},M63={},G05={},M64={}", 
		auth_info.trader_id_, auth_info.sge_id_, auth_info.trader_random_, auth_info.sge_random_, auth_info.authen_signed_data_);
	ssAuthenticate << fmt::format("X01=GTP1.0,X03=AA10,X04=1,X05={},X06=L,X07={},", requestId, requestId) << authenticateBody;
	Log::Debug(fmt::format("Authenticate: [{}]", ssAuthenticate.str().c_str()));

	sge_session_t* theSessionPtr = m_pQuoteContext->SGESessionPtr;
	nRtn = sge_session_write(theSessionPtr, MSG_TYPE_PLAIN, ssAuthenticate.str().c_str(), (unsigned int)ssAuthenticate.str().size());
	if (0 != nRtn) {
		auto const& reportMsg = fmt::format("SGEQuoteHandler: send authenticate request failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}
}
bool SGEQuoteHandler::PraseCertificationMsg(data::CRspInfo* pRspInfo, fld_sessionkey_info_t & sessionKey) {
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
void SGEQuoteHandler::doRspCertification(fld_sessionkey_info_t const& sessionKey) {

	api_safe_client_link_t* theSafeLinkPtr = m_pQuoteContext->SafeLinkPtr;
	api_safe_client_session_t* theSafeSessionPtr = m_pQuoteContext->SafeSessionPtr;

	//验证SGE签名
	int nRtn = sge_safe_client_accept_sessionkey(theSafeSessionPtr, theSafeLinkPtr, &sessionKey);
	if (0 != nRtn) {
		string reportMsg = fmt::format("SGEQuoteHandler: DoCertification failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return;
	}
	Log::Info(fmt::format("SGEQuoteHandler: sessionKey[{}]", m_pThis->m_safeHandler.GetSessionKey()));

	memcpy(m_pThis->m_pQuoteContext->SessionKey, m_pThis->m_safeHandler.GetSessionKey(), 34);
	m_pThis->m_pQuoteContext->IsHandshake = true;
}
void SGEQuoteHandler::ParseErrorMessage() {
	data::CRspInfo rspInfo;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("X03"==m_parser.GetKey() && "M991"== m_parser.GetStringValue()) {	//M991通用非业务报错
			parseRspError(pRspInfo);
			OnRspError(pRspInfo);
		}
	}
}
void SGEQuoteHandler::parseRspError(data::CRspInfo* pRspInfo) {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		COPY_ERRORID("X39", pRspInfo->ErrorID);
		COPY_CONTINUE("X40", pRspInfo->ErrorMsg);
	}
}


void SGEQuoteHandler::parseRspLogin(data::CRspInfo* pRspInfo) {
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

void SGEQuoteHandler::notifySeatLoginSuccess() {

// 	data::CSeatLogin field;
// 	field.ExchangeID = "SGE";
// 	field.SeatID = m_seatInfo.SeatID;
// 	field.TraderID = m_seatInfo.TraderID;
// 	field.OrderLocalID = m_seatInfo.OrderLocalID;
// 	field.TradingDay = m_tradingDay;
// 	m_pTradePackage->PreparePublish(TID_NotifySeatLoginSuccess);
// 	m_pTradePackage->SetSubjectID(PUBLIC_FLOW);
// 	m_pTradePackage->SetSequenceNo(m_seriesSequence);
// 	m_pTradePackage->AddField(field);
// 	m_pTradePackage->MakePackage();
// 	m_pSeriesFlow->Append(m_pTradePackage->GetAddress(), m_pTradePackage->GetLength());
}

void SGEQuoteHandler::ParseTradeMessage() {
	data::CRspInfo rspInfo;
	auto* pRspInfo = &rspInfo;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "A101") {		//登录应答
				parseRspLogin(pRspInfo);
			}
		}
	}
}

void SGEQuoteHandler::ParsePublicMessage() {
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "X03") {
			auto value = m_parser.GetStringValue();
			if (value == "B102") {		//合约信息推送（行情也要合约信息）
				parseInstrumentInfo();
			}
		}
	}
}

void SGEQuoteHandler::parseInstrumentInfo() {
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
void SGEQuoteHandler::parseInstrumentStatus() {
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
void SGEQuoteHandler::ParseQueryMessage() {
// 	Log::Debug(FT("SGEQuoteHandler: QueryMessage[%s]", praseBuf));
}

//行情部分
void SGEQuoteHandler::ParseMarketMessage(string_view const& plainMsg) {

	SGEParser parser(*this);
	parser.ParseMarket(plainMsg);
}

void SGEQuoteHandler::getLocalIp() {
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
bool SGEQuoteHandler::encryptKeyDomain(char* encryptedPwd, char* randomCode) {
	string pwd = m_seatInfo.Password.value();
	pwd += m_seatInfo.TraderID.value();

	//SM3加密关键域
	unsigned char hashKey[32] = "";
	int procSocket = m_safeHandler.ApplySocket();
 	int nRtn = m_safeHandler.m_pSYD_SM3_Hash(procSocket, (unsigned char*)pwd.c_str(), (const int)pwd.size(), hashKey);
	if (0 != nRtn) {
		m_safeHandler.ReleaseSocket(procSocket);
		auto const& reportMsg = fmt::format("SGEQuoteHandler: get hashKey failed, rtnCode[{}]", nRtn);
		RUNTIME_INFO_REPORT(reportMsg.c_str());
		Log::Error(reportMsg.c_str());
		return false;
	}

	//获取二进制随机数(固定长度32)
	unsigned char randomBin[32] = "";
	nRtn = m_safeHandler.m_pSYD_GenRandom_Bin(procSocket, 32, randomBin);
	if (0 != nRtn) {
		m_safeHandler.ReleaseSocket(procSocket);
		auto const& reportMsg = fmt::format("SGEQuoteHandler: GenRandomBin failed, rtnCode[{}]", nRtn);
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
		auto const& reportMsg = fmt::format("SGEQuoteHandler: safe_keyfld_enc failed, rtnCode[{}]", nRtn);
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
