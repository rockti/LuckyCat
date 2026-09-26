#pragma once

const int TOKEN_MAX_LEN = 512;
enum CSV_PARSER_ERROR
{
	CPE_HAS_NEXT,
	CPE_END,
	CPE_MARK_NOT_MATCH,
	CPE_TOKEN_TOO_LONG,
};

class CCSVParser  
{
public:
	CCSVParser();
	CCSVParser(char const* pszData);
	virtual ~CCSVParser();
	char* GetNextToken();
	void Parse(char const* pszData);
	inline CSV_PARSER_ERROR GetErrorCode()	{ return m_errorCode; }
private:
	void NextChar();
	void MakeWord(const char *pszEnd);
private:
	CSV_PARSER_ERROR m_errorCode;
	char const* m_pszData;
	char m_szCurrWord[TOKEN_MAX_LEN+1];
	char const* m_pCurr;
	char m_chC;
	char m_chNC;
};
