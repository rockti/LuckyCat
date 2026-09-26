#pragma once
#include <fmt/format.h>
template<typename S, typename... Args>
void println(const S& format_str, Args&&... args) {
	fmt::print(format_str, args...);
	fmt::print("\n");
}

template<size_t S>
class Formatter
{
public:
	Formatter()
	{
		memset(m_buff,0,S);
	}

	Formatter(char const* format, ...)
	{
		va_list arglist;
		va_start(arglist, format);
		_vsnprintf_s_l(m_buff, S, S-1, format, NULL, arglist);
	}

	operator char const*()
	{
		return m_buff;
	}

private:
	char m_buff[S];
};
typedef Formatter<8192> FT;

class Log {
public:
	static void Init(char const* tag, char const* fileName);
	static void Shutdown();
	static void Debug(char const* msg);	//调试
	static void Info(char const* msg);	//正常
	static void Warn(char const* msg);	//警告
	static void Error(char const* msg);	//业务错
	static void Fatal(char const* msg);	//系统错

	static void Debug(std::string const& msg);	//调试
	static void Info(std::string const& msg);	//正常
	static void Warn(std::string const& msg);	//警告
	static void Error(std::string const& msg);	//业务错
	static void Fatal(std::string const& msg);	//系统错
};

template<typename S, typename... Args>
void printNlog(const S& format_str, Args&&... args) {
	auto msg = fmt::format(format_str, args...);
	println(msg);
	Log::Info(msg);
}

