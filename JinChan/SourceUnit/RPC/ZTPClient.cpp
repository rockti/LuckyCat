#include "ZTPClient.h"
#include "ztp_client.hpp"
#include <fmt/format.h>
#include "ServiceDef.h"

namespace ztp {
	using asio::ip::tcp;

// 	struct ZTPMarketClient::Impl {
// 		asio::io_context io_context;
// 		tcp::resolver resolver;
// 		tcp::resolver::results_type endpoints;
// 		ztp_client client;
// 		Impl(std::string const& ip, std::string const& port, ztp_dispatcher* dispatcher) :
// 			resolver(io_context),
// 			endpoints(resolver.resolve(ip, port)),
// 			client(io_context, endpoints, dispatcher) {
// 		}
// 	};
// 	ZTPMarketClient::ZTPMarketClient(std::string const& ip, int port) {
// 		impl_ = std::make_shared<Impl>(ip, fmt::format("{}", port), this);
// 	}
// 
// 	void ZTPMarketClient::Start() {
// 		impl_->client.connect();
// 		t_ = std::thread([this]() {
// 			for (;;) {
// 				impl_->io_context.run();
// 				impl_->io_context.reset();
// 				std::this_thread::sleep_for(std::chrono::seconds(5));
// 				impl_->client.reconnect();
// 			}
// 			});
// 	}
// 	void ZTPMarketClient::Join() {
// 		t_.join();
// 	}
// 	void ZTPMarketClient::AuthReq(const message& msg) {
// 	}
// 	void ZTPMarketClient::OnAuthRsp(const message& msg) {
// 	}
// 	void ZTPMarketClient::OnPublicNotify(const message& msg) {
// 	}
// 	void ZTPMarketClient::OnMarketNotify(const message& msg) {
// 		data::CDepthMarketData market;
// 		msg.retrieve(market);
// 		fmt::print("market : Instrument[{}] Ask1[{} {}]", market.InstrumentID, market.AskPrice1, market.AskVolume1);
// 
// 	}
// 	void ZTPMarketClient::on_disconnected() {
// 	}
// 	void ZTPMarketClient::on_connected() {
// 	}

	struct ZTPTradeClient::Impl {
		asio::io_context io_context;
		tcp::resolver resolver;
		tcp::resolver::results_type endpoints;
		ztp_client client;
		Impl(std::string const& ip, std::string const& port, ztp_dispatcher* dispatcher):
			resolver(io_context),
			endpoints(resolver.resolve(ip, port)),
			client(io_context, endpoints, dispatcher){
		}
		void reset_endpoints(std::string const& ip, int port) {
			auto sPort = fmt::format("{}", port);
			client.reset_endpoints(tcp::resolver::results_type(resolver.resolve(ip, sPort)));
		}
	};

	ZTPTradeClient::ZTPTradeClient(type::BrokerID const& brokerID,std::string const& ip, int port, TradeHandler* handler)
		:broker_(brokerID),handler_(handler) {
		impl_ = std::make_shared<Impl>(ip, fmt::format("{}", port), this);
	}
	void ZTPTradeClient::SetServer(type::BrokerID const& brokerID, std::string const& ip, int port) {
		broker_ = brokerID;
		impl_->reset_endpoints(ip, port);
	}
	void ZTPTradeClient::Start() {
		impl_->client.connect();
		t_ = std::thread([this]() {
			for (;;) {
				impl_->io_context.run();
				impl_->io_context.reset();
				std::this_thread::sleep_for(std::chrono::seconds(5));
//				impl_->client.reconnect();
			}
		});
	}
	void ZTPTradeClient::Close() {
		impl_->client.close();
	}

	void ZTPTradeClient::Connect() {
		if (!IsConnected()) {
			impl_->client.connect();
		}
	}
	void ZTPTradeClient::Join() {
		t_.join();
	}

	int ZTPTradeClient::QueryInvestors() {
		using Proto = Op::QueryInvestors;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		msg.req_body(req);

		OpReq(msg);
		return request_id_;
	}

	int ZTPTradeClient::Deposit(type::InvestorID const& investor, double amount) {
		using Proto = Op::ChangeInvestorBalance;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		req.InvestorID = investor;
		req.Deposit = amount;
		msg.req_body(req);
		OpReq(msg);
		return request_id_;
	}

	int ZTPTradeClient::Withdraw(type::InvestorID const& investor, double amount) {
		using Proto = Op::ChangeInvestorBalance;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		req.InvestorID = investor;
		req.Withdraw = amount;
		msg.req_body(req);
		OpReq(msg);
		return request_id_;
	}

	int ZTPTradeClient::QueryAccount(type::InvestorID const& investor) {
		using Proto = Op::QueryInvestorAccount;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		req.InvestorID = investor;
		msg.req_body(req);
		OpReq(msg);
		return request_id_;
	}

	int ZTPTradeClient::QueryPosition(type::InvestorID const& investor) {
		using Proto = Op::QueryInvestorPosition;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		req.InvestorID = investor;
		msg.req_body(req);
		OpReq(msg);
		return request_id_;
	}

