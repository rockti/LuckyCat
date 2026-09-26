#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <tuple>
namespace ztp {
	enum class StreamType: char {
		Unknown = 0,
		Auth,				// Auth stream for login, logout, password etc
		Trade,				// trade stream for order insert/cancel request/response
		OP,					// OP stream for order insert/cancel request/response
		Market,				// market stream, 
		Public,				// public stream, exchange open/close message etc
		Query,				// query stream for query request/response
		Private,			// private stream for order/trade notification
		Internal,			// internal stream
		Max
	};

	using MessageID = uint32_t;
	const MessageID MaxMessageId = 0xFFFF;

	static constexpr char FTDContinousMessage = 'C';
	static constexpr char FTDLastMessage = 'L';

	const uint16_t FTD_DIALOG_FLOW = 1;				//对话流：直接发给对应Session
	const uint16_t FTD_PUBLIC_FLOW = 2;				//公共流：交易系统主动通知流
	const uint16_t FTD_USER_FLOW = 3;				//用户流：返回客户的回报信息
	const uint16_t FTD_PRIVATE_FLOW = 4;			//会员流：返回操作员下所有客户的回报信息
	const uint16_t FTD_QUERY_FLOW = 5;				//查询流：
	const uint16_t FTD_MAX_SEQUENCE_SERIES = 5;		//最大流订阅数

	enum class CompressType : char { None = 'N', Zip = 'Z', Customized = 'C' };
	enum class YesOrNo : char { No = 0, Yes = 1 };
	struct ztp_header {
		char version[12] = "ZTP 1.0.1";
		YesOrNo is_last = YesOrNo::Yes;								// last one or continuous one
		CompressType compressed = CompressType::None;				// compressed or not, Z means zip, C means customized compression.
		YesOrNo has_rsp = YesOrNo::No;								// has response header in body
		StreamType stream_type = StreamType::Unknown;				// see StreamType;
		MessageID func_id = 0;										// the unique id for the message purpose
		MessageID body_id = 0;										// the unique id for the body type
		int32_t request_id = -1;									// the unique id for a request/response
		int32_t session_id = -1;									// session id for a connection
		uint16_t body_length = 0;									// the length of the body
		char investor_id[16] = "";									// for investor id
		int32_t int_reserved = -1;									// int reserved
		char char_reserved[16] = "";								// reserved space;

		StreamType stream() const {
			if (char(stream_type) >= char(StreamType::Max)) {
				return StreamType::Unknown;
			}
			return stream_type;
		}
	};

	struct ftd_header {
		char		Chain;				//报文链	1字节	ASCII码字符。
		uint16_t	SequenceSeries;		//序列类别号	2字节	二进制无符号短整数。
		uint16_t	FieldCount;			//数据域数量	2字节	二进制无符号短整数。
		uint16_t	FTDContentLength;	//信息正文长度	2字节	二进制无符号短整数。
		uint32_t	TransactionID;		//信息正文类型	4字节	二进制无符号整数。(TID)
		uint32_t	SequenceNumber;		//序列号	4字节	二进制无符号整数。
		uint32_t	RequestID;			//请求编号  4字节   二进制无符号整数。(由发送请求者维护，应答中会带回) 
		uint32_t	InstrumentIDCount;	//合约号的整数值，不用解包就知道是哪个合约  4 二进制无符号整数。
	};

	class message {
	public:
		static constexpr uint8_t header_length = sizeof(ztp_header);			// the length of header	
		inline char const* data() const {
			return (char*)(&header_);
		}
		inline char* data() {
			return (char*)(&header_);
		}

		inline char const* body() const {
			return (char*)(&body_);
		}
		inline char* body() {
			return (char*)(&body_);
		}

		inline void copy_header(message const& msg) {
			header_ = msg.header_;
		}

		inline void clear_body() {
			memset(body_, 0, sizeof(body_));
		}
		inline void clear() {
			memset(&header_, 0, sizeof(header_));
			clear_body();
		}

		inline char const* char_deserved() const { return &header_.char_reserved[0]; }
		inline char const* investor_id() const { return &header_.investor_id[0]; }
		inline void investor_id(const char* investor) { strcpy_s(header_.investor_id, investor); }

		inline uint16_t body_length() const {
			return header_.body_length;
		}

		inline StreamType stream_type() const { return header_.stream(); }
		inline void stream_type(StreamType st) { header_.stream_type = st; }

		inline uint16_t length() const {
			return message::header_length + header_.body_length;
		}
		inline bool decode_header() {
			return true;
		}

		inline MessageID func_id() const { return header_.func_id; }
		inline MessageID body_id() const { return header_.body_id; }
		inline int32_t request_id() const { return header_.request_id; }
		inline int32_t session_id() const { return header_.session_id; }
		inline bool is_last() const { return header_.is_last == YesOrNo::Yes; }
		inline void func_id(MessageID id) { header_.func_id = id; }
		inline void body_id(MessageID id) { header_.body_id = id; }
		inline void request_id(int32_t id) { header_.request_id = id; }
		inline void session_id(int32_t id) { header_.session_id = id; }
		inline void is_last(bool isLast) { header_.is_last = isLast ? YesOrNo::Yes : YesOrNo::No; }

