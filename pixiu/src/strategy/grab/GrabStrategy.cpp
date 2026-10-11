#include "pch.h"
#include "utils/Log.h"
#include "GrabStrategy.h"
#include "runner/Queues.h"
#include "model/factory/InstrumentFactory.h"

extern std::atomic_bool g_breakTag;
using BusinessSnapshot = tg::grab::BusinessSnapshot;

template<>
inline void StrategyPublisher::NotifyUIBusinessSnapshot<BusinessSnapshot>(BusinessSnapshot const& update) {
	Data data;
	data.Type = DataType::BusinessUpdate;
	data.Size = sizeof(BusinessSnapshot);
	memcpy(data.Data, &update, data.Size);
	m_queues.NotifyUI(data);
}

namespace tg::grab {
	GrabStrategy::GrabStrategy(UIUpdater* updater, Arguments const& arguments, StrategyPublisher& publisher, DealingRepository& dealings)
		: Strategy<Arguments>(updater, arguments, publisher, dealings, arguments.Order.Inst, arguments.ExOrder.Inst) {
		m_snapshot.ID = this->m_id;
		m_snapshot.Status = BusinessStatus::New;
		m_snapshot.UIUpdaterPtr = this->m_updater;
	}

	GrabStrategy::~GrabStrategy() {
		if (nullptr != m_pBusiness) {
			delete m_pBusiness;
		}
		if (nullptr != m_pExBusiness) {
			delete m_pExBusiness;
		}
	}

	void GrabStrategy::DoOnInit() {
		m_publisher.SubscribeQuote(m_arg.Order.Inst);
		if (!IsInvalidInstID(m_arg.ExOrder.Inst)) {
			m_publisher.SubscribeQuote(m_arg.ExOrder.Inst);
		}
	}
	void GrabStrategy::DoOnQuote(Quote const& quote) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (nullptr != m_pBusiness && (SGE_OC_Delivery == m_orderInfo.ProductClass || SGE_OC_Neutral == m_orderInfo.ProductClass)) {
			return;
		}	// SGE目前不解析交割行情，因此不会触发此分支
		if (nullptr != m_pBusiness && quote.GetInstID() == m_orderInfo.InstID) {
			m_pBusiness->OnQuote();
		}
		if (nullptr != m_pExBusiness && quote.GetInstID() == m_exOrderInfo.InstID) {
			m_pExBusiness->OnQuote();
		}
		
