#include "stdafx.h"
#include "OperatorServiceLink.h"
#include "LinkManager.h"
#include "ErrorDefine.h"
#include "Investor.h"
#include "ZTPType.h"

using namespace ztp;

COperatorServiceLink::COperatorServiceLink(
	IZTPServer& ztpServer, 
	SystemStatusRepository& systemStatusRepo, 
	BaseInfoRepository& baseInfoRepo, 
	InvestorRepository& investorRepo, 
	InstrumentRepository& instRepo)
:m_ztpServer(ztpServer),m_systemStatusRepo(systemStatusRepo), m_baseInfoRepo(baseInfoRepo),m_investorRepo(investorRepo), m_instRepo(instRepo) {
	m_ztpPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
}
COperatorServiceLink::~COperatorServiceLink() {
	m_ztpPackage.Release();
}


void LogBalanceRequest(data::COpInvestorBalance const& balanceRequest) {
	Log::Info(fmt::format("BalanceRequest: OperatorID[{}] InvestorID[{}] Deposit[{:.2f}] Withdraws[{:.2f}] ",
		balanceRequest.OperatorID, balanceRequest.InvestorID, balanceRequest.Deposit, balanceRequest.Withdraw));
}

void COperatorServiceLink::HandleMessage(CZTPPackage& package) {
	switch (package.GetTID()) {
	case Op::ChangeInvestorBalance::RequestID: {	
		handleBalanceRequest(package);
		break;
	}
	case Op::QueryInvestors::RequestID: {
		handleQueryInvestors(package);
		break;
	}
	case Op::QueryInvestorAccount::RequestID: {
		handleQueryAccount(package);
		break;
	}
	case Op::QueryInvestorPosition::RequestID: {
		handleQueryPosition(package);
		break;
	}
	case Op::QueryInvestorBalanceChange::RequestID: {
		handleQueryBalanceChange(package);
		break;
	}
	case Op::QueryInvestorPositionChange::RequestID: {
		handleQueryPositionChange(package);
		break;
	}
	default:
		break;
	}
    return;
}


const std::string currentTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%T", &tstruct);

	return buf;
}

void COperatorServiceLink::handleBalanceRequest(CZTPPackage& package) {

	using Proto = Op::ChangeInvestorBalance;

	data::COpInvestorBalance req;
	data::CRspInfo rspInfo = from_error(NoError);

	if (!package.GetSingleField(req)) {
		return;
	}
	LogBalanceRequest(req);

	ztp::message msg;
	msg.header(Proto::StreamType, Proto::ResponseID, package.GetRequestID(), package.GetSessionID(), true, req.InvestorID.value());

	auto investor = Investors::GetInvestor(m_investorRepo,m_instRepo,req.BrokerID, req.InvestorID);
	if (!investor) {
		Log::Error(fmt::format("BalanceRequest:can not find investor by InvestorID [{}] Error[{}]", req.InvestorID, investor.Error.Msg));
		fill_rsp(rspInfo, InvalidInvestorError);
		msg.rsp_body<Proto>(rspInfo);
		m_ztpServer.OpRsp(msg);
		return;
	}

	if (req.Deposit < 0.0 || req.Withdraw < 0.0) {
		Log::Error(fmt::format("BalanceRequest:Negative Amount D[{:.2f}] W[{:.2f}]", req.Deposit, req.Withdraw));
		fill_rsp(rspInfo, NegativeBalanceChangeAmountError);
		msg.rsp_body<Proto>(rspInfo);
		m_ztpServer.OpRsp(msg);
		return;

	}

	if (req.Deposit > 0.0) {
		data::CBalanceChange balanceChange;
		balanceChange.BrokerID = req.BrokerID;
		balanceChange.InvestorID = req.InvestorID;
		balanceChange.TradingDay = m_systemStatusRepo.Get()->TradingDay;
		balanceChange.BalanceChangeType = type::BalanceChangeFlag::Deposit;
		balanceChange.Amount = req.Deposit;
		balanceChange.ExchangeSequenceNo = m_investorRepo.GetLastSGESeqNo();
		balanceChange.ChangeTime = currentTime().c_str();
		auto const* result = m_investorRepo.Add(balanceChange);
		Saver saver(m_investorRepo, *result, true);
		investor->OnBalanceChange(*result);
	}

	if (req.Withdraw > 0.0) {
		data::CBalanceChange balanceChange;
		balanceChange.BrokerID = req.BrokerID;
		balanceChange.InvestorID = req.InvestorID;
		balanceChange.TradingDay = m_systemStatusRepo.Get()->TradingDay;
		balanceChange.BalanceChangeType = type::BalanceChangeFlag::Withdraw;
		balanceChange.Amount = req.Withdraw;
		balanceChange.ExchangeSequenceNo = m_investorRepo.GetLastSGESeqNo();
		balanceChange.ChangeTime = currentTime().c_str();
		auto const* result = m_investorRepo.Add(balanceChange);
		Saver saver(m_investorRepo, *result, true);
		investor->OnBalanceChange(*result);
	}
	msg.rsp_body(rspInfo,investor->GetAccount());
	m_ztpServer.OpRsp(msg);
}


