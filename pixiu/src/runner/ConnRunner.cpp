#include "pch.h"

#include "runner/ConnRunner.h"
#include "conn/ExchangeConn.h"
#include "notify/Notify.h"
#include "model/dealing/ExchangeRequest.h"
#include "model/dealing/MarketingDealing.h"
#include "model/Timer.h"

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

std::atomic_bool g_breakTag(false);
static VOID CALLBACK TimerCallbackFunc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
	TimerCallback* parameter = (TimerCallback*)dwUser;
	TriggerTimer* timerPtr = parameter->GetTimerPtr();

	if (g_breakTag.load(std::memory_order_relaxed)) {
		parameter->RestetOrderSet();
		return;
	}
	
	Log::Info(fmt::format("ConnRunnerHandler: callback start, TotalCnts[{}] WameupCnts[{}]", timerPtr->MaxLoopCnts, timerPtr->AwakeCnts));
	if (TriggerTimer::DELAY == timerPtr->GetType()) {
		parameter->Callback();
	}
	else {

		LARGE_INTEGER frequency{};
		QueryPerformanceFrequency(&frequency);
		uint32_t completedRampSteps = 0;

		const uint32_t rampSteps = timerPtr->AwakeCnts;
		const uint32_t awakePeriodUs = timerPtr->GetAwakePeriod();
		const uint32_t targetPeriodUs = timerPtr->GetTargetPeriod();
		const double startHz = 1'000'000.0 / awakePeriodUs;	// 起始频率
		const double maxHz = 1'000'000.0 / targetPeriodUs;	// 最高频率

		LARGE_INTEGER end{};
		LARGE_INTEGER begin{};
		QueryPerformanceCounter(&begin);
		while (timerPtr->MaxLoopCnts > 0 && false == g_breakTag.load(std::memory_order_relaxed)) {

			uint32_t periodUs = targetPeriodUs;
			if (rampSteps > 1 && completedRampSteps < rampSteps) {
				const double ratio = static_cast<double>(completedRampSteps) / (rampSteps - 1);
				const double currentHz = startHz + (maxHz - startHz) * ratio;			// 频率递增
				periodUs = static_cast<uint32_t>(std::ceil(1'000'000.0 / currentHz));	// 向上取整
				if (completedRampSteps == rampSteps - 1) {
					periodUs = targetPeriodUs;
				}	// 到最高频
			}
			else if (rampSteps == 1 && completedRampSteps == 0) {
				periodUs = targetPeriodUs;
			}	// 直接最高频
			const int64_t periodTicks = (static_cast<int64_t>(periodUs) * frequency.QuadPart + 999'999) / 1'000'000;
					
			while (false == g_breakTag.load(std::memory_order_relaxed)) {
				QueryPerformanceCounter(&end);
				if (end.QuadPart - begin.QuadPart >= periodTicks) {
					break;
				}
			}	// 计时循环
			parameter->Callback();
			Log::Info(fmt::format("TimerCallbackFunc: callback, periodTicks[{}]", periodTicks));

			if (completedRampSteps < rampSteps) {
				++completedRampSteps;
			}
			QueryPerformanceCounter(&begin);
		}
		g_breakTag.store(true, std::memory_order_relaxed);	// MaxLoopCnts
		Log::Info(fmt::format("ConnRunnerHandler: callback stop, remainCnts[{}]", timerPtr->MaxLoopCnts));
	}
	parameter->RestetOrderSet();
}

void TimerCallback::Callback() {
	if (IsSGE(m_orderInfo[0]->Exchange)) {	// 尝试连续发4单看看结果会如何
		m_connection->InsertOrder(*m_orderInfo[0]);			// 主席位发单
		m_connection->InsertOrder(*m_orderInfo[0]);			// 主席位发单
		m_connection->InsertExOrder(*m_orderInfo[0]);		// 次席位发单
		m_connection->InsertOrder(*m_orderInfo[0]);			// 主席位发单
		m_pTimer->MaxLoopCnts -= 3;
		if (nullptr != m_orderInfo[1]) {
			m_connection->InsertOrder(*m_orderInfo[1]);
			m_connection->InsertExOrder(*m_orderInfo[1]);
			m_pTimer->MaxLoopCnts -= 2;
		}
	}	// SGE是重复委托号，不更新LocalID
	else {
// 		--m_pTimer->AwakeCnts;
// 		m_connection->InsertOrder(*m_orderInfo[0]);
// 		m_orderInfo[0]->LocalId = LocalIdACC::NextNormal();	// 更新下次的委托号
// 		--m_pTimer->MaxLoopCnts;
// 		if (nullptr != m_orderInfo[1]) {
// 			m_connection->InsertOrder(*m_orderInfo[1]);
// 			m_orderInfo[1]->LocalId = LocalIdACC::NextNormal();
// 			--m_pTimer->MaxLoopCnts;
// 		}

		// 以下为易达批量发单功能代码（目前上海10席位, 中金5席位）
		unsigned int& batchCnt = IsSHFE(m_orderInfo[0]->Exchange) ? m_shfeCnt : m_cffexCnt;
		m_connection->InsertMultiOrders(batchCnt, m_orderInfo);
		m_pTimer->MaxLoopCnts -= batchCnt;

		if (IsSHFE(m_orderInfo[0]->Exchange)) {
			if (batchCnt < 10) {
				batchCnt += 1;
			}
			else {
				batchCnt = 10;
			}
		}
		else {
			if (m_increaseFreqTag) {
				if (batchCnt < 5) {
					batchCnt += 1;
				}
				else {
					batchCnt = 5;
				}
			}
			m_increaseFreqTag = !m_increaseFreqTag;
		}
	}
}