		if (g_breakTag.load(std::memory_order_relaxed)) {
			CheckStrategyStatus();
		}
	}
	void GrabStrategy::DoOnCommand(StrategyCommand const& command) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		switch (command.Type) {
		case StrategyCommand::UPDATE_ARGUMENT: {
			m_arg = *(Arguments*)command.Data;
			if (BusinessStatus::Running == m_status/*Timer开启*/ && false == g_breakTag.load(std::memory_order_relaxed)/*还未发单*/ && false == m_isNeturalTag/*非中立仓*/) {
				Start();	// 如果Timer已经开启，修改参数会重置Timer，中立仓除外
			}
		}
		break;
		case StrategyCommand::START:
			Start();
		break;
		case StrategyCommand::PAUSE:
			Pause();
		break;
		default:
			break;
		}
		m_snapshot.Status = m_status;
	}
	void GrabStrategy::DoOnOrdered(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (SGE_OC_Delivery == dealing.GetProductClass() || SGE_OC_Neutral == dealing.GetProductClass()) {
			if (m_orderInfo.InstID == dealing.GetInstID() && dealing.IsOpenOrder()/*只处理开仓回报*/) {
				m_pBusiness->OnActiveOrder(dealing);
			}
			else if (m_exOrderInfo.InstID == dealing.GetInstID() && dealing.IsOpenOrder()/*只处理开仓回报*/) {
				m_pExBusiness->OnActiveOrder(dealing);
			}
			Log::Info(fmt::format("GrabStrategy: DoOnOrdered, Inst[{}] [{}: {:.3f},{} {}] LocalID[{}] SystemID[{}]", dealing.GetInstCode().Str(),
				Desc(dealing.GetDirection()), dealing.GetOrderPrice(), dealing.GetOrderQuantity(), dealing.GetProductClass(), dealing.GetLocalId(), dealing.GetSystemNo().Str()));
		}	// 只处理交割和中立仓的委托回报
// 		Log::Info(fmt::format("GrabStrategy: DoOnOrdered, Inst[{}] [{}: {:.3f},{} {}] LocalID[{}] SystemID[{}]", dealing.GetInstCode().Str(),
// 			Desc(dealing.GetDirection()), dealing.GetOrderPrice(), dealing.GetOrderQuantity(), dealing.GetProductClass(), dealing.GetLocalId(), dealing.GetSystemNo().Str()));
	}
	void GrabStrategy::DoOnCancelled(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (Operation::OPEN == dealing.GetOperation()) {
			if (dealing.GetInstID() == m_orderInfo.InstID) {
				m_pBusiness->OnActiveCancel(dealing.GetCancelQuantity());
			}
			else if (dealing.GetInstID() == m_exOrderInfo.InstID) {
				m_pExBusiness->OnActiveCancel(dealing.GetCancelQuantity());
			}
		}
		else {
			GrabBusiness* businessPtr = (GrabBusiness*)dealing.GetHookPtr();
			if (nullptr != businessPtr) {
				businessPtr->OnPassiveCancel(dealing.GetCancelQuantity());
			}	// 平仓的dealing是绑定到具体business上的，取Hook即可
		}

		CheckStrategyStatus();
	}
	void GrabStrategy::DoOnQuantityTrade(Dealing const& dealing, int quantity) {
		// 非SGE的成交在这里处理
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		auto instID = dealing.GetInstID();
		int sign = (Operation::OPEN == dealing.GetOperation()) ? 1 : -1;	// 开仓增加平仓减少
		if (m_arg.Order.Inst == instID) {
			m_snapshot.Position1 += sign * quantity;
		}
		else {
			m_snapshot.Position2 += sign * quantity;
		}

		if (Operation::OPEN == dealing.GetOperation()) {
			if (instID == m_orderInfo.InstID) {
				m_pBusiness->OnActiveTrade(quantity);
			}
			else if (instID == m_exOrderInfo.InstID) {
				m_pExBusiness->OnActiveTrade(quantity);
			}
		}
		else {
			GrabBusiness* businessPtr = (GrabBusiness*)dealing.GetHookPtr();
			if (nullptr != businessPtr) {
				businessPtr->OnPassiveTrade(quantity);
			}	// 平仓的dealing是绑定到具体business上的，取Hook即可
		}
		CheckStrategyStatus();
	}
	void GrabStrategy::DoOnTrade(Trade const& trade) {
		// SGE的成交在这里处理
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		Dealing const& dealing = trade.GetDealing();
		auto instID = dealing.GetInstID();
		int sign = (Operation::OPEN == dealing.GetOperation()) ? 1 : -1;	// 开仓增加平仓减少
		if (m_arg.Order.Inst == instID) {
			m_snapshot.Position1 += sign * trade.GetDealQuantity();
		}
		else {
			m_snapshot.Position2 += sign * trade.GetDealQuantity();
		}

		if (Operation::OPEN == dealing.GetOperation()) {
			if (instID == m_orderInfo.InstID) {
				m_pBusiness->OnActiveTrade(trade.GetDealQuantity());
			}
			else if (instID == m_exOrderInfo.InstID) {
				m_pExBusiness->OnActiveTrade(trade.GetDealQuantity());
			}
		}
		else {
			GrabBusiness* businessPtr = (GrabBusiness*)dealing.GetHookPtr();
			if (nullptr != businessPtr) {
				businessPtr->OnPassiveTrade(trade.GetDealQuantity());
			}	// 平仓的dealing是绑定到具体business上的，取Hook即可
		}
		CheckStrategyStatus();
	}
	void GrabStrategy::DoOnOther(Dealing const& dealing) {
		GrabBusiness* businessPtr = (GrabBusiness*)dealing.GetHookPtr();
		if (nullptr != businessPtr) {
			Log::Error(fmt::format("GrabStrategy: DoOnOther, Inst[{}] [{} {}：{:.3f},{}] localID[{}]", dealing.GetInstCode().Str()
				, Desc(dealing.GetDirection()), Desc(dealing.GetOperation()), dealing.GetOrderPrice(), dealing.GetOrderQuantity(), dealing.GetLocalId()));

			if (Operation::OPEN != dealing.GetOperation()) {
				businessPtr->m_passiveSent -= dealing.GetOrderQuantity();	// 平仓委托遭到流控，等下个行情重发
			}	// 平仓的dealing是绑定到具体business上的，取Hook即可
		}
	}
	void GrabStrategy::DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		auto& instrument = InstrumentFactory::GetInstruments().Get(m_arg.Order.Inst);
		if (BusinessStatus::Running == m_status && instrument.GetExchange() == exchange && ExchangeStatus::CONTINOUS == status) {
			if (m_pBusiness) {
				if (IsCFFEX(m_orderInfo.InstID)) {
					m_orderInfo.Quantity += m_orderInfo.Quantity;		// 委托量翻倍
					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 50);		// 中金所最大委托量不得超过50
				}
// 				else if (IsSGE(m_orderInfo.InstID) && 0 == strcmp("Au(T+D)", m_orderInfo.InstCode.Str())) {
// 					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 200);		// Au(T+D)最大委托量不得超过200
// 				}
// 				else if (IsSGE(m_orderInfo.InstID) && 0 == strcmp("mAu(T+D)", m_orderInfo.InstCode.Str())) {
// 					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 2000);	// mAu(T+D)最大委托量不得超过200
// 				}
// 				else if (IsSGE(m_orderInfo.InstID) && 0 == strcmp("Ag(T+D)", m_orderInfo.InstCode.Str())) {
// 					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 10000);	// Ag(T+D)最大委托量不得超过200
// 				}		// 除CFFEX外开盘委托量不再翻倍，所以手数限制可以去掉
				m_orderInfo.LocalId = LocalIdACC::NextNormal();
				Dealing& dealing = Strategy::PushOrder(m_orderInfo);
				Log::Info(fmt::format("Exchange[{}] Status[{}], send open order at once: Inst[{}] [{} {} {:.3f},{} {}] LocalID[{}]", Desc(exchange), Desc(status), m_orderInfo.InstCode.Str()
					, Desc(m_orderInfo.Direction), Desc(m_orderInfo.Operation), m_orderInfo.Price, m_orderInfo.Quantity, m_orderInfo.ProductClass, m_orderInfo.LocalId));
			}	// 只有start后才会有pBusiness，判断一下保证已经启动策略，防止instID为0（初始值）
			if (m_pExBusiness) {
				if (IsCFFEX(m_orderInfo.InstID)) {
					m_exOrderInfo.Quantity += m_exOrderInfo.Quantity;	// 委托量翻倍
					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 50);		// 中金所最大委托量不得超过50
				}
