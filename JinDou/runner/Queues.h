#pragma once

#include "notify/Notify.h"
#include "container/RingBuffer.h"
#include "runner/Runner.h"

struct UIExecutorQueues {
	ReadQueue<Data> UINotifyQueue;					// read UI Command
	WriteQueue<Data> StrategyCommandQueue;			// write command to strategy
	WriteQueue<Data> ConnCommandQueue;				// write command to APIs
	bool Pop(Data& data) {
		return UINotifyQueue.Pop(data);
	}

	bool PushStrategyCommand(Data const& command) {
		return StrategyCommandQueue.Push(command);
	}

	bool PushConnCommand(Data const& command) {
		 return ConnCommandQueue.Push(command);
	}
};

struct ConnPlatformExecutorQueues {
	ReadQueue<Data> OrderQueue;				// read strategy orders
	WriteQueue<Data> ResponseQueue;			// write quote/response/ack to strategy

	ReadQueue<Data> ConnCommandQueue;		// read APICommand come from  UI
	WriteQueue<Data> UINotifyQueue;			// notify info to UI

	bool Pop(Data& data) {
		if (OrderQueue.Pop(data)) {
			return true;
		}
		return ConnCommandQueue.Pop(data);
	}

	bool PushResponse(Data const& data) {
		return ResponseQueue.Push(data);
	}

	bool NotifyUI(Data const& data) {
		return UINotifyQueue.Push(data);
	}
};

struct StrategyExecutorQueues {
	ReadQueue<Data> ResponseQueue;			// read quote/response/ack from Conn platform
	WriteQueue<Data> OrderQueue;			// write orders to Conn Platform

	ReadQueue<Data> StrategyCommandQueue;	// read strategy Command come from UI
	WriteQueue<Data> UINotifyQueue;			// notify info to UI

	bool Pop(Data& data) {
		if (ResponseQueue.Pop(data)) {
			return true;
		}
		return StrategyCommandQueue.Pop(data);
	}

	bool Push(Data const& data) {
		return OrderQueue.Push(data);
	}

	bool NotifyUI(Data const& data) {
		return UINotifyQueue.Push(data);
	}
};

struct ExectuorQueues {

	UIExecutorQueues UI() {
		return UIExecutorQueues{
			ReadQueue<Data>(UINotifyQueue),
			WriteQueue<Data>(StrategyCommandQueue),
			WriteQueue<Data>(ConnCommandQueue)
		};
	}

	ConnPlatformExecutorQueues& ConnPlatform() {
		static ConnPlatformExecutorQueues queues{
			ReadQueue<Data>(OrderQueue),
			WriteQueue<Data>(ResponseQueue),
			ReadQueue<Data>(ConnCommandQueue),
			WriteQueue<Data>(UINotifyQueue)
		};
		return queues;
	}

	StrategyExecutorQueues& Strategy() {
		static StrategyExecutorQueues queues{
			ReadQueue<Data>(ResponseQueue),
			WriteQueue<Data>(OrderQueue),
			ReadQueue<Data>(StrategyCommandQueue),
			WriteQueue<Data>(UINotifyQueue)
		};
		return queues;
	}
	ringbuffer::Queue<Data> UINotifyQueue;
	ringbuffer::Queue<Data> StrategyCommandQueue;
	ringbuffer::Queue<Data> ConnCommandQueue;
	ringbuffer::Queue<Data> OrderQueue;
	ringbuffer::Queue<Data> ResponseQueue;
};







