#include "stdafx.h"
#include "SGESerializer.h"
#include "FtdDataStruct.h"

#include <fmt/format.h>
#include <fmt/compile.h>

namespace {
	class SGERequestIdACC {
	public:
		static int Generate() {
			static int s_base = 1;
			return s_base++;
		}
	};

	static char ConvertDirectionToSGE(type::Direction direction) {
		return type::IsBuy(direction) ? 'b' : 's';
	}

	static char ConvertOperationToSGE(type::OffsetFlag operation) {
		switch (operation) {
		case type::OffsetFlag::Open: return '0';
		case type::OffsetFlag::Close: return '1';
		default: return '2';		//其他全是强平
		}
	}
	using CC = char const*;

}
namespace {
	template<int l>
	void assign(CStringType<l>& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), str.length());
		target = tmp;
	}

	void assign(CIntType& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), str.length());
		target = atoi(tmp);
	}

	void assign(CLongType& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), str.length());
		target = atoi(tmp);
	}

	void assign(CCharType& target, string_view const& str) {
		target = str[0];
	}

	template<int l, int p>
	void assign(CDoubleType<l, p>& target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), str.length());
		target = atof(tmp);
	}
	void assign(char* target, string_view const& str) {
		char tmp[1024] = "";
		memcpy(tmp, str.data(), str.length());
		strcpy_s(target, str.length() + 1, tmp);
	}

	void assign(char& target, string_view const& str) {
		target = str[0];
	}

	void assign(long long& target, string_view const& str) {
		target = _atoi64(str.data());
	}
}

template<class Target>
bool copy_if_key(char const* expectedKey, string_view const& key, Target& target, Parser& parser) {
	if (expectedKey == key) {
		assign(target, parser.GetStringValue());
		return true;
	}
	return false;
}
inline bool CopyErrorID(char const* ekey, string_view const& key, CErrorIDType& errorID, Parser& parser) {
	if (key == ekey) {
		errorID = parser.GetIntValue(3);
		return true;
	}
	return false;
}
#define COPY_CONTINUE(ekey,target) if(copy_if_key(ekey, key, target, m_parser)) continue;
#define COPY_ERRORID(ekey,target) if (CopyErrorID(ekey, key, target,m_parser)) continue;


inline char const* SGEFormatter::head(char const* msgType) {
	memset(m_headBuffer, 0, sizeof(m_headBuffer));
	constexpr auto compiled_format =
		fmt::compile<char const*, int, int>(
			FMT_STRING("X01=GTP1.0,X03={},X04=1,X05={},X06=L,X07={}"));
	int requestId = SGERequestIdACC::Generate();
	fmt::format_to(m_headBuffer, compiled_format, 
		msgType, requestId, requestId);
	return m_headBuffer;
}

FormatResult SGEFormatter::ToLoginRequest(char const* encryptedPwd, char const* randomStr, char const* localIP, char const* sgeVersion) {
	const char* msgType = "A100";
	memset(m_msgBuffer, 0, sizeof(m_msgBuffer));
	constexpr auto compiled_format =
		fmt::compile<CC, CC, CC, CC, CC, CC, CC>(
			FMT_STRING("{},M20={},M60={},M66={},X98={},X25=TRA,X50={},X55={}"));
	fmt::format_to(m_msgBuffer, compiled_format,
		head(msgType), m_seatInfo.ParticipantID.value(), m_seatInfo.TraderID.value(),
		encryptedPwd, randomStr, localIP, sgeVersion);
//	strcat_s(m_msgBuffer, ",X20=[{X04=3,X05=-1},{X04=4,X05=-1},{X04=6,X05=-1}]");
	/*
	X04=3 私有流，指回报，这里不能填写-1 0等，否则只返回自然日的数据，对于夜盘重启回放就有问题
	X04=4 公共流, 指公共的信息，比如开盘信息，这里不填，原因与X04=3相同，我们需要得到最后的开盘信号
	X04=6 行情流，我们需要指定-1，也就是quick模式，只收最新的数据
	*/
	strcat_s(m_msgBuffer, ",X20=[{X04=6,X05=-1}]"); // 
	return { int(strlen(m_msgBuffer)) ,m_msgBuffer };
}

FormatResult SGEFormatter::ToStorageLoginRequest(char const* encryptedPwd, char const* randomStr, char const* localIP, char const* sgeVersion) {
	const char* msgType = "A100";
	memset(m_msgBuffer, 0, sizeof(m_msgBuffer));
	constexpr auto compiled_format =
		fmt::compile<CC, CC, CC, CC, CC, CC, CC>(
			FMT_STRING("{},M20={},M60={},M66={},X98={},X25=TRA,X50={},X55={}"));
	fmt::format_to(m_msgBuffer, compiled_format,
		head(msgType), m_seatInfo.ParticipantID.value(), m_seatInfo.TraderID.value(),
		encryptedPwd, randomStr, localIP, sgeVersion);
//	strcat_s(m_msgBuffer, ",X20=[{X04=3,X05=0}]");
	return { int(strlen(m_msgBuffer)) ,m_msgBuffer };
}

