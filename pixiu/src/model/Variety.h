#pragma once

#include "model/Constants.h"
#include <string>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <date/date.h>


class Variety {
public:
	enum TYPE {
		Future = 0, 
		Bond, 
		Index, 
		Defer,	// 延期
		Spot,	// 现货
		Unknown
	};
public:
	Variety(Exchange exchange, char const* code, money unitPrice, int multiplier, date::year_month_day const& today)
		:m_exchangeFlag(exchange), m_code(code), m_unitPrice(unitPrice), m_multiplier(multiplier) {
		initType();
		initInstrumentCodes(today);
	}

	Variety(Variety const& other)
		:m_exchangeFlag(other.m_exchangeFlag), m_type(other.m_type),m_code(other.m_code)
		, m_unitPrice(other.m_unitPrice), m_multiplier(other.m_multiplier) {
	}

	Exchange GetExchange() const { return m_exchangeFlag; }
	std::string const& GetCode() const { return m_code; }
	money GetUnitPrice() const { return m_unitPrice; }
	int GetMultiplier() const { return m_multiplier; }
	TYPE GetType() const { return m_type; }
	bool IsSpot() const { return Spot == m_type; }

	std::string GetExchangeName() const;
	std::vector<std::string> const& GetInstrumentCodes() const { return m_instrumentCodes; }

private:
	Exchange m_exchangeFlag;		// 交易所
	TYPE m_type;					// 品种类型
	std::string m_code;				// 品种名称
	money m_unitPrice;				// 最小变动价
	int m_multiplier;				// 乘数
	std::vector<std::string> m_instrumentCodes;

	void initType();
	void initInstrumentCodes(date::year_month_day const& today);
	std::string instsrumentCode(int year, unsigned month) const;
};

typedef std::vector<Variety> Varieties;

