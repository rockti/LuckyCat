#pragma once

#include <map>
#include "type/Money.h"

class Funds 
{
public:
	typedef std::map<unsigned int, money> FreezeSerial;		//流水，金额

public:
	explicit Funds(money initialCapital = 0.00);

public:
	money GetCapital() const { return m_capital; }
	money GetAvailable() const { return m_available; }
	money GetFee() const { return m_fee; }
	money GetMargin() const { return m_margin; }
	money GetFreezeAmount(unsigned int serialId);
	money GetTotalFreeze();

public:
	void SyncFunds(money capital, money margin, money available, money fee);		//接口同步账户可用资金
	void SetAvailable(money available);		//设置最大可用资金

	bool HaveEnoughCapital(money budget);					//资金试算
	bool Freeze(unsigned int serialId, money budget);		//冻结
	bool UnFreeze(unsigned int serialId);					//解冻
	bool OpenHold(money occupiedMargin, money fee);			//扣除资金
	void OffsetRelease(money returnMargin, money fee);		//释放资金

private:
	money m_capital;			//总资产
	money m_margin;				//占用保证金
	money m_fee;				//手续费
	money m_available;			//可用资金
	FreezeSerial m_freezeSerial;
};
