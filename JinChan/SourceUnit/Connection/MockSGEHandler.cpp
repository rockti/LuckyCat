#include "stdafx.h"
#include "MockSGEHandler.h"
#include "MockSGEConnection.h"
#include "Profile.h"
#include "InstrumentManager.h"
#include <iostream>
#include <thread>
#include <chrono>

class Timer {
	bool clear = false;

public:
	template<typename Function>
	void setTimeout(Function function, int delay);
	template<typename Function>
	void setInterval(Function function, int interval);
	void stop();
};

template<typename Function>
void Timer::setTimeout(Function function, int delay) {
	this->clear = false;
	std::thread t([=]() {
		if (this->clear) return;
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		if (this->clear) return;
		function();
		});
	t.detach();
}

template<typename Function>
void Timer::setInterval(Function function, int interval) {
	this->clear = false;
	std::thread t([=]() {
		while (true) {
			if (this->clear) return;
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			if (this->clear) return;
			function();
		}
		});
	t.detach();
}
inline int next() {
	static int s_seqNo = 1;
	return s_seqNo++;
}
void Timer::stop() {
	this->clear = true;
}

SGEMockHandler::SGEMockHandler(SgeLinkType::TYPE type, CMarketFlowManager* pFlowManager, /*CMarketRepository* pMarketRepository,*/ int topicID, string const& sgeAddress, int sgePort, SafeContext& safeContext)
:m_linkType(type), m_pMarketFlowManager(pFlowManager),/* m_pMarketRepository(pMarketRepository),*/ m_topicID(topicID), m_address(sgeAddress), m_port(sgePort), m_encryptedMsgLen(0), m_safeHandler(safeContext.Address, safeContext.Port)
, m_marketSeqNo(0), m_privateSeqNo(0), m_maxOrderLocalID(0),m_exchangeStatus(type::InstrumentStatus::Initting), m_playbookFactory(this)

{

	m_pFtdPackage = new CFTDPackage();
	m_pFtdPackage->ConstructAllocate(FTD_PACKAGE_MAX_SIZE, 2000);
	m_pZtpPackage = new CZTPPackage();
	m_pZtpPackage->ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);

// 	static Timer t = Timer();

// 	t.setInterval([&]() {
// 		for (auto& order:m_orders) {
// 			order.second.OnTimer();
// 		}
// 		}, 5);

// 	t.setTimeout([&]() {
// 		t.stop();
// 		}, 520000000);

	static type::Price lastPrice = 999.0;
	static Timer marketTimer;
	FTDInstance().Add("Au(T+D)");
	marketTimer.setInterval([&]() {
		lastPrice += 0.1;
		if (m_pMarketFlowManager != nullptr) {
			CFTDDepthMarketDataField market;
			market.InstrumentID = "Au(T+D)";
			market.AskPrice1 = 1010.0;
			market.AskPrice2 = 1009.0;
			market.AskPrice3 = 1008.0;
			market.AskPrice4 = 1007.0;
			market.AskPrice5 = 1006.0;
			market.BidPrice1 = 1011.0;
			market.BidPrice2 = 1012.0;
			market.BidPrice3 = 1013.0;
			market.BidPrice4 = 1014.0;
			market.BidPrice5 = 1015.0;
			market.AskVolume1 = 5;
			market.AskVolume2 = 4;
			market.AskVolume3 = 3;
			market.AskVolume4 = 2;
			market.AskVolume5 = 1;
			market.BidVolume1 = 5;
			market.BidVolume2 = 4;
			market.BidVolume3 = 3;
			market.BidVolume4 = 2;
			market.BidVolume5 = 1;
			market.ClosePrice = 1000;
			market.HighestPrice = 1050;
			market.LastPrice = lastPrice;
			market.LowerLimitPrice = 1000;
			market.LowestPrice = 950;
			market.Volume = 10000;

			auto instIndex = FTDInstance().UpdateSnapshot(market);
			m_pFtdPackage->PreparePackage(FTD_TID_IntlRtnDepthMarketData, FTD_CHAIN_LAST);
			m_pFtdPackage->SetInstrumentIDCount(instIndex.Index());
			FTD_ADD_FIELD(m_pFtdPackage, &market);
			m_pMarketFlowManager->PulishToTopicFlow(m_pFtdPackage,1000);

		}
	}, 5);


