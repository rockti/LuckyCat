#pragma once
#include "conn/ExchangeConn.h"
#include "conn/esunny/ESUNNYQuoteHandler.h"
#include "conn/esunny/ESUNNYTradeHandler.h"
#include "model/Instrument.h"
#include "config/Config.h"
#include <set>
#include <map>
#include <optional>


using Varieties = std::set<type::VarietyCode>;
struct VarietyInfo {
	type::VarietyCode Code;
	type::UnitPrice UnitPrice;
	type::VolumeMultiple VolumeMultiple;
};
using VarietyInfos = std::map<type::VarietyCode, VarietyInfo>;
class VarietyProgressManager {
public:
	static inline VarietyProgressManager& Instance() {
		static VarietyProgressManager s_manager;
		return s_manager;
	}
	void InitVarieties(Varieties const& varieties) {
		varieties_ = varieties;
		remainVarieties_ = varieties_;
	}
	void AddVarietyDone(type::VarietyCode const& code) {
		remainVarieties_.erase(code);
	}
	void AddVarietyInfo(VarietyInfo const& info) {
		infos_.emplace(info.Code, info);
	}

	std::optional<VarietyInfo> GetInfo(type::VarietyCode const& code) {
		auto iter = infos_.find(code);
		if (iter == infos_.end()) {
			return {};
		} else {
			return iter->second;
		}
	}
	bool IsDone() const {
		return remainVarieties_.empty();
	}
	bool IsRemain(type::VarietyCode const& code) const {
		return remainVarieties_.find(code) != remainVarieties_.end();
	}

	bool Has(type::VarietyCode const& code) const {
		return varieties_.find(code) != varieties_.end();
	}
	
private:
	std::set<type::VarietyCode> varieties_;
	VarietyInfos infos_;
	std::set<type::VarietyCode> remainVarieties_;
};


class Dealing;
class ConnPublisher;
class ESUNNYConnection : public ExchangeConn
{
public:
	ESUNNYConnection(TradeSystemConfig const& config, ConnPublisher& publisher);
	virtual void Init();
	virtual void Release();
	virtual void Login(type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
		type::InvestorID const& quoteInvestor, type::Password const& quotePassword) override;
	virtual void ReLogin(SubSystem subSystem) override {};
	virtual void InsertOrder(OrderInsertRequest const& order);
	virtual void CancelOrder(OrderCancelRequest const& action);
	virtual void SubscribeQuote(type::InstrumentCode const& instCode);
	virtual void UnSubscribeQuote(type::InstrumentCode const& instCode);
	virtual void QueryAccount();
	virtual void QueryOrders();
	virtual void QueryTrades();
	virtual void QueryPositions();
	virtual void QueryInstruments();
	virtual void QueryFees();
	virtual void QueryMargins();

	virtual void QueryInstrument(type::InstrumentCode const& instCode);
	virtual void QueryFee(type::InstrumentCode const& instCode);
	virtual void QueryMargin(type::InstrumentCode const& instCode);

	virtual void SetVertificateCode(char const* code);


	virtual void ChangePassword(type::Password const& oldPassword, type::Password const& newPassword) override;

private:
	void InitQuoteApi();
	void InitTradeApi();
	std::string GetErrorDescription(TAPIINT32 errorCode);

	TradeSystemConfig quoteConfig(TradeSystemConfig const& config);
	TradeSystemConfig tradeConfig(TradeSystemConfig const& config);
	std::pair<std::string, int> SeparatePort(std::string const& hostUrl);
private:
	TradeSystemConfig m_quoteConfig;
	TradeSystemConfig m_tradeConfig;

	ESUNNYTradeHandler m_tradeHandler;
	ESUNNYQuoteHandler m_quoteHandler;
	ITapTrade::ITapTradeAPI* m_pTradeApi;
	ITapQuoteAPI* m_pQuoteApi;
	ConnPublisher& m_publisher;
	type::InvestorID m_tradeInvestor;
	type::Password m_tradePassword;
	type::InvestorID m_quoteInvestor;
	type::Password m_quotePassword;


};