#pragma once
#include "SGEDataType.h"

// 席位信息
struct CSeatInfo {
	std::string SeatID;		// 席位号
	std::string TraderID;	// 交易员号
	std::string ClientID;	// 交易编码
};

// 响应信息
struct CSGERspInfoField {	
	TSGERspCodeType	RspCode;
	TSGERspMsgType	RspMsg;
	bool IsTrade;
};

// 登录信息
struct CSGERspLoginField {
	TSGESeatIDType			SeatID;
	TSGETraderIDType		TraderID;
	TSGEDateType			TradingDay;
	TSGEMaxLocalOrderIDType	MaxOrderLocalID;
};

// 深度行情
struct CSGEDepthMarketDataField {
	TSGEInstrumentIDType	InstrumentID;	
	TSGEPriceType		OpenPrice;	
	TSGEPriceType		HighestPrice;
	TSGEPriceType		LowestPrice;
	TSGEPriceType		ClosePrice;
	TSGEPriceType		LastPrice;
	TSGEPriceType		AvgPrice;
	TSGEPriceType		UpperLimitPrice;
	TSGEPriceType		LowerLimitPrice;	
	TSGEQuantityType	OpenInterest;		// 持仓量
	TSGEQuantityType	TradeVolume;		// 成交量
	TSGEVolumeType		Turnover;			// 成交金额
	TSGEDateType		QuoteDate;
	TSGETimeType		QuoteTime;
	TSGEPriceType		BidPrice1;
	TSGEQuantityType	BidVolume1;			// 也做交割买量
	TSGEPriceType		AskPrice1;
	TSGEQuantityType	AskVolume1;			// 也做交割卖量
	TSGEPriceType		BidPrice2;
	TSGEQuantityType	BidVolume2;			// 也做中立仓买量
	TSGEPriceType		AskPrice2;
	TSGEQuantityType	AskVolume2;			// 也做中立仓卖量
	TSGEPriceType		BidPrice3;
	TSGEQuantityType	BidVolume3;
	TSGEPriceType		AskPrice3;
	TSGEQuantityType	AskVolume3;
	TSGEPriceType		BidPrice4;
	TSGEQuantityType	BidVolume4;
	TSGEPriceType		AskPrice4;
	TSGEQuantityType	AskVolume4;
	TSGEPriceType		BidPrice5;
	TSGEQuantityType	BidVolume5;
	TSGEPriceType		AskPrice5;
	TSGEQuantityType	AskVolume5;
	TSGEQuantityType	MidBidLot;
	TSGEQuantityType	MidAskLot;
};

// 递延费
struct CSGEDeferFee {
	TSGEInstrumentIDType InstrumentID;
	TSGEDefferFeeDirectionType Direction;
	TSGERateType FeeRate;
};

// 订单委托信息
struct CSGEInputOrderField {
	TSGESeatIDType			SeatID;			// 席位代码
	TSGEClientIDType		ClientID;		// 交易编码	
	TSGEMarketIDType		MarketID;		// 市场代码（00：现货、02：递延）
	TSGEInstrumentIDType	InstrumentID;	// 合约代码	
	TSGESystemIDType		OrderSysID;		// 系统号	
	TSGEDirectionType		Direction;		// 买卖方向	
	TSGEOffsetFlagType		OffsetFlag;		// 开平标志	
	TSGEDateType			ApplyDate;		// 申报日期	
	TSGETimeType			ApplyTime;		// 申报时间
	TSGETraderIDType		TraderID;		// 交易员号
	TSGEPriceType			OrderPrice;		// 申报价格	
	TSGEQuantityType		OrderQuantity;	// 申报数量
	TSGEQuantityType		RemainQuantity;	// 剩余数量
	TSGEOrderStatusType		OrderStatus;	// 报单状态
	TSGEMatchType			MatchType;		// 撮合类型	
	TSGEMaxLocalOrderIDType	OrderLocalID;	// 本地报单编号	
	TSGEOrderPriceType		OrderType;		// 指令类型	
	TSGEOrderSourceType		OrderSource;	// 报单来源
	TSGEProductClassType	ProductClass;	// 报单类别（延期、现货、中立仓、交割申报）
};

