#pragma once
#include "IClient.h"
#include <string>
#include <memory>
#include <thread>
#include "ServiceDef.h"
namespace ztp {

// 	class ZTPMarketClient : public IZTPMarketClient {
// 	public:
// 		ZTPMarketClient(std::string const& ip, int port);
// 		virtual void AuthReq(const message& msg) override;
// 
// 		virtual void OnAuthRsp(const message& msg) override;
// 		virtual void OnPublicNotify(const message& msg) override;
// 		virtual void OnMarketNotify(const message& msg) override;
// 		virtual void on_connected() override;
// 		virtual void on_disconnected() override;
// 
// 		void Start();
// 		void Join();
// 	private:
// 		struct Impl;
// 		std::thread t_;
// 		std::shared_ptr<Impl> impl_;
// 
// 	};

	class TradeHandler {
	public:
		virtual void OnConnected(type::BrokerID const& brokerID) = 0;
		virtual void OnConnectTimeout(type::BrokerID const& brokerID) = 0;
		virtual void OnDisconnected(type::BrokerID const& brokerID) = 0;
		virtual void OnRspLogin(data::CRspInfo const& rsp, data::CUserLogin const* data = nullptr) = 0;
		virtual void OnRspQueryInvestors(data::CRspInfo const& rsp, data::CInvestorInfo const* data = nullptr, bool isLast = true) = 0;
		virtual void OnRspQueryInvestorAccount(data::CRspInfo const& rsp, data::CInvestorAccount const* data = nullptr) = 0;
		virtual void OnRspQueryInvestorPosition(data::CRspInfo const& rsp, data::CInvestorPosition const* data = nullptr, bool isLast = true) = 0;
		virtual void OnRspChangeBalance(data::CRspInfo const& rsp, data::CInvestorAccount const* data = nullptr) = 0;
		virtual void OnRspQueryBalanceChange(data::CRspInfo const& rsp, data::CBalanceChange const* data = nullptr, bool isLast = true) = 0;

		virtual void OnOpRsp(message const& msg) = 0;
		virtual void OnAuthRsp(message const& msg) = 0;
	};

	class ZTPTradeClient : public IZTPTradeClient {
	public:
		ZTPTradeClient(type::BrokerID const& brokerID,std::string const& ip, int port, TradeHandler* handler);
		char const* GetBroker() const { return broker_.value(); }
		void SetServer(type::BrokerID const& brokerID, std::string const& ip, int port);
		void Start();
		void Close();
		void Connect();
		void Join();
		void SetLogin(data::CUserLogin const& loginReq) {
			login_request_ = loginReq;
		}
		int QueryInvestors();
		int Deposit(type::InvestorID const& investor, double amount);
		int Withdraw(type::InvestorID const& investor, double amount);
		int QueryAccount(type::InvestorID const& investor);
		int QueryPosition(type::InvestorID const& investor);
		int QueryBalanceChange(type::InvestorID const& investor);
		int QueryPositionChange(type::InvestorID const& investor);
		int Login();

		bool IsConnected() const { return is_connected_; }

	private:
		virtual void AuthReq(const message& msg) override;
		virtual void TradeReq(const message& msg) override;
		virtual void QueryReq(const message& msg) override;
		virtual void OpReq(const message& msg) override;

		virtual void OnAuthRsp(const message& msg) override;
		virtual void OnTradeRsp(const message& msg) override;
		virtual void OnQueryRsp(const message& msg) override;
		virtual void OnOpRsp(const message& msg) override;

		virtual void OnPrivateNotify(const message& msg) override;
		virtual void OnPublicNotify(const message& msg) override;

		virtual void on_disconnected() override;
		virtual void on_connected() override;
		virtual void on_connect_timeout() override;

		type::BrokerID broker_;
		TradeHandler* handler_;
		data::CUserLogin login_request_;
		struct Impl;
		std::thread t_;
		std::shared_ptr<Impl> impl_;
		int request_id_ = 0;
		bool is_connected_ = false;;
	};

}