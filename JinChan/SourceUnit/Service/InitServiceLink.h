#pragma once
#include "ErrorEngine.h"
#include "IFlow.h"
#include "ZTPPackage.h"
#include "MaxOrderLocalIDSinglton.h"
#include "ZTPType.h"
using namespace ztp;
class SystemStatusRepository;
class InvestorRepository;
class OperatorRepository;
class InstrumentRepository;
class BaseInfoRepository;
class InitOrderStore;
class CInvestorPosition;
class CInstrument;

//CInitServiceZTPLink是一个在内存运行的Init服务的连接
class CInitServiceLink
{
public:
	//构造函数，根据一个Init服务，创建一个使用ZTP包的连接
	CInitServiceLink(SystemStatusRepository& systemRepo,
		InvestorRepository& investorRepository,
		OperatorRepository& operatorRepository,
		InstrumentRepository& instrumentRepository,
		BaseInfoRepository& baseInfoRepo,
		InitOrderStore& orderStore,
		std::string const& brokerID);
	virtual ~CInitServiceLink();

	bool HandleMessage(CZTPPackage& package);

	void InitBalanceAndPositionChange();
	void InitInvestorUsedMarginByPosition();
		
	uint32_t GetLocalID() { return m_localId; }

private:
	void initMargin(CZTPPackage& package, std::string& brokerID);
	void initFee(CZTPPackage& package, std::string& brokerID);
	void initAccount(CZTPPackage& package, std::string& brokerID);
	void initBalanceChange(CZTPPackage& package);
	void initPositionChange(CZTPPackage& package);
	void initPosition(CZTPPackage& package);
	bool recalcPositionUsedMargin(data::CInstrument const& inst, data::CInvestorPosition& position);
	void initSeat(CZTPPackage& package, std::string& brokerID);
	void initInvestor(CZTPPackage& package, std::string& brokerID);
	void initTrade(CZTPPackage& package, std::string& brokerID);
	void initOrder(CZTPPackage& package, std::string& brokerID);
	void initInstrument(CZTPPackage& package);

private:
	SystemStatusRepository& m_systemRepo;
	InvestorRepository& m_investorRepo;
	OperatorRepository& m_operRepo;
	InstrumentRepository& m_instRepo;
	BaseInfoRepository& m_baseInfoRepo;
	InitOrderStore& m_orderStore;

	uint32_t m_sequenceNo = 0;
	uint32_t m_localId = MaxOrderLocalIDSinglton::MIN_LOCAL_ID;
	std::string m_brokerID;

	std::vector<type::InvestorID> m_investors;
};
