#include "pch.h"

#include "atomic/IdleStrategy.h"

#include "runner/Framework.h"
#include "runner/Queues.h"
#include "runner/StrategyRunner.h"
#include "runner/ConnRunner.h"

#include "conn/ExchangeConn.h"
#include "conn/factory/ConnFactory.h"
#include "model/factory/InstrumentFactory.h"

#include "notify/ConnPublisher.h"
#include "notify/StrategyPublisher.h"
#include "notify/UIPublisher.h"

#include "config/Config.h"
#include "strategy/Business.h"
#include "service/DealingRepository.h"


template<class IdleStrategy>
using ConnPlatformRunner = Runner<ConnPlatformExecutorQueues, ConnRunnerHandler, IdleStrategy>;

template<class IdleStrategy>
using StrategyRunner = Runner<StrategyExecutorQueues, StrategyHandler, IdleStrategy>;


ConnPublisher& Framework::GetConnPublisher() {
	static ConnPlatformExecutorQueues s_queues = m_queuesPtr->ConnPlatform();
	static ConnPublisher s_publisher(&s_queues);
	return s_publisher;
}

StrategyPublisher& Framework::GetStrategyPublisher() {
	static StrategyExecutorQueues s_queues = m_queuesPtr->Strategy();
	static StrategyPublisher s_publisher(&s_queues);
	return s_publisher;
}

UIPublisher& Framework::GetUIPublisher() {
	static UIExecutorQueues s_queues = m_queuesPtr->UI();
	static UIPublisher s_publisher(&s_queues);
	return s_publisher;
}

Framework::Framework() :m_queuesPtr(new ExectuorQueues()) {
}

Framework::~Framework() {
	for (ExchangeConn* conn : m_conns) {
		delete conn;
	}
}


Framework& Framework::Instance() {
	static Framework s_framework;
	return s_framework;
}

void Framework::Start(Config const& config) {
	initInstrumments();
	initConn(config);
	startThreads(config);
}
void Framework::Join() {
	m_connPlatformThread.join();
	m_strategyThread.join();
}
void Framework::Stop() {
	for (ExchangeConn* const& pConn : m_conns) {
		if (nullptr != pConn) {
			pConn->Release();
		}
	}

	m_quit = true;
}

void Framework::Login(ConnPlatform connPlatform, char const* password) {
	GetUIPublisher().Login(connPlatform, password);
}

void Framework::SetVertificateCode(ConnPlatform connPlatform, char const* code) {
	GetUIPublisher().SetVertificateCode(connPlatform, code);
}

void Framework::initInstrumments() {
	InstrumentFactory::Init();
}

void Framework::initConn(Config const& config) {
	for(ConnConfig const& connConfig : config.ConnConfigs) {
		m_conns[PlatformIndex(connConfig.Platform)] = conn::ConnFactory::Create(connConfig, InstrumentFactory::GetInstruments(), GetConnPublisher());
	}
}

void Framework::initConnPlatformHandler(Config const& config) {
	static ConnRunnerHandler s_connPlatformHandler(m_conns);
	m_connPlatformHandler = &s_connPlatformHandler;
}

void Framework::startThreads(Config const& config) {
	ExectuorQueues& queues = *m_queuesPtr.get();
	std::atomic<bool>& quit = m_quit;
	m_connPlatformThread = std::thread([&]() {
		initConnPlatformHandler(config);
		atomic::BusySpinIdleStrategy idleStrategy;
		auto connPlatformRunner = ConnPlatformRunner<atomic::BusySpinIdleStrategy>(queues.ConnPlatform(), *m_connPlatformHandler, idleStrategy, quit);
		connPlatformRunner.Run();
		});

// 	if (!SetThreadAffinityMask(m_connPlatformThread.native_handle(), 0x00000002)) {
// 		Log::Error(fmt::format("SetAffinityMask Failed![%u]", GetLastError()));
// 	}
// 	Log::Info("SetAffinityMask success!");

	m_strategyThread = std::thread([&]() {
		StrategyHandler s_handler(GetStrategyPublisher(), DealingRepository::Instance());
		atomic::BusySpinIdleStrategy idleStrategy;
		auto runner = StrategyRunner<atomic::BusySpinIdleStrategy>(queues.Strategy(), s_handler, idleStrategy, quit);
		runner.Run();
		});
}



