#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZTPUserApiDataType.h"

//响应信息
struct CZtpApiRspInfoField
{
	//响应代码
	TZtpApiErrorIDType	ErrorID;
	//响应信息
	TZtpApiErrorMsgType	ErrorMsg;
};

//登录请求
struct CZtpApiReqLoginField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//资金帐号(必填)
	TZtpApiAccountIDType	AccountID;
	//密码(必填)
	TZtpApiPasswordType		Password;
	//用户类型(必填)
	TZtpApiUserType			UserType;
	//用户产品协议信息(必填)
	TZtpApiProtocolInfoType	UserProductInfo;
	//IP地址
	TZtpApiIPAddressType	IPAddress;
};
//登录应答
struct CZtpApiRspLoginField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//资金帐号
	TZtpApiAccountIDType	AccountID;
	//用户类型
	TZtpApiUserType			UserType;
	//交易日
	TZtpApiDateType			TradeDate;
	//系统产品协议信息
	TZtpApiProtocolInfoType	SystemProductInfo;
	//会话编号
	TZtpApiSessionIDType	SessionID;
	//最大本地报单号(目前不支持)
	TZtpApiMaxLocalOrderIDType	MaxOrderLocalID;
};

//录出请求
struct CZtpApiReqLogoutField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//交易用户代码(必填)
	TZtpApiAccountIDType	AccountID;
};
//录出应答
struct CZtpApiRspLogoutField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//交易用户代码
	TZtpApiAccountIDType	AccountID;
};

//用户口令修改
struct CZtpApiPasswordUpdateField 
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//交易用户代码
	TZtpApiAccountIDType	AccountID;
	//用户类型
	TZtpApiUserType			UserType;
	//旧密码
	TZtpApiPasswordType		OldPassword;
	//新密码
	TZtpApiPasswordType		NewPassword;
};

//合约查询请求
struct CZtpApiQryInstrumentField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//交易用户代码(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//产品类型 (ZTP_API_PCT_Unknown = '0',代表查询所有类型）(必填)
	TZtpApiProductClassType	ProductClass;
	//合约代码(*代表查询所有合约)(必填)
	TZtpApiInstrumentIDType	InstrumentID;
};
//合约信息
struct CZtpApiRspInstrumentField
{
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//合约名称
	TZtpApiInstrumentNameType	InstrumentName;
	//品种代码
	TZtpApiProductIDType	ProductID;
	//产品类型
	TZtpApiProductClassType	ProductClass;
	//多头限仓
	TZtpApiVolumeType	LongPosLimit;
	//空头限仓
	TZtpApiVolumeType	ShortPosLimit;
	//限价单最大下单量
	TZtpApiVolumeType	MaxLimitOrderVolume;
	//限价单最小下单量
	TZtpApiVolumeType	MinLimitOrderVolume;
	//市价单最大下单量
	TZtpApiVolumeType	MaxMarketOrderVolume;
	//市价单最小下单量
	TZtpApiVolumeType	MinMarketOrderVolume;
	//合约乘数
	TZtpApiMultipleType	VolumeMultiple;
	//最小变动价位
	TZtpApiPriceTickType	PriceTick;
};

//投资者资金账户查询请求
struct CZtpApiQryAccountField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
};
//投资者资金账户信息应答
struct CZtpApiRspAccountField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
	//交易编码
	TZtpApiDateType			TradingDay;
	//上日权益
	TZtpApiMoneyType		PreBalance;
	//上日可用
	TZtpApiMoneyType		PreAvaillable;
	//当日权益
	TZtpApiMoneyType		Balance;
	//当日可用
	TZtpApiMoneyType		Availlable;
	//入金金额
	TZtpApiMoneyType		Deposit;
	//出金金额
	TZtpApiMoneyType		Withdraw;
	//占用保证金
	TZtpApiMoneyType		Margin;
	//手续费
	TZtpApiMoneyType		Fee;
	//其他费用
	TZtpApiMoneyType		OtherFee;
	//平仓盈亏
	TZtpApiMoneyType		CloseProfit;
	//持仓盈亏
	TZtpApiMoneyType		PositionProfit;
	//冻结的保证金
	TZtpApiMoneyType		FrozenMargin;
	//冻结手续费
	TZtpApiMoneyType		FrozenFee;
	//多头占用保证金
	TZtpApiMoneyType		LongMargin;
	//空头占用保证金
	TZtpApiMoneyType		ShortMargin;
	//多头冻结的保证金
	TZtpApiMoneyType		LongFrozenMargin;
	//空头冻结的保证金
	TZtpApiMoneyType		ShortFrozenMargin;
	//动态权益
	TZtpApiMoneyType		DynamicRights;
	//风险度
	TZtpApiRateType			Risk;
};

