#pragma once
#include <vector>
#include <string>
#include <map>
#include "model/Constants.h"
#include "model/Variety.h"


class Charge		//手续费
{
public:
	enum TYPE { NONE = 0, FIXED,/*固定金额*/ PERCENT/*比例*/ };

	Charge(TYPE type, money rate)
		:m_type(type), m_rate(rate)
	{
	}

	money CalculateCharge(money turnover, int quantity) const
	{
		if (FIXED == m_type)
			return m_rate * quantity;
		if (PERCENT == m_type)
			return m_rate * turnover * quantity;
		return 0.00;
	}

private:
	TYPE m_type;
	money m_rate;
};
typedef std::vector<Charge> ChargeCombination;		//手续费组合{固定&比例}

class Instrument		//品种
{
public:
	inline Instrument(int index, Variety const& variety, std::string const& yearMonth)
		:m_id(InstIDGen(variety.GetExchange(),index)),m_variety(variety), m_yearMonth(yearMonth), m_code(variety.GetCode() + yearMonth), m_margin(0.0) {
		if (Exchange::SGE == variety.GetExchange()) {
			m_code = variety.GetCode().c_str();
			m_yearMonth = "";
		}
	}

	inline Instrument(Instrument const& other)
		:m_id(other.m_id), m_variety(other.m_variety), m_yearMonth(other.m_yearMonth), m_code(other.m_code), m_margin(other.m_margin), m_operationFee(other.m_operationFee) {
	}

	inline InstID GetID() const { return m_id; }
	inline Exchange GetExchange() const { return m_variety.GetExchange(); }
	inline char const* GetContractCode() const { return m_yearMonth.c_str(); }
	inline InstCodeT const& GetInstCode() const { return m_code; }
	inline Variety GetVariety() const { return m_variety; }
	inline money GetMargin() const { return m_margin; }
	inline money GetUnitPrice() const { return m_variety.GetUnitPrice(); }
	inline int GetMultiplier() const { return m_variety.GetMultiplier(); }

	inline void SetMargin(money margin) { m_margin = margin; }
	inline void SetOperationFee(Operation operation, ChargeCombination const& chargeCombination) {
		m_operationFee[operation] = chargeCombination;
	}

	inline money CalculateTotal(money price, int quantity, Operation operation) const {
		return CalculateMargin(price, quantity, operation) + CalculateFee(price, quantity, operation);
	}
	inline money CalculateMargin(money price, int quantity, Operation operation) const {		
		return (Operation::OPEN == operation) ? (price * GetMargin() * GetMultiplier() * quantity) : 0;
	}
	inline money CalculateFee(money price, int quantity, Operation operation) const {
		//FIXED + PERCENT
		money fee = 0.00;
		OperationFee::const_iterator iterator = m_operationFee.find(operation);
		if (iterator != m_operationFee.end()) {
			ChargeCombination const& chargeCombine = iterator->second;
			for (auto charge : chargeCombine) {
				money sum = price * GetMultiplier();
				fee += charge.CalculateCharge(sum, quantity);
			}
		}
		return fee;
	}
private:
	InstID m_id;
	Variety m_variety;
	std::string m_yearMonth;
	InstCodeT m_code;			//合约名称（品种名称+月份）
	money m_margin;				//保证金

	typedef std::map<Operation, ChargeCombination> OperationFee;
	OperationFee m_operationFee;	//不同的操作对应不同的手续费组合{开（比例&固定）；平（比例&固定）；平今（比例&固定）}
};


class Instruments {
public:
	inline size_t Count() const { return m_instruments.size(); }

	inline void Add(Instrument const& inst) {
		m_instruments.push_back(inst);
	}

	inline Instrument const& Get(InstID instID) const {
		return m_instruments[InstIDIndex(instID)];
	}

	inline std::vector<Instrument> const& GetAll() const {
		return m_instruments;
	}
private:
	std::vector<Instrument> m_instruments;
};

