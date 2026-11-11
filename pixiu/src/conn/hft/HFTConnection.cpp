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
	, m_scorer(m_frontCnts/*前置数*/, m_scoreBatchCnts/*打分次数*/), m_traderHandler(m_config, m_instHelper, publisher, m_scorer), m_marketHandler(m_config, m_instHelper, publisher)
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
		m_probeBatchesRemaining = m_scoreBatchCnts - 1;
		m_rankedFront.clear();
		m_nextRankIndex = 0;
		m_rankedFrontReady = false;
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
			m_orderSet[i].ConnectionSelectionType = YD_CS_Any;
			m_orderSet[i].ConnectionID = 0;
		}

		m_isOrderReady = true;
		m_scorer.start(m_orderSet[0].OrderRef);
		Log::Info(fmt::format("HFTConnection: Start to insertMultiOrders[{}], FirstLocalID[{}], ScoreBatches[{}], ProbeBatchesRemaining[{}]"
			, m_frontCnts, m_orderSet[0].OrderRef, m_scoreBatchCnts, m_probeBatchesRemaining));
	}	// Timer启动后的首批报单：YD_CS_Any
	else if (m_probeBatchesRemaining > 0) {
		for (uint32_t i = 0; i < m_frontCnts; ++i) {
			m_orderSet[i].OrderRef = LocalIdACC::NextNormal();
			m_orderSet[i].ConnectionID = 0;
		}
		--m_probeBatchesRemaining;
	}	// 打分批次报单
	else {
		if (!m_scorer.isDone()) {
			return 0;
		}	// 等待评分，结果出来之前不再发委托
		
		if (!m_rankedFrontReady) {
			constexpr std::size_t dropWorst = 2;	//淘汰掉的前置数
			m_rankedFront = m_scorer.select_rankRange(0, m_frontCnts - 1 - dropWorst);
			m_nextRankIndex = 0;
			m_rankedFrontReady = true;
			if (m_rankedFront.empty()) {
				return 0;
			}
		}	// 评分完成后缓存一次前置名次

		const std::size_t total = m_rankedFront.size();
		const std::size_t firstHalf = (total + 1) / 2;
		m_orderCnts = static_cast<uint32_t>(m_nextRankIndex == 0 ? firstHalf : total - m_nextRankIndex);
		for (uint32_t i = 0; i < m_orderCnts; ++i) {
			const std::size_t rankIndex = m_nextRankIndex + i;
			m_orderSet[i].OrderRef = LocalIdACC::NextNormal();
			m_orderSet[i].ConnectionSelectionType = YD_CS_Prefered;
			m_orderSet[i].ConnectionID = m_rankedFront[rankIndex];
			Log::Info(fmt::format("HFTConnection: Pick ConnectionID[{:d}], LocalID[{}]", m_orderSet[i].ConnectionID, m_orderSet[i].OrderRef));
		}
	}

	if (m_pTraderApi->insertMultiOrders(m_orderCnts, m_orderSet, m_instruments)) {
		if (m_rankedFrontReady) {
			m_nextRankIndex += m_orderCnts;
			if (m_nextRankIndex >= m_rankedFront.size()) {
				m_nextRankIndex = 0;
			}
		}
		return m_orderCnts;
	}
	else {
		Log::Info(fmt::format("HFTConnection: InsertMultiOrders[{}] API returned failure, currentLocalID[{}]", m_orderCnts, LocalIdACC::GetCurrentID()));
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