template<class Proto, class QueryAndForEach>
void COperatorServiceLink::handleQuery(CZTPPackage& package, QueryAndForEach func) {
	Proto::RequestType req;
	data::CRspInfo rsp = from_error(NoError);
	if (!package.GetSingleField(req)) {
		return;
	}
	ztp::message msg;
	msg.header(Proto::StreamType, Proto::ResponseID, package.GetRequestID(), package.GetSessionID());
	int count = func(req,rsp, msg);
	if (count == 0) {
		msg.rsp_body<Proto>(rsp);
		m_ztpServer.OpRsp(msg);
	}
}


void COperatorServiceLink::handleQueryAccount(CZTPPackage& package) {
	using Proto = Op::QueryInvestorAccount;

	handleQuery<Proto>(package, [&](Proto::RequestType const& req, data::CRspInfo& rsp, message& msg)->int {
		if (auto* account = m_investorRepo.GetAccount(req.InvestorID); account) {
			msg.rsp_body(rsp, *account);
			m_ztpServer.OpRsp(msg);
			return 1;
		}
		Log::Error(fmt::format("OpQueryAccount:can not find account by InvestorID [{}]", req.InvestorID));
		fill_rsp(rsp, InvalidInvestorError);
		return 0;
		});
}

void COperatorServiceLink::handleQueryInvestors(CZTPPackage& package) {
	using Proto = Op::QueryInvestors;

	handleQuery<Proto>(package, [&](Proto::RequestType const& req, data::CRspInfo& rsp, message& msg) -> int {
		return m_investorRepo.InvestorForEach([&](data::CInvestorInfo const& info, bool isLast) {
			data::CInvestorInfo result = info;
			result.Password = "";
			msg.is_last(isLast);
			msg.rsp_body(rsp, result);
			m_ztpServer.OpRsp(msg);
		});
	});
}

void COperatorServiceLink::handleQueryPosition(CZTPPackage& package) {
	using Proto = Op::QueryInvestorPosition;
	handleQuery<Proto>(package, [&](Proto::RequestType const& req, data::CRspInfo& rsp, message& msg)->int {
		return m_investorRepo.PositionForEach(req.InvestorID, [&](Proto::ResponseType const& pos, bool isLast) {
			msg.header(Proto::StreamType, Proto::ResponseID, package.GetRequestID(), package.GetSessionID(), isLast);
			msg.rsp_body(rsp, pos);
			m_ztpServer.OpRsp(msg);
		});
	});
}

void COperatorServiceLink::handleQueryBalanceChange(CZTPPackage& package) {
	using Proto = Op::QueryInvestorBalanceChange;

	handleQuery<Proto>(package, [&](Proto::RequestType const& req, data::CRspInfo& rsp, message& msg)->int {
		return m_investorRepo.BalanceChangeForEach(req.InvestorID, [&](Proto::ResponseType const& pos, bool isLast) {
			msg.header(Proto::StreamType, Proto::ResponseID, package.GetRequestID(), package.GetSessionID(), isLast);
			msg.rsp_body(rsp, pos);
			m_ztpServer.OpRsp(msg);
		});
	});

}

void COperatorServiceLink::handleQueryPositionChange(CZTPPackage& package) {
	using Proto = Op::QueryInvestorPositionChange;

	handleQuery<Proto>(package, [&](Proto::RequestType const& req, data::CRspInfo& rsp, message& msg)->int {
		return 	m_investorRepo.PositionChangeForEach(req.InvestorID, [&](Proto::ResponseType const& pos, bool isLast) {
			msg.header(Proto::StreamType, Proto::ResponseID, package.GetRequestID(), package.GetSessionID(), isLast);
			msg.rsp_body(rsp, pos);
			m_ztpServer.OpRsp(msg);
			});
		});

}