	int ZTPTradeClient::QueryBalanceChange(type::InvestorID const& investor) {
		using Proto = Op::QueryInvestorBalanceChange;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		req.InvestorID = investor;
		msg.req_body(req);
		OpReq(msg);
		return request_id_;
	}

	int ZTPTradeClient::QueryPositionChange(type::InvestorID const& investor) {
		using Proto = Op::QueryInvestorPositionChange;
		message msg;
		msg.header(Proto::StreamType, Proto::RequestID, ++request_id_);

		Proto::RequestType req;
		req.BrokerID = login_request_.BrokerID;
		req.OperatorID = login_request_.LoginID.value();
		req.InvestorID = investor;
		msg.req_body(req);
		OpReq(msg);
		return request_id_;
	}


	int ZTPTradeClient::Login() {
		using Proto = Auth::Login;
		message msg;
		msg.header(Proto::StreamType,Proto::RequestID,++request_id_);
		msg.req_body(login_request_);
		AuthReq(msg);
		return request_id_;
	}

	void ZTPTradeClient::AuthReq(const message& msg) {
		if (msg.is_auth()) {
			impl_->client.write(msg);
		}
	}
	void ZTPTradeClient::TradeReq(const message& msg) {
		if (msg.is_trade()) {
			impl_->client.write(msg);
		}
	}
	void ZTPTradeClient::QueryReq(const message& msg) {
		if (msg.is_query()) {
			impl_->client.write(msg);
		}
	}
	void ZTPTradeClient::OpReq(const message& msg) {
		if (msg.is_op()) {
			impl_->client.write(msg);
		}
	}
	void ZTPTradeClient::OnAuthRsp(const message& msg) {
		data::CRspInfo rsp;
		switch (msg.func_id()) {
		case Auth::Login::ResponseID: {
			Auth::Login::ResponseType data;
			msg.retrieve(rsp, data);
			if (rsp.ErrorID == NoError.Code) {
				handler_->OnRspLogin(rsp, &data);
			} else {
				handler_->OnRspLogin(rsp);
			}
		}

		default:
			break;
		}
		handler_->OnAuthRsp(msg);
    }
	void ZTPTradeClient::OnTradeRsp(const message& msg) {
	}
	void ZTPTradeClient::OnQueryRsp(const message& msg) {
	}
	void ZTPTradeClient::OnOpRsp(const message& msg) {
		using namespace Op;
		data::CRspInfo rsp;
		switch (msg.func_id()) {
		case QueryInvestors::ResponseID: {
			QueryInvestors::ResponseType data;
			msg.retrieve(rsp, data);
			if (rsp.ErrorID == NoError.Code) {
				handler_->OnRspQueryInvestors(rsp, &data, msg.is_last());
			} else {
				handler_->OnRspQueryInvestors(rsp);
			}
			break;
		}
		case QueryInvestorAccount::ResponseID: {
			QueryInvestorAccount::ResponseType data;
			msg.retrieve(rsp, data);
			if (rsp.ErrorID == NoError.Code) {
				handler_->OnRspQueryInvestorAccount(rsp, &data);
			} else {
				handler_->OnRspQueryInvestorAccount(rsp);
			}
			break;
		}
		case QueryInvestorPosition::ResponseID: {
			QueryInvestorPosition::ResponseType data;
			msg.retrieve(rsp, data);
			if (rsp.ErrorID == NoError.Code) {
				handler_->OnRspQueryInvestorPosition(rsp, &data, msg.is_last());
			} else {
				handler_->OnRspQueryInvestorPosition(rsp);
			}
			break;
		}
		case ChangeInvestorBalance::ResponseID: {
			ChangeInvestorBalance::ResponseType data;
			msg.retrieve(rsp, data);
			if (rsp.ErrorID == NoError.Code) {
				handler_->OnRspChangeBalance(rsp, &data);
			} else {
				handler_->OnRspChangeBalance(rsp);
			}
			break;
		}
		case QueryInvestorBalanceChange::ResponseID: {
			QueryInvestorBalanceChange::ResponseType data;
			msg.retrieve(rsp, data);
			if (rsp.ErrorID == NoError.Code) {
				handler_->OnRspQueryBalanceChange(rsp, &data, msg.is_last());
			} else {
				handler_->OnRspQueryBalanceChange(rsp);
			}
			break;
		}
		default:
			break;
		}
		handler_->OnOpRsp(msg);
	}
	void ZTPTradeClient::OnPrivateNotify(const message& msg) {
	}
	void ZTPTradeClient::OnPublicNotify(const message& msg) {
	}

	void ZTPTradeClient::on_connected() {
//		login();
		is_connected_ = true;
		handler_->OnConnected(broker_);
	}

	void ZTPTradeClient::on_connect_timeout() {
		handler_->OnConnectTimeout(broker_);
	}

	void ZTPTradeClient::on_disconnected() {
		is_connected_ = false;
		handler_->OnDisconnected(broker_);
	}
}