void TimerCallback::RestetOrderSet() {
	m_connection->ResetOrderSet();
}

void ConnRunnerHandler::Handle(Data const& data) {
	switch (data.Type) {
	case DataType::Order: {
		OrderInfo const& orderInfo = *(OrderInfo*)data.Data;
		auto& conn = *m_conns[ConnIndex(orderInfo.Exchange)];
		conn.InsertOrder(orderInfo);
		break;
	}
	case DataType::Cancel: {
		OrderCancel const& action = *(OrderCancel*)data.Data;
		auto& conn = *m_conns[ConnIndex(action.GetExchange())];
		conn.CancelOrder(action);
		break;
	}
	case DataType::MarketingQuote: {
		MarketingOrderInfo const& order = *(MarketingOrderInfo*)data.Data;
		auto& conn = *m_conns[ConnIndex(order.Exchange)];
		conn.InsertMarketingOrder(order);
		break;
	}
	case DataType::MarketingCancel: {
		MarketingOrderCancel const& cancel = *(MarketingOrderCancel*)data.Data;
		auto& conn = *m_conns[ConnIndex(cancel.Exchange)];
		conn.CancelMarketingOrder(cancel);
		break;
	}
	case DataType::ConnCommand: {
		ConnCommand const& command = *(ConnCommand*)data.Data;
		onCommand(command);
		break;
	}
	case DataType::TimerCommand: {
		TimerCommand const& command = *(TimerCommand*)data.Data;
		onCommand(command);
		break;
	}
	default:
		throw ILLEGAL_MESSAGE_TYPE;
	}
}

void ConnRunnerHandler::onCommand(ConnCommand const& command) {
	switch (command.Type) {
	case ConnCommand::LOGIN: {
		m_conns[PlatformIndex(command.Platform)]->Login(command.Data);
		break;
	}
	case ConnCommand::LOGOUT: {
		m_conns[PlatformIndex(command.Platform)]->Release();
		break;
	}
	case ConnCommand::SET_CODE: {		// 易盛二次认证
		m_conns[PlatformIndex(command.Platform)]->SetVertificateCode(command.Data);
		break;
	}
	case ConnCommand::MARKET_SUBSCRIBE: {
		char const* instrumentCode = command.Data;
		m_conns[PlatformIndex(command.Platform)]->SubscribeQuote(instrumentCode);
		break;
	}
	case ConnCommand::FUND_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryFund();
		break;
	}
	case ConnCommand::POSITION_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryPosition();
		break;
	}
	case ConnCommand::ORDER_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryOrder();
		break;
	}
	case ConnCommand::TRADE_QRY: {
		m_conns[PlatformIndex(command.Platform)]->QueryTrade();
		break;
	}
	default:
		throw ILLEGAL_MESSAGE_TYPE;
		break;
	}
}

void ConnRunnerHandler::onCommand(TimerCommand const& command) {
	if (nullptr == command.TimerPtr) {
		Log::Info(fmt::format("ConnRunner: DelTimer[{}] ID[{}] by 'Pause' command", (void*)m_callbackPtr, m_eventID));
		DelTimer();
		return;
	}	// 空指针是删除Timer指令

	TriggerTimer* timerPtr = command.TimerPtr;
	auto pExchConn = m_conns[ConnIndex(command.pInfo->Exchange)];
	if (0 == m_eventID) {
		m_callbackPtr = new TimerCallback(timerPtr, pExchConn, command.pInfo, command.pExInfo);
// 		Log::Info(fmt::format("ConnRunner: Create TimerPtr[{}]", (void*)m_callbackPtr));
	}
	else {
// 		Log::Info(fmt::format("ConnRunner: DelTimer[{}] ID[{}] by Restart", (void*)m_callbackPtr, m_eventID));
		DelTimer();
		m_callbackPtr = new TimerCallback(timerPtr, pExchConn, command.pInfo, command.pExInfo);
// 		Log::Info(fmt::format("ConnRunner: Create new TimerPtr[{}]", (void*)m_callbackPtr));
	}	// 只允许有一个Timer，重复Start的话会删掉老的Timer用新的替代

	// Remark: timeSetEvent函数最多支持提前1000s(16.6min)创建Timer，超过则创建失败
	m_eventID = timeSetEvent(timerPtr->GetDelay(), 0, (LPTIMECALLBACK)TimerCallbackFunc, (DWORD_PTR)m_callbackPtr, TIME_ONESHOT);
	if (m_eventID > 0) {
		Log::Info(fmt::format("ConnRunner: SetTimer[{}] success, ID[{}] Delay[{}] Awake[{},{}] Period[{},{}]", (void*)m_callbackPtr, 
			m_eventID, timerPtr->GetDelay(), timerPtr->GetAwakePeriod(), timerPtr->AwakeCnts, timerPtr->GetTargetPeriod(), timerPtr->MaxLoopCnts));
	}
	else {
		Log::Info(fmt::format("ConnRunner: SetTimer failed because of Delay[{}] is overflow", timerPtr->GetDelay()));
		return;
	}
}

void ConnRunnerHandler::DelTimer() {
	if (m_eventID) {
		timeKillEvent(m_eventID);
		Log::Info(fmt::format("KillEvent, ID[{}]", m_eventID));
		m_eventID = 0;
		delete m_callbackPtr;
		m_callbackPtr = nullptr;
	}
}
