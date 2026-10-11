#pragma once

class TriggerTimer {
public:
	enum Type { DELAY/*定时触发*/, PERIOD/*间隔触发*/, AWAKE/*唤醒触发*/ };
	TriggerTimer(Type type, uint32_t delay, uint32_t period = 0, uint32_t awakePeriod = 0)
		:m_type(type), m_delay(delay), m_targetPeriod(period), m_awakePeriod(awakePeriod) {
	}
	Type GetType() const { return m_type; }
	uint32_t GetDelay() const { return m_delay; }
	uint32_t GetAwakePeriod() const { return m_awakePeriod; }
	uint32_t GetTargetPeriod() const { return m_targetPeriod; }

	void InitCounters(uint32_t maxLoop, int awakeLoop=0) {
		MaxLoopCnts = maxLoop;
		AwakeCnts = awakeLoop;
	}

	// 普通定时
	static TriggerTimer* CreateDelayTimer(uint32_t delay) { return new TriggerTimer(DELAY, delay); }
	// 直接进入高频定时
	static TriggerTimer* CreatePeriodTimer(uint32_t delay, uint32_t period) { return new TriggerTimer(PERIOD, delay, period); }
	// 唤醒n次后再进入高频定时
	static TriggerTimer* CreateAwakeTimer(uint32_t delay, uint32_t period, uint32_t exPeriod) { return new TriggerTimer(AWAKE, delay, period, exPeriod); }

public:
	int AwakeCnts = 0;
	int MaxLoopCnts = 100;	// 缺省值
private:
	Type m_type;
	uint32_t m_delay;		// 延时：ms	
	uint32_t m_awakePeriod;	// 唤醒精度：ms
	uint32_t m_targetPeriod;// 执行精度：us
};
