#include "pch.h"
#include "HFTMarketHandler.h"

#include "model/Constants.h"
#include "notify/Notify.h"
#include "notify/ConnPublisher.h"
#include "utils/Log.h"
#include "config/Config.h"


HFTMarketHandler::HFTMarketHandler(ConnConfig const& config, ExchangeConnInstrumentHelper const& instHelper, ConnPublisher& publisher)
	:m_config(config), m_instHelper(instHelper), m_publisher(publisher), m_pMdApi(0) {
}

void HFTMarketHandler::OnConnect() {
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HFT;
	notification.Type = ExchangeNotification::QUOTE_LOGINED;
	m_publisher.NotifyUI(notification);
	Log::Info("HFTMarketHandler FrontConnected");
}
void HFTMarketHandler::OnDisconnect() {
	ExchangeNotification notification;
	notification.Platform = ConnPlatform::HFT;
	notification.Type = ExchangeNotification::QUOTE_DISCONNECTED;
	m_publisher.NotifyUI(notification);
	Log::Error("HFTMarketHandler: FrontDisConnected");
}
void HFTMarketHandler::OnRtnDepthMarketData(int id, char const* buff, unsigned int len) {
	if (QUOTE_TYPE::FAST == id) {
		if (0 == (len % sizeof(efh3_2_fut_lev1))) {
			unsigned int procLen = 0;
			while (procLen < len) {
				efh3_2_fut_lev1* pInfo = (efh3_2_fut_lev1*)(buff + procLen);
				InstID instID = m_instHelper.GetInstID(pInfo->m_symbol);
				if (m_subscribeList.end() != m_subscribeList.find(instID) && pInfo->m_last_px > 0) {
					Quote quote(OnProcSimpleQuote(instID, pInfo));
					m_publisher.PushPrice(quote);
// 					Log::Info(fmt::format("HFTMarketHandler: notifyMarketData, Instr[{}] Ask[{:.2f},{}] Last[{:.2f},{}] Bid[{:.2f},{}] Interest[{}] Time[{:02d}:{:02d}:{}]",
// 						instID, quote.GetSell().GetPrice(), quote.GetSell().GetQuantity(), quote.GetLastPrice(), quote.GetVolume(),
// 						quote.GetBuy().GetPrice(), quote.GetBuy().GetQuantity(), quote.GetOpenInterest(), pInfo->m_update_time_m, pInfo->m_update_time_s, pInfo->m_millisecond));
				}
				procLen += sizeof(efh3_2_fut_lev1);
			}
			return;
		}
	}
	else if (QUOTE_TYPE::DEPTH == id) {
		if (0 == (len % sizeof(efh3_2_fut_lev2))) {
			unsigned int procLen = 0;
			while (procLen < len) {
				efh3_2_fut_lev2* pInfo = (efh3_2_fut_lev2*)(buff + procLen);
				InstID instID = m_instHelper.GetInstID(pInfo->m_symbol);
				if (m_subscribeList.end() != m_subscribeList.find(instID) && pInfo->m_last_px > 0) {
					Quote quote(OnProcDepthQuote(instID, pInfo));
					m_publisher.PushPrice(quote);
					Log::Info(fmt::format("HFTMarketHandler: notifyMarketData, Instr[{}] Ask2[{:.2f},{}] Last[{:.2f}] Bid2[{:.2f},{}]", instID,
						pInfo->m_ask2_px, pInfo->m_ask2_share, pInfo->m_last_px, pInfo->m_bid2_px, pInfo->m_bid2_share));
				}
				procLen += sizeof(efh3_2_fut_lev2);
			}
			return;
		}
	}
}

void HFTMarketHandler::SubscribeMarket(InstID instID) {
	m_subscribeList.insert(instID);
}

Quote HFTMarketHandler::OnProcSimpleQuote(InstID instID, efh3_2_fut_lev1* pInfo) {
	SimpleQuoteArray buyPriceArray;
	buyPriceArray[0] = SimpleQuote(pInfo->m_bid_px, pInfo->m_bid_share);
	SimpleQuoteArray sellPriceArray;
	sellPriceArray[0] = SimpleQuote(pInfo->m_ask_px, pInfo->m_ask_share);
	int openInterest = int(pInfo->m_total_pos);		// 持仓
	double lastPrice = pInfo->m_last_px;			// 最新
	int volume = pInfo->m_last_share;				// 成交
	money turnover = pInfo->m_total_value;

	Quote quote(instID, &m_instHelper.GetInstrument(instID), buyPriceArray, sellPriceArray, 0, 0, 0, 0, 0,
		openInterest, lastPrice, 0, volume, turnover, false, pInfo->m_update_time_m, SimpleQuote(), SimpleQuote());
	return quote;
}
Quote HFTMarketHandler::OnProcDepthQuote(InstID instID, efh3_2_fut_lev2* pInfo) {
	SimpleQuoteArray buyPriceArray;
	buyPriceArray[0] = SimpleQuote(pInfo->m_bid1_px, pInfo->m_bid1_share);
	buyPriceArray[1] = SimpleQuote(pInfo->m_bid2_px, pInfo->m_bid2_share);
	buyPriceArray[2] = SimpleQuote(pInfo->m_bid3_px, pInfo->m_bid3_share);
	buyPriceArray[3] = SimpleQuote(pInfo->m_bid4_px, pInfo->m_bid4_share);
	buyPriceArray[4] = SimpleQuote(pInfo->m_bid5_px, pInfo->m_bid5_share);
	SimpleQuoteArray sellPriceArray;
	sellPriceArray[0] = SimpleQuote(pInfo->m_ask1_px, pInfo->m_ask1_share);
	sellPriceArray[1] = SimpleQuote(pInfo->m_ask2_px, pInfo->m_ask2_share);
	sellPriceArray[2] = SimpleQuote(pInfo->m_ask3_px, pInfo->m_ask3_share);
	sellPriceArray[3] = SimpleQuote(pInfo->m_ask4_px, pInfo->m_ask4_share);
	sellPriceArray[4] = SimpleQuote(pInfo->m_ask5_px, pInfo->m_ask5_share);
	int openInterest = int(pInfo->m_total_pos);
	double lastPrice = pInfo->m_last_px;
	int volume = pInfo->m_last_share;
	money turnover = pInfo->m_total_value;

	Quote quote(instID, &m_instHelper.GetInstrument(instID), buyPriceArray, sellPriceArray, 0, 0, 0, 0, 0,
		openInterest, lastPrice, 0.0, volume, turnover, false, pInfo->m_update_time_m, SimpleQuote(), SimpleQuote());
	return quote;
}
