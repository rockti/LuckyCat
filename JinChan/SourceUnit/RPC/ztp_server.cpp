#include "ztp_server.hpp"

namespace ztp {
	ztp_server::ztp_server(asio::io_context& io_context, const tcp::endpoint& endpoint, 
		session_mgr& session_mgr,
		IZTPServerDispatcher* dispatcher)
		: acceptor_(io_context, endpoint), session_mgr_(session_mgr),dispatcher_(dispatcher) {
		do_accept();
	}
	inline void ztp_server::do_accept() {
		acceptor_.async_accept(
			[this](std::error_code ec, tcp::socket socket) {
				if (!ec) {
					std::make_shared<ztp_session>(std::move(socket), session_mgr_, dispatcher_)->start();
				}
				do_accept();
			});
	}

	inline ztp_session::ztp_session(tcp::socket socket, session_mgr& room, IZTPServerDispatcher* dispatcher)
		: base_session(),socket_(std::move(socket)),
		session_mgr_(room), dispatcher_(dispatcher) {
	}

	inline void ztp_session::deliver(const message& msg) {
		bool write_in_progress = !write_msgs_.empty();
		write_msgs_.push(msg);
		if (!write_in_progress) {
			do_write();
		}
	}

	inline void ztp_session::do_read_header() {
		auto self(shared_from_this());
		asio::async_read(socket_,
			asio::buffer(read_msg_.data(), message::header_length),
			[this, self](std::error_code ec, std::size_t /*length*/) {
				if (!ec && read_msg_.decode_header()) {
					read_msg_.session_id(id());
					do_read_body();
				} else {
					printNlog("do_read_body failed session[{}] error[{}] [{}]", id(), ec.value(),ec.message());
					session_mgr_.leave(shared_from_this());
				}
			});
	}

	inline void ztp_session::do_read_body() {
		auto self(shared_from_this());
		asio::async_read(socket_,
			asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this, self](std::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					auto reqStream = read_msg_.stream_type();
					if (reqStream == StreamType::Trade) {
						dispatcher_->OnTradeReq(read_msg_);
					} else if (reqStream == StreamType::Query) {
						dispatcher_->OnQueryReq(read_msg_);
					} else if (reqStream == StreamType::Auth) {
						dispatcher_->OnAuthReq(read_msg_);
					} else if (reqStream == StreamType::OP) {
						dispatcher_->OnOpReq(read_msg_);
					}
					do_read_header();
				} else {
					printNlog("do_read_body failed session[{}] error[{}] [{}]", id(), ec.value(), ec.message());
					session_mgr_.leave(shared_from_this());
				}
			});
	}

	 void ztp_session::do_write() {
		 message msg;
		 write_msgs_.pop(msg);
		auto self(shared_from_this());
		asio::async_write(socket_,
			asio::buffer(msg.data(),
				msg.length()),
			[this, self](std::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					if (!write_msgs_.empty()) {
						do_write();
					}
				} else {
					printNlog("do_write failed session[{}] error[{}]", id(), ec.value());
					session_mgr_.leave(shared_from_this());
				}
			});
	}

}