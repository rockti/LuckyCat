//日志读取工具
#pragma once
#include "BaseExchApi.h"

class Profile {
	friend Profile& theProfile();
public:
	Profile();
	std::string GetPrivateProfile(char const* appName, char const* keyName, char const* defaultValue);
	int GetPrivateProfile(char const* appName, char const* keyName, int defaultValue);
	void WritePrivateProfile(char const* appName, char const* keyName, char const* value);
	void WritePrivateProfile(char const* appName, char const*keyName, int value);
	char const* GetFilePath() const { return m_filePath.c_str(); }
private:
	std::string m_filePath;
	std::string m_fileName;
};

class StringSplit
{
public:
	int GetByDelimiter(char* originalStr, char const* delimit, std::vector<std::string>& destVector);
	int GetByDelimiter(char* originalStr, char const* delimit, std::vector<LinkApi::ID>& destVector);
	friend StringSplit& theStringSplit();
};