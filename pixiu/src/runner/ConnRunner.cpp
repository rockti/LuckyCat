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

bool g_breakTag = false;
int const MICRO_SECOND = 1000000;
static VOID CALLBACK TimerCallbackFunc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
	TimerCallback* parameter = (TimerCallback*)dwUser;
	TriggerTimer* timerPtr = parameter->GetTimerPtr();

	if (g_breakTag) {
		parameter->RestetOrderSet();
		return;
	}
	
	Log::Info(fmt::format("ConnRunnerHandler: callback start, remainCnts[{}]", timerPtr->MaxLoopCnts));
	if (TriggerTimer::DELAY == timerPtr->GetType()) {
		parameter->Callback();
	}
	else {
		LARGE_INTEGER frequency = { 0 };
		QueryPerformanceFrequency(&frequency);
		LARGE_INTEGER endTime = { 0 };
		LARGE_INTEGER beginTime = { 0 };
		QueryPerformanceCounter(&beginTime);
		while (timerPtr->MaxLoopCnts > 0 && false == g_breakTag) {
			QueryPerformanceCounter(&endTime);
			int64_t timeSpan = (endTime.QuadPart - beginTime.QuadPart) * MICRO_SECOND/*精度微秒*/ / frequency.QuadPart/*精度0.1微秒：10000000*/;
			uint32_t period = (timerPtr->AwakeCnts > 0) ? timerPtr->GetAwakePeriod() : timerPtr->GetPeriod();
			int64_t quotient = timeSpan / period;
			if (quotient > 0) {
				parameter->Callback();
				QueryPerformanceCounter(&beginTime);
			}
		}
		g_breakTag = true;
		Log::Info(fmt::format("ConnRunnerHandler: callback stop, remainCnts[{}]", timerPtr->MaxLoopCnts));
	}
	parameter->RestetOrderSet();
}

void TimerCallback::Callback() {
	if (IsSGE(m_orderInfo[0]->Exchange)) {
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

		// 以下为易达批量发单功能代码（目前上海12席位, 中金5席位）
		unsigned int& batchCnt = IsSHFE(m_orderInfo[0]->Exchange) ? m_shfeCnt : m_cffexCnt;
		m_connection->InsertMultiOrders(batchCnt, m_orderInfo);
		m_pTimer->MaxLoopCnts -= batchCnt;

		if (--m_pTimer->AwakeCnts <= 0) {
			if (IsSHFE(m_orderInfo[0]->Exchange)) {
				if (batchCnt < 12) {
					batchCnt += 2;
				}
				else if (batchCnt > 12) {
					batchCnt = 12;
				}
			}
			else {
				if (m_increaseFreqTag) {
					if (batchCnt < 5) {
						batchCnt += 1;
					}
					else if (batchCnt > 5) {
						batchCnt = 5;
					}
				}
				m_increaseFreqTag = !m_increaseFreqTag;
			}	// 中金所每个频率发两次再升频
		}
	}	// 预热单发完后逐渐升频
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
			m_eventID, timerPtr->GetDelay(), timerPtr->GetAwakePeriod(), timerPtr->AwakeCnts, timerPtr->GetPeriod(), timerPtr->MaxLoopCnts));
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
