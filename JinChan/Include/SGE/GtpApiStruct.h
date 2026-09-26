#if !defined(_GTPSTRUCT_H)
#define _GTPSTRUCT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GtpApiDataType.h"

//响应信息
struct CGtpRspInfoField
{
	//请求号
	TGtpSequenceNoType RequestID;
	///响应代码  RSP＋6位数字，RSP000000表示操作成功，其余数字表示操作错误
	TGtpRspCodeType	ErrorID;
	///响应信息
	TGtpRspMsgType	ErrorMsg;
};

struct CGtpRspLoginField
{
	//交易员代码
	TGtpTraderIdType		TraderId;
	//最大本地报单号
	TGtpMaxLocalOrderIdType	MaxOrderLocalId;
	//交易日期
	TGtpDateType			TradeDate;
	//席位代码
	TGtpSeatIdType			SeatId;
	//私有流序号
	TGtpSequenceNoType		SequenceNo;
};

struct CGtpInputOrderField
{
	//席位代码
	TGtpSeatIdType			SeatId;
	//客户代码
	TGtpClientIdType		ClientId;
	//合约代码
	TGtpInstrumentIDType	InstrumentId;
	//市场代码
	TGtpMarketIdType		MarketId;
	//报单编号
	TGtpSystemIdType		OrderSysID;
	//本地报单编号
	TGtpMaxLocalOrderIdType	OrderLocalID;
	//买卖方向
	TGtpDirectionType		Direction;
	//申报数量
	TGtpQuantityType		OrderQuantity;
	//申报日期
	TGtpDateType			ApplyDate;
	//申报时间
	TGtpTimeType			ApplyTime;
	//报单状态
	TGtpOrderStatusType		OrderStatus;
	//交易员代码
	TGtpTraderIdType		TraderId;
	//剩余数量
	TGtpQuantityType		RemainQuantity;
	//开平标志
	TGtpOffsetFlagType		OffsetFlag;
	//撮合类型
	TGtpMatchType			MatachType;
	//报单价格
	TGtpPriceType			OrderPrice;
	//指令类型
	TGtpOrderPriceType		OrderPriceType;
	//报单类别
	TGtpOrderClassType		OrderClassType;
};
struct CGtpOrderActionField
{
	//席位代码
	TGtpSeatIdType			SeatId;
	//客户代码
	TGtpClientIdType		ClientId;
	//合约代码
	TGtpInstrumentIDType	InstrumentId;
	//市场代码
	TGtpMarketIdType		MarketId;
	//报单编号
	TGtpSystemIdType		OrderSysID;
	//交易员代码
	TGtpTraderIdType		TraderId;
	//报单类别
	TGtpOrderClassType		OrderClassType;
};

struct CGtpOrderField
{
	//席位代码
	TGtpSeatIdType			SeatID;
	//客户代码
	TGtpClientIdType		ClientId;
	//合约代码
	TGtpInstrumentIDType	InstrumentID;
	//市场代码
	TGtpMarketIdType		MarketId;
	//报单编号
	TGtpSystemIdType		OrderSysID;
	//本地报单编号
	TGtpMaxLocalOrderIdType	OrderLocalID;
	//买卖方向
	TGtpDirectionType		Direction;
	//申报数量
	TGtpQuantityType		OrderQuantity;
	//申报日期
	TGtpDateType			ApplyDate;
	//申报时间
	TGtpTimeType			ApplyTime;
	//撤销时间
	TGtpTimeType			CancelTime;
	//撤单交易员代码
	TGtpTraderIdType		CancelTraderId;
	//报单状态
	TGtpOrderStatusType		OrderStatus;
	//交易员代码
	TGtpTraderIdType		TraderId;
	//剩余数量
	TGtpQuantityType		RemainQuantity;
	//开平标志
	TGtpOffsetFlagType		OffsetFlag;
	//撮合类型
	TGtpMatchType			MatachType;
	//报单价格
	TGtpPriceType			OrderPrice;
	//指令类型
	TGtpOrderPriceType		OrderPriceType;
	//报单类别
	TGtpOrderClassType		OrderClassType;
};
struct CGtpTradeField
{
	//席位代码
	TGtpSeatIdType			SeatID;
	//客户代码
	TGtpClientIdType		ClientId;
	//合约代码
	TGtpInstrumentIDType	InstrumentID;
	//市场代码
	TGtpMarketIdType		MarketId;
	//报单编号
	TGtpSystemIdType		OrderSysID;
	//本地报单编号
	TGtpMaxLocalOrderIdType	OrderLocalID;
	//买卖方向
	TGtpDirectionType		Direction;
	//成交数量
	TGtpQuantityType		TradeVolume;
	//成交日期
	TGtpDateType			TradeDate;
	//成交时间
	TGtpTimeType			TradeTime;
	//成交编号
	TGtpTradeIdType			TradeID;
	//开平标志
	TGtpOffsetFlagType		OffsetFlag;
	//撮合类型
	TGtpMatchType			MatachType;
	//成交价格
	TGtpPriceType			TradePrice;
	//报单类别
	TGtpOrderClassType		OrderClassType;
};

