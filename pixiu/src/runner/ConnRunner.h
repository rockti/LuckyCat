#pragma once

#include "model/Constants.h"
#include <array>

#ifdef WIN32
#include <intrin.h>
static uint32_t __inline ctz(uint32_t x) {
	DWORD r = 0;
	_BitScanReverse(&r, x);
	return r;
}
#endif

struct Data;
struct OrderInfo;
struct ConnCommand;
struct TimerCommand;
class ExchangeConn;
class Business;
class TriggerTimer;

class TimerCallback {
public:
	TimerCallback(TriggerTimer* pTimer, ExchangeConn* exchConn, OrderInfo* pInfo1, OrderInfo* pInfo2)
		:m_increaseFreqTag(false), m_shfeCnt(4), m_cffexCnt(1), m_pTimer(pTimer), m_connection(exchConn) {
		Init(pInfo1, pInfo2);
	}
	~TimerCallback() {
		std::fill(m_orderInfo, m_orderInfo + 15, nullptr);
	}
	TriggerTimer* GetTimerPtr() const { return m_pTimer; }
	void Callback();
	void RestetOrderSet();
private:
	void Init(OrderInfo* pInfo1, OrderInfo* pInfo2) {
		if (nullptr != pInfo2) {
			m_orderInfo[0] = pInfo1;
			m_orderInfo[1] = pInfo2;
			m_orderInfo[2] = pInfo1;
			m_orderInfo[3] = pInfo2;
			m_orderInfo[4] = pInfo1;
			m_orderInfo[5] = pInfo2;
			m_orderInfo[6] = pInfo1;
			m_orderInfo[7] = pInfo2;
			m_orderInfo[8] = pInfo1;
			m_orderInfo[9] = pInfo2;
			m_orderInfo[10] = pInfo1;
			m_orderInfo[11] = pInfo2;
			m_orderInfo[12] = pInfo1;
			m_orderInfo[13] = pInfo2;
			m_orderInfo[14] = pInfo1;
			m_orderInfo[15] = pInfo2;
		}
		else {
			std::fill(m_orderInfo, m_orderInfo + 15, pInfo1);
		}
	}
private:
	bool m_increaseFreqTag;
	unsigned int m_shfeCnt;
	unsigned int m_cffexCnt;
	TriggerTimer* m_pTimer;
	ExchangeConn* m_connection;
	const OrderInfo* m_orderInfo[16] = { nullptr };		// 易达接口最多一次发送16笔委托
};


class ConnRunnerHandler {
public:
	ConnRunnerHandler(std::array<ExchangeConn*, 3> & conns) :m_conns(conns) {
	}
	~ConnRunnerHandler() { 
		if (nullptr != m_callbackPtr) {
			delete m_callbackPtr;
		}
	}
	void Handle(Data const& data);

private:
	void onCommand(ConnCommand const& command);
	void onCommand(TimerCommand const& command);
	void DelTimer();

	std::array<ExchangeConn*, 3>& m_conns;
	uint32_t m_eventID = 0;
	TimerCallback* m_callbackPtr = nullptr;
};