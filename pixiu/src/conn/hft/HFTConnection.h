#pragma once
#include <atomic>
#include "utils/Log.h"
#include "config/Config.h"
#include "conn/ExchangeConn.h"
#include "conn/hft/HFTMarketHandler.h"
#include "conn/hft/HFTTraderHandler.h"


// 高频率交易接口
// 行情使用盛立组播
// 交易使用易达Api

// 前置打分
class FrontScorer {
public:
	static constexpr int MaxFronts = 12;	// 前置数量最大值

	FrontScorer(uint32_t frontCnts/*前置数*/, uint32_t batchCnt/*发几批*/)
		:m_frontCnt(frontCnts), m_batchCnt(batchCnt)
		,m_scores(frontCnts, 0), m_batchReceived(batchCnt, 0)
		,m_batchSeen(static_cast<std::size_t>(batchCnt) * frontCnts, 0)
		,m_firstLocalID(0), m_completedBatch(0), m_isScoreCompleted(false) {
	}

	void reset() {
		std::fill(m_scores.begin(), m_scores.end(), 0);
		std::fill(m_batchSeen.begin(), m_batchSeen.end(), 0);
		std::fill(m_batchReceived.begin(), m_batchReceived.end(), 0);

		m_firstLocalID = 0;
		m_completedBatch = 0;
		std::lock_guard<std::mutex> lock(m_doneMutex);
		m_isScoreCompleted = false;
	}
	void setFrontCnt(uint32_t frontCnt) { 
		m_frontCnt = frontCnt;
		m_scores.assign(frontCnt, 0);
		m_batchSeen.assign(static_cast<std::size_t>(m_batchCnt) * frontCnt, 0);
		std::fill(m_batchReceived.begin(), m_batchReceived.end(), 0);
		Log::Info(fmt::format("FrontScorer: SetFrontCnt[{}]! BatchCnts[{}]", m_frontCnt, m_batchCnt));
	}
	void start(uint32_t firstLocalId) { m_firstLocalID = firstLocalId; }

	bool isDone() const { 
		std::lock_guard<std::mutex> lock(m_doneMutex); 
		return m_isScoreCompleted; 
	}
	bool wait_done_for(std::chrono::microseconds timeout) const {
		std::unique_lock<std::mutex> lock(m_doneMutex);
		return m_doneCv.wait_for(lock, timeout, [this] { return m_isScoreCompleted; });
	}

	bool on_Return(uint32_t localID, uint32_t connectionID) {
		{
			std::lock_guard<std::mutex> lock(m_doneMutex);
			if (m_isScoreCompleted) {
				return false;
			}	// 打分已完成
		}	// Lock要独立作用域

		if (localID < m_firstLocalID || connectionID >= m_frontCnt) {
			Log::Info(fmt::format("FrontScorer: Invalid localID[{}] < firstID[{}] or connectionID[{}] >= frontCnts[{}]", localID, m_firstLocalID, connectionID, m_frontCnt));
			return false;
		}	// 委托号或前置号不合法

		const uint32_t  offset = (localID - m_firstLocalID) / 10;	//localID放大了10倍，这里要缩小10倍
		const uint32_t  batchID = offset / m_frontCnt;
		const uint32_t  slot = offset % m_frontCnt;
		if (batchID >= m_batchCnt) {
			Log::Info(fmt::format("FrontScorer: Invalid batchID[{}] offset[{}]", batchID, offset));
			return false;
		}	// 批次号不合法(超出打分批次)，（打分前发了太多批次委托，阻塞模式不会出现）

		const std::size_t seenIndex = static_cast<std::size_t>(batchID) * m_frontCnt + slot;
		if (0 != m_batchSeen[seenIndex]) {
			Log::Info(fmt::format("FrontScorer: Repeated orderRtn, localID[{}] connectionID[{}]", localID, connectionID));
			return false;
		}	// 已对该回报打过分
		m_batchSeen[seenIndex] = 1;

		const uint32_t rank = m_batchReceived[batchID]++;
		const uint32_t points = m_frontCnt - rank;
		m_scores[connectionID] += points;

		if (rank == m_frontCnt - 1) {
			if (++m_completedBatch == m_batchCnt) {
				{
					std::lock_guard<std::mutex> lock(m_doneMutex);
					m_isScoreCompleted = true;
				}	// Lock要独立作用域

				m_doneCv.notify_one();
				Log::Info(fmt::format("FrontScorer: Score done, batchCnt[{}|{}]", m_completedBatch, m_batchCnt));
				return true;
			}
			Log::Info(fmt::format("FrontScorer: Score once, batchCnt[{}|{}]", m_completedBatch, m_batchCnt));
		}

		return false;
	}

