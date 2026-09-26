#pragma once

char const TRADE_STREAM = '1';		//交易流	对话模式 非可靠（没有应答，重连后根据本地委托号再次发送|发起查询）
char const RETURN_STREAM = '2';		// 回报流	私有模式 可靠（针对多个接入单元，不使用）
char const PRIVATE_STREAM = '3';	//私有流	私有模式 可靠（针对单个接入单元）
char const PUBLIC_STREAM = '4';		//公共流	广播模式 可靠（采用QUICK模式，不收过期的市场信息）
char const QUERY_STREAM = '5';		// 查询流	对话模式 非可靠	使用Timeout机制
char const MARKET_STREAM = '6';		//行情流	广播模式 可靠（直接-1，采用QUICK模式）
// 可靠数据流序号如果小于最大序号，过期消息，丢弃；如果跳号直接断开连接（要查询后恢复）

int const SGE_MSG_LEN = 4096;

// TSGESeatIDType是一个交易席位代码类型（4位会员代码+2位序列号，从01开始递增）
typedef char TSGESeatIDType[8];

// TSGETraderIDType是一个交易员代码类型
typedef char TSGETraderIDType[12];

// TSGEClientIDType是一个客户代码类型
typedef char TSGEClientIDType[12];

// TSGEAddressType是一个IP地址类型
typedef char TSGEAddressType[30];

// TSGEPasswordType是一个登录密码类型
typedef char TSGEPasswordType[60];

// TSGEInstrumentIDType是一个合约代码类型
typedef char TSGEInstrumentIDType[10];

// TSGEPriceType是一个价格类型	N(12,3)
typedef double TSGEPriceType;

// TSGEWeightType是一个重量类型	N(12,3)
typedef double TSGEWeightType;

// TSGERateType是一个比例类型	N(16,3)
typedef double TSGERateType;

// TSGEVolumeType是一个金额类型	N18(按分推送)
typedef double TSGEVolumeType;

// TSGEQuantityType是一个数量类型
typedef int TSGEQuantityType;

// TSGEDateType是一个日期类型YYYYMMDD
typedef char TSGEDateType[9];

// TSGETimeType是一个时间类型HH:MM::SS
typedef char TSGETimeType[9];

// TSGEMaxLocalOrderIDType是一个最大报单号类型
typedef char TSGEMaxLocalOrderIDType[16];

// TSGESystemIDType是系统号
typedef char TSGESystemIDType[21];

// TSGERspCodeType是一个响应代码类型（RSP＋6位数字，000000表示操作成功，其余数字表示操作错误）
typedef int TSGERspCodeType;

// TSGERspMsgType是一个响应消息类型
typedef char TSGERspMsgType[256];

// TSGEVarietyID是一个交割品种代码
typedef int TSGEVarietyID;


#define SGE_D_Buy 'b'				// 买
#define SGE_D_Sell 's'				// 卖
// TSGEDirectionType是一个买卖方向类型
typedef char TSGEDirectionType;

#define SGE_OP_Open '0'				// 开仓
#define SGE_OP_Close '1'			// 平仓
#define SGE_OP_ForceClose '2'		// 强平
// TSGEOffsetFlagType是一个开平标志类型
typedef char TSGEOffsetFlagType;

#define SGE_LimitPrice '0'			// 限价普通指令
#define SGE_LimitPrice_FOK '1'		// 限价FOK:立即全部成交否则撤单
#define SGE_LimitPrice_FAK '2'		// 限价FAK:立即成交剩余撤单
// TSGEOrderPriceType是一个报单价格条件类型
typedef char TSGEOrderPriceType;

#define SGE_OC_Defer '0'			// 延期
#define SGE_OC_Spot	'1'				// 现货
#define SGE_OC_Delivery '2'			// 黄金交割
#define SGE_OC_Neutral '3'			// 黄金中立仓
#define SGE_OC_Unknown '9'			// 暂时未知
// TSGEProductClassType是一个产品类别类型
typedef char TSGEProductClassType;

#define SGE_LONG_PAY '1'			// 多付空
#define SGE_SHORT_PAY '2'			// 空付多
#define SGE_EQUAL '3'				// 平衡
// TSGEDirectionType是一个买卖方向类型
typedef char TSGEDefferFeeDirectionType;

#define SGE_IS_BeforeTrading '2'	// 开盘
#define SGE_IS_AuctionOrdering '3'	// 集合竞价报单
#define SGE_IS_AuctionMatch '4'		// 集合竞价撮合
#define SGE_IS_Continous '5'		// 连续交易
#define SGE_IS_NoTrading '6'		// 暂停交易
#define SGE_IS_DeliveryAppend '7'	// 交割申报
#define SGE_IS_DeliveryEnd '8'		// 交割结束
#define SGE_IS_MidAppend '9'		// 中立仓申报
#define SGE_IS_MidBalance 'A'		// 中立仓平衡
#define SGE_IS_Closed 'B'			// 收盘
// TSGEInstrumentStatusType是一个合约状态类型
typedef char TSGEInstrumentStatusType;

#define SGE_Market_Spot "00"		// 现货市场
#define SGE_Market_Extension "02"	// 延期市场
//TSGEMarketIdType是一个市场代码类型
typedef char TSGEMarketIDType[3];

#define SGE_E_NORMAL '1'			// 普通
#define SGE_E_Emergency '2'			// 应急
// TSGEMatchType是一个撮合类型
typedef char TSGEMatchType;

#define SGE_S_Member			'1'	// 会服系统
#define SGE_S_Broker			'2'	// 二级系统
#define SGE_S_Emergency			'3'	// 紧急资金划转
#define SGE_S_Warehouse			'4'	// 仓储交割系统
#define SGE_S_App				'a'	// APP渠道
#define SGE_S_Trader			'c'	// 红马甲
// TSGEOrderSourceType是一个报单来源类型
typedef char TSGEOrderSourceType;

#define SGE_IS_NoTrade			'o'	// 未成交
#define SGE_IS_PartTraded		'p'	// 部分成交
#define SGE_IS_AllTraded		'c'	// 完全成交
#define SGE_IS_Canceled			'd'	// 用户删除
#define SGE_IS_SysCanceled		's'	// 系统删除
#define SGE_IS_UrgencyCanceled	'u'	// 应急撤单
#define SGE_IS_AutoCanceled		'a'	// 自动撤单
#define SGE_IS_Queueing			'1'	// 正在报入
#define SGE_IS_Abandon			'2'	// 无效
#define SGE_IS_PartFinish		'3'	// 部成部撤
// TSGEOrderStatusType是一个报单状态类型
typedef char TSGEOrderStatusType;

inline char const* Desc(TSGEOrderStatusType status) {
	switch (status) {
	case SGE_IS_NoTrade:			return "Ordered-(o)";
	case SGE_IS_PartTraded:			return "Partial-(p)";
	case SGE_IS_AllTraded:			return "Traded-(c)";
	case SGE_IS_Canceled:			return "Canceled-(d)";
	case SGE_IS_SysCanceled:		return "SysCanceled-(s)";
	case SGE_IS_UrgencyCanceled:	return "UrgencyCanceled-(u)";
	case SGE_IS_AutoCanceled:		return "AutoCanceled-(a)";
	case SGE_IS_Queueing:			return "Queuing-(1)";
	case SGE_IS_Abandon:			return "Abandon-(2)";
	case SGE_IS_PartFinish:			return "PartFinish-(3)";
	default:	return "NA";
	}
}