//投资者持仓查询请求
struct CZtpApiQryPositionField
 {
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
};
//投资者持仓信息应答
struct CZtpApiRspPositionField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
	//交易日
	TZtpApiDateType			TradingDay;
	//交易所
	TZtpApiExchangeIDType	ExchangeID;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//合约类型
	TZtpApiProductClassType	ProductClass;
	//买卖方向
	TZtpApiDirectionType	Direction;
	//占用保证金
	TZtpApiMoneyType		UsedMargin;
	//今持仓量
	TZtpApiVolumeType		Position;
	//今持仓均价		
	TZtpApiMoneyType		PositionCost;
	//昨持仓量
	TZtpApiVolumeType		YdPosition;
	//昨持仓结算价		
	TZtpApiMoneyType		YdPositionPrice;
	//开仓冻结保证金
	TZtpApiMoneyType		FrozenMargin;
	//开平冻结手续费
	TZtpApiMoneyType		FrozenFee;
	//平仓冻结持仓
	TZtpApiVolumeType		FrozenPosition;
};

//投资者保证金查询请求
struct CZtpApiQryMarginField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//合约代码(*表示查询所有合约)(必填)
	TZtpApiInstrumentIDType	InstrumentID;
};

//投资者保证金查询应答
struct CZtpApiRspMarginField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//多头占用保证金按比例
	TZtpApiRateType		LongMarginRate;
	//多头保证金按手数
	TZtpApiMoneyType	LongMarginAmt;
	//空头占用保证金按比例
	TZtpApiRateType		ShortMarginRate;
	//空头保证金按手数
	TZtpApiMoneyType	ShortMarginAmt;
};
 
//投资者手续费查询请求
struct CZtpApiQryFeeField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//合约代码(*表示查询所有合约)(必填)
	TZtpApiInstrumentIDType	InstrumentID;
};
//投资者手续费查询应答
struct CZtpApiRspFeeField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//开仓手续费按比例
	TZtpApiRateType		OpenFeeRate;
	//开仓手续费按手数
	TZtpApiMoneyType	OpenFeeAmt;
	//平仓手续费按比例
	TZtpApiRateType		OffsetFeeRate;
	//平仓手续费按手数
	TZtpApiMoneyType	OffsetFeeAmt;
	//平今仓手续费按比例
	TZtpApiRateType		OffsetTodayFeeRate;
	//平今仓手续费按手数
	TZtpApiMoneyType	OffsetTodayFeeAmt;
};

//合约交易状态推送
struct CZtpApiRtnInstrumentStatusField
{
	//合约代码
	TZtpApiInstrumentIDType		InstrumentID;
	//合约交易信息
	TZtpApiInstrumentStatusType	InstrumentStatus;
};
//合约递延费率推送
struct CZtpApiRtnDeferFee
{
	//交易日
	TZtpApiDateType	TradeDate;
	//合约代码
	TZtpApiInstrumentIDType		InstrumentID;
	//支付方向
	TZtpApiDeferDirectionType	DeferDirection;
	//支付比例
	TZtpApiFeeType	DeferFee;
};

