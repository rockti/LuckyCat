#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/lockfree/queue.hpp>
//#include <boost/interprocess/managed_shared_memory.hpp>
namespace ringbuffer {
	template<class T>
	using RingBuffer = boost::lockfree::spsc_queue<T, boost::lockfree::capacity<4096>>;

	template<class T>
	using Queue = boost::lockfree::queue<T, boost::lockfree::capacity<4096>>;

}