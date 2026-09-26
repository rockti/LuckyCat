#pragma once

char const TRADE_STREAM = '1';		//交易流	对话模式 非可靠（没有应答，重连后根据本地委托号再次发送|发起查询）
char const RETURN_STREAM = '2';		//回报流	私有模式 可靠（针对多个接入单元，不使用）
char const PRIVATE_STREAM = '3';	//私有流	私有模式 可靠（针对单个接入单元）
char const PUBLIC_STREAM = '4';		//公共流	广播模式 可靠（采用QUICK模式，不收过期的市场信息）
char const QUERY_STREAM = '5';		//查询流	对话模式 非可靠	使用Timeout机制
char const MARKET_STREAM = '6';		//行情流	广播模式 可靠（直接-1，采用QUICK模式）
//可靠数据流序号如果小于最大序号，过期消息，丢弃；如果跳号直接断开连接（要查询后恢复）

int const SGE_MSG_LEN = 2048;

//TGtpSequenceSeriesType是一个数据流标号类型(N2)
typedef int TGtpSequenceSeriesType;

//TGtpLengthType是一个长度类型(N4)
typedef int TGtpLengthType;

//TGtpSequenceNoType是一个序号类型
typedef int TGtpSequenceNoType;

//TGtpPriceType是一个金额类型	N18(按分推送)
typedef __int64 TGtpVolumeType;

//TGtpQuantityType是一个数量类型 N12
typedef int TGtpQuantityType;

//TGtpVarietyID是一个交割品种代码
typedef int TGtpVarietyID;

//TGtpPriceType是一个价格类型	N(12,3)
typedef double TGtpPriceType;

//TGtpWeightType是一个重量类型	N(12,3)
typedef double TGtpWeightType;

//TGtpRateType是一个比例类型	N(16,3)
typedef double TGtpRateType;

//TUstpGtpDateType是一个日期类型YYYYMMDD
typedef char TGtpDateType[9];

//TUstpGtpTimeType是一个时间类型HH:MM::SS
typedef char TGtpTimeType[9];

//TGtpRootIdType是一个消息来源标识
typedef char TGtpRootIdType[8];

//TGtpSenderIdType是一个发送方标识
typedef char TGtpSenderIdType[8];

//TGtpReceiverIdType是一个接收方标识
typedef char TGtpReceiverIdType[8];

//TGtpMsgTypeType是一个消息类型
typedef char TGtpMsgTypeType[4];

//TGtpBeginString是一个协议版本号类型
typedef char TGtpBeginString[8];

//TGtpVersionInfoType是一个版本信息类型
typedef char TGtpVersionInfoType[16];

//TGtpIPAddressType是一个IP地址类型
typedef char TGtpIPAddressType[16];

//TGtpSafeInfoType是一个安全信息类型
typedef char TGtpSafeInfoType[8192];

//TGtpSystemIdTYpe是系统号
typedef char TGtpSystemIdType[18];

//TGtpMaxLocalOrderIdType是一个最大报单号类型
typedef char TGtpMaxLocalOrderIdType[16];

//TGtpTradeIdType是一个成交编号类型
typedef char TGtpTradeIdType[18];

//TGtpAddressType是一个IP地址类型
typedef char TGtpAddressType[30];

//TGtpPassword是一个登录密码类型
typedef char TGtpPasswordType[60];

//TGtpInstrumentIDType是一个合约代码类型
typedef char TGtpInstrumentIDType[10];

//TGtpInstrumentNameType是一个合约名称类型
typedef char TGtpInstrumentNameType[12];

//TGtpTraderIdType是一个交易员代码类型
typedef char TGtpTraderIdType[12];

//TGtpSeatIdType是一个交易席位代码类型（4位会员代码+2位序列号，从01开始递增）
typedef char TGtpSeatIdType[8];

//TGtpClientIdType是一个客户代码类型
typedef char TGtpClientIdType[12];

//TGtpRspCodeType是一个响应代码类型（RSP＋6位数字，000000表示操作成功，其余数字表示操作错误）
typedef int TGtpRspCodeType;

//TGtpRspMsgType是一个响应消息类型
typedef char TGtpRspMsgType[200];