//行情订阅信息
struct CZtpApiSpecificInstrumentField
{
	//合约代码(不支持*)(必填)
	TZtpApiInstrumentIDType	InstrumentID;
};
//深度行情
struct CZtpApiDepthMarketDataField
{
	//行情Topic
	TZtpApiTopicIDType TopicNo;
	//行情序号
	TZtpApiSequenceNoType	SequenceNo;
	//合约代码
	TZtpApiInstrumentIDType	  InstrumentID;
	//昨收盘
	TZtpApiPriceType	PreClosePrice;
	//昨结算
	TZtpApiPriceType	PreSettlementPrice;
	//开盘价
	TZtpApiPriceType	OpenPrice;
	//最高价
	TZtpApiPriceType	HighestPrice;
	//最低价
	TZtpApiPriceType	LowestPrice;
	//最新价
	TZtpApiPriceType	LastPrice;
	//收盘价
	TZtpApiPriceType	ClosePrice;
	//结算价
	TZtpApiPriceType	SettlementPrice;
	//持仓量
	TZtpApiQuantityType OpenInterest;
	//涨停板
	TZtpApiPriceType	UpperLimitPrice;
	//跌停板
	TZtpApiPriceType	LowerLimitPrice;
	//成交重量
	TZtpApiWeightType	TradeWeight;
	//成交数量
	TZtpApiVolumeType	TradeVolume;
	//成交额
	TZtpApiMoneyType	Turnover;
	//当日均价
	TZtpApiPriceType	AvgPrice;
	//行情日期
	TZtpApiDateType		QuoteDate;
	//行情时间
	TZtpApiTimeType		QuoteTime;
	//行情毫秒
	TZtpApiMillisecType	Millisec;
	//申买价一
	TZtpApiPriceType	BidPrice1;
	//申买量一(交割|中立仓买量)
	TZtpApiVolumeType	BidVolume1;
	//申卖价一
	TZtpApiPriceType	AskPrice1;
	//申卖量一(交割|中立仓卖量)
	TZtpApiVolumeType	AskVolume1;
	//申买价二
	TZtpApiPriceType	BidPrice2;
	//申买量二
	TZtpApiVolumeType	BidVolume2;
	//申卖价二
	TZtpApiPriceType	AskPrice2;
	//申卖量二
	TZtpApiVolumeType	AskVolume2;
	//申买价三
	TZtpApiPriceType	BidPrice3;
	//申买量三
	TZtpApiVolumeType	BidVolume3;
	//申卖价三
	TZtpApiPriceType	AskPrice3;
	//申卖量三
	TZtpApiVolumeType	AskVolume3;
	//申买价四
	TZtpApiPriceType	BidPrice4;
	//申买量四
	TZtpApiVolumeType	BidVolume4;
	//申卖价四
	TZtpApiPriceType	AskPrice4;
	//申卖量四
	TZtpApiVolumeType	AskVolume4;
	//申买价五
	TZtpApiPriceType	BidPrice5;
	//申买量五
	TZtpApiVolumeType	BidVolume5;
	//申卖价五
	TZtpApiPriceType	AskPrice5;
	//申卖量五
	TZtpApiVolumeType	AskVolume5;
	//申买价六
	TZtpApiPriceType	BidPrice6;
	//申买量六
	TZtpApiVolumeType	BidVolume6;
	//申卖价六
	TZtpApiPriceType	AskPrice6;
	//申卖量六
	TZtpApiVolumeType	AskVolume6;
	//申买价七
	TZtpApiPriceType	BidPrice7;
	//申买量七
	TZtpApiVolumeType	BidVolume7;
	//申卖价七
	TZtpApiPriceType	AskPrice7;
	//申卖量七
	TZtpApiVolumeType	AskVolume7;
	//申买价八
	TZtpApiPriceType	BidPrice8;
	//申买量八
	TZtpApiVolumeType	BidVolume8;
	//申卖价八
	TZtpApiPriceType	AskPrice8;
	//申卖量八
	TZtpApiVolumeType	AskVolume8;
	//申买价九
	TZtpApiPriceType	BidPrice9;
	//申买量九
	TZtpApiVolumeType	BidVolume9;
	//申卖价九
	TZtpApiPriceType	AskPrice9;
	//申卖量九
	TZtpApiVolumeType	AskVolume9;
	//申买价十
	TZtpApiPriceType	BidPrice10;
	//申买量十
	TZtpApiVolumeType	BidVolume10;
	//申卖价十
	TZtpApiPriceType	AskPrice10;
	//申卖量十
	TZtpApiVolumeType	AskVolume10;
};

//用户账户关系查询请求
struct CZtpApiQryUserInvestorField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//资金帐号
	TZtpApiAccountIDType	AccountID;
};
//用户账户关系查询应答
struct CZtpApiRspUserInvestorField 
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//资金帐号
	TZtpApiAccountIDType	AccountID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
};

//用户交易编码查询请求
struct CZtpApiQryClientInfoField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//资金帐号
	TZtpApiAccountIDType	AccountID;
};
//用户交易编码查询应答
struct CZtpApiRspClientInfoField 
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//资金帐号
	TZtpApiAccountIDType	AccountID;
	//交易编码
	TZtpApiClientIDType		ClientID;
	//套保标志
	TZtpApiHedgeFlagType	HedgeFlag;
};