// 				else if (IsSGE(m_orderInfo.InstID) && 0 == strcmp("Au(T+D)", m_orderInfo.InstCode.Str())) {
// 					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 200);		// Au(T+D)最大委托量不得超过200
// 				}
// 				else if (IsSGE(m_orderInfo.InstID) && 0 == strcmp("mAu(T+D)", m_orderInfo.InstCode.Str())) {
// 					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 2000);	// mAu(T+D)最大委托量不得超过200
// 				}
// 				else if (IsSGE(m_orderInfo.InstID) && 0 == strcmp("Ag(T+D)", m_orderInfo.InstCode.Str())) {
// 					m_orderInfo.Quantity = std::min(m_orderInfo.Quantity, 10000);	// Ag(T+D)最大委托量不得超过200
// 				}		// 除CFFEX外开盘委托量不再翻倍，所以手数限制可以去掉
				m_exOrderInfo.LocalId = LocalIdACC::NextNormal();
				Dealing& dealing = Strategy::PushOrder(m_exOrderInfo);
				Log::Info(fmt::format("Exchange[{}] Status[{}], send exOpen order at once: Inst[{}] [{} {} {:.3f},{} {}] LocalID[{}]", Desc(exchange), Desc(status), m_exOrderInfo.InstCode.Str()
					, Desc(m_exOrderInfo.Direction), Desc(m_exOrderInfo.Operation), m_exOrderInfo.Price, m_exOrderInfo.Quantity, m_exOrderInfo.ProductClass, m_exOrderInfo.LocalId));
			}
			g_breakTag.store(true, std::memory_order_relaxed);
