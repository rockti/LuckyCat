
#include "atomic/IdleStrategy.h"

#include "runner/Framework.h"
#include "runner/Queues.h"
#include "runner/StrategyRunner.h"
#include "runner/ConnRunner.h"
#include "runner/UIRunner.h"

#include "conn/ExchangeConn.h"
#include "conn/factory/ConnFactory.h"
#include "model/factory/InstrumentFactory.h"

#include "notify/ConnPublisher.h"
#include "notify/StrategyPublisher.h"
#include "notify/UIPublisher.h"

#include "config/Config.h"
#include "strategy/Business.h"

#include "service/Service.h"

template<class IdleStrategy>
using ConnPlatformRunner = Runner<ConnPlatformExecutorQueues, ConnRunnerHandler, IdleStrategy>;

template<class IdleStrategy>
using StrategyRunner = Runner<StrategyExecutorQueues, StrategyHandler, IdleStrategy>;

// template<class IdleStrategy>
// using UIRunner = Runner<UIExecutorQueues, UIRunnerHandler, IdleStrategy>;


ConnPublisher& Framework::GetConnPublisher() {
	static ConnPlatformExecutorQueues s_queues = m_queuesPtr->ConnPlatform();
	static ConnPublisher s_publisher(&s_queues);
	return s_publisher;
}

bool Framework::IsInited(TradePlatform platform) const {
	return m_conns[PlatformIndex(platform)]->IsInited();
}

void Framework::InitConnect(TradePlatform platform) {
	m_conns[PlatformIndex(platform)]->Init();
}

ExectuorQueues& Framework::GetQueues() {
	return *m_queuesPtr.get();
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
	initConn(config);
	startThreads(config);
}

void Framework::Join() {
	m_connPlatformThread.join();
	m_strategyThread.join();
//	m_uiThread.join();
}

void Framework::Stop() {
	for (auto& conn : m_conns) {
		if (conn) {
			conn->Release();
		}
	}
	m_quit = true;
}

void Framework::Login(TradePlatform connPlatform, char const* password) {
	GetUIPublisher().Login(connPlatform, password);
}

void Framework::initConn(Config const& config) {
	for(TradeSystemConfig const& connConfig : config.TradeSystemConfigs) {
		m_conns[PlatformIndex(connConfig.Platform)] = conn::ConnFactory::Create(connConfig, GetConnPublisher());
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
		ConnPlatformRunner<atomic::BusySpinIdleStrategy> connPlatformRunner(queues.ConnPlatform(), *m_connPlatformHandler, idleStrategy, quit);
		connPlatformRunner.Run();
		});

	m_strategyThread = std::thread([&]() {
		auto& services = ServiceSinglton::Services();
		StrategyHandler s_handler(GetStrategyPublisher());

		atomic::BusySpinIdleStrategy idleStrategy;
		StrategyRunner<atomic::BusySpinIdleStrategy> runner(queues.Strategy(), s_handler, idleStrategy, quit);
		runner.Run();
		});


// 	m_uiThread = std::thread([&]() {
// 		UIRunnerHandler handler;
// 		atomic::SleepingIdleStrategy idleStrategy(std::chrono::milliseconds(1));
// 		auto runner = UIRunner<atomic::SleepingIdleStrategy>(queues.UI(), handler, idleStrategy, quit);
// 		runner.Run();
// 		});


}



