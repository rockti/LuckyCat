#pragma once

#include "model/Constants.h"
#include <array>
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

	void Login(ConnPlatform api, char const* password);
	void SetVertificateCode(ConnPlatform api, char const* code);

	StrategyPublisher& GetStrategyPublisher();
	UIPublisher& GetUIPublisher();
	ConnPublisher& GetConnPublisher();

private:
	std::atomic<bool> m_quit{ false};

	void initInstrumments();
	void initConn(Config const& config);
	void initConnPlatformHandler(Config const& config);
	void startThreads(Config const& config);
	
	std::shared_ptr<ExectuorQueues> m_queuesPtr;
	ConnRunnerHandler* m_connPlatformHandler = nullptr;
	std::array<ExchangeConn*, 3> m_conns; // 最多只支持三个连接(期货、现货、海外)
	
	std::thread m_connPlatformThread;
	std::thread m_strategyThread; 
};