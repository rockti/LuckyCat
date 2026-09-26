#pragma once
#include "message.hpp"
namespace ztp {

	class IZTPServerDispatcher {
	public:
		virtual void OnAuthReq(const message& msg) = 0;
		virtual void OnTradeReq(const message& msg) = 0;
		virtual void OnQueryReq(const message& msg) = 0;
		virtual void OnOpReq(const message& msg) = 0;
	};

	class IZTPServer {
	public:
		virtual void AuthRsp(const message& msg) = 0;
		virtual void TradeRsp(const message& msg) = 0;
		virtual void OpRsp(const message& msg) = 0;
		virtual void PublicNotify(const message& msg) = 0;
		virtual void MarketNotify(const message& msg) = 0;
		virtual void QueryRsp(const message& msg) = 0;
		virtual void PrivateNotify(const message& msg) = 0;
		virtual void Close() = 0;
	};

}