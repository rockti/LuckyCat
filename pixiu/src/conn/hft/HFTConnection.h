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

	FrontScorer(uint32_t frontCnts/*前置数*/, uint32_t batchCnts/*发几批*/)
		:m_frontCnts(frontCnts), m_batchCnts(batchCnts)
		,m_scores(frontCnts, 0), m_fastestCount(frontCnts, 0)
		,m_batchReceived(batchCnts, 0)
		,m_batchSeen(static_cast<std::size_t>(batchCnts) * frontCnts, 0)
		,m_firstLocalID(0), m_completedBatch(0), m_isScoreCompleted(false) {
		Log::Info(fmt::format("FrontScorer: Ready to work! FrontCnts[{}] BatchCnts[{}]", m_frontCnts, batchCnts));
	}

	void reset() {
		std::fill(m_scores.begin(), m_scores.end(), 0);
		std::fill(m_fastestCount.begin(), m_fastestCount.end(), 0);
		std::fill(m_batchSeen.begin(), m_batchSeen.end(), 0);
		std::fill(m_batchReceived.begin(), m_batchReceived.end(), 0);

		m_firstLocalID = 0;
		m_completedBatch = 0;
		std::lock_guard<std::mutex> lock(m_doneMutex);
		m_isScoreCompleted = false;
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

		if (localID < m_firstLocalID || connectionID >= m_frontCnts) {
			Log::Info(fmt::format("FrontScorer: Invalid localID[{}] < firstID[{}] or connectionID[{}] >= frontCnts[{}]", localID, m_firstLocalID, connectionID, m_frontCnts));
			return false;
		}	// 委托号或前置号不合法

		const uint32_t  offset = (localID - m_firstLocalID) / 10;	//localID放大了10倍，这里要缩小10倍
		const uint32_t  batchID = offset / m_frontCnts;
		const uint32_t  slot = offset % m_frontCnts;
		if (batchID >= m_batchCnts) {
			Log::Info(fmt::format("FrontScorer: Invalid batchID[{}] offset[{}]", batchID, offset));
			return false;
		}	// 批次号不合法(超出打分批次)，（打分前发了太多批次委托，阻塞模式不会出现）

		const std::size_t seenIndex = static_cast<std::size_t>(batchID) * m_frontCnts + slot;
		if (0 != m_batchSeen[seenIndex]) {
			Log::Info(fmt::format("FrontScorer: Repeated orderRtn, localID[{}] connectionID[{}]", localID, connectionID));
			return false;
		}	// 已对该回报打过分
		m_batchSeen[seenIndex] = 1;

		const uint32_t rank = m_batchReceived[batchID]++;
		if (rank == 0) {
			m_scores[connectionID] += 2;
            ++m_fastestCount[connectionID];
		}	// 先来的回报得2分
		else if (rank < m_frontCnts - 1) {
			++m_scores[connectionID];
		}	// 非最后的回报得1分

		if (rank == m_frontCnts - 1) {
			if (++m_completedBatch == m_batchCnts) {
				{
					std::lock_guard<std::mutex> lock(m_doneMutex);
					m_isScoreCompleted = true;
				}	// Lock要独立作用域

				m_doneCv.notify_one();
				Log::Info(fmt::format("FrontScorer: Score done, batchCnt[{}|{}]", m_completedBatch, m_batchCnts));
				return true;
			}
			Log::Info(fmt::format("FrontScorer: Score once, batchCnt[{}|{}]", m_completedBatch, m_batchCnts));
		}	// 最后一个回报不得分

		return false;
	}

	std::vector<uint32_t> select_top(uint32_t rank/*取前几*/) const {
		if (!isDone()) {
			return {};
		}

		std::array<uint32_t, MaxFronts> candidates{};
		uint32_t candidateCount = 0;

		for (uint32_t id = 0; id < m_frontCnts; ++id) {
			const auto index = id;
			if (m_scores[index] > 1 && m_fastestCount[index] > 0) {
				candidates[candidateCount++] = static_cast<int>(id);
			}
		}	// 剔除无效前置

		if (0 == candidateCount) {
			return {};
		}

		std::sort(candidates.begin(), candidates.begin() + candidateCount, [this](int lhs, int rhs) {
				const auto left = static_cast<std::size_t>(lhs);
				const auto right = static_cast<std::size_t>(rhs);
				if (m_scores[left] != m_scores[right]) {
					return m_scores[left] > m_scores[right];
				}
				return lhs < rhs;
			});


		uint32_t keep = candidateCount;
		if (candidateCount > rank) {
			const uint32_t cutoff_score = m_scores[static_cast<std::size_t>(candidates[rank - 1])];
			keep = rank;
			while (keep < candidateCount && m_scores[static_cast<std::size_t>(candidates[keep])] == cutoff_score) {
				++keep;
			}
		}
		Log::Info(fmt::format("FrontScorer: FrontCnt[{}] FastestFront is [{}]", m_frontCnts, candidates[0]));
		return std::vector<uint32_t>(candidates.begin(), candidates.begin() + keep);
	}

private:
	const uint32_t m_frontCnts;		// 前置数量（同没批委托数量）
	const uint32_t m_batchCnts;		// 打分批次数量

	std::vector<uint32_t> m_scores;			// 每个前置的累计得分（最快2分；最慢0分；其余1分）
	std::vector<uint32_t> m_fastestCount;	// 每个前置得第一名的次数（下标是前置号）

	std::vector<uint8_t> m_batchSeen;		// 检查某个LocalID是否已经处理过（0：未处理；1：已处理)
	std::vector<uint32_t> m_batchReceived;	// 记录每批已经收到并处理的回报数量

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

	uint32_t m_frontCnts = 6;	// 席位数：要与TimerCallback里的一样
	uint32_t m_batchCnts = 3;	// 批次数：前置打分所需要预发的批次数
	uint32_t m_orderCnts = 0;
	std::vector<uint32_t> m_selectedFront{};	// 筛选过的前置
	FrontScorer m_scorer;
	HFTTraderHandler m_traderHandler;
	HFTMarketHandler m_marketHandler;

	YDApi* m_pTraderApi;
	EESQuoteApi* m_pMdApi;

	std::string m_apiVersion;
	YDExchange const* m_pExchange;

	// 以下变量用于InsertMultiorders
	
	bool m_isOrderReady = false;
	YDInputOrder m_orderSet[16] = {};
	const YDInstrument* m_instruments[16] = {};
};