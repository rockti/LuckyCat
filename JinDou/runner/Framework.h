#pragma once

#include "model/Constants.h"
#include <array>
#include <atomic>
#include <memory>
#include <thread>

class StrategyPublisher;
class UIPublisher;
class ConnPublisher;
struct Config;
class ConnRunnerHandler;
struct ExectuorQueues;
class ExchangeConn;

class Framework	{
public:
	Framework();
	~Framework();

	static Framework& Instance();
	void Start(Config const& config);
	void Join();
	void Stop();

	void Login(TradePlatform api, char const* password);
	
	StrategyPublisher& GetStrategyPublisher();
	UIPublisher& GetUIPublisher();
	ConnPublisher& GetConnPublisher();

	bool IsInited(TradePlatform platform) const;
	void InitConnect(TradePlatform platform);
	ExectuorQueues& GetQueues();
private:
	std::atomic<bool> m_quit{ false};

	void initConn(Config const& config);
	void initConnPlatformHandler(Config const& config);
	void startThreads(Config const& config);
	
	std::shared_ptr<ExectuorQueues> m_queuesPtr;
	ConnRunnerHandler* m_connPlatformHandler = nullptr;
	std::array<ExchangeConn*,10> m_conns; // only support CTP, SGE, HKEX_ESUNNY
	
	std::thread m_connPlatformThread;
	std::thread m_strategyThread; 
//	std::thread m_uiThread;
};