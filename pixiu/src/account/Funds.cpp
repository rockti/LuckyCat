#include "pch.h"

#include "account/Funds.h"

Funds::Funds(money initialCapital)
	:m_capital(initialCapital),	m_available(initialCapital),m_fee(0), m_margin(0)
{
	m_freezeSerial.clear();
}

money Funds::GetFreezeAmount(unsigned int serialId)
{
	auto it = m_freezeSerial.find(serialId);
	if (it != m_freezeSerial.end())	{
		return it->second;
	}
	return 0.00;
}
money Funds::GetTotalFreeze() {
	money totalFreeze = 0;
	for (auto const& [_, v] : m_freezeSerial) {
		totalFreeze += v;
	}
	return totalFreeze;
}

void Funds::SetAvailable(money available)
{
	m_available = available;
}
void Funds::SyncFunds(money capital, money margin, money available, money fee)
{
	m_capital = capital;
	m_margin = margin;
	m_available = available;
	m_fee = fee;
}

bool Funds::HaveEnoughCapital(money budget)
{
	return m_available >= budget;
}
bool Funds::Freeze(unsigned int serialId, money budget)
{
	if (HaveEnoughCapital(budget))
	{
		auto it = m_freezeSerial.find(serialId);
		if (it != m_freezeSerial.end()) {
			return false;
		}

		m_freezeSerial[serialId] = budget;
		m_available -= budget;
		return true;
	}
	return false;
}
bool Funds::UnFreeze(unsigned int serialId)
{
	auto it = m_freezeSerial.find(serialId);
	if (it == m_freezeSerial.end()) {
		return false;
	}

	m_available += it->second;
	m_freezeSerial.erase(it);
	return true;
}
bool Funds::OpenHold(money occupiedMargin, money fee)
{
	money totalHold = occupiedMargin + fee;
	if (m_available >= totalHold) {
		m_margin += occupiedMargin;
		m_available -= totalHold;
		m_fee += fee;
		return true;
	}
	return false;
}
void Funds::OffsetRelease(money returnMargin, money fee)
{
	m_margin -= returnMargin;
	m_available += returnMargin;
	m_available -= fee;
	m_fee += fee;
}

