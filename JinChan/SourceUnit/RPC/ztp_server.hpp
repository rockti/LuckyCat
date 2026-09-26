#pragma once
#include "message.hpp"
#include <asio.hpp>
#include <deque>
#include <map>
#include "IServer.h"
#include "Logger.h"
#include "container.hpp"

namespace ztp {
	using asio::ip::tcp;

	class base_session {
	public:
		base_session() {
			static int s_id = 0;
			id_ = ++s_id;
		}
		virtual ~base_session() {}
		virtual void deliver(const message& msg) = 0;
		int id() const {
			return id_;
		}
		template<class T>
		void set_hook(T const& t) {
			memcpy(hook_, &t, sizeof(T));
		}
		template<bool y>
		struct yes;
		template<>
		struct yes<true> {};

		template<class T>
		T& get_hook() const {
			yes<sizeof(T) <= sizeof(hook_)> y{};

			return *(T*)(hook_);
		}
	private:
		int id_ = -1;
		char hook_[64] = "";
	};

	typedef std::shared_ptr<base_session> session_ptr;

	struct session_less {
		inline bool operator()(const session_ptr& l, const session_ptr& r) const {
			return l->id() < r->id();
		}
	};

	class session_mgr {
	public:
		virtual void join(session_ptr session) {
			sessions_.emplace(session->id(),session);
			printNlog("session id[{}] joined", session->id());
		}

		virtual void leave(session_ptr session) {
			sessions_.erase(session->id());
			printNlog("session id[{}] leaved", session->id());
		}

		void deliver(const message& msg) {
			auto iter = sessions_.find(msg.session_id());
			if (iter != sessions_.end()) {
				iter->second->deliver(msg);
			} else {
				printNlog("deliver msg failed, session id[{}]", msg.session_id());
			}
		}

		void deliver_to_all(const message& msg) {
			for (auto& kv : sessions_) {
				kv.second->deliver(msg);
			}
		}
		base_session* get_session(int session_id) const {
			auto iter = sessions_.find(session_id);
			if (iter != sessions_.end()) {
				return iter->second.operator->();
			}
			return nullptr;
		}
	protected:
		std::map<int, session_ptr> sessions_;
	};

	class ztp_session
		: public base_session,
		public std::enable_shared_from_this<ztp_session> {
	public:
		ztp_session(tcp::socket socket, session_mgr& room, IZTPServerDispatcher* dispatcher);

		void start() {
			session_mgr_.join(shared_from_this());
			do_read_header();
		}

		void deliver(const message& msg);

	private:
		void do_read_header();

		void do_read_body();

		void do_write();

		tcp::socket socket_;
		session_mgr& session_mgr_;
		message read_msg_;
		message_queue write_msgs_;
		IZTPServerDispatcher* dispatcher_;
	};

	class ztp_server {
	public:
		ztp_server(asio::io_context& io_context,
			const tcp::endpoint& endpoint, session_mgr&  session_mgr,IZTPServerDispatcher* dispatcher);

		void deliver(const message& msg) {
			session_mgr_.deliver(msg);
		}

		void deliver_to_all(const message& msg) {
			session_mgr_.deliver_to_all(msg);
		}

	private:
		void do_accept();

		tcp::acceptor acceptor_;
		session_mgr& session_mgr_;
		IZTPServerDispatcher* dispatcher_;
	};
}