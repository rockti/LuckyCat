#pragma once
#include "ZTPType.h"

#include "ztp_protocol.hpp"
#include "IFlow.h"
#include "ErrorDefine.h"
namespace ztp {
	const MessageID TID_DataSyncStart = 0x00005000;
	//数据同步结束
	const MessageID TID_DataSyncEnd = 0x00005001;
	//初始化席位信息
	const MessageID TID_InitSeatInfo = 0x00005002;
	//初始化操作员信息
	const MessageID TID_InitOperatorInfo = 0x00005003;
	//初始化投资者信息
	const MessageID TID_InitInvestorInfo = 0x00005004;
	//初始化用户投资者关系
	const MessageID TID_InitOperatorInvestor = 0x00005005;
	//初始化投资者资金账户
	const MessageID TID_InitInvestorAccount = 0x00005006;		//通过导入的上日结算资金同步
	//初始化投资者持仓
	const MessageID TID_InitInvestorPosition = 0x00005007;		//通过导入的上日结算持仓同步
	//初始化合约列表
	const MessageID TID_InitInstrument = 0x00005008;
	//初始化投资者保证金率
	const MessageID TID_InitInvestorMargin = 0x00005009;
	//初始化投资者手续费
	const MessageID TID_InitInvestorFee = 0x0000500A;

	//初始化投资者手续费
	const MessageID TID_InitOrder = 0x0000500B;

	//初始化投资者手续费
	const MessageID TID_InitTrade = 0x0000500C;

	//初始化投资者手续费
	const MessageID TID_InitInvestorBalanceChange = 0x0000500D;

	//初始化投资者手续费
	const MessageID TID_InitInvestorPositionChange = 0x0000500E;


	const MessageID TID_RspError = 0x00005100;
	/************************************LoginServiceLink************************************/
		//用户登录请求
	const MessageID TID_ReqUserLogin = 0x00005101;
	//用户登录应答
	const MessageID TID_RspUserLogin = TID_ReqUserLogin + 1;
	//用户录出请求
	const MessageID TID_ReqUserLogout = 0x00005103;
	//用户录出应答
	const MessageID TID_RspUserLogout = TID_ReqUserLogout + 1;
	//强制用户录出请求（交易员用）
	const MessageID TID_ReqForceLogout = 0x00005105;
	//强制用户录出应答
	const MessageID TID_RspForceLogout = TID_ReqForceLogout + 1;
	//用户密码修改请求
	const MessageID TID_ReqUserPasswordUpdate = 0x00005107;
	//用户密码修改应答
	const MessageID TID_RspUserPasswordUpdate = TID_ReqUserPasswordUpdate + 1;

	//报单录入请求
	const MessageID TID_ReqOrderInsert = 0x00005201;
	//报单录入应答
	const MessageID TID_RspOrderInsert = TID_ReqOrderInsert + 1;
	//报单操作请求
	const MessageID TID_ReqOrderAction = 0x00005204;
	//报单操作应答
	const MessageID TID_RspOrderAction = TID_ReqOrderAction + 1;

	const MessageID TID_NotifyInstrumentStatus = 0x00005501;
	//合约信息通知
	const MessageID TID_NotifyInstrumentInfo = 0x00005502;
	//递延费通知
	const MessageID TID_RtnDeferFee = 0x00005503;
	//报单录入错误通知（交易所录入失败）
	const MessageID TID_ErrOrderInsert = 0x00005504;
	//报单错误通知(交易所撤单失败)
	const MessageID TID_ErrOrderAction = 0x00005505;

	//报单通知
	const MessageID TID_NotifyOrder = 0x00005506;
	//成交通知
	const MessageID TID_NotifyTrade = 0x00005507;
	//席位成功登陆通知
	const MessageID TID_NotifySeatLoginSuccess = 0x00005508;
	//席位断开状态通知
	const MessageID TID_NotifySeatDisconnected = 0x00005509;
	//席位流水推送结束
	const MessageID TID_NotifyInitFinish = 0x0000550A;
	//内存表变化通知
	const MessageID TID_NtfMemtableChange = 0x0000550B;
	//风险度计算通知
	const MessageID TID_NotifyDynamicRiskCalculate = 0x0000550C;
	//现货出入库
	const MessageID TID_NotifySpotPosition = 0x0000550D;

