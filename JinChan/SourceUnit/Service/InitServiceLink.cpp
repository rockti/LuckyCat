#include "stdafx.h"
#include "InitServiceLink.h"
#include "ZtpDataStruct.h"
#include "EntityCopy.h"
#include "ErrorDefine.h"
#include "LinkManager.h"
#include "Repository.h"
#include "Profile.h"
#include "RateAlgorithms.h"
#include "Investor.h"

extern CExchangeLinkManager* g_pExchangeLinkManager;
CInitServiceLink::CInitServiceLink(SystemStatusRepository& systemRepo,InvestorRepository& investorRepo, OperatorRepository& operRepo,
	InstrumentRepository& instRepo, BaseInfoRepository& baseInfoRepo, InitOrderStore& orderStore,std::string const& brokerID)
	:m_systemRepo(systemRepo),
	m_investorRepo(investorRepo),
	m_operRepo(operRepo),
	m_instRepo(instRepo),
	m_baseInfoRepo(baseInfoRepo),
	m_orderStore(orderStore),
	m_brokerID(brokerID){
}
CInitServiceLink::~CInitServiceLink() {
}

struct UnilateralInstrumentSetting {
	int Group = -1;
	int SubIndex = -1;
};

std::map<type::InstrumentID, UnilateralInstrumentSetting> s_unilateralSettings = {
	{type::InstrumentID("Au(T+D)"), { 0, 0 } },
	{type::InstrumentID("mAu(T+D)"), { 0, 1 } },
	{type::InstrumentID("Au(T+N1)"), { 0 , 2 } },
	{type::InstrumentID("Au(T+N2)"), { 0 , 3 } },
	{type::InstrumentID("Ag(T+D)"), { 1 , 0 } },
};

template<class Target,class Repo, class Filter>
void AAInitAdd(CZTPPackage& package, Target& target,Repo& repo, char const* errorMsg, Filter f) {
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(target);
		if (f(target)) {
			repo.InitAdd(target);
		} else {
			RUNTIME_INFO_REPORT(errorMsg);
			Log::Error(errorMsg);
		}
		fieldItor.Next();
	}
}

template<class T>
struct BrokerFilter {
	BrokerFilter(T const& t,std::string const& brokerID) :m_brokerID(brokerID) {}
	bool operator()(T const& t) {
		return t.BrokerID == m_brokerID.c_str();
	}
	std::string m_brokerID;
}; 

template<class T>
struct TrueFilter {
	TrueFilter(T const& t) {}
	bool operator()(T const& t) {
		return true;
	}

};

bool CInitServiceLink::HandleMessage(CZTPPackage& package) {
	switch (package.GetTID()) {
	case TID_InitSeatInfo: {
		initSeat(package, m_brokerID);
	}
		break;
	case TID_InitOperatorInfo: {
		data::COperatorInfo info;
		AAInitAdd(package, info, m_operRepo, "InitError: OperatorInfo invalid", TrueFilter{ info });
		}
		break;
	case TID_InitInvestorInfo: {
		initInvestor(package, m_brokerID);
		}
		break;
	case TID_InitOperatorInvestor: {
		data::COperatorInvestor target;
		AAInitAdd(package, target, m_operRepo, "InitError: OperatorInvestor invalid", TrueFilter{ target});
	}
		break;
	case TID_InitInvestorAccount: {
		initAccount(package, m_brokerID);
		}
		break;
	case TID_InitInstrument: {
		initInstrument(package);
		}
		break;
	case TID_InitInvestorMargin: {
		initMargin(package, m_brokerID);
		}
		break;
	case TID_InitInvestorFee: {
		initFee(package, m_brokerID);
		}
		break;
	case TID_InitInvestorPosition: {
		initPosition(package);
		}
		break;
	case TID_InitInvestorBalanceChange: {
		initBalanceChange(package);
	}
		break;
	case TID_InitInvestorPositionChange: {
		initPositionChange(package);
	}
		break;
	case TID_InitOrder: {
		initOrder(package, m_brokerID);
	}
		break;
	case TID_InitTrade: {
		initTrade(package, m_brokerID);
	}

		break;
	default:
		break;
	}
	return true;
}

void CInitServiceLink::InitInvestorUsedMarginByPosition() {
	type::BrokerID brokerID(m_brokerID.c_str());
	for (auto& investorID : m_investors) {
		auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, brokerID, investorID);
		if (investor) {
			investor->InitAccountMarginByPositions();
		}
	}
}

void CInitServiceLink::InitBalanceAndPositionChange() {
	type::BrokerID brokerID(m_brokerID.c_str());
	for (auto& investorID : m_investors) {
		auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, brokerID, investorID);
		if (investor) {
			investor->ReplayChanges(0);
		}
	}
}

