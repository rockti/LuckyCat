#pragma once
#include "Repository.h"
#include "SQL/SQLAPI.h"
#include "SelectThread.h"
#include "ZTPPackage.h"
#include "FlowReader.h"
#include "ZTPType.h"
#include "ServiceDef.h"
#include "BaseExchApi.h"
#include <chrono>
using namespace ztp;

class CFieldTypeIterator;

class SAConnection;
class IFlow;
class Database : public CSeriesReader, public CSelectThread {
public:
	Database(IFlow* flow, SystemStatusRepository& systemStatusRepo);
	bool Connect();
	void Release();
	void KeepAlive();
	bool IsConnected() const;

	void Load();
	void Add(data::COrder const& order);
	void AddOrUpdate(data::COrder const& order);
	void Update(data::COrder const& order);
	void Add(data::CTrade const& trade);

	void Add(data::CBalanceChange const& balanceChange);
	void Add(data::CPositionChange const& positionChange);

	void HandleMemberDBMsg(CZTPPackage* pPackage);
	void ProcField(WORD fid, CFieldTypeIterator& iter, type::MemTableChangeType actType);

	bool ChangePassword(type::InvestorID investor, type::Hash newHash);
private:
	virtual bool InitInstance() override;
	virtual void ExitInstance() override;
	virtual bool HandleOtherTask() override;

	template<uint32_t FuncID, typename Type>
	void LoadAndPushTable(SAConnection& conn, internal_protocol<FuncID, Type> p);

	template<typename IP>
	void PushData(typename IP::Type const& v);
private:
	SAConnection con_;
	template<class T>
	void InternalAddOrUpdate(T const& v);
	template<class T>
	void InternalUpdate(T const& v);
	template<class T>
	void InternalAdd(T const& v);

	IFlow*  flow_;
	SystemStatusRepository& systemStatusRepo_;
	CZTPPackage m_publishPackage;
	CZTPPackage m_loadingPackage;
	CFlowReader m_flowReader;
	std::chrono::system_clock::time_point m_last;
};