FormatResult SGEFormatter::ToQueryPosition(char const* ClientID) {
	const char* msgType = "T460";
	memset(m_msgBuffer, 0, sizeof(m_msgBuffer));
	constexpr auto compiled_format =
		fmt::compile<CC, CC, CC, CC, CC, CC, CC>(
			FMT_STRING("{},I00=02,M20={},M30={}"));
	fmt::format_to(m_msgBuffer, compiled_format,
		head(msgType), m_seatInfo.ParticipantID.value(), ClientID);
	return { int(strlen(m_msgBuffer)) ,m_msgBuffer };
}

FormatResult SGEFormatter::ToDeferInsertRequest(data::COrder const& order) {
	const char* msgType = "T200";
	memset(m_msgBuffer, 0, sizeof(m_msgBuffer));
	
	constexpr auto compiled_format =
		fmt::compile<CC, CC, CC, CC, CC, int, char, char, double,int,char>(
			FMT_STRING("{},M20={},M30={},M60={},I00=02,I10={},O01={},O02={},O03={},O06={:.2f},O07={},O15={},O65=1"));
	fmt::format_to(m_msgBuffer, compiled_format,
		head(msgType), m_seatInfo.ParticipantID.value(), order.ClientID.value(),
		m_seatInfo.TraderID.value(), order.InstrumentID.value(),
		order.OrderLocalID.value(), ConvertDirectionToSGE(order.Direction),
		ConvertOperationToSGE(order.OffsetFlag), order.LimitPrice.value(),
		order.Volume.value(), char(order.OrderPriceType));

	return { int(strlen(m_msgBuffer)) ,m_msgBuffer };
}

FormatResult SGEFormatter::ToSpotInsertRequest(data::COrder const& order) {
	const char* msgType = "T200";
	memset(m_msgBuffer, 0, sizeof(m_msgBuffer));

	constexpr auto compiled_format =
		fmt::compile<CC, CC, CC, CC, CC, int, char, double, int, char>(
			FMT_STRING("{},M20={},M30={},M60={},I00=00,I10={},O01={},O02={},O06={:.2f},O07={},O15={},O65=1"));
	fmt::format_to(m_msgBuffer, compiled_format,
		head(msgType), m_seatInfo.ParticipantID.value(), order.ClientID.value(),
		m_seatInfo.TraderID.value(), order.InstrumentID.value(),
		order.OrderLocalID.value(), ConvertDirectionToSGE(order.Direction),
		order.LimitPrice.value(),order.Volume.value(), char(order.OrderPriceType));

	return { int(strlen(m_msgBuffer)) ,m_msgBuffer};
}

FormatResult SGEFormatter::ToCancelRequest(data::COrderAction const& cancel) {
	const char* msgType = "T210";
	char const* productCode = cancel.ProductClass == type::ProductClass::Defer? "02" : "00";
	memset(m_msgBuffer, 0, sizeof(m_msgBuffer));
	constexpr auto compiled_format =
		fmt::compile<CC, CC, CC, CC, CC, CC, CC>(
			FMT_STRING("{},O00={},I00={},I10={},M20={},M30={},M60={}"));
	fmt::format_to(m_msgBuffer, compiled_format,
		head(msgType), cancel.OrderSysID.value(), productCode, cancel.InstrumentID.value(),
		m_seatInfo.ParticipantID.value(), cancel.ClientID.value(), m_seatInfo.TraderID.value());

	return { int(strlen(m_msgBuffer)) ,m_msgBuffer };
}

ParseResult SGEParser::ParseMarket(string_view const& msg) {
	m_parser.Init(msg);
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		if ("X03" == m_parser.GetKey()) {
			auto value = m_parser.GetStringValue();
			if (value == "Q012") {			//竞价行情
				parseDepthMarket();
			} else if (value == "Q202") {	//交割行情
				parseDeliveryMarket();
			}
		}
	}
	return {};
}