struct CGtpDepthMarketDataField
{
	//合约代码
	TGtpInstrumentIDType	InstrumentID;
	//合约名称
	TGtpInstrumentNameType	InstrumentName;
	//昨收盘
	TGtpPriceType	PreClosePrice;
	//昨结算
	TGtpPriceType	PreSettlementPrice;
	//开盘价
	TGtpPriceType	OpenPrice;
	//最高价
	TGtpPriceType	HighestPrice;
	//最低价
	TGtpPriceType	LowestPrice;
	//最新价
	TGtpPriceType	LastPrice;
	//收盘价
	TGtpPriceType	ClosePrice;
	//结算价
	TGtpPriceType	SettlementPrice;
	//持仓量
	TGtpQuantityType OpenInterest;
	//涨停板
	TGtpPriceType	UpperLimitPrice;
	//跌停板
	TGtpPriceType	LowerLimitPrice;
	//涨跌
	TGtpPriceType	UpDownPrice;
	//成交重量
	TGtpWeightType	TradeWeight;
	//成交数量
	TGtpVolumeType	TradeVolume;
	//行情序号
	TGtpSequenceNoType SequenceNo;
	//涨跌幅
	TGtpRateType	UpDownRate;
	//成交额
	TGtpVolumeType	Turnover;
	//当日均价
	TGtpPriceType	AveragePrice;
	//行情日期
	TGtpDateType	QuoteDate;
	//行情时间
	TGtpTimeType	QuoteTime;
	///申买价一
	TGtpPriceType	BidPrice1;
	///申买量一
	TGtpVolumeType	BidVolume1;
	///申卖价一
	TGtpPriceType	AskPrice1;
	///申卖量一
	TGtpVolumeType	AskVolume1;
	///申买价二
	TGtpPriceType	BidPrice2;
	///申买量二
	TGtpVolumeType	BidVolume2;
	///申卖价二
	TGtpPriceType	AskPrice2;
	///申卖量二
	TGtpVolumeType	AskVolume2;
	///申买价三
	TGtpPriceType	BidPrice3;
	///申买量三
	TGtpVolumeType	BidVolume3;
	///申卖价三
	TGtpPriceType	AskPrice3;
	///申卖量三
	TGtpVolumeType	AskVolume3;
	///申买价四
	TGtpPriceType	BidPrice4;
	///申买量四
	TGtpVolumeType	BidVolume4;
	///申卖价四
	TGtpPriceType	AskPrice4;
	///申卖量四
	TGtpVolumeType	AskVolume4;
	///申买价五
	TGtpPriceType	BidPrice5;
	///申买量五
	TGtpVolumeType	BidVolume5;
	///申卖价五
	TGtpPriceType	AskPrice5;
	///申卖量五
	TGtpVolumeType	AskVolume5;
	///申买价六
	TGtpPriceType	BidPrice6;
	///申买量六
	TGtpVolumeType	BidVolume6;
	///申卖价六
	TGtpPriceType	AskPrice6;
	///申卖量六
	TGtpVolumeType	AskVolume6;
	///申买价七
	TGtpPriceType	BidPrice7;
	///申买量七
	TGtpVolumeType	BidVolume7;
	///申卖价七
	TGtpPriceType	AskPrice7;
	///申卖量七
	TGtpVolumeType	AskVolume7;
	///申买价八
	TGtpPriceType	BidPrice8;
	///申买量八
	TGtpVolumeType	BidVolume8;
	///申卖价八
	TGtpPriceType	AskPrice8;
	///申卖量八
	TGtpVolumeType	AskVolume8;
	///申买价九
	TGtpPriceType	BidPrice9;
	///申买量九
	TGtpVolumeType	BidVolume9;
	///申卖价九
	TGtpPriceType	AskPrice9;
	///申卖量九
	TGtpVolumeType	AskVolume9;
	///申买价十
	TGtpPriceType	BidPrice10;
	///申买量十
	TGtpVolumeType	BidVolume10;
	///申卖价十
	TGtpPriceType	AskPrice10;
	///申卖量十
	TGtpVolumeType	AskVolume10;
	///中立仓买量
	TGtpQuantityType MidBidLot;
	///中立仓卖量
	TGtpQuantityType MidAskLot;
};