	//内部行情通知
	const MessageID TID_IntlRtnDepthMarketData = 0x00008500;
	//深度行情通知
	const MessageID TID_RtnDepthMarketData = 0x00008501;

	//合约查询请求
	const MessageID TID_ReqQryInstrument = 0x00005301;
	//合约查询应答
	const MessageID TID_RspQryInstrument = TID_ReqQryInstrument + 1;
	//投资者资金账户查询请求
	const MessageID TID_ReqQryInvestorAccount = 0x00005303;
	//投资者资金账户查询应答
	const MessageID TID_RspQryInvestorAccount = TID_ReqQryInvestorAccount + 1;
	//投资者持仓查询请求
	const MessageID TID_ReqQryInvestorPosition = 0x00005305;
	//投资者持仓查询应答
	const MessageID TID_RspQryInvestorPosition = TID_ReqQryInvestorPosition + 1;
	//投资者保证金率查询请求
	const MessageID TID_ReqQryInvestorMargin = 0x00005307;
	//投资者保证金率查询应答
	const MessageID TID_RspQryInvestorMargin = TID_ReqQryInvestorMargin + 1;
	//投资者手续费率查询请求
	const MessageID TID_ReqQryInvestorFee = 0x00005309;
	//投资者手续费率查询应答
	const MessageID TID_RspQryInvestorFee = TID_ReqQryInvestorFee + 1;
	//投资者报单查询请求
	const MessageID TID_ReqQryInvestorOrder = 0x0000530B;
	//投资者报单查询应答
	const MessageID TID_RspQryInvestorOrder = TID_ReqQryInvestorOrder + 1;
	//投资者成交查询请求
	const MessageID TID_ReqQryInvestorTrade = 0x0000530D;
	//投资者成交查询应答
	const MessageID TID_RspQryInvestorTrade = TID_ReqQryInvestorTrade + 1;



	namespace Auth {
		using Login = auth_protocol<TID_ReqUserLogin, TID_RspUserLogin, data::CUserLogin>;
		using Logout = auth_protocol<TID_ReqUserLogout, TID_RspUserLogout, data::CUserLogout>;
		using ForceLogout = auth_protocol<TID_ReqForceLogout, TID_RspForceLogout, data::CForceInvestorExit>;
		using PasswordUpdate = auth_protocol<TID_ReqUserPasswordUpdate, TID_RspUserPasswordUpdate, data::CUserPasswordUpdate>;

		inline bool IsMine(MessageID id) {
			return id == Login::RequestID || id == Login::ResponseID ||
				id == Logout::RequestID || id == Logout::ResponseID ||
				id == ForceLogout::RequestID || id == ForceLogout::ResponseID ||
				id == PasswordUpdate::RequestID || id == PasswordUpdate::ResponseID;
		}
	}

	namespace Sync {
		using NotifyOrder = internal_protocol<TID_NotifyOrder, data::COrder>;
		using NotifyTrade = internal_protocol<TID_NotifyTrade, data::CTrade>;
		using NotifySeatLoginSuccess = internal_protocol<TID_NotifySeatLoginSuccess, data::CSeatLogin>;
		using NotifySeatDisconnect = internal_protocol<TID_NotifySeatDisconnected, data::CSeatLogin>;
		using NotifyInitFinish = internal_protocol<TID_NotifyInitFinish, data::CRspInfo>;
		using NotifySpotPosition = internal_protocol<TID_NotifySpotPosition, data::CSpotPositionNotify>;

		inline bool IsMine(MessageID id) {
			return id == NotifySeatLoginSuccess::ID ||
				id == NotifySeatDisconnect::ID ||
				id == NotifyOrder::ID ||
				id == NotifyTrade::ID ||
				id == NotifySpotPosition::ID ||
				id == NotifyInitFinish::ID;
		}
	}

