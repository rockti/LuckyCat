#pragma once
#include "SGE/SGEStruct.h"

struct FormatResult {
	char const* Message = nullptr;
	int Size = 0;
};

struct CSeatInfo;
struct OrderInfo;
class OrderCancel;
class SGEFormatter {
public:
	SGEFormatter(CSeatInfo const& seatInfo) : m_seatInfo(seatInfo) {}

	FormatResult ToQueryFund();				// 资金查询T450-T451
	FormatResult ToQueryStorage();			// 库存查询T470-T471
	FormatResult ToQueryPosition();			// 持仓查询T460-T461
	FormatResult ToLoginRequest(char const* seatID, char const* traderID, char const* encryptedKey, char const* randomStr, char const* localIP, char const* sgeVersion);
	FormatResult ToInsertRequest(OrderInfo const& order);
	FormatResult ToCancelRequest(OrderCancel const& cancel);

private:
	char const* head(char const* msgType);
	CSeatInfo const& m_seatInfo;
	char m_headBuffer[128] = "";
	char m_bodyBuffer[1024] = "";
};

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
		return m_msg.substr(m_pos - 3, 3);	// 截取3位域号 eg：X39
	}
	std::string_view GetStringValue(int subPos = 0) {
		size_t beginPos = ++m_pos;	//plus'='
		size_t length = 0;
		for (size_t i = beginPos; i < m_msg.length(); ++i) {
			if (isValueEndToken(m_msg[i])) {
				m_pos += length;
				return m_msg.substr(beginPos + subPos, length - subPos);
			}
			++length;
		}
		return m_msg.substr(beginPos + subPos, m_msg.length() - m_pos);
	}

	int32_t GetIntValue(int subPos = 0) {
		return atoi(GetStringValue(subPos).data());
	}
	int64_t GetInt64Value() {
		return _atoi64(GetStringValue().data());
	}
	char GetCharValue() {
		return *GetStringValue().data();
	}
	double GetDoubleValue() {
		return atof(GetStringValue().data());
	}

	size_t GetPos() const { return m_pos; }

	std::string_view GetRawMessage() const {
		return m_msg;
	}
private:
	bool isValueEndToken(char c) {
		return c == ',' || c == '}' || c == ']';	// 结束转义符
	}
private:
	size_t m_pos = 0;
	std::string_view m_msg;
};