// 	static Timer deferFeeTimer = Timer();
// 
// 	deferFeeTimer.setInterval([&]() {
// 		PushDeferFee();
// 		}, 5000);
// 
// 	deferFeeTimer.setTimeout([&]() {
// 		deferFeeTimer.stop();
// 		}, 520000000);

	m_pSeriesFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x200000);
	m_seriesReader.AttachFlow(m_pSeriesFlow, 0);

}
SGEMockHandler::~SGEMockHandler()
{
	if (m_pFtdPackage) {
		delete m_pFtdPackage;
		m_pFtdPackage = NULL;
	}
	if (m_pZtpPackage) {
		delete m_pZtpPackage;
		m_pZtpPackage = NULL;
	}
}


void SGEMockHandler::Login()
{
	m_isLogined = true;
	PushLogin();
	PushInstrumentReady();
	PushSpotPosition();
	PushDeferFee();
}
bool SGEMockHandler::ReqOrderInsert(data::COrder& inputOrderField) {

	int requestId = inputOrderField.OrderLocalID.value();
	m_maxOrderLocalID++;
	m_orders.insert(make_pair(requestId,m_playbookFactory.Normal(inputOrderField)));
	return true;
}

bool SGEMockHandler::ReqOrderAction(data::COrderAction& actionField) {

	int requestId = actionField.OrderLocalID;

	auto iter = m_orders.find(requestId);
	if (iter != m_orders.end()) {
		iter->second.OnCancel();
	}
	else {
		//PushErrorCancel(); //TODO
	}
	return true;
}


void SGEMockHandler::StartWork() {

}


inline void OnlyStatusChanger::Action(data::COrder& order, data::CTrade& trade) {
	order.OrderStatus = m_status;
	order.SequenceNo = next();
	order.TradingDay = m_handler->GetTradingDay();
	m_handler->PushOrder(order, 0, "SGE:Success");
	if (m_status == type::OrderStatus::Canceled) {
//		m_handler->PushInstrumentStatus(false);
	}
}

inline void AllTradeChanger::Action(data::COrder& order, data::CTrade& trade) {
	order.VolumeRemain = 0;
	order.VolumeTrade = order.Volume;
	order.OrderStatus = type::OrderStatus::AllTraded;
	order.SequenceNo = next();
	order.TradingDay = m_handler->GetTradingDay();
	trade.TradePrice = order.LimitPrice;
	trade.TradeVolume = order.Volume;
	trade.TradeAmt = order.Volume * order.LimitPrice;
	trade.OrderSysID = order.OrderSysID;
	trade.TradeDate = m_handler->GetTradingDay();
	trade.SequenceNo = next();
	m_handler->PushOrder(order, 0,"SGE:Success");
	m_handler->PushTrade(trade);
}

inline void PartialTradeChanger::Action(data::COrder& order, data::CTrade& trade) {
	order.VolumeRemain = 0;
	order.VolumeTrade = order.Volume;
	order.OrderStatus = type::OrderStatus::PartTradedQueueing;
	order.SequenceNo = next();
	order.TradingDay = m_handler->GetTradingDay();
	trade.TradePrice = order.LimitPrice;
	trade.TradeVolume = order.Volume-1;
	trade.TradeAmt = order.Volume * order.LimitPrice;
	trade.OrderSysID = order.OrderSysID;
	trade.TradeDate = m_handler->GetTradingDay();
	trade.SequenceNo = next();
	m_handler->PushOrder(order, 0, "SGE:Success");
	m_handler->PushTrade(trade);
}


inline void OrderedChanger::Action(data::COrder& order, data::CTrade& trade) {
	order.OrderStatus = type::OrderStatus::NoTradeQueueing;
	order.OrderSysID = m_systemNo.c_str();
	order.TradingDay = m_handler->GetTradingDay();
	order.SequenceNo = next();
	m_handler->PushOrder(order, 0, "SGE:Success");
}
