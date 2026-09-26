#pragma once
#include "stdafx.h"
#include "message.hpp"
#include "ZTPServer.h"
#include "ztp_server.hpp"
#include <fmt/format.h>

#include "ServiceDef.h"

namespace ztp {
	struct ZTPServer::Impl {
		asio::io_context io_context;
		asio::steady_timer timer;
		session_mgr session_mgr;
		ztp_server server;
		Impl(int port, IZTPServerDispatcher* dis) :
		timer(io_context),server(io_context, tcp::endpoint(tcp::v4(), port), session_mgr, dis){

		}
	};
	ZTPServer::ZTPServer(OperatorRepository& operRepo, InvestorRepository& investorRepo,int port)
		:operRepo_(operRepo),investorRepo_(investorRepo) {
		impl_ = std::make_shared<Impl>(port, this);
		m_pSeriesFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x200000);
		m_seriesReader.AttachFlow(m_pSeriesFlow, 0);
		ztpPackage_.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
	}

	struct LoginedSession {
		bool isLogined = false;
		type::UserType UserType = type::UserType::Investor;
		type::LoginID LoginID;
	};

	void ZTPServer::OnAuthReq(const message& msg) {
		fmt::print("received auth request {} ", msg.func_id());
		switch (msg.func_id()) {
		case Auth::Login::RequestID: {
			login(msg);
			break;
		}
		case Auth::Logout::RequestID: {
			logout(msg);
			break;
		}
		default:
			break;
		}
	}
	void ZTPServer::logout(const ztp::message& msg) {
		data::CUserLogout logout;
		msg.retrieve(logout);

		auto* session_pointer = impl_->session_mgr.get_session(msg.session_id());
		if (session_pointer != nullptr) {
			auto& loginInfo = session_pointer->get_hook<LoginedSession>();
			if (loginInfo.LoginID == logout.LoginID && loginInfo.isLogined) {
				session_pointer->set_hook(LoginedSession{});
				data::CRspInfo rspInfo{ NoError.Code, type::ErrorMsg(NoError.Msg) };
				message rsp_msg;
				rsp_msg.rsp_body(rspInfo, logout);
				AuthRsp(rsp_msg);
			}
		}
	}
	void ZTPServer::login(const ztp::message& msg) {
		data::CUserLogin userLogin;
		msg.retrieve(userLogin);
		type::Password password;
		if (userLogin.UserType == type::UserType::Investor) {
			auto* investor = investorRepo_.GetInfo(type::InvestorID(userLogin.LoginID.value()));
			if (investor != nullptr) {
				password = investor->Password;
			}
		} else {
			auto* oper = operRepo_.Get(userLogin.BrokerID, type::OperatorID(userLogin.LoginID.value()));
			if (oper != nullptr) {
				password = oper->Password;
			}
		}
		using Proto = Auth::Login;
		data::CRspInfo rspInfo{ InvalidLoginError.Code, type::ErrorMsg(InvalidLoginError.Msg) };
		message rsp_msg;
		rsp_msg.header(Proto::StreamType, Proto::ResponseID, msg.request_id(), msg.session_id(), true, msg.investor_id());
		if (password == userLogin.Password) {
			auto* session_pointer = impl_->session_mgr.get_session(msg.session_id());
			if (session_pointer != nullptr) {
				session_pointer->set_hook(LoginedSession{ true,userLogin.UserType, userLogin.LoginID });
				rspInfo.ErrorID = NoError.Code;
				rspInfo.ErrorMsg = NoError.Msg;
				rsp_msg.rsp_body(rspInfo, userLogin);
				AuthRsp(rsp_msg);
				return;
			}
		}
		rsp_msg.rsp_body<Proto::ResponseType::ID>(rspInfo);
		AuthRsp(rsp_msg);
	}
	template<class Proto>
	void ZTPServer::forward(const message& msg) {
		Proto::RequestType balanceReq;
		msg.retrieve(balanceReq);

		message rsp_msg;
		rsp_msg.header(Proto::StreamType, Proto::ResponseID, msg.request_id(), msg.session_id(), true, msg.investor_id());

		auto* session_pointer = impl_->session_mgr.get_session(msg.session_id());
		if (session_pointer != nullptr) {
			auto& loginInfo = session_pointer->get_hook<LoginedSession>();
			if (loginInfo.LoginID == balanceReq.OperatorID.value() && loginInfo.isLogined) {
				appendReq<Proto>(msg, balanceReq);
			} else {
				data::CRspInfo rspInfo{ InvalidLoginError.Code, type::ErrorMsg(InvalidLoginError.Msg) };
				rsp_msg.rsp_body<Proto::ResponseType::ID>(rspInfo);
				OpRsp(rsp_msg);
			}
		}
	}
	void ZTPServer::OnOpReq(const message& msg) {
		fmt::print("received Op request {} \n", msg.func_id());
		switch (msg.func_id()) {
		case Op::ChangeInvestorBalance::RequestID: {
			using Proto = Op::ChangeInvestorBalance;
			forward<Proto>(msg);
			break;
		}
		case Op::QueryInvestors::RequestID: {
			using Proto = Op::QueryInvestors;
			forward<Proto>(msg);
			break;
		}
		case Op::QueryInvestorAccount::RequestID: {
			using Proto = Op::QueryInvestorAccount;
			forward<Proto>(msg);
			break;
		}
		case Op::QueryInvestorPosition::RequestID: {
			using Proto = Op::QueryInvestorPosition;
			forward<Proto>(msg);
			break;
		}
		case Op::QueryInvestorBalanceChange::RequestID: {
			using Proto = Op::QueryInvestorBalanceChange;
			forward<Proto>(msg);
			break;
		}
		case Op::QueryInvestorPositionChange::RequestID: {
			using Proto = Op::QueryInvestorPositionChange;
			forward<Proto>(msg);
			break;
		}
		default:
			break;
		}

	}
	template<class Proto>
	void ZTPServer::appendReq(const message& msg, typename Proto::RequestType& req) {
		ztpPackage_.PrepareRequest(Proto::RequestID);
		ztpPackage_.SetSessionID(msg.session_id());
		ztpPackage_.SetRequestID(msg.request_id());
		ztpPackage_.SetGroupID(ZTP_GROUP);
		ztpPackage_.AddField(req);
		ztpPackage_.MakePackage();
		m_pSeriesFlow->Append(ztpPackage_.GetAddress(), ztpPackage_.GetLength());
	}

	void ZTPServer::OnTradeReq(const message& msg) {

	}
	void ZTPServer::OnQueryReq(const message& msg) {
	}



	void ZTPServer::AuthRsp(const message& msg) {
		if (msg.is_auth()) {
			impl_->server.deliver(msg);
		}
	}
	void ZTPServer::TradeRsp(const message& msg) {
		if (msg.is_trade()) {
			impl_->server.deliver(msg);
		}
	}

	void ZTPServer::OpRsp(const message& msg) {
		if (msg.is_op()) {
			impl_->server.deliver(msg);
		}
	}
	void ZTPServer::PublicNotify(const message& msg) {
		if (msg.is_public()) {
			impl_->server.deliver_to_all(msg);
		}
	}
	void ZTPServer::MarketNotify(const message& msg) {
		if (msg.is_market()) {
			impl_->server.deliver(msg);
		}
	}
	void ZTPServer::QueryRsp(const message& msg) {
		if (msg.is_query()) {
			impl_->server.deliver(msg);
		}
	}
	void ZTPServer::PrivateNotify(const message& msg) {
		if (msg.is_private()) {
			impl_->server.deliver(msg);
		}
	}
	void ZTPServer::Close() {}
	void ZTPServer::Run() {
		impl_->io_context.run();
	}
}