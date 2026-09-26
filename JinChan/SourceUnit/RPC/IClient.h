#pragma once
#include "message.hpp"
namespace ztp {
	class ztp_dispatcher {
	public:
		virtual void dispatch(const message& msg) = 0;
		virtual void on_connected() = 0;
		virtual void on_connect_timeout() = 0;
		virtual void on_disconnected() = 0;
	};
	class IZTPMarketClient : public ztp_dispatcher {
		virtual void dispatch(const message& msg) override {
			switch (msg.stream_type()){
			case StreamType::Market:
				OnMarketNotify(msg);
				return;
			case StreamType::Auth:
				OnAuthRsp(msg);
				return;
			case StreamType::Public:
				OnPublicNotify(msg);
				return;
			default:
				break;
			}
		}
		virtual void AuthReq(const message& msg) = 0;

		virtual void OnAuthRsp(const message& msg) = 0;
		virtual void OnPublicNotify(const message& msg) = 0;
		virtual void OnMarketNotify(const message& msg) = 0;
	};

	class IZTPTradeClient: public ztp_dispatcher {
	public:
		virtual void dispatch(const message& msg) override {
			switch (msg.stream_type()) {
			case StreamType::Trade:
				OnTradeRsp(msg);
				return;
			case StreamType::Private:
				OnPrivateNotify(msg);
				return;
			case StreamType::Query:
				OnQueryRsp(msg);
				return;
			case StreamType::Auth:
				OnAuthRsp(msg);
				return;
			case StreamType::OP:
				OnOpRsp(msg);
				return;
			case StreamType::Public:
				OnPublicNotify(msg);
				return;
			default:
				break;
			}
		}
		virtual void AuthReq(const message& msg) = 0;
		virtual void TradeReq(const message& msg) = 0;
		virtual void QueryReq(const message& msg) = 0;
		virtual void OpReq(const message& msg) = 0;

		virtual void OnAuthRsp(const message& msg) = 0;
		virtual void OnTradeRsp(const message& msg) = 0;
		virtual void OnOpRsp(const message& msg) = 0;

		virtual void OnQueryRsp(const message& msg) = 0;
		virtual void OnPrivateNotify(const message& msg) = 0;
		virtual void OnPublicNotify(const message& msg) = 0;
	};

}