void SGEParser::parseDepthMarket() {
	CFTDDepthMarketDataField marketData;
	memset(&marketData, 0, sizeof(CFTDDepthMarketDataField));
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "I10") {
			if (!marketData.InstrumentID.isNull()) {
				m_marketPublisher.OnRtnDepthMarketData(&marketData);	//合约代码不为空，先发送上一条行情数据
			}
			assign(marketData.InstrumentID, m_parser.GetStringValue());
			continue;
		}
		if (key == "Q00") {
			parsePricesInfo(&marketData);
		}
		COPY_CONTINUE("Q11", marketData.TradingDay);
		COPY_CONTINUE("Q12", marketData.UpdateTime);
		COPY_CONTINUE("Q13", marketData.PreSettlementPrice);
		COPY_CONTINUE("Q14", marketData.PreClosePrice);
		COPY_CONTINUE("Q15", marketData.OpenPrice);
		COPY_CONTINUE("Q16", marketData.HighestPrice);
		COPY_CONTINUE("Q17", marketData.LowestPrice);
		COPY_CONTINUE("Q18", marketData.LastPrice);
		COPY_CONTINUE("Q19", marketData.ClosePrice);
		COPY_CONTINUE("Q20", marketData.SettlementPrice);
		COPY_CONTINUE("Q25", marketData.Weight);
		COPY_CONTINUE("Q26", marketData.UpperLimitPrice);
		COPY_CONTINUE("Q27", marketData.OpenInterest);
		COPY_CONTINUE("Q30", marketData.Turnover);
		COPY_CONTINUE("Q31", marketData.AvgPrice);
		COPY_CONTINUE("Q36", marketData.LowerLimitPrice);
		COPY_CONTINUE("O64", marketData.Volume);
		COPY_CONTINUE("T80", marketData.SequenceNo);
		if (key == "X06") {
			m_marketPublisher.OnRtnDepthMarketData(&marketData);		//结束标志，发送最后一条行情数据
		}
	}
}
void SGEParser::parseDeliveryMarket() {
	CFTDDepthMarketDataField ftdMarketData;
	CFTDDepthMarketDataField* pMarketData = &ftdMarketData;
	memset(pMarketData, 0, sizeof(CFTDDepthMarketDataField));
	for (auto hasKey = m_parser.GotoNextKey(); hasKey; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "I10") {
			if (strlen(pMarketData->InstrumentID) > 0) {
				m_marketPublisher.OnRtnDepthMarketData(pMarketData);	//合约代码不为空，先发送上一条行情数据
			}
			assign(pMarketData->InstrumentID, m_parser.GetStringValue());
			continue;
		}
		COPY_CONTINUE("Q22", pMarketData->BidVolume1);	//交割买量
		COPY_CONTINUE("Q24", pMarketData->AskVolume1);	//交割卖量
		COPY_CONTINUE("Q33", pMarketData->BidVolume2);	//中立仓买量
		COPY_CONTINUE("Q32", pMarketData->AskVolume2);	//中立仓卖量
		if (key == "X06") {
			m_marketPublisher.OnRtnDepthMarketData(pMarketData);		//结束标志，发送最后一条行情数据
		}
	}
}
void SGEParser::parsePricesInfo(CFTDDepthMarketDataField* pMarketData) {
	int shiftTag = 1;
	for (auto hasKey = m_parser.GotoNextKey(); hasKey, shiftTag < 10; hasKey = m_parser.GotoNextKey()) {
		auto key = m_parser.GetKey();
		if (key == "Q21") {
			switch (shiftTag) {
			case 1:	pMarketData->BidPrice1 = m_parser.GetDoubleValue();
				break;
			case 2:	pMarketData->BidPrice2 = m_parser.GetDoubleValue();
				break;
			case 3:	pMarketData->BidPrice3 = m_parser.GetDoubleValue();
				break;
			case 4:	pMarketData->BidPrice4 = m_parser.GetDoubleValue();
				break;
			case 5:	pMarketData->BidPrice5 = m_parser.GetDoubleValue();
				break;
			default:break;
			}
		} else if (key == "Q22") {
			switch (shiftTag) {
			case 1:	pMarketData->BidVolume1 = m_parser.GetIntValue();
				break;
			case 2:	pMarketData->BidVolume2 = m_parser.GetIntValue();
				break;
			case 3:	pMarketData->BidVolume3 = m_parser.GetIntValue();
				break;
			case 4:	pMarketData->BidVolume4 = m_parser.GetIntValue();
				break;
			case 5:	pMarketData->BidVolume5 = m_parser.GetIntValue();
				break;
			default:break;
			}
		} else if (key == "Q23") {
			switch (shiftTag) {
			case 1:	pMarketData->AskPrice1 = m_parser.GetDoubleValue();
				break;
			case 2:	pMarketData->AskPrice2 = m_parser.GetDoubleValue();
				break;
			case 3:	pMarketData->AskPrice3 = m_parser.GetDoubleValue();
				break;
			case 4:	pMarketData->AskPrice4 = m_parser.GetDoubleValue();
				break;
			case 5:	pMarketData->AskPrice5 = m_parser.GetDoubleValue();
				break;
			default:break;
			}
		} else if (key == "Q24") {
			switch (shiftTag) {
			case 1:	pMarketData->AskVolume1 = m_parser.GetIntValue();
				break;
			case 2:	pMarketData->AskVolume2 = m_parser.GetIntValue();
				break;
			case 3:	pMarketData->AskVolume3 = m_parser.GetIntValue();
				break;
			case 4:	pMarketData->AskVolume4 = m_parser.GetIntValue();
				break;
			case 5:	pMarketData->AskVolume5 = m_parser.GetIntValue();
				break;
			default:break;
			}
			++shiftTag;
		}
	}
}