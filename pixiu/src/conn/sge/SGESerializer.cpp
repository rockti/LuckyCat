#include "pch.h"
#include "SGESerializer.h"
#include "conn/ExchangeConn.h"
#include "model/dealing/ExchangeRequest.h"
#include <fmt/format.h>
#include <fmt/compile.h>

namespace {
	using CC = char const*;
	static char ConvertDirectionToSGE(Direction direction) {
		return IsBuy(direction) ? 'b' : 's';
	}
	static char ConvertOperationToSGE(Operation operation) {
		switch (operation) {
		case Operation::OPEN:	return SGE_OP_Open;			
		case Operation::OFFSET:	return SGE_OP_Close;
		default:	return '9';	// 用非法值，诱发委托错误
		}
	}
	static char ConvertOrderTypeToSGE(OrderType orderType) {
		switch (orderType) {
		case OrderType::NORMAL: return '0';
		case OrderType::FAK:	return '2';
		default: return '9';	// 用非法值，诱发委托错误
		}
	}
}

inline char const* SGEFormatter::head(char const* msgType) {
	memset(m_headBuffer, 0, sizeof(m_headBuffer));
	constexpr auto compiled_format = fmt::compile<char const*, int, int>(FMT_STRING("X01=GTP1.0,X03={},X04=1,X05={},X06=L,X07={}"));
	int requestId = RequestIdACC::Generate();
	fmt::format_to(m_headBuffer, compiled_format, msgType, requestId, requestId);		// X07:消息来源标识 (用请求号标识）
	return m_headBuffer;
}

FormatResult SGEFormatter::ToQueryFund() {
	const char* msgType = "T450";
	memset(m_bodyBuffer, 0, sizeof(m_bodyBuffer));
	constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},M20={}"));
	fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID);
	return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
}
FormatResult SGEFormatter::ToQueryStorage() {
	const char* msgType = "T470";
	memset(m_bodyBuffer, 0, sizeof(m_bodyBuffer));
	constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},M20={},M30={}"));
	fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID, m_seatInfo.ClientID);
	return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
}
FormatResult SGEFormatter::ToQueryPosition() {
	const char* msgType = "T460";
	memset(m_bodyBuffer, 0, sizeof(m_bodyBuffer));
	constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},I00=02,M20={},M30={}"));
	fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID, m_seatInfo.ClientID);
	return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
}
FormatResult SGEFormatter::ToLoginRequest(char const* seatID, char const* traderID, char const* encryptedKey, char const* randomStr, char const* localIP, char const* sgeVersion) {
	const char* msgType = "A100";
	memset(m_bodyBuffer, 0, sizeof(m_bodyBuffer));
	constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},M20={},M60={},M66={},X98={},X25=TRA,X50={},X55={}"));
	fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), seatID, traderID, encryptedKey, randomStr, localIP, sgeVersion);

// 	X04=3 私有流，指回报，这里不能填写-1 0等，否则只返回自然日的数据，对于夜盘重启回放就有问题
// 	X04=4 公共流, 指公共的信息，比如开盘信息，这里不填，原因与X04=3相同，我们需要得到最后的开盘信号
// 	X04=6 行情流，我们需要指定-1，也就是quick模式，只收最新的数据s
//	strcat_s(m_bodyBuffer, ",X20=[{X04=3,X05=-1},{X04=4,X05=-1},{X04=6,X05=-1}]");

	strcat_s(m_bodyBuffer, ",X20=[{X04=6,X05=-1}]");
	return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
}

FormatResult SGEFormatter::ToInsertRequest(OrderInfo const& order) {
	memset(m_bodyBuffer, 0, sizeof(m_bodyBuffer));
	switch (order.ProductClass) {
	case SGE_OC_Defer: {
		const char* msgType = "T200";
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, int, char, double, int, char>(
			FMT_STRING("{},M20={},M30={},M60={},I00=02,I10={},O01={},O02={},O03={},O06={:.2f},O07={},O15={},O65=1"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID
			, order.InstCode.Str(), order.LocalId, ConvertDirectionToSGE(order.Direction), ConvertOperationToSGE(order.Operation)
			, order.Price.value(), order.Quantity, ConvertOrderTypeToSGE(order.OrderType));
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };

	}
	break;
	case SGE_OC_Spot: {
		const char* msgType = "T200";
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, int, char, double, int, char>(
			FMT_STRING("{},M20={},M30={},M60={},I00=00,I10={},O01={},O02={},O06={:.2f},O07={},O15={},O65=1"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID
			, order.InstCode.Str(), order.LocalId, ConvertDirectionToSGE(order.Direction)
			, order.Price.value(), order.Quantity, ConvertOrderTypeToSGE(order.OrderType));
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
	}
	break;
	case SGE_OC_Delivery: {
		const char* msgType = "T090";
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, int, char, char, double, int, char>(
			FMT_STRING("{},M20={},M30={},M60={},I10={},O01={},O02={},O07={}"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID
			, order.InstCode.Str(), order.LocalId, ConvertDirectionToSGE(order.Direction), order.Quantity);
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
	}
	break;
	case SGE_OC_Neutral: {
		const char* msgType = "T120";
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, int, char, char, double, int, char>(
			FMT_STRING("{},M20={},M30={},M60={},I10={},O01={},O02={},O07={}"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID
			, order.InstCode.Str(), order.LocalId, ConvertDirectionToSGE(order.Direction), order.Quantity);
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
	}
	break;
	default:	
		return {};
	}
}
FormatResult SGEFormatter::ToCancelRequest(OrderCancel const& cancel) {
	memset(m_bodyBuffer, 0, sizeof(m_bodyBuffer));

	switch (cancel.GetProductClass()) {
	case SGE_OC_Unknown:	return {};
	case SGE_OC_Delivery: {
		const char* msgType = "T100";	// 交割撤单
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},O00={},I10={},M20={},M30={},M60={}"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), cancel.GetSystemNo().Str(), cancel.GetInstCode().Str()
			, m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID);
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
	}
	break;
	case SGE_OC_Neutral: {
		const char* msgType = "T130";	// 中立仓撤单
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},O00={},I10={},M20={},M30={},M60={}"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), cancel.GetSystemNo().Str(), cancel.GetInstCode().Str()
			, m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID);
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
	}
	break;
	default: {
		const char* msgType = "T210";	// 竞价撤单
		char const* productCode = (SGE_OC_Spot == cancel.GetProductClass()) ? "00" : "02";
		constexpr auto compiled_format = fmt::compile<CC, CC, CC, CC, CC, CC, CC>(FMT_STRING("{},O00={},I00={},I10={},M20={},M30={},M60={}"));
		fmt::format_to(m_bodyBuffer, compiled_format, head(msgType), cancel.GetSystemNo().Str(), productCode, cancel.GetInstCode().Str()
			, m_seatInfo.SeatID, m_seatInfo.ClientID, m_seatInfo.TraderID);
		return { m_bodyBuffer, int(strlen(m_bodyBuffer)) };
	}
	}
}

