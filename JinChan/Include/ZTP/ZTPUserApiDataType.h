#pragma once
//对话接口用户数据类型
#include <cstdint>
//TZtpApiSequenceSeriesType是一个数据流标号类型(复用TopicID)
typedef uint32_t TZtpApiSequenceSeriesType;

//TZtpApiTopicIDType是一个主题号类型
typedef uint32_t TZtpApiTopicIDType;

//TZtpApiSequenceNoType是一个序号类型
typedef uint32_t TZtpApiSequenceNoType;

//TZtpApiSessionIDType是一个会话编号类型
typedef uint32_t TZtpApiSessionIDType;

//TZtpApiVarietyID是一个交割品种代码
typedef uint32_t TZtpApiVarietyIDType;

//TZtpApiErrorIDType是一个错误代码类型
typedef uint32_t TZtpApiErrorIDType;

//TZtpApiMillisecType是一个最后修改毫秒类型
typedef uint32_t TZtpApiMillisecType;

//TZtpApiMultipleType是一个合约乘数类型
typedef uint32_t TZtpApiMultipleType;

//TZtpApiVolumeType是一个量类型
typedef uint32_t TZtpApiVolumeType;

//APIIDType是一个通道类型
typedef uint32_t APIIDType;

//TZtpApiQuantityType是一个大额数量类型
typedef int64_t TZtpApiQuantityType;


//TZtpApiMoneyType是一个资金类型
typedef double TZtpApiMoneyType;

//TZtpApiPriceType是一个价格类型
typedef double TZtpApiPriceType;

//TZtpApiPriceTickType是一个最小变动价位类型
typedef double TZtpApiPriceTickType;

//TZtpApiRateType是一个比例类型
typedef double TZtpApiRateType;

//TZtpApiWeightType是一个重量类型
typedef double TZtpApiWeightType;

///TZtpApiFeeType是一个费用类型
typedef double TZtpApiFeeType;

//TZtpApiMsgTypeType是一个消息类型
typedef char TZtpApiMsgTypeType[4];

//TZtpApiDateType是一个日期类型YYYYMMDD
typedef char TZtpApiDateType[9];

//TZtpApiTimeType是一个时间类型HH:MM::SS
typedef char TZtpApiTimeType[9];

//TZtpApiIPAddressType是一个IP地址类型
typedef char TZtpApiIPAddressType[21];

//TZtpApiSystemIDType是系统号
typedef char TZtpApiSystemIDType[21];

//TZtpApiTradeIDType是一个成交编号类型
typedef char TZtpApiTradeIDType[21];

//TZtpApiMaxLocalOrderIDType是一个最大本地报单号类型
typedef char TZtpApiMaxLocalOrderIDType[16];

//TZtpApiOrderLocalIDType是一个用户本地报单号类型
typedef char TZtpApiOrderLocalIDType[16];

//TZtpApiBrokerIDType是一个经纪公司编号类型
typedef char TZtpApiBrokerIDType[11];

//TZtpApiParticipantID是一个会员编号类型
typedef char TZtpApiParticipantIDType[11];

//TZtpApiAccountIDType是一个资金帐号类型
typedef char TZtpApiAccountIDType[11];

//TZtpApiInvestorIDType是一个投资帐号类型
typedef char TZtpApiInvestorIDType[11];

//TZtpApiExchangeIDType是一个交易(所)通道类型
typedef char TZtpApiExchangeIDType[11];

//TZtpApiClientIDType是一个交易编码类型
typedef char TZtpApiClientIDType[13];

//TZtpApiPasswordType是一个登录密码类型
typedef char TZtpApiPasswordType[16];

//TZtpApiProtocolInfoType是一个协议信息类型
typedef char TZtpApiProtocolInfoType[21];

//TZtpApiInstrumentIDType是一个合约代码类型
typedef char TZtpApiInstrumentIDType[11];

//TZtpApiProductIDType是一个品种代码类型
typedef char TZtpApiProductIDType[11];

//TZtpApiInstrumentNameType是一个合约名称类型
typedef char TZtpApiInstrumentNameType[31];


//TZtpApiUserCustom是一个用户自定义字段
typedef char TZtpApiUserCustom[21];


//TZtpApiErrorMsgType是一个响应消息类型
typedef char TZtpApiErrorMsgType[121];

