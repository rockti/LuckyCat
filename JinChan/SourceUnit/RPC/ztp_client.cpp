#include "ztp_client.hpp"
#include <fmt/format.h>
#include "Logger.h"
using namespace asio;

namespace ztp {
	ztp_client::ztp_client(asio::io_context& io_context, const tcp::resolver::results_type& endpoints, ztp_dispatcher* dispatcher)
	:io_context_(io_context),
	socket_(std::make_shared<tcp::socket>(io_context)),
	endpoints_(endpoints),
	dispatcher_(dispatcher),
	deadline_(io_context),
	heartbeat_timer_(io_context){
		io_context_.reset();
//		do_connect(endpoints);
	}
	void ztp_client::reset_endpoints(const tcp::resolver::results_type& endpoints) {
		endpoints_ = endpoints;
	}
	void ztp_client::write(const message& msg) {
		asio::post(io_context_,
			[this, msg]() {
				bool write_in_progress = !write_msgs_.empty();
				write_msgs_.push(msg);
				if (!write_in_progress) {
					do_write();
				}
			});
	}
	void ztp_client::close() {
		asio::post(io_context_, [this]() { socket_->close(); });
	}

	void ztp_client::connect() {
		auto a = std::make_shared<tcp::socket>(io_context_);
		socket_.reset();
 		socket_.swap(a);
		write_msgs_.consume_all([](message& msg) {});
 		do_connect(endpoints_);
//		deadline_.async_wait(std::bind(&ztp_client::check_deadline, this));
	}

	void ztp_client::reconnect() {
		socket_.swap(std::make_shared<tcp::socket>(io_context_));
		do_connect(endpoints_);
//		deadline_.async_wait(std::bind(&ztp_client::check_deadline, this));

	}
	inline void ztp_client::do_connect(const tcp::resolver::results_type& endpoints) {
		deadline_.expires_after(asio::chrono::seconds(10));
		asio::async_connect(*socket_, endpoints,
			[this](std::error_code ec, tcp::endpoint) {
				if (!ec) {
					dispatcher_->on_connected();
					do_read_header();
				} else {
//					socket_.reset();
					socket_->close();
					printNlog("connect failed: code=[{}] msg =[{}] \n", ec.value(), ec.message());
				}
			});
	}
	inline void ztp_client::do_read_header() {
		asio::async_read(*socket_,
			asio::buffer(read_msg_.data(), message::header_length),
			[this](std::error_code ec, std::size_t /*length*/) {
				if (!ec && read_msg_.decode_header()) {
					if (read_msg_.body_length() > 0) {
						read_msg_.clear_body();
						do_read_body();
					} else {
						dispatch_msg(read_msg_);
						read_msg_.clear();
						do_read_header();
					}
				} else {
					printNlog("client read header failed");
					socket_->close();
					dispatcher_->on_disconnected();
				}
			});
	}
	inline void ztp_client::do_read_body() {
		asio::async_read(*socket_,
			asio::buffer(read_msg_.body(), read_msg_.body_length()),
			[this](std::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					dispatch_msg(read_msg_);
					read_msg_.clear();
					do_read_header();
				} else {
					printNlog("client read body failed");
					socket_->close();
					dispatcher_->on_disconnected();
				}
			});
	}
	inline void ztp_client::do_write() {
		message msg;
		write_msgs_.pop(msg);
		asio::async_write(*socket_,
			asio::buffer(msg.data(),
				msg.length()),
			[this](std::error_code ec, std::size_t /*length*/) {
				if (!ec) {
					if (!write_msgs_.empty()) {
						do_write();
					}
				} else {
					printNlog("client write failed");
					socket_->close();
					dispatcher_->on_disconnected();
				}
			});
	}
	void ztp_client::dispatch_msg(const message& msg) {
		dispatcher_->dispatch(msg);
	}

	void ztp_client::check_deadline() {
		if (stopped_)
			return;

		// Check whether the deadline has passed. We compare the deadline against
		// the current time since a new asynchronous operation may have moved the
		// deadline before this actor had a chance to run.
		if (deadline_.expiry() <= steady_timer::clock_type::now()) {
			// The deadline has passed. The socket is closed so that any outstanding
			// asynchronous operations are cancelled.
			dispatcher_->on_connect_timeout();
			close();

			// There is no longer an active deadline. The expiry is set to the
			// maximum time point so that the actor takes no action until a new
			// deadline is set.
			deadline_.expires_at(steady_timer::time_point::max());
		}

		// Put the actor back to sleep.
		deadline_.async_wait(std::bind(&ztp_client::check_deadline, this));
	}
}