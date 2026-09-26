#pragma once

class Log {
public:
	static void Init(char const* tag, char const* fileName);
	static void Shutdown();

	static void Error(char const* msg);
	static void Warn(char const* msg);
	static void Info(char const* msg);
	static void Debug(char const* msg);

	static void Error(std::string const& msg);
	static void Warn(std::string const& msg);
	static void Info(std::string const& msg);
	static void Debug(std::string const& msg);

};