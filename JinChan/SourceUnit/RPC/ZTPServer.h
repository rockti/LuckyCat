#pragma once
#include "IServer.h"
#include <string>
#include <memory>
#include "Repository.h"
#include "BaseExchApi.h"
#include "ZTPPackage.h"
namespace ztp {

	class ZTPServer;

	class ZTPServer : public IZTPServer, public IZTPServerDispatcher, public CSeriesReader {
	public:
		ZTPServer(OperatorRepository& operRepo, InvestorRepository& investorRepo,int port);

		virtual void OnAuthReq(const message& msg) override;
		void logout(const ztp::message& msg);
		void login(const ztp::message& msg);
		virtual void OnTradeReq(const message& msg) override;
		virtual void OnQueryReq(const message& msg) override;
		virtual void OnOpReq(const message& msg) override;


		virtual void AuthRsp(const message& msg) override;
		virtual void TradeRsp(const message& msg) override;
		virtual void QueryRsp(const message& msg) override;
		virtual void OpRsp(const message& msg) override;
		virtual void PublicNotify(const message& msg) override;
		virtual void MarketNotify(const message& msg) override;
		virtual void PrivateNotify(const message& msg) override;
		virtual void Close() override;

		void Run();
	private:
		template<class Proto>
		void appendReq(const message& msg, typename Proto::RequestType& req);

		template<class Proto>
		void forward(const message& msg);

		OperatorRepository& operRepo_;
		InvestorRepository& investorRepo_;
		CZTPPackage ztpPackage_;
		struct Impl;
		std::shared_ptr<Impl> impl_;
	};

}