// 订单撤销信息
struct CSGEOrderActionField {
	TSGESystemIDType		OrderSysID;		// 系统号	
	TSGEMarketIDType		MarketID;		// 市场代码
	TSGEInstrumentIDType	InstrumentID;	// 合约代码	
	TSGEClientIDType		ClientID;		// 交易编码	
	TSGESeatIDType			SeatID;			// 席位代码
	TSGETraderIDType		TraderID;		// 交易员号
	TSGEProductClassType	ProductClass;	// 报单类别（延期、现货、中立仓、交割申报）
};

// 订单回报信息
struct CSGEOrderField {	
	TSGEClientIDType		ClientID;		// 交易编码	
	TSGEInstrumentIDType	InstrumentID;	// 合约代码	
	TSGEMarketIDType		MarketID;		// 市场代码	
	TSGESystemIDType		OrderSysID;		// 报单编号
	TSGEMaxLocalOrderIDType	OrderLocalID;	// 本地报单编号	
	TSGEDirectionType		Direction;		// 买卖方向	
	TSGEQuantityType		OrderQuantity;	// 申报数量
	TSGETimeType			ApplyTime;		// 申报时间	
	TSGETimeType			CancelTime;		// 撤销时间
	TSGEOrderStatusType		OrderStatus;	// 订单状态
	TSGEQuantityType		RemainQuantity;	// 剩余数量
	TSGEOffsetFlagType		OffsetFlag;		// 开平标志	
	TSGEPriceType			OrderPrice;		// 申报价格	
	TSGEOrderPriceType		OrderType;		// 指令类型	
	TSGEProductClassType	ProductClass;	// 报单类别（延期、现货、中立仓、交割申报）
};

// 成交回报信息
struct CSGETradeField {
	TSGEClientIDType		ClientID;		// 交易编码	
	TSGEInstrumentIDType	InstrumentID;	// 合约代码	
	TSGEMarketIDType		MarketID;		// 市场代码	
	TSGESystemIDType		OrderSysID;		// 报单编号
	TSGEMaxLocalOrderIDType	OrderLocalID;	// 本地报单编号	
	TSGEDirectionType		Direction;		// 买卖方向	
	TSGEQuantityType		TradeVolume;	// 成交数量	
	TSGETimeType			TradeTime;		// 成交时间	
	TSGEOffsetFlagType		OffsetFlag;		// 开平标志		
	TSGEPriceType			TradePrice;		// 成交价格
	TSGEProductClassType	ProductClass;	// 报单类别（延期、现货、中立仓、交割申报）
};

// 席位资金查询应答
struct CSGERspAccountField {
	TSGESeatIDType			SeatID;			// 席位代码
	TSGEPriceType			Available;		// 可用资金
};

// 客户库存查询应答
struct CSGERspClientStorageField {
	TSGEClientIDType		ClientID;		// 交易编码	
	TSGEVarietyID			VarietyID;		// 交割品种代码	
	TSGEWeightType			TotalStorage;	// 库存总量	
	TSGEWeightType			Available;		// 可用库存	
	TSGEWeightType			Frozen;			// 现货冻结库存(卖出)	
	TSGEWeightType			PendStorage;	// 待提库存(交割)
};

// 客户持仓明细查询应答
struct CSGERspClientPositionField {
	TSGEClientIDType		ClientID;		// 交易编码		
	TSGEInstrumentIDType	InstrumentID;	// 合约代码		
	TSGEQuantityType		LongPosition;	// 多头持仓总量
	TSGEQuantityType		ShortPosition;	// 空头持仓总量
	TSGEQuantityType		YDLongPosition;	// 多头老仓量
	TSGEQuantityType		YDShortPosition;// 空头老仓量	
	TSGEQuantityType		TDLongOpen;		// 多头今开仓量	
	TSGEQuantityType		TDShortOpen;	// 空头今开仓量
	TSGEQuantityType		LongOffset;		// 多头平仓量（先平今再平昨）
	TSGEQuantityType		ShortOffset;	// 空头平仓量（先平今再平昨）
	TSGEQuantityType		LongFrozen;		// 多头平仓冻结	
	TSGEQuantityType		ShortFrozen;	// 空头平仓冻结
	TSGEVolumeType			LongTurnover;	// 多头持仓金额
	TSGEVolumeType			ShortTurnover;	// 空头持仓金额
};
