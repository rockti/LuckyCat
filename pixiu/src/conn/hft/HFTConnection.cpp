#include "pch.h"

#include "conn/hft/HFTConnection.h"
#include "model/Constants.h"
#include "model/dealing/Dealing.h"
#include "model/dealing/ExchangeRequest.h"
#include "utils/Log.h"
#include "notify/ConnPublisher.h"
#include "config/Config.h"
#include "YD/ydApi.h"
#include "EESQuoteApi.h"

static char ConvertDirectionToYD(Direction direction) {
	return (direction == Direction::BUY) ? YD_D_Buy : YD_D_Sell;
}
static char ConvertOperationToYD(Operation operation) {
	switch (operation) {
	case Operation::OPEN:
		return YD_OF_Open;
	case Operation::OFFSET:
		return YD_OF_Close;
	case Operation::OFFSET_TODAY:
		return YD_OF_CloseToday;
	default:
		throw ILLEGAL_OPERATION_TYPE;
	}
}
static char ConvertOrderTypeToYD(OrderType ordertype) {
	switch (ordertype) {
	case OrderType::NORMAL:
		return YD_ODT_Limit;
	case OrderType::FAK:
		return YD_ODT_FAK;
	case OrderType::FOK:
		return YD_ODT_FOK;
	default:
		throw ILLEGAL_OPERATION_TYPE;
	}
}
static char ConvertHedgeFlagToYD(HedgeType hedgeType) {
	return (hedgeType == HedgeType::SPECULATE) ? YD_HF_Speculation : YD_HF_Hedge;
}


HFTConnection::HFTConnection(ConnConfig& config, Instruments const& instruments, ConnPublisher& publisher, bool isMulticast)
	:ExchangeConn(ConnPlatform::HFT), m_config(config), m_instHelper(instruments), m_publisher(publisher), m_isMulticast(isMulticast)
	, m_scorer(m_frontCnts/*前置数*/, m_batchCnts/*打分次数*/), m_traderHandler(m_config, m_instHelper, publisher, m_scorer), m_marketHandler(m_config, m_instHelper, publisher)
	, m_pTraderApi(0), m_pMdApi(EESQuoteApi::CreateEESQuoteApi()) {
}

void HFTConnection::Init() {
	InitQuoteApi();
	InitTradeApi();
}

void HFTConnection::Release() {
	if (m_pMdApi) {
		m_pMdApi->Release();	
	}

	if (m_pTraderApi) {
		m_pTraderApi->startDestroy();
	}
	Log::Info("HFTConnection Released");
}

void HFTConnection::ResetOrderSet() {
	m_isOrderReady = false;
	Log::Info(fmt::format("HFTConnection: ResetOrderSetTag[{}]", m_isOrderReady));
}

void HFTConnection::InsertOrder(OrderInfo const& order) {
	const YDInstrument* pInstrument = m_pTraderApi->getInstrumentByID(order.InstCode.Str());
	
	YDInputOrder inputOrder = { 0 };
	inputOrder.Direction = ConvertDirectionToYD(order.Direction);
	inputOrder.OffsetFlag = ConvertOperationToYD(order.Operation);
	inputOrder.OrderType = ConvertOrderTypeToYD(order.OrderType);
	inputOrder.HedgeFlag = YD_HF_Speculation;
	inputOrder.Price = order.Price.value();
	inputOrder.OrderVolume = order.Quantity;
	inputOrder.OrderRef = order.LocalId;
	inputOrder.YDOrderFlag = YD_YOF_Normal;
	inputOrder.ConnectionSelectionType = YD_CS_Any;	// 席位连接方式
	inputOrder.ConnectionID = 0;

	if (!m_pTraderApi->insertOrder(&inputOrder, pInstrument)) {
		Log::Error("HFTConnection: insertOrder send failed!");
	}

// 	LogOrderInsert(0, order, RequestIdACC::Generate());
	m_pExchange = m_pTraderApi->getExchangeByID(Desc(order.Exchange));
}

