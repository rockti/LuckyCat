#pragma once
#include "ZTPType.h"
using namespace ztp;

struct FormatResult {
	int Size = 0;
	char const* Message = nullptr;
};

class SGEFormatter {
public:
	SGEFormatter(data::CSeatInfo const& seatInfo) :m_seatInfo(seatInfo) {}

	FormatResult ToLoginRequest(char const* encryptedPwd, char const* randomStr, char const* localIP, char const* sgeVersion);
	FormatResult ToStorageLoginRequest(char const* encryptedPwd, char const* randomStr, char const* localIP, char const* sgeVersion);
	FormatResult ToDeferInsertRequest(data::COrder const& order);
	FormatResult ToSpotInsertRequest(data::COrder const& order);
	FormatResult ToCancelRequest(data::COrderAction const& cancel);
	FormatResult ToQueryPosition(char const* ClientID);
private:

	char const* head(char const* msgType);

	data::CSeatInfo const& m_seatInfo;
	char m_msgBuffer[1024] = "";
	char m_headBuffer[128] = "";
};

struct ParseResult {

};

class CFTDDepthMarketDataField;

class Parser {
public:

	void Init(std::string_view const& msg) {
		m_pos = 0;
		m_msg = msg;
	}

	bool GotoNextKey() {
		++m_pos;
		while (m_pos < m_msg.length()) {
			if (m_msg[m_pos] == '=') {
				return true;
			}
			++m_pos;
		}
		return false;
	}

	std::string_view GetKey() {
		return m_msg.substr(m_pos - 3, 3);
	}
	std::string_view GetStringValue(int subPos = 0) {
		size_t beginPos = ++m_pos;	//plus'='
		size_t length = 0;
		for (size_t i = beginPos; i < m_msg.length(); ++i) {
			if (isValueEndToken(m_msg[i])) {
				m_pos += length;
				return m_msg.substr(beginPos + subPos, length - subPos);	//'='后直到（',' | ']' | '}'�?�?
			}
			++length;
		}
		return m_msg.substr(beginPos + subPos, m_msg.length() - m_pos);
	}

	int GetIntValue(int subPos = 0) {
		return atoi(GetStringValue(subPos).data());
	}
	__int64 GetInt64Value() {
		return _atoi64(GetStringValue().data());
	}
	char GetCharValue() {
		return *GetStringValue().data();
	}
	double GetDoubleValue() {
		return atof(GetStringValue().data());
	}

	size_t GetPos() const { return m_pos; }

	string_view GetRawMessage() const {
		return m_msg;
	}
private:
	bool isValueEndToken(char c) {
		return c == ',' || c == '}' || c == ']';
	}


private:
	size_t m_pos = 0;
	std::string_view m_msg;
};

class MarketPublisher {
public:
	virtual void OnRtnDepthMarketData(CFTDDepthMarketDataField* pMarketData) = 0;
};

class SGEParser {
public:
	SGEParser(MarketPublisher& marketPublisher) : m_marketPublisher(marketPublisher) {}
	ParseResult ParseMarket(string_view const& msg);
private:
	void parseDepthMarket();
	void parseDeliveryMarket();
	void parsePricesInfo(CFTDDepthMarketDataField* pMarketData);
	Parser m_parser;
	MarketPublisher& m_marketPublisher;
};