// 			GetPublisher().PushTimerOrder(TimerCommand{});	// 发个空的TimerCommand删除Timer	//这里不能删除timer，可能队列里的委托还没发干净，删后会导致空指针 20250930
			return;
		}	// 提前开盘，先收到开盘信号的话，暂停Timer并直接发1次单
		else if (BusinessStatus::Running == m_status && instrument.GetExchange() == Exchange::SGE && ExchangeStatus::CLOSED == status && false == m_isFirstCloseTag) {
			SYSTEMTIME nowTime;
			::GetLocalTime(&nowTime);
			m_neturalBase = nowTime.wSecond * 1000 + nowTime.wMilliseconds;
			m_isFirstCloseTag = true;
		}	// 第一次收到收盘信号的时间（SGE接双交易员，会收到两个交易所信号）
		else if (BusinessStatus::Running == m_status && instrument.GetExchange() == Exchange::SGE && ExchangeStatus::DELIVERY_END == status && m_isNeturalTag) {
			MatchStart();
			Log::Info(fmt::format("Exchange[{}] Status[{}] MatchStart", Desc(exchange), Desc(status)));
		}	// 收盘信号后1分钟，开始中立仓申报
		else if (BusinessStatus::Running == m_status && instrument.GetExchange() == Exchange::SGE && ExchangeStatus::DELIVERY_APPEND == status && false == m_isNeturalTag) {
			m_orderInfo.LocalId = LocalIdACC::NextNormal();
			m_orderInfo.ProductClass = SGE_OC_Delivery;
			Dealing& dealing = Strategy::PushOrder(m_orderInfo);
			Log::Info(fmt::format("Exchange[{}] Status[{}], send delivery order at once: Inst[{}] [{} {} {:.3f},{} {}] LocalID[{}]", Desc(exchange), Desc(status), m_orderInfo.InstCode.Str()
				, Desc(m_orderInfo.Direction), Desc(m_orderInfo.Operation), m_orderInfo.Price, m_orderInfo.Quantity, m_orderInfo.ProductClass, m_orderInfo.LocalId));
			g_breakTag.store(true, std::memory_order_relaxed);
		}	// 收到交割申报信号,且非中立仓
	}

	int const MAX_PERIOD = 1000 * 1000;
	void GrabStrategy::Pause() {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		g_breakTag.store(true, std::memory_order_relaxed);
		GetPublisher().PushTimerOrder(TimerCommand{});	// 发个空的TimerCommand删除Timer
		m_isNeturalTag = false;

		if (m_pBusiness) {
			m_pBusiness->OnPause();
		}
		if (m_pExBusiness) {
			m_pExBusiness->OnPause();
		}
		SetStop();
	}
	void GrabStrategy::Start() {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (GetGrabInst() != m_arg.Order.Inst) {
			auto& argumentInst = InstrumentFactory::GetInstruments().Get(m_arg.Order.Inst);
			auto& businessInst = InstrumentFactory::GetInstruments().Get(GetGrabInst());
			auto msg = fmt::format("Start failed, because BusinessInst[{}] is different from ArgumentInst[{}]"
				, businessInst.GetInstCode().Str(), argumentInst.GetInstCode().Str());
			Log::Info(msg);
			m_publisher.NotifyUI(msg);
			return;
		}

		if (m_arg.StartTime.wHour > 14 && m_arg.StartTime.wHour < 16 && m_arg.StartTime.wMinute > 15/*超过15:15:00交割申报时间*/ && false == m_isNeturalTag) {
			auto msg = fmt::format("Set Netural Timer: [{}:{}:{}]", m_arg.StartTime.wMinute, m_arg.StartTime.wSecond, m_arg.StartTime.wMilliseconds);
			Log::Info(msg);
			m_publisher.NotifyUI(msg);
			m_isNeturalTag = true;
			m_status = BusinessStatus::Running;
			return;
		}

		SYSTEMTIME nowTime;
		::GetLocalTime(&nowTime);
		auto nowMS = 1000 * (nowTime.wHour * 3600 + nowTime.wMinute * 60 + nowTime.wSecond) + nowTime.wMilliseconds;	// 当前毫秒数
		SYSTEMTIME dstTime = m_arg.StartTime;
		auto dstMS = 1000 * (dstTime.wHour * 3600 + dstTime.wMinute * 60 + dstTime.wSecond) + dstTime.wMilliseconds;	// 目标毫秒数
		uint32_t delay = dstMS - nowMS;
		if (delay > MAX_PERIOD) {	// 超过1000秒的Timer不可启动
			auto retryDelay = (delay - MAX_PERIOD) / 1000;
			auto msg = fmt::format("Timer start failed[{}:{:02d}:{}], because delay[{}] is over [1000]s, please retry after [{}]s"
				, nowTime.wMinute, nowTime.wSecond, nowTime.wMilliseconds, delay / 1000, retryDelay);
			Log::Info(msg);
			m_publisher.NotifyUI(msg);
			g_breakTag.store(true, std::memory_order_relaxed);
			SetStop();				// 无法开启状态机
			return;
		}

		TimerCommand command;
		if (m_arg.AwakeCnt <= 0) {
			command.TimerPtr = TriggerTimer::CreatePeriodTimer(delay, m_arg.Period);
		}
		else {
			command.TimerPtr = TriggerTimer::CreateAwakeTimer(delay, m_arg.Period, m_arg.ExPeriod/*唤醒间隔*/);
		}
		command.TimerPtr->InitCounters(m_arg.TotalCnt/*触发次数*/, m_arg.AwakeCnt/*唤醒次数*/);
		command.BusinessPtr = this;
	
		if (!IsSGE(m_arg.Order.Inst)) {
			LocalIdACC::InitGrab();
		}	// 非SGE品种初始化抢单本地委托号
		if (m_arg.StartTime.wHour > 14 && m_arg.StartTime.wHour < 16) {
			m_arg.Order.ProductClass = SGE_OC_Delivery;
			m_arg.ExOrder.ProductClass = SGE_OC_Delivery;
		}	// 超过14点的是交割申报单
		m_orderInfo = CreateOrderInfo(m_arg.Order);
		command.pInfo = &m_orderInfo;
		if (nullptr != m_pBusiness) {
			delete m_pBusiness;
		}
		m_pBusiness = new GrabBusiness{ *this, m_arg.Order.Inst, m_arg.Order.OpenQty, m_arg.Order.HoldQty, m_arg.Order.OffsetPrice, m_arg.Order.StopPrice };

		if (!IsInvalidInstID(m_arg.ExOrder.Inst) && m_arg.ExOrder.OpenQty > 0) {
			m_exOrderInfo = CreateOrderInfo(m_arg.ExOrder);
			command.pExInfo = &m_exOrderInfo;
			if (nullptr != m_pExBusiness) {
				delete m_pExBusiness;
			}
			m_pExBusiness = new GrabBusiness{ *this, m_arg.ExOrder.Inst, m_arg.ExOrder.OpenQty, m_arg.ExOrder.HoldQty, m_arg.ExOrder.OffsetPrice, m_arg.ExOrder.StopPrice };
		}

		g_breakTag.store(false, std::memory_order_relaxed);
		GetPublisher().PushTimerOrder(command);			// 发送Timer给connRunner
		m_status = BusinessStatus::Running;				// 开启状态机		
	}
	void GrabStrategy::MatchStart() {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		SYSTEMTIME nowTime;
		::GetLocalTime(&nowTime);
		int timeInterval = nowTime.wSecond * 1000 + nowTime.wMilliseconds/*交割结束*/ - m_neturalBase/*首次收盘信号*/;
		if (timeInterval < 0) {
			timeInterval = 0;
		}
		if (0 == m_neturalBase) {
			timeInterval = 160;
		}	// 15:30之后设置，没有取到首次收盘毫秒数
		uint32_t delay = 60000/*延迟60秒*/ - timeInterval - m_arg.StartTime.wMilliseconds/*提前毫秒数*/;
		uint32_t priorMS = timeInterval + m_arg.StartTime.wMilliseconds;
		auto msg = fmt::format("Start Netural Timer: deviation[{}] prior[{}]ms", timeInterval, priorMS);
		Log::Info(msg);
		m_publisher.NotifyUI(msg);

		TimerCommand command;
		if (m_arg.AwakeCnt <= 0) {
			command.TimerPtr = TriggerTimer::CreatePeriodTimer(delay, m_arg.Period);
		}
		else {
			command.TimerPtr = TriggerTimer::CreateAwakeTimer(delay, m_arg.Period, m_arg.ExPeriod/*唤醒间隔*/);
		}
		command.TimerPtr->InitCounters(m_arg.TotalCnt/*触发次数*/, m_arg.AwakeCnt/*唤醒次数*/);
		command.BusinessPtr = this;
		m_arg.Order.ProductClass = SGE_OC_Neutral;
		m_orderInfo = CreateOrderInfo(m_arg.Order);
		command.pInfo = &m_orderInfo;
		if (nullptr != m_pBusiness) {
			delete m_pBusiness;
		}
		m_pBusiness = new GrabBusiness{ *this, m_arg.Order.Inst, m_arg.Order.OpenQty, m_arg.Order.HoldQty, m_arg.Order.OffsetPrice, m_arg.Order.StopPrice };

		if (!IsInvalidInstID(m_arg.ExOrder.Inst) && m_arg.ExOrder.OpenQty > 0) {
			m_arg.ExOrder.ProductClass = SGE_OC_Neutral;
			m_exOrderInfo = CreateOrderInfo(m_arg.ExOrder);
			command.pExInfo = &m_exOrderInfo;
			if (nullptr != m_pExBusiness) {
				delete m_pExBusiness;
			}
			m_pExBusiness = new GrabBusiness{ *this, m_arg.ExOrder.Inst, m_arg.ExOrder.OpenQty, m_arg.ExOrder.HoldQty, m_arg.ExOrder.OffsetPrice, m_arg.ExOrder.StopPrice };
		}

		g_breakTag.store(false, std::memory_order_relaxed);
		GetPublisher().PushTimerOrder(command);			// 发送Timer给connRunner
		m_status = BusinessStatus::Running;				// 开启状态机
	}
	void GrabStrategy::CheckStrategyStatus() {
		if (nullptr == m_pBusiness) {
			return;
		}
		if (nullptr == m_pExBusiness && true == m_pBusiness->m_endTag && BusinessStatus::Paused != m_status) {
			SetStop();
			Log::Info(fmt::format("GrabStrategy SetStop, entTag[{}]", m_pBusiness->m_endTag));
		}
		else if (nullptr != m_pExBusiness && true == m_pBusiness->m_endTag && true == m_pExBusiness->m_endTag && BusinessStatus::Paused != m_status) {
			SetStop();
			Log::Info(fmt::format("GrabStrategy SetStop, entTag[{}][{}]", m_pBusiness->m_endTag, m_pExBusiness->m_endTag));
		}
		m_snapshot.Status = m_status;
	}

	Dealing& GrabStrategy::PushPassiveOrderWithHook(OrderInfo const& orderInfo, GrabBusiness* businessPtr) {
		Dealing& dealing = Strategy::PushOrder(orderInfo);
		dealing.SetHookPtr(businessPtr);
		return dealing;
	}
	OrderInfo GrabStrategy::CreateOrderInfo(GBOrder const& order) {
		auto& instrument = InstrumentFactory::GetInstruments().Get(order.Inst);
		OrderInfo orderInfo;
		orderInfo.InstCode = instrument.GetInstCode();
		orderInfo.Exchange = instrument.GetExchange();
		orderInfo.InstID = order.Inst;
		orderInfo.Direction = (order.OpenPrice > order.StopPrice) ? Direction::BUY : Direction::SELL;
		orderInfo.Operation = Operation::OPEN;
		orderInfo.Quantity = order.OpenQty;
		orderInfo.Price = order.OpenPrice;
		orderInfo.OrderType = OrderType::FAK;
		orderInfo.LocalId = LocalIdACC::NextNormal();
		orderInfo.ProductClass = order.ProductClass;
		if (SGE_OC_Delivery == orderInfo.ProductClass || SGE_OC_Neutral == orderInfo.ProductClass) {
			orderInfo.Price = money();
			orderInfo.OrderType = OrderType::NORMAL;
		}	// 交割或者中立仓的价格是0,委托类型是Normal

		Log::Info(fmt::format("!!NewGrabOrder: Inst[{}] [{} {} {:.3f},{} {}] LocalID[{}] Offset[{:.3f}] Hold[{}] Stop[{:.3f}] Time[{}:{:02d}:{}]"
			, orderInfo.InstCode.Str(), Desc(orderInfo.Direction), Desc(orderInfo.Operation), orderInfo.Price, orderInfo.Quantity, orderInfo.ProductClass
			, orderInfo.LocalId, order.OffsetPrice, order.HoldQty, order.StopPrice, m_arg.StartTime.wMinute, m_arg.StartTime.wSecond, m_arg.StartTime.wMilliseconds));
		return orderInfo;
	}
}
