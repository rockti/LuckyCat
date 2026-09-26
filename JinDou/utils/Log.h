#pragma once
#include <string>
#include <fmt/format.h>
class Log {
public:
	static void Init(char const* tag, char const* fileName);
	static void Shoudown();

	static void Error(char const* msg);
	static void Warn(char const* msg);
	static void Info(char const* msg);
	static void Debug(char const* msg);

	static void Error(std::string const& msg);
	static void Warn(std::string const& msg);
	static void Info(std::string const& msg);
	static void Debug(std::string const& msg);

};

template<typename S, typename... Args>
void println(const S& format_str, Args&&... args) {
	fmt::print(format_str, args...);
	fmt::print("\n");
}

template<typename S, typename... Args>
void printNlog(const S& format_str, Args&&... args) {
	auto msg = fmt::format(format_str, args...);
	println(msg);
	Log::Info(msg);
}