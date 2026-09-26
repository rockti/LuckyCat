#include "stdafx.h"
#include "DBInit.h"

void DBInit::Init(std::string const& marketBorkerID) {
	auto* pMemChangedFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x200000);
	auto* database = new Database(pMemChangedFlow, ReposInstance().SystemStatusRepo);
	database->Create();
	DBInstance() = database;
	CZTPPackage initPackage;
	//等待InitLoad数据库操作完成
	while (!isInited_) {
		for (int i = 0; i < 1000; ++i) {
			if (!database->GetNextUseNoBuf(&initPackage))	//获取ZTP有序数据包
				break;
			HandleMessage(&initPackage, marketBorkerID);		//丢给交易引擎处理
		}
		::Sleep(200);
	}
}

inline void DBInit::HandleMessage(CZTPPackage* pPackage, std::string const& marketBorkerID) {

	switch (pPackage->GetTID()) {
	case TID_DataSyncEnd:
		isInited_ = true;
		break;
	case TID_InitSeatInfo: {
		auto fieldItor = pPackage->GetFieldTypeIterator();
		data::CSeatInfo seatInfo;
		while (!fieldItor.IsEnd()) {
			fieldItor.Retrieve(seatInfo);
			if (seatInfo.AppName == "Market" && seatInfo.ParticipantID == marketBorkerID.c_str()) {
				ReposInstance().BaseRepo.InitAdd(seatInfo);
				printNlog("Init Seat Info Success BrokerID[{}]\n", seatInfo.ParticipantID);
			}
			fieldItor.Next();
		}
		break;
	}
	case TID_InitInvestorInfo: {
		auto fieldItor = pPackage->GetFieldTypeIterator();
		data::CInvestorInfo info;
		while (!fieldItor.IsEnd()) {
			fieldItor.Retrieve(info);
			printNlog(fmt::format("Init Investor [{}] [{}] [{}]", info.BrokerID, info.InvestorID, info.InvestorName));
			ReposInstance().InvestorRepo.InitAdd(info);
			fieldItor.Next();
		}
		break;
	}
	default:
		break;
	}
}

static Repos s_repos;
static Database* s_database = nullptr;
Repos& ReposInstance() {
	return s_repos;
}

Database*& DBInstance() {
	return s_database;
}


