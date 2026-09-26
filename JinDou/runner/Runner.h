#pragma once


template<class Queues, class Handler, class IdleStrategy>
class Runner {
public:
	Runner(Queues& queues, Handler& handler, IdleStrategy& idleStrategy, std::atomic<bool>& quit)
		:m_queues(queues), m_handler(handler), m_idleStrategy(idleStrategy), m_quit(quit) {
	}

	void Run() {
		while (!m_quit) {
			Data data;
			while (m_queues.Pop(data)) {
				m_handler.Handle(data);
			}
			m_idleStrategy.idle();
		}
	}
private:
	Queues& m_queues;
	Handler& m_handler;
	IdleStrategy& m_idleStrategy;
	std::atomic<bool>& m_quit;
};

template<class T>
class ReadQueue {
public:
	ReadQueue(ringbuffer::Queue<T>& queue) :m_queue(queue) {}
	bool Pop(T& data) {
		return m_queue.pop(data);
	}
private:
	ringbuffer::Queue<T>& m_queue;
};

template<class T>
class WriteQueue {
public:
	WriteQueue(ringbuffer::Queue<T>& queue) :m_queue(queue) {}
	bool Push(T const& data) {
		return m_queue.push(data);
	}
private:
	ringbuffer::Queue<T>& m_queue;
};