		void header(StreamType stream_type, MessageID func_id, int32_t request_id, 
			int32_t session_id = -1, bool last = true, char const* investor_id = nullptr) {
			header_.stream_type = stream_type;
			header_.func_id = func_id;
			header_.request_id = request_id;
			header_.session_id = session_id;
			if (investor_id != nullptr) {
				strcpy_s(header_.investor_id, investor_id);
			}
			header_.is_last = last ? YesOrNo::Yes : YesOrNo::No;
		}

		void empty_body() {
			header_.body_length = 0;
			header_.has_rsp = YesOrNo::No;
		}

		template<class Body>
		void req_body(Body const& data) {
			header_.body_length = sizeof(Body);
			header_.has_rsp = YesOrNo::No;
			header_.body_id = Body::ID;
			memcpy(body_, &data, sizeof(Body));
		}

		template<class Rsp, class Body>
		void rsp_body(Rsp const& rsp, Body const& data) {
			header_.body_length = sizeof(Rsp) + sizeof(Body);
			header_.has_rsp = YesOrNo::Yes;
			header_.body_id = Body::ID;
			memcpy(body_, &rsp, sizeof(Rsp));
			memcpy(body_ + sizeof(Rsp), &data, sizeof(Body));
		}

		template<int ID,class Rsp>
		void rsp_body(Rsp const& rsp) {
			header_.body_length = sizeof(Rsp);
			header_.has_rsp = YesOrNo::Yes;
			header_.body_id = ID;
			memcpy(body_, &rsp, sizeof(Rsp));
		}

		template<typename Proto, class Rsp>
		void rsp_body(Rsp const& rsp) {
			header_.body_length = sizeof(Rsp);
			header_.has_rsp = YesOrNo::Yes;
			header_.body_id = Proto::ResponseType::ID;
			memcpy(body_, &rsp, sizeof(Rsp));
		}

		template<class Body>
		void rsp_append_body(Body const& rsp) {
			auto cur_len = header_.body_length;
			header_.body_length += sizeof(Body);
			header_.has_rsp = YesOrNo::Yes;
			header_.body_id = Body::ID;
			memcpy(body_ + cur_len, &rsp, sizeof(Body));
		}

		template<class Body>
		void notify_body(Body const& data) {
			header_.body_length = sizeof(Body);
			header_.has_rsp = YesOrNo::No;
			header_.body_id = Body::ID;
			memcpy(body_, &data, sizeof(Body));
		}

		template<class Rsp, class Body>
		void notify_error_body(Rsp const& rsp, Body const& data) {
			header_.body_length = sizeof(Rsp) + sizeof(Body);
			header_.has_rsp = YesOrNo::Yes;
			header_.body_id = Body::ID;
			memcpy(body_, &rsp, sizeof(Rsp));
			memcpy(body_ + sizeof(Rsp), &data, sizeof(Body));
		}

		bool has_rsp() const { return header_.has_rsp == YesOrNo::Yes ? true : false; }

		inline bool is_auth() const {
			return stream_type() == StreamType::Auth;
		}

		inline bool is_trade() const {
			return stream_type() == StreamType::Trade;
		}

		inline bool is_op() const {
			return stream_type() == StreamType::OP;
		}

		inline bool is_private() const {
			return  stream_type() == StreamType::Private;
		}
		inline bool is_market() const {
			return stream_type() == StreamType::Market;
		}
		inline bool is_public() const {
			return stream_type() == StreamType::Public;
		}

		inline bool is_query() const {
			return stream_type() == StreamType::Query;
		}

		inline bool is_unknown() const {
			return stream_type() == StreamType::Unknown;
		}

		template<class Target>
		bool retrieve(Target& target) const {
			if (sizeof(Target) != header_.body_length) {
				return false;
			}
			if (Target::ID != header_.body_id) {
				return false;
			}
			std::memcpy(&target, body_, sizeof(Target));
			return true;
		}

		template<class Rsp,class Target>
		Target* retrieve(Rsp& rsp, Target& target) const {
			if (header_.has_rsp == YesOrNo::No) {
				return nullptr;
			}
			if (header_.body_length == sizeof(Rsp)) {
				std::memcpy(&rsp, body_, sizeof(Rsp));
				return nullptr;
			}
			if (sizeof(Target) + sizeof(Rsp) == header_.body_length) {
				if (Target::ID != header_.body_id) {
					return false;
				}
				std::memcpy(&rsp, body_, sizeof(Rsp));
				std::memcpy(&target, body_ + sizeof(Rsp), sizeof(Target));
				return &target;
			}
			return nullptr;
		}

	private:
		ztp_header header_;				// the header
		char body_[2048] = "";			// body, max size is 2048 bytes;
	};
}