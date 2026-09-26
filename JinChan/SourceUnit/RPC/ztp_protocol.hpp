#pragma once
#include "message.hpp"

namespace ztp {

	static constexpr char RequestPrefix = 'R';
	static constexpr char ResponsePrefix = 'S';
	static constexpr char PublishPrefix = 'N';

	template<MessageID ReqID, MessageID RspID, class Req, class Rsp = Req>
	struct auth_protocol {
		static constexpr StreamType StreamType = StreamType::Auth;
		static constexpr MessageID RequestID = ReqID;
		static constexpr MessageID ResponseID = RspID;
		typedef Req RequestType;
		typedef Rsp ResponseType;
	};

	template<MessageID ReqID, MessageID RspID, class Req, class Rsp>
	struct trade_protocol {
		static constexpr StreamType StreamType = StreamType::Trade;
		static constexpr MessageID RequestID = ReqID;
		static constexpr MessageID ResponseID = RspID;
		typedef Req RequestType;
		typedef Rsp ResponseType;
	};

	template<MessageID ReqID, MessageID RspID, class Req, class Rsp>
	struct op_protocol {
		static constexpr StreamType StreamType = StreamType::OP;
		static constexpr MessageID RequestID = ReqID;
		static constexpr MessageID ResponseID = RspID;
		typedef Req RequestType;
		typedef Rsp ResponseType;
	};

	template<MessageID ReqID, MessageID RspID, class Req, class Rsp>
	struct query_protocol {
		static constexpr StreamType StreamType = StreamType::Query;
		static constexpr MessageID RequestID = ReqID;
		static constexpr MessageID ResponseID = RspID;
		typedef Req RequestType;
		typedef Rsp ResponseType;
	};
	template<MessageID NotifyID, class Notify>
	struct notify_protocol {
		static constexpr StreamType StreamType = StreamType::Private;
		static constexpr MessageID ID = NotifyID;
		typedef Notify Type;
	};
	template<MessageID ReqID, MessageID RspID, class Req, class Rsp>
	struct market_protocol {
		static constexpr StreamType StreamType = StreamType::Trade;
		static constexpr MessageID RequestID = ReqID;
		static constexpr MessageID ResponseID = RspID;
		typedef Req RequestType;
		typedef Rsp ResponseType;
	};

	template<MessageID NotifyID, class Msg>
	struct public_protocol {
		static constexpr StreamType StreamType = StreamType::Public;
		static constexpr MessageID ID = NotifyID;
		typedef Msg Type;
	};

	template<MessageID NotifyID, class Msg>
	struct internal_protocol {
		static constexpr StreamType StreamType = StreamType::Internal;
		static constexpr MessageID ID = NotifyID;
		typedef Msg Type;
	};
}