	namespace Query {
		using Account = query_protocol<TID_ReqQryInvestorAccount, TID_RspQryInvestorAccount, data::CQryInvestorAccount, data::CInvestorAccount>;
		using Fee = query_protocol<TID_ReqQryInvestorFee, TID_RspQryInvestorFee, data::CQryInvestorFee, data::CInvestorFee>;
		using Margin = query_protocol<TID_ReqQryInvestorMargin, TID_RspQryInvestorMargin, data::CQryInvestorMargin, data::CInvestorMargin>;
		using Position = query_protocol<TID_ReqQryInvestorPosition, TID_RspQryInvestorPosition, data::CQryInvestorPosition, data::CInvestorPosition>;
		using Order = query_protocol<TID_ReqQryInvestorOrder, TID_RspQryInvestorOrder, data::CQryInvestorOrder, data::COrder>;
		using Trade = query_protocol<TID_ReqQryInvestorTrade, TID_RspQryInvestorTrade, data::CQryInvestorTrade, data::CTrade>;
		using Instrument = query_protocol<TID_ReqQryInstrument, TID_RspQryInstrument, data::CQryInstrument, data::CInstrument>;

		inline MessageID GetResponseID(MessageID RequestID) {
			switch (RequestID) {
			case Account::RequestID:
				return Account::ResponseID;
			case Fee::RequestID:
				return Fee::ResponseID;
			case Margin::RequestID:
				return Margin::ResponseID;
			case Position::RequestID:
				return Position::ResponseID;
			case Order::RequestID:
				return Order::ResponseID;
			case Trade::RequestID:
				return Trade::ResponseID;
			case Instrument::RequestID:
				return Instrument::ResponseID;
			default:
				return 0;
			}
		}

		inline bool IsMine(MessageID id) {
			return id == Account::RequestID || id == Account::ResponseID ||
				id == Instrument::RequestID || id == Instrument::ResponseID ||
				id == Fee::RequestID || id == Fee::ResponseID ||
				id == Margin::RequestID || id == Margin::ResponseID ||
				id == Position::RequestID || id == Position::ResponseID ||
				id == Order::RequestID || id == Order::ResponseID ||
				id == Trade::RequestID || id == Trade::ResponseID;
		}
	}

	namespace Trading {
		using OrderInsert = trade_protocol<TID_ReqOrderInsert, TID_RspOrderInsert, data::COrder, data::COrder>;
		using OrderAction = trade_protocol<TID_ReqOrderAction, TID_RspOrderAction, data::COrderAction, data::COrderAction>;

		inline MessageID GetResponseID(MessageID RequestID) {
			switch (RequestID) {
			case OrderInsert::RequestID:
				return OrderInsert::ResponseID;
			case OrderAction::RequestID:
				return OrderAction::ResponseID;
			default:
				return 0;
			}
		}

		inline bool IsMine(MessageID id) {
			return id == OrderInsert::RequestID || id == OrderInsert::ResponseID ||
				id == OrderAction::RequestID || id == OrderAction::ResponseID;
		}

		inline bool IsMineReq(MessageID id) {
			return id == OrderInsert::RequestID ||id == OrderAction::RequestID;
		}
	}

	namespace Notify {
		using Order = notify_protocol<TID_NotifyOrder, data::COrder>;
		using Trade = notify_protocol<TID_NotifyTrade, data::CTrade>;
		using InsertError = notify_protocol<TID_ErrOrderInsert, data::COrder>;
		using ActionError = notify_protocol<TID_ErrOrderAction, data::COrderAction>;

		inline bool IsMine(MessageID id) {
			return id == Order::ID ||
				id == Trade::ID ||
				id == InsertError::ID ||
				id == ActionError::ID;
		}
	}

	namespace Market {
	}

