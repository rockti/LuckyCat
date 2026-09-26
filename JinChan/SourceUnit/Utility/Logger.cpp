#include "stdafx.h"
#include "Logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>


void Log::Init(char const* tag, char const* fileName) {
	static auto async_file = spdlog::daily_logger_mt<spdlog::async_factory>(tag, fileName);
	async_file->flush_on(spdlog::level::debug);
	spdlog::set_default_logger(async_file);
	spdlog::flush_every(std::chrono::seconds(1));

	spdlog::set_level(spdlog::level::debug);
	spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%f [%t] %v");
}

void Log::Shutdown() {
	spdlog::shutdown();
}

void Log::Fatal(char const* msg) {
	SPDLOG_ERROR(msg);
}

void Log::Error(char const* msg) {
	SPDLOG_ERROR(msg);
}

void Log::Warn(char const* msg) {
	SPDLOG_WARN(msg);
}

void Log::Info(char const* msg) {
	SPDLOG_INFO(msg);
}

void Log::Debug(char const* msg) {
	SPDLOG_DEBUG(msg);
}

void Log::Fatal(std::string const& msg) {
	SPDLOG_ERROR(msg.c_str());
}

void Log::Error(std::string const& msg) {
	SPDLOG_ERROR(msg.c_str());
}

void Log::Warn(std::string const& msg) {
	SPDLOG_WARN(msg.c_str());
}

void Log::Info(std::string const& msg) {
	SPDLOG_INFO(msg.c_str());
}

void Log::Debug(std::string const& msg) {
	SPDLOG_DEBUG(msg.c_str());
}