//席位资金查询应答
struct CGtpRspAccountField
{
	//席位代码
	TGtpSeatIdType			SeatID;
};
//客户库存查询应答
struct CGtpRspClientStorageField
{
	//席位代码
	TGtpSeatIdType			SeatID;
	//客户代码
	TGtpClientIdType		ClientID;
	//交割品种代码
	TGtpVarietyID			VarietyID;
	//库存总量
	TGtpWeightType			TotalStorage;
	//可用库存
	TGtpWeightType			AvailableStorage;
	//现货冻结库存(卖出)
	TGtpWeightType			FrozenStorage;
	//待提库存(交割)
	TGtpWeightType			PendStorage;
};
//客户持仓查询应答
struct CGtpRspClientPositionField
{
	//客户代码
	TGtpClientIdType		ClientID;
	//合约代码
	TGtpInstrumentIDType	InstrumentID;
	//多头持仓量
	TGtpQuantityType		LongPosition;
	//空头持仓量
	TGtpQuantityType		ShortPosition;
	//上日多头持仓量
	TGtpQuantityType		YdLongPosition;
	//上日空头持仓量
	TGtpQuantityType		YdShortPosition;
	//今日多头持仓量
	TGtpQuantityType		TodayLongPosition;
	//今日空头持仓量
	TGtpQuantityType		TodayShortPosition;
	//今日多头平仓量
	TGtpQuantityType		TodayOffsetLongPosition;
	//今日空头平仓量
	TGtpQuantityType		TodayOffsetShortPosition;
	//多头持仓金额
	TGtpVolumeType			LongTurnover;
	//空头持仓金额
	TGtpVolumeType			ShortTurnover;
	//多头冻结
	TGtpQuantityType		LongFrozen;
	//空头冻结
	TGtpQuantityType		ShortFrozen;
	//交割申报多头冻结持仓
	TGtpQuantityType		DeliveryApplyLong;
	//交割申报使用空头持仓
	TGtpQuantityType		DeliveryApplyShort;
	//交割申报使用多头持仓
	TGtpQuantityType		LongLimitFrozen;
	//交割申报使用空头持仓
	TGtpQuantityType		ShortLimitFrozen;
	//交割申报使用多头持仓
	TGtpQuantityType		OffsetLongFrozen;
	//交割申报使用空头持仓
	TGtpQuantityType		OffsetShortFrozen;
};


//合约状态通知
struct CGtpInstrumentStatusField
{
	//合约代码
	TGtpInstrumentIDType	InstrumentID;
	//合约交易状态
	TGtpInstrumentStatusType	InstrumentStatus;
};

//合约递延费率推送
struct CGtpDeferFeeField
{
	//交易日期
	TGtpDateType			TradeDate;
	//合约代码
	TGtpInstrumentIDType	InstrumentID;
	//支付方向
	TGtpDirectionType		DeferDirection;
	//支付比例
	TGtpRateType			DeferFee;
};

#endif
