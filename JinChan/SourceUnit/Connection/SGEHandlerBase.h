#pragma once

#include <string_view>

#include "SGE/sge_api.h"
#include "SGE/sge_packet_struct.h"
#include "SGE/sge_safe_client_interface.h"
#include "SGE/keyfld_enc.h"

#include "SafeHandler.h"
#include "SGESerializer.h"
#include "Repository.h"
#include "ZTPType.h"


char const TRADE_STREAM = '1';		//交易流	对话模式 非可靠（没有应答，重连后根据本地委托号再次发送|发起查询）
char const RETURN_STREAM = '2';		//回报流	私有模式 可靠（针对多个接入单元，不使用）
char const PRIVATE_STREAM = '3';	//私有流	私有模式 可靠（针对单个接入单元）
char const PUBLIC_STREAM = '4';		//公共流	广播模式 可靠（采用QUICK模式，不收过期的市场信息）
char const QUERY_STREAM = '5';		//查询流	对话模式 非可靠	使用Timeout机制
char const MARKET_STREAM = '6';		//行情流	广播模式 可靠（直接-1，采用QUICK模式）
int const SGE_MSG_LEN = 4096;

//API类型
struct SgeLinkType {
	enum TYPE { QUOTE = 0, TRADE };
};

//国密接口连接上下文
typedef struct context {
	api_safe_client_t* SafeApiPtr;					//国密Api
	api_safe_client_link_t* SafeLinkPtr;			//设备Handle
	api_safe_client_session_t* SafeSessionPtr;		//国密会话
	sge_session_t* SGESessionPtr;					//SGE会话

	sge_session_callbacks_t SGECBList;				//连接回调接口
	client_dev_cb_list_t SafeCBList;				//安全回调接口
	unsigned char SessionKey[34] = "";					//加密密钥
	bool IsHandshake;								//握手状态
} SGEContext;

class CFTDPackage;
class CZTPPackage;
class SafeHandler;
class CMarketFlowManager;
class CMarketRepository;

std::string Base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::vector<BYTE> Base64_decode(std::string const& encoded_string);
void FormatSafeInfo(char const* oriBuffer, size_t bufLen, char* destBuffer);
char const* SGEStatusDescribe(type::InstrumentStatus status);