//TGtpChainFlagType是一个消息连续标志类型
#define GTP_USTP_CF_CONTINUE 'C';
#define GTP_USTP_CF_LAST 'L';
typedef char TGtpChainFlagType;

//现货市场
#define SGE_USTP_Market_Spot "00"
//现延期市场
#define SGE_USTP_Market_Extension "02"
//TGtpMarketIdType是一个市场代码类型
typedef char TGtpMarketIdType[3];

//普通
#define SGE_USTP_E_NORMAL '1'
//应急
#define SGE_USTP_E_Emergency '2'
//TGtpMatchType是一个撮合类型
typedef char TGtpMatchType;

//会服系统
#define SGE_USTP_S_Member		'1'
//二级系统
#define SGE_USTP_S_Broker		'2'
//紧急资金划转
#define SGE_USTP_S_Emergency	'3'
//仓储交割系统
#define SGE_USTP_S_Warehouse	'4'
//APP渠道
#define SGE_USTP_S_App			'a'
//红马甲
#define SGE_USTP_S_Trader		'c'
//TGtpOrderSourceType是一个报单来源类型
typedef char TGtpOrderSourceType;

//买
#define SGE_USTP_D_Buy 'b'
//卖
#define SGE_USTP_D_Sell 's'
//TGtpDirectionType是一个买卖方向类型
typedef char TGtpDirectionType;

///开仓
#define SGE_USTP_OF_Open '0'
///平仓
#define SGE_USTP_OF_Close '1'
///强平
#define SGE_USTP_OF_ForceClose '2'
//TGtpOffsetFlagType是一个开平标志类型
typedef char TGtpOffsetFlagType;

//未知
#define SGE_OC_Unknown '0'
//延期
#define SGE_OC_Defer '1'
//现货
#define SGE_OC_Spot	'2'
//交割申报
#define SGE_OC_Delivery '3'
//中立仓
#define SGE_OC_Neutral '4'
//TGtpOrderClassType是一个报单类别类型
typedef char TGtpOrderClassType;

///限价普通指令
#define SGE_USTP_LimitPrice '0'
///限价FOK:立即全部成交否则撤单
#define SGE_USTP_LimitPrice_FOK '1'
///限价FAK:立即成交剩余撤单
#define SGE_USTP_LimitPrice_FAK '2'
//TGtpOrderPriceType是一个报单价格条件类型
typedef char TGtpOrderPriceType;

//非交易
#define SGE_USTP_IS_BeforeTrading '2'
//集合竞价报单
#define SGE_USTP_IS_AuctionOrdering '3'
//集合竞价撮合
#define SGE_USTP_IS_AuctionMatch '4'
//连续交易
#define SGE_USTP_IS_Continous '5'
//暂停交易
#define SGE_USTP_IS_NoTrading '6'
//交割申报
#define SGE_USTP_IS_DeliveryAppend '7'
//交割结束
#define SGE_USTP_IS_DeliveryEnd '8'
//中立仓申报
#define SGE_USTP_IS_MidAppend '9'
//中立仓平衡
#define SGE_USTP_IS_MidBalance 'A'
//收盘
#define SGE_USTP_IS_Closed 'B'
//TGtpInstrumentStatusType是一个合约状态类型
typedef char TGtpInstrumentStatusType;

//未成交
#define SGE_USTP_IS_NoTrade			'o'
//部分成交
#define SGE_USTP_IS_PartTraded		'p'
//完全成交
#define SGE_USTP_IS_AllTraded		'c'
//用户删除
#define SGE_USTP_IS_Canceled		'd'
//系统删除
#define SGE_USTP_IS_SysCanceled		's'
//应急撤单
#define SGE_USTP_IS_UrgencyCanceled	'u'
//自动撤单
#define SGE_USTP_IS_AutoCanceled	'a'
//正在报入
#define SGE_USTP_IS_Queueing		'1'
//无效
#define SGE_USTP_IS_Abandon			'2'
//部成部撤
#define SGE_USTP_IS_PartFinish		'3'
//TGtpOrderStatusType是一个报单状态类型
typedef char TGtpOrderStatusType;