//输入报单
struct CZtpApiInputOrderField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//产品类型(必填)
	TZtpApiProductClassType	ProductClass;
	//合约代码(必填)
	TZtpApiInstrumentIDType	InstrumentID;
	//买卖方向(必填)
	TZtpApiDirectionType	Direction;
	//开平标志(必填)
	TZtpApiOffsetFlagType	OffsetFlag;
	//套保标志
	TZtpApiHedgeFlagType	HedgeFlag;
	//申报数量(必填)
	TZtpApiVolumeType		OrderQuantity;
	//报单价格(必填)
	TZtpApiPriceType		OrderPrice;
	//指令类型(必填)
	TZtpApiOrderPriceType	OrderPriceType;
	//本地报单编号(必填)
	TZtpApiOrderLocalIDType	OrderLocalID;
	//报单编号(成功应答会给)
	TZtpApiSystemIDType		OrderSysID;
	//用户自定义字段
	TZtpApiUserCustom		UserCustom;
};
//撤销报单
struct CZtpApiInputOrderActionField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//产品类型
	TZtpApiProductClassType	ProductClass;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//投保标志
	TZtpApiHedgeFlagType	HedgeFlag;
	//报单编号(必填)
	TZtpApiSystemIDType		OrderSysID;
	//待撤订单的本地报单编号(必填,系统会检查OrderSysID与OrderLocalID是否匹配)
	TZtpApiOrderLocalIDType	OrderLocalID;
	//撤单指令的本地编号(必填)
	TZtpApiOrderLocalIDType	OrderActionLocalID;
};

//报单查询请求
struct CZtpApiQryOrderField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//本报单编号(该参数可查指定系统号的委托,为*表示查所有，如果指定，将不使用其它查询条件)(必填)
	TZtpApiSystemIDType		OrderSysID;
	//合约代码(如果为*标识查询该市场下所有合约)(必填)
	TZtpApiInstrumentIDType	InstrumentID;
};
//成交查询请求
struct CZtpApiQryTradeField
{
	//经纪公司编号(必填)
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号(必填)
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码(必填)
	TZtpApiExchangeIDType	ExchangeID;
	//本报单编号(该参数可查指定系统号的委托，为*表示查看有,如果指定，将不使用其它查询条件)(必填)
	TZtpApiSystemIDType		OrderSysID;
	//合约代码(如果为*标识查询该市场下所有合约)(必填)
	TZtpApiInstrumentIDType	InstrumentID;
};

//报单信息
struct CZtpApiOrderField {
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//产品类型
	TZtpApiProductClassType	ProductClass;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//买卖方向
	TZtpApiDirectionType	Direction;
	//开平标志
	TZtpApiOffsetFlagType	OffsetFlag;
	//套保标志
	TZtpApiHedgeFlagType	HedgeFlag;
	//申报数量
	TZtpApiVolumeType		OrderQuantity;
	//报单价格
	TZtpApiPriceType		OrderPrice;
	//指令类型
	TZtpApiOrderPriceType	OrderPriceType;
	//本地报单编号
	TZtpApiOrderLocalIDType	OrderLocalID;
	//报单编号
	TZtpApiSystemIDType		OrderSysID;
	//申报时间
	TZtpApiTimeType			InsertTime;
	//撤销时间
	TZtpApiTimeType			CancelTime;
	//报单状态
	TZtpApiOrderStatusType	OrderStatus;
	//成交数量
	TZtpApiVolumeType		TradeVolume;
	//剩余数量
	TZtpApiVolumeType		RemainQuantity;
	//用户自定义字段
	TZtpApiUserCustom		UserCustom;
};
//成交信息
struct CZtpApiTradeField
{
	//经纪公司编号
	TZtpApiBrokerIDType		BrokerID;
	//投资者帐号
	TZtpApiInvestorIDType	InvestorID;
	//交易所代码
	TZtpApiExchangeIDType	ExchangeID;
	//产品类型
	TZtpApiProductClassType	ProductClass;
	//合约代码
	TZtpApiInstrumentIDType	InstrumentID;
	//买卖方向
	TZtpApiDirectionType	Direction;
	//开平标志
	TZtpApiOffsetFlagType	OffsetFlag;
	//套保标志
	TZtpApiHedgeFlagType	HedgeFlag;
	//成交数量
	TZtpApiVolumeType		TradeVolume;
	//成交价格
	TZtpApiPriceType		TradePrice;
	//成交金额
	TZtpApiMoneyType		TradeAmt;
	//成交日期
	TZtpApiDateType			TradeDate;
	//成交时间
	TZtpApiTimeType			TradeTime;
	//成交编号
	TZtpApiTradeIDType		TradeID;
	//报单编号
	TZtpApiSystemIDType		OrderSysID;
	//本地报单编号
	TZtpApiOrderLocalIDType	OrderLocalID;
};
