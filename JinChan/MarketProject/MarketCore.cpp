#include "stdafx.h"
#include "MarketCore.h"
#include "Profile.h"
#include "MarketFront.h"
#include "SGEQuoteConnection.h"
#include "MockSGEConnection.h"
#include <fmt/format.h>
#include "ServiceDef.h"

using namespace ztp;

CMarketCore::CMarketCore(CSelectThread* pInitThread)
:m_pMarketFlowManager(NULL)
,m_pMarketFlow(NULL),m_pInitThread(pInitThread)
{
	m_marketBrokerID = theProfile().GetPrivateProfile("Broker", "MarketBrokerID", "");
	if (m_marketBrokerID.empty()) {
		printNlog("load market broker id failed, exit");
		exit(-1);
	}

}
CMarketCore::~CMarketCore() {
	delete 	m_pMarketFlowManager;
	delete m_pMarketFlow;
}


bool CMarketCore::InitInstance() {
	string flowPath = theProfile().GetPrivateProfile("Common", "FlowPath", "./");
	string marketFrontUrl = theProfile().GetPrivateProfile("Front", "MarketFrontUrl", "tcp://127.0.0.1:13823");
	m_pMarketFlow = new CSeriesCachedFlow(false, 2000000, 0x100000);
	m_initPackage.ConstructAllocate(1000, 1000);

	static MemSystemStatusRepository s_systemRepo;

	m_database = new Database(m_pMarketFlow, s_systemRepo);
	m_database->Create();


 
 	//等待InitLoad数据库操作完成
	while (!m_isInited) {
		for (int i = 0; i < 1000; ++i) {
			if (!m_database->GetNextUseNoBuf(&m_initPackage))	//获取ZTP有序数据包
				break;
			HandleMessage(&m_initPackage);		//丢给交易引擎处理
		}
		::Sleep(200);
 	}

	m_pMarketFlowManager = new CMarketFlowManager(flowPath.c_str(), m_pMarketFlow);	//生成行情流管理器，负责给Channel和Flow发送数据
//	m_pMarketRepository = new CMarketRepository(this);								//生成行情存储器，保存topic和行情数据，
	CMarketFrontManager* pMarketFrontManager = new CMarketFrontManager(this, m_pMarketFlowManager);
	pMarketFrontManager->RegisterListener(marketFrontUrl.c_str());
	CSelectThread* pFrontThread = pMarketFrontManager->CreateFrontThread(m_investorRepo);			//创建前置线程，管理行情数据流和广播
	return CreateMarketConnection(pFrontThread);
}

void CMarketCore::HandleMessage(CZTPPackage* pPackage) {

	switch (pPackage->GetTID()) {
	case TID_DataSyncEnd:
		m_isInited = true;
		break;
	case TID_InitSeatInfo: {
		auto fieldItor = pPackage->GetFieldTypeIterator();
		data::CSeatInfo seatInfo;
		while (!fieldItor.IsEnd()) {
			fieldItor.Retrieve(seatInfo);
			if (seatInfo.AppName == "Market" && seatInfo.ParticipantID == m_marketBrokerID.c_str()) {
				m_seatInfo = seatInfo;
				printNlog("Init Seat Info Success BrokerID[{}] tradeID[{}] SeatID[{}]\n", seatInfo.ParticipantID, m_seatInfo.TraderID, m_seatInfo.SeatID);
//				return;
			}
			fieldItor.Next();
		}
		break;
	}
	case TID_InitInvestorInfo: {
		auto fieldItor = pPackage->GetFieldTypeIterator();
		data::CInvestorInfo info;
		while (!fieldItor.IsEnd()) {
			fieldItor.Retrieve(info);
			m_investorRepo.InitAdd(info);
			fieldItor.Next();
		}
		break;
	}
	default:
		break;
	}
}

class EmptyInstrumentRepository : public InstrumentRepository {
public:
	virtual void InitAdd(data::CInstrument const& instrument) override {
	}
	virtual void AddOrUpdate(data::CInstrument const& instrument) override {

	}
	virtual data::CInstrument* Get(type::InstrumentID const& instrumentID) override {
		return nullptr;
	}
	virtual void ForEach(type::ProductClass ProductClass, type::InstrumentID const& InstrumentID, ForEachFunc func) override {
	}

};

bool CMarketCore::CreateMarketConnection(CSelectThread* pFrontThread) {

#ifdef JINCHAN_MOCK
	static MockSGEConnection connection(SgeLinkType::QUOTE, m_pMarketFlowManager, 1000, "", 0, "", 0);
	connection.InitInstance(pFrontThread);
	return true;
#else

	if (m_seatInfo.ParticipantID == "") {
		printNlog("Load Market Seat Info failed. exit\n");
		exit(-1);
	}
	int topicID = theProfile().GetPrivateProfile("SGE", "TopicID", 1000);
	string sgeAddr = theProfile().GetPrivateProfile("SGE", "SGEAddress", "188.31.25.50");
	int sgePort = theProfile().GetPrivateProfile("SGE", "SGEPort", 7777);
	string safeAddr = theProfile().GetPrivateProfile("SGE", "SafeAddress", "127.0.0.1");
	int safePort = theProfile().GetPrivateProfile("SGE", "SafePort", 6666);
	static SGEQuoteConnection connection(m_seatInfo, m_pMarketFlow, sgeAddr, sgePort, safeAddr, safePort);	// 20240405，m_seatInfo作为参数传入
// 	connection.GetHandler().InitSeatInfo(m_seatInfo);
	if (connection.InitInstance(pFrontThread)) {
// 		connection.GetHandler().StartWork();	//unit test
		connection.GetHandler().Login();
		return true;
	}
	return false;
#endif
}