int HFTConnection::InsertMultiOrders(OrderInfo const* orders[]) {	
	if (false == m_isOrderReady) {
		m_orderCnts = m_frontCnts;
		m_scorer.reset();
		for (uint32_t i = 0; i < m_frontCnts/*按照前置数量提前准备好报单*/; ++i) {
			m_instruments[i] = m_pTraderApi->getInstrumentByID(orders[i]->InstCode.Str());
			m_orderSet[i].Direction = ConvertDirectionToYD(orders[i]->Direction);
			m_orderSet[i].OffsetFlag = ConvertOperationToYD(orders[i]->Operation);
			m_orderSet[i].OrderType = YD_ODT_FAK;	// 这个接口仅用于FAK报单
			m_orderSet[i].HedgeFlag = YD_HF_Speculation;
			m_orderSet[i].Price = orders[i]->Price.value();
			m_orderSet[i].OrderVolume = orders[i]->Quantity;
			m_orderSet[i].OrderRef = LocalIdACC::NextNormal();
			m_orderSet[i].YDOrderFlag = YD_YOF_Normal;
			m_orderSet[i].ConnectionSelectionType = YD_CS_Fixed;	// 指定席位报单
			m_orderSet[i].ConnectionID = i;
		}
		m_isOrderReady = true;
		m_scorer.start(m_orderSet[0].OrderRef);
		Log::Info(fmt::format("HFTConnection: Start to insertMultiOrders[{}], FirstLocalID[{}]", m_frontCnts, m_orderSet[0].OrderRef));
	}
	else {
		if (m_batchCnts > 0) {
			for (uint32_t i = 0; i < m_frontCnts; ++i) {
				m_orderSet[i].OrderRef = LocalIdACC::NextNormal();
				m_orderSet[i].ConnectionID = i;
			}
			m_batchCnts--;
		}	// 打分批次还未结束，继续无差别委托
		else {			
			if (0 == m_selectedFront.size()) {
				if (!m_scorer.wait_done_for(std::chrono::microseconds(2800))) {
					return 0;
				}	// 打分结果没出来，直接返回，继续等待2ms(交易所应答时间2.5ms）
				else if (m_orderCnts == m_frontCnts) {
					m_selectedFront = m_scorer.select_top(m_frontCnts / 2);
					m_orderCnts = static_cast<uint32_t>(m_selectedFront.size());
				}	// 打分结果出来了，更新委托数量后根据结果选择高效席位发单
			}

			for (uint32_t i = 0; i < m_orderCnts; ++i) {
				m_orderSet[i].OrderRef = LocalIdACC::NextNormal();
				m_orderSet[i].ConnectionID = m_selectedFront[i];
				Log::Info(fmt::format("HFTConnection: Pick ConnectionID[{:d}], ConnectionCnt[{}|{}] LocalID[{}]", m_orderSet[i].ConnectionID, m_orderCnts, m_frontCnts, m_orderSet[i].OrderRef));
			}
		}
	}

	if (m_pTraderApi->insertMultiOrders(m_orderCnts, m_orderSet, m_instruments)) {
		Log::Info(fmt::format("HFTConnection: InsertMultiOrders[{}] are all successed, currentLocalID[{}] batch[{}]", m_orderCnts, LocalIdACC::GetCurrentID(), m_batchCnts));
		return m_orderCnts;
	}

	return 0;
}

void HFTConnection::CancelOrder(OrderCancel const& action) {
	if (0 == action.GetSystemNo().Length()) {
		auto msg = fmt::format("HFTConnection: cancelOrder failed, systemID is empty LocalID[{}]", action.GetLocalId());
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		return;
	}
	YDCancelOrder cancelOrder = { 0 };
	cancelOrder.OrderSysID = std::stoi(action.GetSystemNo().Str(), 0, 10);
	if (!m_pTraderApi->cancelOrder(&cancelOrder, m_pExchange)) {
		Log::Error("HFTConnection: cancelOrder send failed!");
	}
// 	LogOrderCancel(0, action, RequestIdACC::Generate());
}

void HFTConnection::SubscribeQuote(char const* instrumentCode) {
	InstID instID = m_instHelper.GetInstID(instrumentCode);
	m_traderHandler.SubscribeMarket(instID);
	m_marketHandler.SubscribeMarket(instID);
// 	Log::Info(fmt::format("HFTConnection: SubscribeQuote[{}] ", instrumentCode));
}

void HFTConnection::InitTradeApi() {
	m_pTraderApi = makeYDApi("YDConfig.txt");
	if (nullptr == m_pTraderApi) {
		auto msg = fmt::format("HFTConnection: MakeTradeApi failed[{}]", GetLastError());
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		return;
	}

	m_traderHandler.SetTraderApi(m_pTraderApi);
	if (!m_pTraderApi->start(&m_traderHandler)) {
		auto msg = fmt::format("HFTConnection: InitTradeApi failed, TradeURL[{}]", m_config.TradeEndpoint);
		m_publisher.NotifyUI(msg);
		Log::Error(msg);
		return;
	}
	Log::Info(fmt::format("HFTConnection: InitTradeApi success, TradeURL[{}] Version[{}] ", m_config.TradeEndpoint, m_pTraderApi->getVersion()));
}

void HFTConnection::InitQuoteApi() {
	m_marketHandler.SetMdApi(m_pMdApi);
	m_pMdApi->RegisterHandler(&m_marketHandler);
	auto [remoteIp, remotePort] = SeparatePort(m_config.QuoteEndpoint);
	auto [localIp, localPort] = SeparatePort(m_config.LocalEndpoint);
	int nRtn = m_pMdApi->RegisterSocket(remoteIp, remotePort, localIp, localPort, QUOTE_TYPE::FAST);
	if (0 != nRtn) {
		Log::Info(fmt::format("HFTConnection: InitQuoteApi failed[{}], RemoteUrl[{}] LocalURL[{}]", nRtn, m_config.QuoteEndpoint.c_str(), m_config.LocalEndpoint.c_str()));
		return;
	}
	m_pMdApi->Init();	// socket 初始化成功后创建行情接收线程
	Log::Info(fmt::format("HFTConnection: InitQuoteApi success, RemoteURL[{}] LocalURL[{}]", m_config.QuoteEndpoint.c_str(), m_config.LocalEndpoint.c_str()));
}
