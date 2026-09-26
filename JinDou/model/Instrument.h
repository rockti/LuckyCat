#pragma once
#include <vector>
#include <string>
#include <map>
#include "model/Constants.h"
#include "model/Variety.h"
#include "type/ZTPType.h"
using namespace ztp;

// class Instrument		//Æ·ÖÖ
// {
// public:
// 	inline Instrument(int index, Variety const& variety, std::string const& yearMonth)
// 		:m_id(InstIDGen(variety.GetExchange(),index)),m_variety(variety), m_yearMonth(yearMonth), m_code((variety.GetCode() + yearMonth).c_str()) {
// 	}
// 
// 	inline Instrument(Instrument const& other)
// 		:m_id(other.m_id), m_variety(other.m_variety), m_yearMonth(other.m_yearMonth), m_code(other.m_code) {
// 	}
// 
// 	inline InstID GetID() const { return m_id; }
// 	inline Exchange GetExchange() const { return m_variety.GetExchange(); }
// 	inline char const* GetContractCode() const { return m_yearMonth.c_str(); }
// 	inline type::InstrumentCode const& GetInstCode() const { return m_code; }
// 	inline Variety GetVariety() const { return m_variety; }
// 	inline type::Money GetUnitPrice() const { return m_variety.GetUnitPrice(); }
// 	inline int GetMultiplier() const { return m_variety.GetMultiplier(); }
// 
// private:
// 	InstID m_id;
// 	Variety m_variety;
// 	std::string m_yearMonth;
// 	type::InstrumentCode m_code;
// };


class Instruments {
public:
	inline size_t Count() const { return m_instruments.size(); }

	inline void Add(Instrument const& inst) {
		m_instruments.push_back(inst);
	}

	inline Instrument const& Get(type::InstID instID) const {
		return m_instruments[InstIDIndex(instID)];
	}

	inline std::vector<Instrument> const& GetAll() const {
		return m_instruments;
	}
private:
	std::vector<Instrument> m_instruments;
};

