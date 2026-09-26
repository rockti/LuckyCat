#include "stdafx.h"
#include "Profile.h"

Profile::Profile()
{
	char path[MAX_PATH] = "";
	::GetModuleFileNameA(NULL, path, 1024);

	m_fileName = path;
	string::size_type findPos = m_fileName.find_last_of('\\');
	m_filePath = m_fileName.erase(findPos+1);
	m_fileName = m_fileName.erase(findPos+1) + "config.ini";
// 	string::size_type findPos = m_fileName.find_last_of('.');
// 	m_fileName = m_fileName.erase(findPos+1) + "ini";
}

string Profile::GetPrivateProfile(char const* appName, char const* keyName, char const* defaultValue)
{
	assert(!m_fileName.empty());
	char tmpBuf[512] = "";
	::GetPrivateProfileStringA(appName, keyName, defaultValue, tmpBuf, sizeof(tmpBuf), m_fileName.c_str());
	return tmpBuf;
}

int Profile::GetPrivateProfile(char const* appName, char const* keyName, int defaultValue)
{
	assert(!m_fileName.empty());
	return ::GetPrivateProfileIntA(appName, keyName, defaultValue, m_fileName.c_str());
}

void Profile::WritePrivateProfile(char const* appName, char const* keyName, char const* value)
{
	assert(!m_fileName.empty());
	::WritePrivateProfileStringA(appName, keyName, value, m_fileName.c_str());
}

void Profile::WritePrivateProfile(char const* appName, char const*keyName, int value)
{
	assert(!m_fileName.empty());
	char tmpBuf[256] = "";
	sprintf_s(tmpBuf,"%d",value);
	::WritePrivateProfileStringA(appName, keyName, tmpBuf, m_fileName.c_str());
}

Profile& theProfile()
{
	static Profile obj;
	return obj;
}


int StringSplit::GetByDelimiter(char* originalStr, char const* delimit, vector<string>& destVector)
{
	assert((NULL != originalStr) && (NULL != delimit));	
	destVector.clear();

	char* firstToken = NULL;
	char* nextToken = NULL;
	firstToken = strtok_s(originalStr, delimit, &nextToken);
	while (NULL != firstToken)
	{
		string tokenStr = firstToken;
		destVector.push_back(tokenStr);
		firstToken = strtok_s(NULL, delimit, &nextToken);
	}

	return destVector.size();
}
int StringSplit::GetByDelimiter(char* originalStr, char const* delimit, vector<LinkApi::ID>& destVector)
{
	assert((NULL != originalStr) && (NULL != delimit));	
	destVector.clear();

	char* firstToken = NULL;
	char* nextToken = NULL;
	firstToken = strtok_s(originalStr, delimit, &nextToken);
	while (NULL != firstToken)
	{
		string tokenStr = firstToken;
		destVector.push_back(static_cast<LinkApi::ID>(atoi(tokenStr.c_str())));
		firstToken = strtok_s(NULL, delimit, &nextToken);
	}

	return destVector.size();
}

StringSplit& theStringSplit()
{
	static StringSplit obj;
	return obj;
}