	const MessageID TID_OpBase = 0x00005400;
	const MessageID TID_ReqOpChangeInvestorBalance =	TID_OpBase + 1;
	const MessageID TID_RspOpChangeInvestorBalance =	TID_OpBase + 2;
	const MessageID TID_ReqOpQryInvestors =				TID_OpBase + 3;
	const MessageID TID_RspOpQryInvestors =				TID_OpBase + 4;
	const MessageID TID_ReqOpQryInvestorAccount =		TID_OpBase + 5;
	const MessageID TID_RspOpQryInvestorAccount =		TID_OpBase + 6;
	const MessageID TID_ReqOpQryInvestorPosition =		TID_OpBase + 7;
	const MessageID TID_RspOpQryInvestorPosition =		TID_OpBase + 8;
	const MessageID TID_ReqOpQryBalanceChange =			TID_OpBase + 9;
	const MessageID TID_RspOpQryBalanceChange =			TID_OpBase + 10;
	const MessageID TID_ReqOpQryPositionChange =		TID_OpBase + 11;
	const MessageID TID_RspOpQryPositionChange =		TID_OpBase + 12;

	namespace Op {
		using ChangeInvestorBalance = op_protocol<TID_ReqOpChangeInvestorBalance, TID_RspOpChangeInvestorBalance, data::COpInvestorBalance, data::CInvestorAccount>;
		using QueryInvestors = op_protocol<TID_ReqOpQryInvestors, TID_RspOpQryInvestors, data::COpQryInvestors, data::CInvestorInfo>;
		using QueryInvestorAccount = op_protocol<TID_ReqOpQryInvestorAccount, TID_RspOpQryInvestorAccount, data::COpQryInvestorAccount, data::CInvestorAccount>;
		using QueryInvestorPosition = op_protocol<TID_ReqOpQryInvestorPosition, TID_RspOpQryInvestorPosition, data::COpQryInvestorPosition, data::CInvestorPosition>;
		using QueryInvestorBalanceChange = op_protocol<TID_ReqOpQryBalanceChange, TID_RspOpQryBalanceChange, data::COpQryInvestorBalanceChange, data::CBalanceChange>;
		using QueryInvestorPositionChange = op_protocol<TID_ReqOpQryPositionChange, TID_RspOpQryPositionChange, data::COpQryInvestorPositionChange, data::CPositionChange>;

		inline MessageID GetResponseID(MessageID RequestID) {
			switch (RequestID) {
			case ChangeInvestorBalance::RequestID:
				return ChangeInvestorBalance::ResponseID;
			case QueryInvestors::RequestID:
				return QueryInvestors::ResponseID;
			case QueryInvestorAccount::RequestID:
				return QueryInvestorAccount::ResponseID;
			case QueryInvestorPosition::RequestID:
				return QueryInvestorPosition::ResponseID;
			case QueryInvestorBalanceChange::RequestID:
				return QueryInvestorBalanceChange::ResponseID;
			case QueryInvestorPositionChange::RequestID:
				return QueryInvestorPositionChange::ResponseID;
			default:
				return 0;
			}
		}

		inline bool IsMine(MessageID id) {
			return id == ChangeInvestorBalance::RequestID || id == ChangeInvestorBalance::ResponseID ||
				id == QueryInvestors::RequestID || id == QueryInvestors::ResponseID ||
				id == QueryInvestorAccount::RequestID || id == QueryInvestorAccount::ResponseID ||
				id == QueryInvestorPosition::RequestID || id == QueryInvestorPosition::ResponseID ||
				id == QueryInvestorBalanceChange::RequestID || id == QueryInvestorBalanceChange::ResponseID ||
				id == QueryInvestorPositionChange::RequestID || id == QueryInvestorPositionChange::ResponseID;
		}

		inline bool IsMineReq(MessageID id) {
			return id == ChangeInvestorBalance::RequestID || 
				id == QueryInvestors::RequestID ||
				id == QueryInvestorAccount::RequestID ||
				id == QueryInvestorPosition::RequestID ||
				id == QueryInvestorBalanceChange::RequestID ||
				id == QueryInvestorPositionChange::RequestID;
		}
	}
	namespace Public {
		using InstrumentStatus = public_protocol<TID_NotifyInstrumentStatus, data::CInstrumentStatus>;
	}

