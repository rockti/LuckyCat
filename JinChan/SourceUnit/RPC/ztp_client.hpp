#pragma once
#include <asio.hpp>
#include <deque>
#include <set>
#include <asio/steady_timer.hpp>
#include "IClient.h"
#include "container.hpp"
namespace ztp {
	using asio::ip::tcp;

	class ztp_client {
	public:
		ztp_client(asio::io_context& io_context,
			const tcp::resolver::results_type& endpoints, ztp_dispatcher* dispatcher);
		void reset_endpoints(const tcp::resolver::results_type& endpoints);
		void write(const message& msg);
		void close();
		void connect();
		void reconnect();

	private:
		void do_connect(const tcp::resolver::results_type& endpoints);
		void do_read_header();
		void do_read_body();
		void do_write();
		void dispatch_msg(const message& msg);
		void check_deadline();

	private:
		bool stopped_ = true;;
		asio::io_context& io_context_;
		std::shared_ptr<tcp::socket> socket_;
		tcp::resolver::results_type endpoints_;
		message read_msg_;
		message_queue write_msgs_;
		ztp_dispatcher* dispatcher_;
		asio::steady_timer deadline_;
		asio::steady_timer heartbeat_timer_;
	};
}