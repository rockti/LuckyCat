#pragma once
#include "TradeServiceImpl.h"
#include "ErrorEngine.h"
#include "IFlow.h"
#include "ZTPPackage.h"
#include "EntityCopy.h"

struct LocalIdMapping {
	type::OrderLocalID LocalId;
	type::UserOrderLocalID UserLocalId;
	type::SessionID SessionID;
	type::RequestID RequestID;
};

class LocalIdRepository {
public:
	void Add(LocalIdMapping const& mapping) {
		m_localIds[mapping.LocalId] = mapping;
	}

	void Remove(type::OrderLocalID localId) {
		m_localIds.erase(localId);
	}

	std::optional<LocalIdMapping> Find(type::OrderLocalID localID) {
		auto iter = m_localIds.find(localID);
		if (iter == m_localIds.end()) {
			return {};
		} else {
			return iter->second;
		}
	}

private:
	std::map<type::OrderLocalID, LocalIdMapping> m_localIds;
};

class Order;
class Trade;


class CTradeServiceLink
{
public:
	CTradeServiceLink(
		SessionRepository& sessionRepo,
		SystemStatusRepository& systemStatusRepo, 
		BaseInfoRepository& baseInfoRepo, 
		InvestorRepository& investorRepo, 
		InstrumentRepository& instRepo, 
		IFlow* pTradeResult);
	virtual ~CTradeServiceLink();

	void HandleMessage(CZTPPackage& package);

protected:
	void handelNotifyOrder(CZTPPackage& package);
	void handleNotifyTrade(CZTPPackage& package);

	void handleOrderInsert(CZTPPackage& package);
	void handleOrderInsertError(CZTPPackage& package);

	void handleCancel(CZTPPackage& package);
	void handleCancelError(CZTPPackage& package);
	void handleDeferFee(CZTPPackage& package);

	template<class Field>
	void PushRspToResultFlow(uint32_t tID, CZTPPackage& package, Field& field, Error error = NoError);
	void PushErrorRspToResultFlow(uint32_t tID, CZTPPackage& package, Error error = NoError);

	void NotifyOrderToResultFlow(Order const& order);
	void NotifyTradeToResultFlow(Trade const& trade);
	void NotifyTradeToResultFlow(data::CTrade const& field);

	void handleSpotPositionNotify(CZTPPackage& package);

	Error CheckSession(type::BrokerID const& BrokerID, type::InvestorID const& InvestorID, type::SessionID const& SessionID);

private:
	SessionRepository& m_sessionRepo;
	SystemStatusRepository& m_systemStatusRepo;
	BaseInfoRepository& m_baseInfoRepo;
	InvestorRepository& m_investorRepo;
	CTradeServiceImpl m_tradeService;
	IFlow* m_pTradeResultFlow;
	CZTPPackage m_ztpPackage;
	LocalIdRepository m_localIdRepository;
};