	std::vector<uint32_t> select_rankRange(std::size_t firstIndex, std::size_t lastIndex) const {
		if (!isDone() || lastIndex < firstIndex) {
			return {};
		}

		std::vector<uint32_t> candidates;
		candidates.reserve(m_frontCnt);
		for (uint32_t id = 0; id < m_frontCnt; ++id) {
			if (m_scores[id] > 0) {
				candidates.push_back(id);
			}
		}

		std::sort(candidates.begin(), candidates.end(),
			[this](uint32_t lhs, uint32_t rhs) {
				if (m_scores[lhs] != m_scores[rhs]) {
					return m_scores[lhs] > m_scores[rhs];
				}
				return lhs < rhs;
			});

		if (firstIndex >= candidates.size()) {
			return {};
		}

		const std::size_t endIndex = std::min(lastIndex, candidates.size() - 1);
		return std::vector<uint32_t>(candidates.begin() + firstIndex, candidates.begin() + endIndex + 1);
	}

private:
	uint32_t m_frontCnt;			// 前置数量
	const uint32_t m_batchCnt;		// 打分批次数量

	std::vector<uint32_t> m_scores;			// 每个前置的累计得分（回报先来的分数高）
	std::vector<uint8_t> m_batchSeen;		// 检查某个LocalID是否已经处理过（0：未处理；1：已处理)
	std::vector<uint8_t> m_batchReceived;	// 记录每批已经收到并处理的回报数量

	uint32_t m_firstLocalID;
	uint32_t m_completedBatch;
	bool m_isScoreCompleted = false;		 // 由m_doneMutex保护
	mutable std::mutex m_doneMutex;
	mutable std::condition_variable m_doneCv;
};



class ConnPublisher;
class HFTConnection : public ExchangeConn {
public:
	HFTConnection(ConnConfig& config, Instruments const& instruments, ConnPublisher& publisher, bool isMulticast=true/*默认使用组播*/);
	virtual void Init();
	virtual void Release();
	virtual void Login(char const* password) {}
	virtual void ResetOrderSet();
	virtual void InsertOrder(OrderInfo const& order);
	virtual void InsertExOrder(OrderInfo const& order) {}
	virtual int InsertMultiOrders(OrderInfo const* orders[]);
	virtual void CancelOrder(OrderCancel const& action);
	virtual void SubscribeQuote(char const* instrumentCode);
	virtual void UnSubscribeQuote(char const* instrumentCode) {}
	virtual void QueryMblMarket(char const* instrumentCode) {}
	virtual void QueryAccount() {}
	virtual void QueryOrder() {}
	virtual void QueryTrade() {}
	virtual void QueryPosition() {}
	virtual void QueryFund() {}

	virtual void SetVertificateCode(char const* password) { throw std::logic_error("HFT can not set VeritificateCode"); }
	virtual void InsertMarketingOrder(MarketingOrderInfo const& dealing) { throw std::logic_error("HFT not support marketing order"); }
	virtual void CancelMarketingOrder(MarketingOrderCancel const& cancel) { throw std::logic_error("HFT not support marketing order"); }

private:
	void InitTradeApi();		// 易达交易
	void InitQuoteApi();		// 盛立TCP行情
	void InitMulticast();		// 盛立组播行情

private:
	ConnConfig& m_config;
	ExchangeConnInstrumentHelper m_instHelper;
	ConnPublisher& m_publisher;
	bool m_isMulticast = true;

	uint32_t m_frontCnts = 10;		// 上联席位数
	uint32_t m_scoreBatchCnts = 3;	// 评分批次数
	uint32_t m_probeBatchesRemaining = 0;
	uint32_t m_orderCnts = 0;

	FrontScorer m_scorer;
	HFTTraderHandler m_traderHandler;
	HFTMarketHandler m_marketHandler;

	std::vector<uint32_t> m_rankedFront;
	std::size_t m_nextRankIndex = 0;
	bool m_rankedFrontReady = false;

	YDApi* m_pTraderApi;
	EESQuoteApi* m_pMdApi;

	std::string m_apiVersion;
	YDExchange const* m_pExchange;

	// 以下变量用于InsertMultiorders	
	bool m_isOrderReady = false;
	YDInputOrder m_orderSet[16] = {};
	const YDInstrument* m_instruments[16] = {};
};