void CInitServiceLink::initMargin(CZTPPackage& package, std::string& brokerID) {
	data::CInvestorMargin margin;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(margin);
		if (margin.BrokerID == brokerID.c_str()) {
			m_investorRepo.InitAdd(margin);
			auto msg = fmt::format("init add margin InvestorID[{}] BrokerID[{}] InstCode[{}] LongMargin[{:.2f}] ShortMargin[{:.2f}]",
				margin.InvestorID, margin.BrokerID, margin.InstrumentID, margin.LongMarginRate, margin.ShortMarginRate);
			println(msg);
			Log::Info(msg);
		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initFee(CZTPPackage& package, std::string& brokerID) {
	data::CInvestorFee fee;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(fee);
		if (fee.BrokerID == brokerID.c_str()) {
			m_investorRepo.InitAdd(fee);
			auto msg = fmt::format("init add fee InvestorID[{}] BrokerID[{}] InstCode[{}] OpenFee[{:.4f}] OffsetFee[{:.4f}]",
				fee.InvestorID, fee.BrokerID, fee.InstrumentID, fee.OpenFeeRate, fee.OffsetFeeRate);
			println(msg);
			Log::Info(msg);

		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initAccount(CZTPPackage& package, std::string& brokerID) {
	data::CInvestorAccount account;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(account);
		if (account.BrokerID == brokerID.c_str()) {
			m_investorRepo.InitAdd(account);
			auto msg = fmt::format("init add account InvestorID[{}] BrokerID[{}] PreBalance[{:.2f}] Margin[{:.2f}]",
				account.InvestorID, account.BrokerID, account.PreBalance, account.Margin);
			println(msg);
			Log::Info(msg);
		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initBalanceChange(CZTPPackage& package) {
	data::CBalanceChange change;
	auto fieldItor = package.GetFieldTypeIterator();

	for (; !fieldItor.IsEnd(); fieldItor.Next()) {
		fieldItor.Retrieve(change);
		if (change.BrokerID == m_brokerID.c_str()) {
			m_investorRepo.InitAdd(change);
			auto msg = fmt::format("init add balance change InvestorID[{}] BrokerID[{}] Type[{}] Amount[{:.2f}] ExchangeSeqNo[{}] BalanceSeqNo[{}]",
				change.InvestorID, change.BrokerID, change.BalanceChangeType, change.Amount,
				change.ExchangeSequenceNo, change.BalanceSequenceNo);
			printNlog(msg);
		}
	}
}

void CInitServiceLink::initPositionChange(CZTPPackage& package) {
	data::CPositionChange change;
	auto fieldItor = package.GetFieldTypeIterator();

	for (; !fieldItor.IsEnd(); fieldItor.Next()) {
		fieldItor.Retrieve(change);
		if (change.BrokerID == m_brokerID.c_str()) {
			m_investorRepo.InitAdd(change);
			auto msg = fmt::format("init add spot position InvestorID[{}] BrokerID[{}] Dir[{}] OF[{}] Volume[{}] ExchangeSeqNo[{}] PosSeqNo[{}]",
				change.InvestorID, change.BrokerID, change.Direction, change.OffsetFlag, change.Volume,
				change.ExchangeSequenceNo, change.PositionSequenceNo);
			printNlog(msg);
		}
	}

}

void CInitServiceLink::initPosition(CZTPPackage& package) {
	data::CInvestorPosition pos;
	auto fieldItor = package.GetFieldTypeIterator();

	for(; !fieldItor.IsEnd(); fieldItor.Next()){
		fieldItor.Retrieve(pos);
		if (pos.BrokerID == m_brokerID) {
			// recalc usedMargin for defer position because of unilateral position
			auto* pInstrument = m_instRepo.Get(pos.InstrumentID);
			if (pInstrument == nullptr) {
				continue;
			}
			if (type::IsDefer(pos)) {
				if (!recalcPositionUsedMargin(*pInstrument, pos)) {
					continue;
				}
			}
			auto* pPos = m_investorRepo.InitAdd(pos);
			auto investor = Investors::GetInvestor(m_investorRepo, m_instRepo, pos.BrokerID, pos.InvestorID);
			if (investor) {
				investor->InitUnilateralPosition(*pInstrument, pPos);
			}

			auto msg = fmt::format("init position BrokerID[{}] InvestorID[{}] Instrument[{}] Dir[{}] YDPosition[{}] YDPrice[{:.3f}] UsedMargin[{:.3f}]",
				pos.BrokerID, pos.InvestorID, pInstrument->InstrumentID, pos.Direction, pos.YdPosition, pos.YdPositionPrice, pos.UsedMargin);
			println(msg);
			Log::Info(msg);
		}
	}
}

bool CInitServiceLink::recalcPositionUsedMargin(data::CInstrument const& inst, data::CInvestorPosition& position) {
	auto* pMargin = m_investorRepo.GetMargin(position.BrokerID, position.InvestorID, position.InstrumentID);
	type::InvestorID matchAll("*");
	if (pMargin == nullptr) {
		pMargin = m_investorRepo.GetMargin(position.BrokerID, matchAll, position.InstrumentID);
	}

	if (pMargin == nullptr) {
		return false;
	}
	CAlgorithms algo;
	double margin = algo.CalculateMargin(position.YdPosition, position.YdPositionPrice,
		position.Direction, inst.VolumeMultiple, pMargin);
	position.UsedMargin = margin;
	return true;
}

void CInitServiceLink::initSeat(CZTPPackage& package, std::string& brokerID) {
	data::CSeatInfo seat;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(seat);
		auto msg = fmt::format("SeatInfo AppName[{}] ParticipantID[{}] SeatID[{}] TraderID[{}]",
			seat.AppName, seat.ParticipantID, seat.SeatID, seat.TraderID);
		if (seat.AppName == "Trade" && seat.ParticipantID == brokerID.c_str()) {
			printNlog("start init SeatInfo");
			m_baseInfoRepo.InitAdd(seat);
#ifndef UNITTEST
			g_pExchangeLinkManager->GetSGEHandler(&m_instRepo,&seat);
			auto tradingDay = m_systemRepo.Get()->TradingDay;
			g_pExchangeLinkManager->GetSGEHandler()->SetTradingDay(tradingDay); // only for mock
#endif
			auto msg = fmt::format("init add SeatInfo AppName[{}] ParticipantID[{}] SeatID[{}] TraderID[{}]",
				seat.AppName, seat.ParticipantID, seat.SeatID, seat.TraderID);
			println(msg);
			Log::Error(msg);
		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initInvestor(CZTPPackage& package, std::string& brokerID) {
	data::CInvestorInfo info;
	auto fieldItor = package.GetFieldTypeIterator();

	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(info);
		if (info.BrokerID == brokerID.c_str()) {
			m_investors.push_back(info.InvestorID);
			m_investorRepo.InitAdd(info);
#ifndef UNITTEST
			g_pExchangeLinkManager->GetSGEHandler()->AddClientInfo(info.ClientID, info.InvestorID);
#endif
			auto msg = fmt::format("init add investor BrokerID[{}] ClientID[{}] InvestorID[{}] Name[{}]",
				info.BrokerID, info.ClientID, info.InvestorID, info.InvestorName);
			println(msg);
			Log::Error(msg);
		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initTrade(CZTPPackage& package, std::string& brokerID) {
	data::CTrade field;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(field);
		Log::Info(fmt::format("trade TradingDay[{}] SeqNo[{}]", field.TradeDate, field.SequenceNo));
		if (field.TradeDate == m_systemRepo.Get()->TradingDay && field.BrokerID == brokerID.c_str()) {
			if (field.SequenceNo > m_sequenceNo) {
				m_sequenceNo = field.SequenceNo;
				Log::Info(fmt::format("set new trade SeqNo[{}]", m_sequenceNo));
#ifndef UNITTEST
				g_pExchangeLinkManager->GetSGEHandler()->SetSequenceNo(m_sequenceNo);
#endif
			}
		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initOrder(CZTPPackage& package, std::string& brokerID) {
	data::COrder field;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(field);
		Log::Info(fmt::format("Order TradingDay[{}] SeqNo[{}]", field.TradingDay, field.SequenceNo));
		if (field.TradingDay == m_systemRepo.Get()->TradingDay && field.BrokerID == brokerID.c_str()) {
			m_orderStore.Add(field);
			if (field.SequenceNo > m_sequenceNo) {
				m_sequenceNo = field.SequenceNo;
				Log::Info(fmt::format("set new order SeqNo[{}], LocalID[{}], ActionID[{}]", m_sequenceNo,field.OrderLocalID, field.ActionLocalID));
#ifndef UNITTEST
				g_pExchangeLinkManager->GetSGEHandler()->SetSequenceNo(m_sequenceNo);
#endif
			}
			if (MaxOrderLocalIDSinglton::IsMine(field.OrderLocalID)) {
				m_localId = field.OrderLocalID;
			}
			if (MaxOrderLocalIDSinglton::IsMine(field.ActionLocalID)) {
				m_localId = field.ActionLocalID;
			}
		}
		fieldItor.Next();
	}
}

void CInitServiceLink::initInstrument(CZTPPackage& package) {
	data::CInstrument inst;
	auto fieldItor = package.GetFieldTypeIterator();
	while (!fieldItor.IsEnd()) {
		fieldItor.Retrieve(inst);
		auto iter = s_unilateralSettings.find(inst.InstrumentID);
		if (iter != s_unilateralSettings.end()) {
			inst.UnilateralGroup = iter->second.Group;
			inst.UnilateralSubIndex = iter->second.SubIndex;
		}
		auto msg = fmt::format("init add instrument, InstCode[{}] UniGroup[{}] UniSubIndex[{}]",
			inst.InstrumentID, inst.UnilateralGroup, inst.UnilateralSubIndex);
		println(msg);
		Log::Info(msg);
		m_instRepo.InitAdd(inst);
		fieldItor.Next();
	}
}
