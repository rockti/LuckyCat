#pragma once
#include "message.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>


namespace ztp {
	template<class T>
	using RingBuffer = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<128>>;

	template<class T>
	using Queue = boost::lockfree::queue<T, boost::lockfree::capacity<1280>>;
	using message_queue = Queue<message>;
}