	namespace Internal {
		using InitSeatInfo = internal_protocol<TID_InitSeatInfo, data::CSeatInfo>;
		using InitInstrument = internal_protocol<TID_InitInstrument, data::CInstrument>;
		using InitInvestorInfo = internal_protocol<TID_InitInvestorInfo, data::CInvestorInfo>;
		using InitInvestorAccount = internal_protocol<TID_InitInvestorAccount, data::CInvestorAccount>;
		using InitInvestorFee = internal_protocol<TID_InitInvestorFee, data::CInvestorFee>;
		using InitInvestorMargin = internal_protocol<TID_InitInvestorMargin, data::CInvestorMargin>;
		using InitInvestorPosition = internal_protocol<TID_InitInvestorPosition, data::CInvestorPosition>;
		using InitOperatorInfo = internal_protocol<TID_InitOperatorInfo, data::COperatorInfo>;
		using InitOperatorInvestor = internal_protocol<TID_InitOperatorInvestor, data::COperatorInvestor>;
		using InitOrder = internal_protocol<TID_InitOrder, data::COrder>;
		using InitTrade = internal_protocol<TID_InitTrade, data::CTrade>;
		using InitBalanceChange = internal_protocol<TID_InitInvestorBalanceChange, data::CBalanceChange>;
		using InitPositionChange = internal_protocol<TID_InitInvestorPositionChange, data::CPositionChange>;

		using InstrumentStatus = internal_protocol<TID_NotifyInstrumentStatus, data::CInstrumentStatus>;
		using InstrumentInfo = internal_protocol<TID_NotifyInstrumentInfo, data::CInstrument>;
		using SeatLoginSuccess = internal_protocol<TID_NotifySeatLoginSuccess, data::CSeatLogin>;
		using SeatDisconnected = internal_protocol<TID_NotifySeatDisconnected, data::CSeatLogin>;
		using Order = internal_protocol<TID_NotifyOrder, data::COrder>;
		using Trade = internal_protocol<TID_NotifyTrade, data::CTrade>;
		using SpotPosition = internal_protocol<TID_NotifySpotPosition, data::CSpotPositionNotify>;
		using DeferFee = internal_protocol<TID_RtnDeferFee, data::CDeferFee>;
 		using InsertError = internal_protocol<TID_ErrOrderInsert, data::COrder>;
 		using ActionError = internal_protocol<TID_ErrOrderAction, data::COrderAction>;
		using DepthMarket = internal_protocol<TID_IntlRtnDepthMarketData, data::CDepthMarketData>;
		using InsertRsp = internal_protocol<TID_RspOrderInsert, data::COrder>;
		using ActionRsp = internal_protocol<TID_RspOrderAction, data::COrderAction>;

		inline bool IsInit(MessageID id) {
			return id == InitSeatInfo::ID ||
				id == InitInstrument::ID ||
				id == InitInvestorInfo::ID ||
				id == InitInvestorAccount::ID ||
				id == InitInvestorFee::ID ||
				id == InitInvestorMargin::ID ||
				id == InitInvestorPosition::ID ||
				id == InitOperatorInfo::ID ||
				id == InitOperatorInvestor::ID ||
				id == InitOrder::ID ||
				id == InitTrade::ID ||
				id == InitBalanceChange::ID ||
				id == InitPositionChange::ID;
		}

		inline bool IsExchange(MessageID id) {
			return id == InstrumentStatus::ID ||
				id == SeatLoginSuccess::ID ||
				id == SeatDisconnected::ID ||
				id == Order::ID ||
				id == Trade::ID ||
				id == SpotPosition::ID ||
				id == InsertRsp::ID ||
				id == ActionRsp::ID ||
				id == DeferFee::ID;
		}

		inline bool IsMine(MessageID id) {
			return IsInit(id) || IsExchange(id);
		}
	}

	template<class T, bool>
	struct InvestorIDPicker;

	template<class T>
	struct InvestorIDPicker<T, false> {
		static char const* get(T const&) { return ""; }
	};
	template<class T>
	struct InvestorIDPicker<T, true> {
		static char const* get(T const& t) { return t.InvestorID.value(); }
	};

	template<typename Protocol>
	void reply(IFlow* flow, message const& reqMsg, typename Protocol::ResponseType const& response, Error error = NoError) {
		data::CRspInfo rspInfo;
		rspInfo.ErrorID = error.Code;
		rspInfo.ErrorMsg = error.Msg;
		message msg;
		msg.header(Protocol::StreamType, Protocol::ResponseID, reqMsg.request_id(), reqMsg.session_id());
		if (rspInfo.ErrorID != 0) {
			msg.rsp_body(rspInfo);
		} else {
			msg.rsp_body(rspInfo, response);
		}
		flow->Append(msg);
	}