const char ZTP_API_CF_CONTINUE = 'C';
const char ZTP_API_CF_LAST = 'L';
//TZtpApiChainFlagType是一个消息连续标志类型
typedef char TZtpApiChainFlagType;

enum TE_RESUME_TYPE
{
	ZTP_TERT_RESTART = 0,
	ZTP_TERT_RESUME,
	ZTP_TERT_QUICK
};

//交易用户
const char ZTP_API_UT_Person = '1';
//管理员
const char ZTP_API_UT_Manager = '2';
//TZtpApiUserType是一个用户类型
typedef char TZtpApiUserType;

//未知市场
const char ZTP_API_PCT_Unknown = '0';
//延期市场
const char ZTP_API_PCT_Defer = '1';
//现货市场
const char ZTP_API_PCT_Spot = '2';
//交割申报
const char ZTP_API_PCT_Delivery = '3';
//中立仓
const char ZTP_API_PCT_Neutral = '4';

//TZtpApiProductClassType是一个产品类型
typedef char TZtpApiProductClassType;

//买
const char ZTP_API_D_Buy = '0';
//卖
const char ZTP_API_D_Sell = '1';
//TZtpApiDirectionType是一个买卖方向类型
typedef char TZtpApiDirectionType;

//开仓
const char ZTP_API_OF_Open = '0';
//平仓
const char ZTP_API_OF_Close = '1';
//平今
const char ZTP_API_OF_CloseToday = '2';
//强平
const char ZTP_API_OF_ForceClose = '3';
//TZtpApiOffsetFlagType是一个开平标志类型
typedef char TZtpApiOffsetFlagType;

///投机（延期）
const char ZTP_API_Speculation = '1';
///套利（中立仓）
const char ZTP_API_Arbitrage = '2';
///套保（交割）
const char ZTP_API_Hedge = '3';
///做市商（现货）
const char ZTP_API_MarketMaker = '4';
//TZtpApiHedgeFlagType是一个套保标志类型
typedef char TZtpApiHedgeFlagType;

//限价普通指令
const char ZTP_API_LimitPrice = '0';
//限价FOK:全部成交否则撤单
const char ZTP_API_LimitPrice_FOK = '1';
//限价FAK:立即成交剩余撤单
const char ZTP_API_LimitPrice_FAK = '2';
typedef char TZtpApiOrderPriceType;

//初始化中
const char ZTP_API_IS_Initialize = '0';
//初始化完成，未开盘
const char ZTP_API_IS_BeforeTrading =  '1';
//开盘
const char ZTP_API_IS_Trading = '2';
//集合竞价报单
const char ZTP_APi_IS_AuctionOrdering = '3';
//集合竞价撮合
const char ZTP_API_IS_AuctionMatch = '4';
//连续交易
const char ZTP_API_IS_Continous = '5';
//暂停交易
const char ZTP_API_IS_Pause = '6';
//交割申报
const char ZTP_API_IS_DeliveryApp = '7';
//交割申报结束
const char ZTP_API_IS_DeliveryEnd = '8';
//中立仓申报
const char ZTP_API_IS_MiddleApp	= '9';
//交割申报撮合
const char ZTP_API_IS_DeliveryMatch = 'A';
//收盘
const char ZTP_API_IS_Closed = 'B';
//TZtpApiInstrumentStatusType是一个合约状态类型
typedef char TZtpApiInstrumentStatusType;

//系统拒绝
const char ZTP_API_IS_Abandon = 'a';
//未成交
const char ZTP_API_IS_NoTrade = 'n';
//部分成交
const char ZTP_API_IS_PartTraded = 'p';
//完全成交
const char ZTP_API_IS_AllTraded	= 't';
//已撤单
const char ZTP_API_IS_Canceled = 'c';
//正在报入
const char ZTP_API_IS_Queueing = 'i';
const char ZTP_API_IS_Failed = 'f';
//TZtpApiOrderStatusType是一个报单状态类型
typedef char TZtpApiOrderStatusType;

//未知
const char ZTP_API_DD_Unknown = '0';
//多付空
const char ZTP_API_DD_BuyToSell = '1';
//空付多
const char ZTP_API_DD_SellToBuy = '2';
//平
const char ZTP_API_DD_Balance = '3';
//TZtpApiDeferDirectionType是一个延期补偿费率支付方向类型
typedef char TZtpApiDeferDirectionType;