	template<typename Protocol>
	void reply_error_only(IFlow* flow, message const& reqMsg, Error error) {
		data::CRspInfo rspInfo;
		rspInfo.ErrorID = error.Code;
		rspInfo.ErrorMsg = error.Msg;
		message msg;
		msg.header(Protocol::StreamType, Protocol::ResponseID, reqMsg.request_id(), reqMsg.session_id());
		msg.rsp_body(rspInfo);
		flow->Append(msg);
	}

	template<typename Protocol>
	void reply_success_only(IFlow* flow, message const& reqMsg) {
		data::CRspInfo rspInfo;
		rspInfo.ErrorID = 0;
		rspInfo.ErrorMsg = "Success";
		message msg;
		msg.header(Protocol::StreamType, Protocol::ResponseID, reqMsg.request_id(), reqMsg.session_id());
		msg.rsp_body(rspInfo);
		flow->Append(msg);
	}

	template<typename Protocol>
	void reply_success(IFlow* flow, message const& reqMsg, typename Protocol::ResponseType const& response, bool isLast = true) {
		data::CRspInfo rspInfo;
		rspInfo.ErrorID = 0;
		rspInfo.ErrorMsg = "Success";
		message msg;
		msg.header(Protocol::StreamType, Protocol::ResponseID, reqMsg.request_id(), reqMsg.session_id(), isLast);
		msg.rsp_body(rspInfo, response);
		flow->Append(msg);
	}

	template<typename Protocol>
	void notify(IFlow* flow, typename Protocol::Type const& data) {
		message msg;
		msg.investor_id(InvestorIDPicker<Protocol::Type,Has_InvestorID<Protocol::Type>::value>::get(data));
		msg.header(Protocol::StreamType, Protocol::ID, -1);
		msg.notify_body(data);
		flow->Append(msg);
	}

	template<typename ptl>
	void notify(IFlow* flow, data::CRspInfo const& rsp, typename ptl::Type const& data) {
		message msg;
		msg.investor_id(InvestorIDPicker < ptl::Type, Has_InvestorID<ptl::Type>::value> ::get(data));
		msg.header(ptl::StreamType, ptl::ID, -1);
		msg.notify_error_body(rsp,data);
		flow->Append(msg);
	}


	template<typename Protocol>
	void notify(IFlow* flow, message const& reqMsg, typename Protocol::Type const& data) {
		message msg;
		msg.header(Protocol::StreamType, Protocol::ID, reqMsg.request_id(), reqMsg.session_id());
		msg.notify_body(data);
		flow->Append(msg);
	}

	template<typename Protocol, class Rsp>
	void notify_error(IFlow* flow, message const& reqMsg, Rsp const& rsp, typename Protocol::Type const& data) {
		message msg;
		msg.header(Protocol::StreamType, Protocol::ID, reqMsg.request_id(), reqMsg.session_id());
		msg.notify_error_body(rsp, data);
		flow->Append(msg);
	}

	typedef struct {
		type::SessionID	SessionID;		//会话编号
		type::RequestID  RequestID;		//请求编号
	}	TSessionRef;

	class IZTPServiceLink {
	public:
		virtual ~IZTPServiceLink() {};
		virtual bool HandleMessage(message& msg, TSessionRef const& sessionRef) = 0;
	};

	//IServiceRsponser是一个ZTP应答发送接口
	class IServiceRsponser {
	public:
		virtual ~IServiceRsponser() {};
		virtual void SendResponse(message& msg, type::SessionID dwSessionID = 0) = 0;
	};

	inline data::CRspInfo from_error(Error const& error) {
		return data::CRspInfo{ error.Code, type::ErrorMsg(error.Msg) };
	}

	inline void fill_rsp(data::CRspInfo& rsp, Error const& error) {
		rsp.ErrorID = error.Code;
		rsp.ErrorMsg = error.Msg;
	}
}
