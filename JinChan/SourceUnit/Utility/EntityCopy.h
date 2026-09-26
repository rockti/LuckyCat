#pragma once

/******************************************************对应数据库表******************************************************/
//交易日
template<class T, class S>
void CopyTradingDayEntity(T* target, S const* source) {
	(target)->TradingDay = (source)->TradingDay;
}

//席位信息
template<class T, class S>
void CopySeatInfoEntity(T* target, S const* source) {
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->ParticipantID = (source)->ParticipantID; 
	(target)->SeatID = (source)->SeatID;
	(target)->TraderID = (source)->TraderID;
	(target)->Password = (source)->Password;
	(target)->SeatStatus = (source)->SeatStatus;
	(target)->FrontAddr = (source)->FrontAddr;
	(target)->OrderLocalID = (source)->OrderLocalID;
}

//操作员信息
template<class T, class S>
void CopyOperatorInfoEntity(T* target,S const* source)	{
	(target)->BrokerID=(source)->BrokerID;	
	(target)->OperatorID=(source)->OperatorID;
	(target)->UserName=(source)->UserName;	
	(target)->Password=(source)->Password;	
	(target)->PermissionRight=(source)->PermissionRight;	
}

//投资者信息
template<class T, class S>
void CopyInvestorInfoEntity(T* target, S const* source)	{
	(target)->BrokerID=(source)->BrokerID;	
	(target)->InvestorID=(source)->InvestorID;	
	(target)->InvestorName=(source)->InvestorName;	
	(target)->ClientID=(source)->ClientID;	
	(target)->TradeRight=(source)->TradeRight;	
	(target)->IsActive=(source)->IsActive;	
	(target)->Password=(source)->Password;
	(target)->IsNewAuth=(source)->IsNewAuth;
	(target)->Salt=(source)->Salt;
	(target)->PasswordHash=(source)->PasswordHash;
}

//操作员投资者关系
template<class T, class S>
void CopyOperatorInvestorEntity(T* target,S const* source) {
	(target)->BrokerID=(source)->BrokerID;	
	(target)->OperatorID=(source)->OperatorID;
	(target)->InvestorID=(source)->InvestorID;
}

//投资者资金账户
template<class T, class S>
void CopyInvestorAccountEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->TradingDay = (source)->TradingDay;
	(target)->PreBalance = (source)->PreBalance;
	(target)->PreAvailable = (source)->PreAvailable;
	(target)->Balance = (source)->Balance;
	(target)->Available = (source)->Available;
	(target)->Deposit = (source)->Deposit;
	(target)->Withdraw = (source)->Withdraw;
	(target)->Margin = (source)->Margin;
	(target)->Fee = (source)->Fee;
	(target)->OtherFee = (source)->OtherFee;
	(target)->CloseProfit = (source)->CloseProfit;
	(target)->PositionProfit = (source)->PositionProfit;
	(target)->FrozenMargin = (source)->FrozenMargin;
	(target)->FrozenFee = (source)->FrozenFee;
	(target)->LongMargin = (source)->LongMargin;
	(target)->ShortMargin = (source)->ShortMargin;
	(target)->LongFrozenMargin = (source)->LongFrozenMargin;
	(target)->ShortFrozenMargin = (source)->ShortFrozenMargin;
	(target)->DynamicRights = (source)->DynamicRights;
	(target)->Risk = (source)->Risk;
}

//合约信息
template<class T, class S>
void CopyInstrumentInfoEntity(T* target, S const* source) {
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->InstrumentName = (source)->InstrumentName;
	(target)->ProductID = (source)->ProductID;
	(target)->ProductClass = (source)->ProductClass;
	(target)->LongPosLimit = (source)->LongPosLimit;
	(target)->ShortPosLimit = (source)->ShortPosLimit;
	(target)->MaxOrderVolume = (source)->MaxOrderVolume;
	(target)->MinOrderVolume = (source)->MinOrderVolume;
	(target)->VolumeMultiple = (source)->VolumeMultiple;
	(target)->PriceTick = (source)->PriceTick;
}
template<class T, class S>
void CopyZTP2FTDInstrumentInfoEntity(T* target, S const* source) {
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->InstrumentName = (source)->InstrumentName;
	(target)->ProductID = (source)->ProductID;
	(target)->ProductClass = char((source)->ProductClass);
	(target)->LongPosLimit = (source)->LongPosLimit;
	(target)->ShortPosLimit = (source)->ShortPosLimit;
	(target)->MaxLimitOrderVolume = (source)->MaxOrderVolume;
	(target)->MinLimitOrderVolume = (source)->MinOrderVolume;
	(target)->MaxMarketOrderVolume = (source)->MaxOrderVolume;
	(target)->MinMarketOrderVolume = (source)->MinOrderVolume;
	(target)->VolumeMultiple = (source)->VolumeMultiple;
	(target)->PriceTick = (source)->PriceTick;
}

//投资者保证金
template<class T, class S>
void CopyInvestorMarginEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->LongMarginRate = (source)->LongMarginRate;
	(target)->LongMarginAmt = (source)->LongMarginAmt;
	(target)->ShortMarginRate = (source)->ShortMarginRate;
	(target)->ShortMarginAmt = (source)->ShortMarginAmt;
}

//投资者手续费
template<class T, class S>
void CopyInvestorFeeEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->OpenFeeRate = (source)->OpenFeeRate;
	(target)->OpenFeeAmt = (source)->OpenFeeAmt;
	(target)->OffsetFeeRate = (source)->OffsetFeeRate;
	(target)->OffsetFeeAmt = (source)->OffsetFeeAmt;
	(target)->OffsetTodayFeeRate = (source)->OffsetTodayFeeRate;
	(target)->OffsetTodayFeeAmt = (source)->OffsetTodayFeeAmt;
}

//投资者持仓
template<class T, class S>
void CopyInvestorPositionEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->TradingDay = (source)->TradingDay;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
	memcpy(&(target)->ProductClass, &(source)->ProductClass, sizeof((target)->ProductClass));
// 	(target)->ProductClass = char((source)->ProductClass);
	memcpy(&(target)->Direction, &(source)->Direction, sizeof((target)->Direction));
//	(target)->Direction = char((source)->Direction);
	(target)->UsedMargin = (source)->UsedMargin;
	(target)->Position = (source)->Position;
	(target)->PositionCost = (source)->PositionCost;
	(target)->YdPosition = (source)->YdPosition;
	(target)->YdPositionPrice = (source)->YdPositionPrice;
	(target)->FrozenMargin = (source)->FrozenMargin;
	(target)->FrozenFee = (source)->FrozenFee;
	(target)->FrozenPosition = (source)->FrozenPosition;
}

//用户连接信息
#ifndef CopyUserSessionEntity
#define CopyUserSessionEntity(target,source)	\
	(target)->BrokerID=(source)->BrokerID;	\
	(target)->InvestorID=(source)->InvestorID;	\
	(target)->SessionID=(source)->SessionID;	\
	(target)->LoginTime=(source)->LoginTime;	\
	(target)->IPAddress=(source)->IPAddress;	\
	(target)->UserProductInfo=(source)->UserProductInfo;
#endif

//用户最大本地委托号
#ifndef CopyMaxLocalIDEntity
#define CopyMaxLocalIDEntity(target,source)	\
	(target)->BrokerID=(source)->BrokerID;	\
	(target)->InvestorID=(source)->InvestorID;	\
	(target)->MaxOrderLocalID=(source)->MaxOrderLocalID;
#endif

//交易编码信息
#ifndef CopyClientInfoEntity
#define CopyClientInfoEntity(target,source)	\
	(target)->BrokerID=(source)->BrokerID;	\
	(target)->InvestorID=(source)->InvestorID;	\
	(target)->ExchangeID=(source)->ExchangeID;	\
	(target)->ClientID=(source)->ClientID;	\
	(target)->TradeRight=(source)->TradeRight;	\
	(target)->HedgeFlag=(source)->HedgeFlag;	\
	(target)->IsActive=(source)->IsActive;	
#endif

//交易权限
#ifndef CopyPermissionRightEntity
#define CopyPermissionRightEntity(target,source)	\
	(target)->BrokerID=(source)->BrokerID;	\
	(target)->ExchangeID=(source)->ExchangeID;	\
	(target)->UserID=(source)->UserID;	\
	(target)->PermissionRight=(source)->PermissionRight;
#endif



//投资者出入金
#ifndef CopyInvestorAccountDepositEntity
#define CopyInvestorAccountDepositEntity(target,source)	\
	(target)->BrokerID=(source)->BrokerID;	\
	(target)->InvestorID=(source)->InvestorID;	\
	(target)->AccountSeqNo=(source)->AccountSeqNo;	\
	(target)->AccountInsideSeqNo=(source)->AccountInsideSeqNo;	\
	(target)->Amount=(source)->Amount;	\
	(target)->Direction=(source)->Direction;
#endif








/******************************************************对应数据库表******************************************************/

/******************************************************FTD请求转换成ZTP******************************************************/
//AdapterEngine用户登录信息
template<class T, class S>
void CopyLoginBaseInfoEntity(T* target, S const* source) {
	(target)->TradingDay=(source)->TradingDay;	
	(target)->BrokerID=(source)->BrokerID;	
	(target)->LoginID =(source)->LoginID;
	(target)->Password=(source)->Password;	
	memcpy(&(target)->UserType, &(source)->UserType, sizeof((target)->UserType));
//	(target)->UserType=(source)->UserType;
	(target)->UserProductInfo=(source)->UserProductInfo;	
	(target)->IPAddress=(source)->IPAddress;	
}

//用户录出信息
template<class T, class S>
void CopyUserLogoutEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->LoginID = (source)->LoginID;
}

//ZTP登录额外信息
// #ifndef CopyUserLoginExtraInfoEntity
// #define CopyUserLoginExtraInfoEntity(target,source)	\
// 	(target)->TradingSystemName=(source)->TradingSystemName;	\
// 	(target)->LoginTime=(source)->LoginTime;	\
// 	(target)->MaxOrderLocalID=(source)->MaxOrderLocalID;
// #endif



//强制用户退出
template<class T, class S>
void CopyForceUserExitEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
}

//用户口令修改请求
template<class T, class S>
void CopyUserPasswordUpdateReqEntity(T* target, S const* source)	{
	(target)->BrokerID=(source)->BrokerID;	
	(target)->InvestorID=(source)->InvestorID;
	(target)->OldPassword=(source)->OldPassword;	
	(target)->NewPassword=(source)->NewPassword;
}

//API查询合约请求
template<class T, class S>
void CopyQryInstrumentReqEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	memcpy(&(target)->ProductClass, &(source)->ProductClass, sizeof((target)->ProductClass));
//	(target)->ProductClass = type::ProductClass((source)->ProductClass);
	(target)->InstrumentID = (source)->InstrumentID;
}

//API查询合约应答
#ifndef CopyAPIInstrumentEntity
#define CopyAPIInstrumentEntity(target,source)	\
	CopyInstrumentInfoEntity(target,source)
#endif

//API查询资金请求
template<class T, class S>
void CopyAPIQryInvestorAccountEntity(T* target, S* source){
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
}

//API查询持仓请求
#ifndef CopyAPIQryInvestorPositionEntity
#define CopyAPIQryInvestorPositionEntity(target,source)	\
	(target)->BrokerID=(source)->BrokerID;	\
	(target)->InvestorID=(source)->InvestorID;	\
	(target)->ExchangeID=(source)->ExchangeID;
#endif

//API查询保证金请求
template<class T, class S>
void CopyQryInvestorMarginEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
}

//查询手续费请求
template<class T, class S>
void CopyQryInvestorFeeEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
}

template<class T, class S>
void CopyQryInvestorOrderEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->OrderSysID = (source)->OrderSysID;
}

template<class T, class S>
void CopyQryInvestorTradeEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->OrderSysID = (source)->OrderSysID;
}

//API输入报单
template<class T, class S>
void CopyAPIInputOrderEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->OrderOperatorID = (source)->OrderOperatorID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	memcpy(&(target)->ProductClass, &(source)->ProductClass, sizeof((target)->ProductClass));
	(target)->InstrumentID = (source)->InstrumentID;
	memcpy(&(target)->Direction, &(source)->Direction, sizeof((target)->Direction));
	memcpy(&(target)->OffsetFlag, &(source)->OffsetFlag, sizeof((target)->OffsetFlag));
	(target)->LimitPrice = (source)->LimitPrice;
	(target)->Volume = (source)->Volume;
	memcpy(&(target)->OrderPriceType, &(source)->OrderPriceType, sizeof((target)->OrderPriceType));
	(target)->UserOrderLocalID = (source)->UserOrderLocalID;
	(target)->OrderSysID = (source)->OrderSysID;
	(target)->UserCustom = (source)->UserCustom;
}

//FTD报单回报信息
template<class T, class S>
void CopyFTDOrderEntity(T* target, S const* source) {
	CopyAPIInputOrderEntity(target, source);
	memcpy(&(target)->OrderStatus, &(source)->OrderStatus, sizeof((target)->OrderStatus));
	CopyTradingDayEntity(target, source);
	(target)->SeatID = (source)->SeatID;	
	(target)->ClientID = (source)->ClientID;	
	(target)->InsertTime = (source)->InsertTime;	
	(target)->CancelTime = (source)->CancelTime;	
	(target)->CancelOperatorID = (source)->CancelOperatorID;
	(target)->VolumeTrade = (source)->VolumeTrade;	
	(target)->VolumeRemain = (source)->VolumeRemain;	
	(target)->OrderLocalID = (source)->OrderLocalID;
}


//ZTP报单信息
template<class T, class S>
void CopyZTPOrderEntity(T* target, S const* source) {
	CopyFTDOrderEntity(target, source);
	(target)->FrozenMargin = (source)->FrozenMargin;
	(target)->FrozenFee = (source)->FrozenFee;
	(target)->TradeMargin = (source)->TradeMargin;
	(target)->TradeFee = (source)->TradeFee;
}

//API输入撤单
template<class T, class S>
void CopyAPIOrderActionEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;	
	(target)->CancelOperatorID = (source)->CancelOperatorID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	memcpy(&(target)->ProductClass, &(source)->ProductClass, sizeof((target)->ProductClass));
//	(target)->ProductClass = char((source)->ProductClass);
	(target)->InstrumentID = (source)->InstrumentID;
	(target)->OrderSysID = (source)->OrderSysID;
	(target)->UserOrderActionLocalID = (source)->UserOrderActionLocalID;
	(target)->UserOrderLocalID = (source)->UserOrderLocalID;
	memcpy(&(target)->ActionFlag, &(source)->ActionFlag, sizeof((target)->ActionFlag));
//	(target)->ActionFlag = char((source)->ActionFlag);
	(target)->LimitPrice = (source)->LimitPrice;
	(target)->VolumeChange = (source)->VolumeChange;

}
//ZTP撤单信息
#ifndef CopyOrderActionEntity
#define CopyOrderActionEntity(target,source)	\
	CopyAPIOrderActionEntity(target,source);	\
	(target)->SeatID=(source)->SeatID;	\
	(target)->ClientID=(source)->ClientID;	\
	(target)->ActionLocalID=(source)->ActionLocalID;\
	(target)->OrderLocalID=(source)->OrderLocalID;	\
	(target)->CancelTime=(source)->CancelTime;	
#endif

//FTD成交信息
template<class T, class S>
void CopyFTDTradeEntity(T* target, S const* source) {
	(target)->BrokerID = (source)->BrokerID;
	(target)->InvestorID = (source)->InvestorID;
	(target)->ExchangeID = (source)->ExchangeID;
	memcpy(&(target)->ProductClass, &(source)->ProductClass, sizeof((target)->ProductClass));
	(target)->InstrumentID = (source)->InstrumentID;
	memcpy(&(target)->Direction, &(source)->Direction, sizeof((target)->Direction));
	memcpy(&(target)->OffsetFlag, &(source)->OffsetFlag, sizeof((target)->OffsetFlag));
	(target)->TradeVolume = (source)->TradeVolume;
	(target)->TradePrice = (source)->TradePrice;
	(target)->TradeAmt = (source)->TradeAmt;
	(target)->TradeDate = (source)->TradeDate;
	(target)->TradeTime = (source)->TradeTime;
	(target)->TradeID = (source)->TradeID;
	(target)->OrderSysID = (source)->OrderSysID;
	(target)->UserOrderLocalID = (source)->UserOrderLocalID;
	(target)->ClientID = (source)->ClientID;
	(target)->OrderOperatorID = (source)->OrderOperatorID;
}

//成交信息
template<class T, class S>
void CopyTradeEntity(T* target, S const* source) {
	CopyFTDTradeEntity(target, source);
	(target)->OrderLocalID = (source)->OrderLocalID;
	(target)->UsedMargin = (source)->UsedMargin;
	(target)->UsedFee = (source)->UsedFee;
	(target)->SequenceNo = (source)->SequenceNo;
}

//交易状态信息
template<class T, class S>
void CopyInstrumentStatusEntity(T* target, S const* source) {
	(target)->ExchangeID = (source)->ExchangeID;
	memcpy(&(target)->InstrumentStatus, &(source)->InstrumentStatus, sizeof((target)->InstrumentStatus));
}

//行情信息
// #ifndef CopyMarketDataEntity
// #define CopyMarketDataEntity(target,source)	\
// 	CopyTradingDayEntity(target,source);	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 	(target)->OpenPrice=(source)->OpenPrice;	\
// 	(target)->AskPrice1=(source)->AskPrice1;	\
// 	(target)->AskVolume1=(source)->AskVolume1;	\
// 	(target)->BidPrice1=(source)->BidPrice1;	\
// 	(target)->BidVolume1=(source)->BidVolume1;	\
// 	(target)->AskPrice2=(source)->AskPrice2;	\
// 	(target)->AskVolume2=(source)->AskVolume2;	\
// 	(target)->BidPrice2=(source)->BidPrice2;	\
// 	(target)->BidVolume2=(source)->BidVolume2;	\
// 	(target)->AskPrice3=(source)->AskPrice3;	\
// 	(target)->AskVolume3=(source)->AskVolume3;	\
// 	(target)->BidPrice3=(source)->BidPrice3;	\
// 	(target)->BidVolume3=(source)->BidVolume3;	\
// 	(target)->AskPrice4=(source)->AskPrice4;	\
// 	(target)->AskVolume4=(source)->AskVolume4;	\
// 	(target)->BidPrice4=(source)->BidPrice4;	\
// 	(target)->BidVolume4=(source)->BidVolume4;	\
// 	(target)->AskPrice5=(source)->AskPrice5;	\
// 	(target)->AskVolume5=(source)->AskVolume5;	\
// 	(target)->BidPrice5=(source)->BidPrice5;	\
// 	(target)->BidVolume5=(source)->BidVolume5;	\
// 	(target)->HighestPrice=(source)->HighestPrice;	\
// 	(target)->LowestPrice=(source)->LowestPrice;	\
// 	(target)->LastPrice=(source)->LastPrice;	\
// 	(target)->AvgPrice=(source)->AvgPrice;	\
// 	(target)->Weight=(source)->Weight;	\
// 	(target)->Volume=(source)->Volume;	\
// 	(target)->Turnover=(source)->Turnover;	\
// 	(target)->LowerLimitPrice=(source)->LowerLimitPrice;	\
// 	(target)->UpperLimitPrice=(source)->UpperLimitPrice;	\
// 	(target)->OpenInterest=(source)->OpenInterest;	\
// 	(target)->PreClosePrice=(source)->PreClosePrice;	\
// 	(target)->PreOpenInterest=(source)->PreOpenInterest;	\
// 	(target)->PreSettlementPrice=(source)->PreSettlementPrice;	\
// 	(target)->ClosePrice=(source)->ClosePrice;	\
// 	(target)->SettlementPrice=(source)->SettlementPrice;	\
// 	(target)->UpdateTime=(source)->UpdateTime;	\
// 	(target)->Millisec=(source)->Millisec;		\
// 	(target)->SequenceNo=(source)->SequenceNo;
// #endif

// #ifndef CopyRspInfoEntity
// #define CopyRspInfoEntity(target,source)	\
// 	(target)->ErrorID=(source)->ErrorID;	\
// 	(target)->ErrorMsg=(source)->ErrorMsg;
// #endif

template<class T, class S>
void CopyRspInfoEntity(T* target, S const* source) {
	(target)->ErrorID = (source)->ErrorID;
	(target)->ErrorMsg = (source)->ErrorMsg;
}

// #ifndef CopySessionRefInfoEntity
// #define CopySessionRefInfoEntity(target,source) \
// 	(target)->FrontID=(source)->FrontID; \
// 	(target)->SessionID=(source)->SessionID; \
// 	(target)->RequestID=(source)->RequestID;
// #endif


// #ifndef CopyInvestorAccountDepositNtfEntity
// #define CopyInvestorAccountDepositNtfEntity(target,source)	\
// 	CopyAPIInvestorAccountDepositResEntity(target,source);	\
// 	(target)->UserID=(source)->UserID;	\
// 
// #endif


// #ifndef CopyClientTradingIDEntity
// #define CopyClientTradingIDEntity(target,source)	\
// 	CopyAPIClientTradingIDEntity(target,source);	\
// 
// #endif


// #ifndef CopyInvestorMarginEntity
// #define CopyInvestorMarginEntity(target,source)	\
// 	CopyAPIInvestorMarginEntity(target,source);	\
// 
// #endif

// #ifndef CopyInvestorFeeEntity
// #define CopyInvestorFeeEntity(target,source)	\
// 	CopyAPIInvestorFeeEntity(target,source);	\
// 
// #endif
// #ifndef CopyBankAccEntity
// #define CopyBankAccEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->AccountID=(source)->AccountID;	\
// 	(target)->Currency=(source)->Currency;	\
// 	(target)->BankID=(source)->BankID;	\
// 	(target)->BankName=(source)->BankName;	\
// 	(target)->BankAccount=(source)->BankAccount;	\
// 
// #endif
// #ifndef CopyBFTransferInterfaceEntity
// #define CopyBFTransferInterfaceEntity(target,source)	\
// 	(target)->APIID=(source)->APIID;	\
// 	(target)->FrontID=(source)->FrontID;	\
// 	(target)->RequestID=(source)->RequestID;	\
// 	(target)->InstallID=(source)->InstallID;	\
// 	CopyTransferSerialEntity(target,source);	\
// 
// #endif
// #ifndef CopyTransferSerialEntity
// #define CopyTransferSerialEntity(target,source)	\
// 	(target)->PlateSerial=(source)->PlateSerial;	\
// 	(target)->TradeDate=(source)->TradeDate;	\
// 	(target)->TradingDay=(source)->TradingDay;	\
// 	(target)->TradingTime=(source)->TradingTime;	\
// 	(target)->TradeCode=(source)->TradeCode;	\
// 	(target)->SessionID=(source)->SessionID;	\
// 	(target)->BankID=(source)->BankID;	\
// 	(target)->BankBrchID=(source)->BankBrchID;	\
// 	(target)->BankAccType=(source)->BankAccType;	\
// 	(target)->BankAccount=(source)->BankAccount;	\
// 	(target)->Currency=(source)->Currency;	\
// 	(target)->BankSerial=(source)->BankSerial;	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->BrokerBranchID=(source)->BrokerBranchID;	\
// 	(target)->FutureAccType=(source)->FutureAccType;	\
// 	(target)->AccountID=(source)->AccountID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->FutureSerial=(source)->FutureSerial;	\
// 	(target)->IdCardType=(source)->IdCardType;	\
// 	(target)->IdentifiedCardNo=(source)->IdentifiedCardNo;	\
// 	(target)->TradeAmount=(source)->TradeAmount;	\
// 	(target)->CustFee=(source)->CustFee;	\
// 	(target)->BrokerFee=(source)->BrokerFee;	\
// 	(target)->AvailabilityFlag=(source)->AvailabilityFlag;	\
// 	(target)->OperatorCode=(source)->OperatorCode;	\
// 	(target)->BankNewAccount=(source)->BankNewAccount;	\
// 	CopyErrorTypeEntity(target,source);	\
// 
// #endif


// #ifndef CopyUserInvestorEntity
// #define CopyUserInvestorEntity(target,source)	\
// 	CopyAPIUserInvestorEntity(target,source);	\
// 	(target)->GrantFuncSet=(source)->GrantFuncSet;	\
// 
// #endif

// #ifndef CopyUserFunctionRightEntity
// #define CopyUserFunctionRightEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->GrantFuncSet=(source)->GrantFuncSet;	\
// 
// #endif
// #ifndef CopyDataSyncStatusEntity
// #define CopyDataSyncStatusEntity(target,source)	\
// 	(target)->TradingDay=(source)->TradingDay;	\
// 	(target)->DataSyncStatus=(source)->DataSyncStatus;	\
// 
// #endif
// #ifndef CopyTBCommandEntity
// #define CopyTBCommandEntity(target,source)	\
// 	(target)->CommandNo=(source)->CommandNo;	\
// 	(target)->CommandDate=(source)->CommandDate;	\
// 	(target)->CommandTime=(source)->CommandTime;	\
// 	(target)->CommandType=(source)->CommandType;	\
// 	(target)->FieldName=(source)->FieldName;	\
// 	(target)->FieldContent=(source)->FieldContent;	\
// 
// #endif

// #ifndef CopyInstrumentEntity
// #define CopyInstrumentEntity(target,source)	\
// 	CopyAPIInstrumentEntity(target,source);	\
// 	(target)->ProductGroupID=(source)->ProductGroupID;	\
// 	(target)->ProductClass=(source)->ProductClass;	\
// 	(target)->APIID=(source)->APIID;	\
// 
// #endif
// #ifndef CopyInstrumentStatusEntity
// #define CopyInstrumentStatusEntity(target,source)	\
// 	CopyInstrumentEntity(target,source);	\
// 
// #endif



// #ifndef CopyOrderInsertFailedEntity
// #define CopyOrderInsertFailedEntity(target,source)	\
// 	CopyOrderEntity(target,source);	\
// 	(target)->ErrorMsg=(source)->ErrorMsg;	\
// 
// #endif

// #ifndef CopyTradeEntity
// #define CopyTradeEntity(target,source)	\

// #ifndef CopyQryMarketDataEntity
// #define CopyQryMarketDataEntity(target,source)	\
// 	CopyAPIQryMarketDataEntity(target,source);	\
// 
// #endif


// #ifndef CopyMemtableChangeEntity
// #define CopyMemtableChangeEntity(target,source)	\
// 	(target)->MemtableChangeType=(source)->MemtableChangeType;	\
// 	(target)->MemTableName=(source)->MemTableName;	\
// 
// #endif
// #ifndef CopyTmdbAccomplishEntity
// #define CopyTmdbAccomplishEntity(target,source)	\
// 	(target)->CommPhaseNo=(source)->CommPhaseNo;	\
// 	(target)->SequenceNo=(source)->SequenceNo;	\
// 
// #endif
// #ifndef CopyDbmtRefEntity
// #define CopyDbmtRefEntity(target,source)	\
// 
// #endif
// #ifndef CopyDbmtUserEntity
// #define CopyDbmtUserEntity(target,source)	\
// 	CopyUserEntity(target,source);	\
// 	CopyDbmtRefEntity(target,source);	\
// 
// #endif

// #ifndef CopyLoginRspEntity
// #define CopyLoginRspEntity(target,source)	\
// 	(target)->TradingDay=(source)->TradingDay;	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->LoginTime=(source)->LoginTime;	\
// 	(target)->MaxOrderLocalID=(source)->MaxOrderLocalID;	\
// 	(target)->TradingSystemName=(source)->TradingSystemName;	\
// 	(target)->DataCenterID=(source)->DataCenterID;	\
// 	(target)->PrivateFlowSize=(source)->PrivateFlowSize;	\
// 	(target)->UserFlowSize=(source)->UserFlowSize;	\
// 
// #endif


// 
// #endif

// #ifndef CopyCommPhaseEntity
// #define CopyCommPhaseEntity(target,source)	\
// 	(target)->TradingDay=(source)->TradingDay;	\
// 	(target)->CommPhaseNo=(source)->CommPhaseNo;	\
// 
// #endif
// #ifndef CopyExchangeTradingDayEntity
// #define CopyExchangeTradingDayEntity(target,source)	\
// 	CopyTradingDayEntity(target,source);	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 
// #endif
// #ifndef CopyCurrentTimeEntity
// #define CopyCurrentTimeEntity(target,source)	\
// 	(target)->CurrDate=(source)->CurrDate;	\
// 	(target)->CurrTime=(source)->CurrTime;	\
// 	(target)->CurrMillisec=(source)->CurrMillisec;	\
// 
// #endif

// #ifndef CopyMemDbEntity
// #define CopyMemDbEntity(target,source)	\
// 	(target)->MemTableName=(source)->MemTableName;	\
// 
// #endif

// #ifndef CopyCurrentTradeTimeEntity
// #define CopyCurrentTradeTimeEntity(target,source)	\
// 	CopyCurrentTimeEntity(target,source);	\
// 	(target)->OldTime=(source)->OldTime;	\
// 	(target)->OldMillisec=(source)->OldMillisec;	\
// 
// #endif
// #ifndef CopyMoneyEntity
// #define CopyMoneyEntity(target,source)	\
// 	(target)->Margin=(source)->Margin;	\
// 	(target)->Fee=(source)->Fee;	\
// 	(target)->Premium=(source)->Premium;	\
// 	(target)->MarginChanged=(source)->MarginChanged;	\
// 	(target)->FrozenMarginChanged=(source)->FrozenMarginChanged;	\
// 	(target)->UsedMargin=(source)->UsedMargin;	\
// 
// #endif
// #ifndef CopyAPIQryUserEntity
// #define CopyAPIQryUserEntity(target,source)	\
// 	(target)->StartUserID=(source)->StartUserID;	\
// 	(target)->EndUserID=(source)->EndUserID;	\
// 
// #endif
// #ifndef CopyAPIQryInvestorEntity
// #define CopyAPIQryInvestorEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 
// #endif
// #ifndef CopyQryInvestorEntity
// #define CopyQryInvestorEntity(target,source)	\
// 	CopyAPIQryInvestorEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryInvestorAccountEntity
// #define CopyAPIQryInvestorAccountEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 
// #endif
// #ifndef CopyQryInvestorAccountEntity
// #define CopyQryInvestorAccountEntity(target,source)	\
// 	CopyAPIQryInvestorAccountEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 	(target)->SeatID=(source)->SeatID;	\
// 
// #endif
// #ifndef CopyQryInnerInvestorAccountEntity
// #define CopyQryInnerInvestorAccountEntity(target,source)	\
// 	CopyQryInvestorAccountEntity(target,source);	\
// 	(target)->FrontID=(source)->FrontID;	\
// 	(target)->SessionID=(source)->SessionID;	\
// 	(target)->RequestID=(source)->RequestID;	\
// 
// #endif
// #ifndef CopyQryInnerInvestorPositionEntity
// #define CopyQryInnerInvestorPositionEntity(target,source)	\
// 	CopyQryInvestorPositionEntity(target,source);	\
// 	(target)->FrontID=(source)->FrontID;	\
// 	(target)->SessionID=(source)->SessionID;	\
// 	(target)->RequestID=(source)->RequestID;	\
// 
// #endif
// #ifndef CopyAPIQryClientTradingIDEntity
// #define CopyAPIQryClientTradingIDEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 
// #endif
// #ifndef CopyQryClientTradingIDEntity
// #define CopyQryClientTradingIDEntity(target,source)	\
// 	CopyAPIQryClientTradingIDEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryOrderEntity
// #define CopyAPIQryOrderEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->OrderSysID=(source)->OrderSysID;	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 
// #endif
// #ifndef CopyQryOrderEntity
// #define CopyQryOrderEntity(target,source)	\
// 	CopyAPIQryOrderEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryTradeEntity
// #define CopyAPIQryTradeEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->TradeID=(source)->TradeID;	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 
// #endif
// #ifndef CopyQryTradeEntity
// #define CopyQryTradeEntity(target,source)	\
// 	CopyAPIQryTradeEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryExchangeEntity
// #define CopyAPIQryExchangeEntity(target,source)	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 
// #endif

// #endif
// #ifndef CopyAPIQryMarketDataEntity
// #define CopyAPIQryMarketDataEntity(target,source)	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 
// #endif

// #endif
// #ifndef CopyQryInvestorPositionEntity
// #define CopyQryInvestorPositionEntity(target,source)	\
// 	CopyAPIQryInvestorPositionEntity(target,source);	\
// 	(target)->ClientID=(source)->ClientID;	\
// 	(target)->SeatID=(source)->SeatID;	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryUserInvestorEntity
// #define CopyAPIQryUserInvestorEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 
// #endif
// #ifndef CopyQryUserInvestorEntity
// #define CopyQryUserInvestorEntity(target,source)	\
// 	CopyAPIQryUserInvestorEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryComplianceParamEntity
// #define CopyAPIQryComplianceParamEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 
// #endif
// #ifndef CopyQryComplianceParamEntity
// #define CopyQryComplianceParamEntity(target,source)	\
// 	CopyAPIQryComplianceParamEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif


// #endif
// #ifndef CopyAPIReqQueryAccountEntity
// #define CopyAPIReqQueryAccountEntity(target,source)	\
// 	(target)->TradeCode=(source)->TradeCode;	\
// 	(target)->BankID=(source)->BankID;	\
// 	(target)->BankBrchID=(source)->BankBrchID;	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->BrokerBranchID=(source)->BrokerBranchID;	\
// 	(target)->TradeDate=(source)->TradeDate;	\
// 	(target)->TradeTime=(source)->TradeTime;	\
// 	(target)->BankSerial=(source)->BankSerial;	\
// 	(target)->TradingDay=(source)->TradingDay;	\
// 	(target)->PlateSerial=(source)->PlateSerial;	\
// 	(target)->LastFragment=(source)->LastFragment;	\
// 	(target)->SessionID=(source)->SessionID;	\
// 	(target)->CustomerName=(source)->CustomerName;	\
// 	(target)->IdCardType=(source)->IdCardType;	\
// 	(target)->IdentifiedCardNo=(source)->IdentifiedCardNo;	\
// 	(target)->CustType=(source)->CustType;	\
// 	(target)->BankAccount=(source)->BankAccount;	\
// 	(target)->BankPassWord=(source)->BankPassWord;	\
// 	(target)->AccountID=(source)->AccountID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->Password=(source)->Password;	\
// 	(target)->FutureSerial=(source)->FutureSerial;	\
// 	(target)->InstallID=(source)->InstallID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->VerifyCertNoFlag=(source)->VerifyCertNoFlag;	\
// 	(target)->Currency=(source)->Currency;	\
// 	(target)->Digest=(source)->Digest;	\
// 	(target)->BankAccType=(source)->BankAccType;	\
// 	(target)->DeviceID=(source)->DeviceID;	\
// 	(target)->BankSecuAccType=(source)->BankSecuAccType;	\
// 	(target)->BankCodingForFuture=(source)->BankCodingForFuture;	\
// 	(target)->BankSecuAcc=(source)->BankSecuAcc;	\
// 	(target)->BankPwdFlag=(source)->BankPwdFlag;	\
// 	(target)->SecuPwdFlag=(source)->SecuPwdFlag;	\
// 	(target)->OperNo=(source)->OperNo;	\
// 	(target)->RequestID=(source)->RequestID;	\
// 	(target)->TID=(source)->TID;	\
// 
// #endif
// #ifndef CopyReqQueryAccountEntity
// #define CopyReqQueryAccountEntity(target,source)	\
// 	CopyAPIReqQueryAccountEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIReqTransferEntity
// #define CopyAPIReqTransferEntity(target,source)	\
// 	(target)->TradeCode=(source)->TradeCode;	\
// 	(target)->BankID=(source)->BankID;	\
// 	(target)->BankBrchID=(source)->BankBrchID;	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->BrokerBranchID=(source)->BrokerBranchID;	\
// 	(target)->TradeDate=(source)->TradeDate;	\
// 	(target)->TradingTime=(source)->TradingTime;	\
// 	(target)->BankSerial=(source)->BankSerial;	\
// 	(target)->TradingDay=(source)->TradingDay;	\
// 	(target)->PlateSerial=(source)->PlateSerial;	\
// 	(target)->LastFragment=(source)->LastFragment;	\
// 	(target)->SessionID=(source)->SessionID;	\
// 	(target)->CustomerName=(source)->CustomerName;	\
// 	(target)->IdCardType=(source)->IdCardType;	\
// 	(target)->IdentifiedCardNo=(source)->IdentifiedCardNo;	\
// 	(target)->CustType=(source)->CustType;	\
// 	(target)->BankAccount=(source)->BankAccount;	\
// 	(target)->BankPassWord=(source)->BankPassWord;	\
// 	(target)->AccountID=(source)->AccountID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->Password=(source)->Password;	\
// 	(target)->InstallID=(source)->InstallID;	\
// 	(target)->FutureSerial=(source)->FutureSerial;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->VerifyCertNoFlag=(source)->VerifyCertNoFlag;	\
// 	(target)->Currency=(source)->Currency;	\
// 	(target)->TradeAmount=(source)->TradeAmount;	\
// 	(target)->FutureFetchAmount=(source)->FutureFetchAmount;	\
// 	(target)->FeePayFlag=(source)->FeePayFlag;	\
// 	(target)->CustFee=(source)->CustFee;	\
// 	(target)->BrokerFee=(source)->BrokerFee;	\
// 	(target)->Message=(source)->Message;	\
// 	(target)->Digest=(source)->Digest;	\
// 	(target)->BankAccType=(source)->BankAccType;	\
// 	(target)->DeviceID=(source)->DeviceID;	\
// 	(target)->BankSecuAccType=(source)->BankSecuAccType;	\
// 	(target)->BankCodingForFuture=(source)->BankCodingForFuture;	\
// 	(target)->BankSecuAcc=(source)->BankSecuAcc;	\
// 	(target)->BankPwdFlag=(source)->BankPwdFlag;	\
// 	(target)->SecuPwdFlag=(source)->SecuPwdFlag;	\
// 	(target)->OperNo=(source)->OperNo;	\
// 	(target)->RequestID=(source)->RequestID;	\
// 	(target)->TID=(source)->TID;	\
// 	(target)->TransferStatus=(source)->TransferStatus;	\
// 
// #endif
// #ifndef CopyReqTransferEntity
// #define CopyReqTransferEntity(target,source)	\
// 	CopyAPIReqTransferEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIQryTransferSerialEntity
// #define CopyAPIQryTransferSerialEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->AccountID=(source)->AccountID;	\
// 	(target)->BankID=(source)->BankID;	\
// 	(target)->Currency=(source)->Currency;	\
// 
// #endif
// #ifndef CopyQryTransferSerialEntity
// #define CopyQryTransferSerialEntity(target,source)	\
// 	CopyAPIQryTransferSerialEntity(target,source);	\
// 	CopyLoginUserEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPITransferSerialEntity
// #define CopyAPITransferSerialEntity(target,source)	\
// 	CopyTransferSerialEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPINotifyQueryAccountEntity
// #define CopyAPINotifyQueryAccountEntity(target,source)	\
// 	CopyAPIReqQueryAccountEntity(target,source);	\
// 	(target)->BankUseAmount=(source)->BankUseAmount;	\
// 	(target)->BankFetchAmount=(source)->BankFetchAmount;	\
// 	CopyErrorTypeEntity(target,source);	\
// 
// #endif
// #ifndef CopyAPIRspTransferEntity
// #define CopyAPIRspTransferEntity(target,source)	\
// 	CopyAPIReqTransferEntity(target,source);	\
// 	CopyErrorTypeEntity(target,source);	\
// 
// #endif
// #ifndef CopyLoginUserEntity
// #define CopyLoginUserEntity(target,source)	\
// 	(target)->LogBrokerID=(source)->LogBrokerID;	\
// 	(target)->LogUserID=(source)->LogUserID;	\
// 
// #endif
// #ifndef CopyMarketDataLastMatchUSKernelEntity
// #define CopyMarketDataLastMatchUSKernelEntity(target,source)	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 	(target)->LastPrice=(source)->LastPrice;	\
// 	(target)->Volume=(source)->Volume;	\
// 
// #endif
#ifndef CopyMarketDataBid23Entity
#define CopyMarketDataBid23Entity(target,source)	\
	(target)->BidPrice2=(source)->BidPrice2;	\
	(target)->BidVolume2=(source)->BidVolume2;	\
	(target)->BidPrice3=(source)->BidPrice3;	\
	(target)->BidVolume3=(source)->BidVolume3;
#endif

#ifndef CopyMarketDataAsk23Entity
#define CopyMarketDataAsk23Entity(target,source)	\
	(target)->AskPrice2=(source)->AskPrice2;	\
	(target)->AskVolume2=(source)->AskVolume2;	\
	(target)->AskPrice3=(source)->AskPrice3;	\
	(target)->AskVolume3=(source)->AskVolume3;
#endif

#ifndef CopyMarketDataBid45Entity
#define CopyMarketDataBid45Entity(target,source)	\
	(target)->BidPrice4=(source)->BidPrice4;	\
	(target)->BidVolume4=(source)->BidVolume4;	\
	(target)->BidPrice5=(source)->BidPrice5;	\
	(target)->BidVolume5=(source)->BidVolume5;
#endif

#ifndef CopyMarketDataAsk45Entity
#define CopyMarketDataAsk45Entity(target,source)	\
	(target)->AskPrice4=(source)->AskPrice4;	\
	(target)->AskVolume4=(source)->AskVolume4;	\
	(target)->AskPrice5=(source)->AskPrice5;	\
	(target)->AskVolume5=(source)->AskVolume5;
#endif

#ifndef CopyMarketDataExchangeIDEntity
#define CopyMarketDataExchangeIDEntity(target,source)	\
	(target)->ExchangeID=(source)->ExchangeID;
#endif

#ifndef CopyMarketDataUpdateTimeEntity
#define CopyMarketDataUpdateTimeEntity(target,source)	\
	(target)->UpdateTime=(source)->UpdateTime;	\
	(target)->Millisec=(source)->Millisec;
#endif

#ifndef CopyDepthMarketDataEntity
#define CopyDepthMarketDataEntity(target,source)	\
	CopyTradingDayEntity(target,source);	\
	CopyMarketDataExchangeIDEntity(target,source);	\
	(target)->InstrumentID=(source)->InstrumentID;	\
	(target)->OpenPrice=(source)->OpenPrice;	\
	(target)->AskPrice1=(source)->AskPrice1;	\
	(target)->AskVolume1=(source)->AskVolume1;	\
	(target)->AskPrice2=(source)->AskPrice2;	\
	(target)->AskVolume2=(source)->AskVolume2;	\
	(target)->AskPrice3=(source)->AskPrice3;	\
	(target)->AskVolume3=(source)->AskVolume3;	\
	(target)->AskPrice4=(source)->AskPrice4;	\
	(target)->AskVolume4=(source)->AskVolume4;	\
	(target)->AskPrice5=(source)->AskPrice5;	\
	(target)->AskVolume5=(source)->AskVolume5;	\
	(target)->BidPrice1=(source)->BidPrice1;	\
	(target)->BidVolume1=(source)->BidVolume1;	\
	(target)->BidPrice2=(source)->BidPrice2;	\
	(target)->BidVolume2=(source)->BidVolume2;	\
	(target)->BidPrice3=(source)->BidPrice3;	\
	(target)->BidVolume3=(source)->BidVolume3;	\
	(target)->BidPrice4=(source)->BidPrice4;	\
	(target)->BidVolume4=(source)->BidVolume4;	\
	(target)->BidPrice5=(source)->BidPrice5;	\
	(target)->BidVolume5=(source)->BidVolume5;	\
	(target)->LastPrice=(source)->LastPrice;	\
	(target)->Volume=(source)->Volume;	\
	(target)->Turnover=(source)->Turnover;	\
	(target)->HighestPrice=(source)->HighestPrice;	\
	(target)->LowestPrice=(source)->LowestPrice;	\
	(target)->UpperLimitPrice=(source)->UpperLimitPrice;	\
	(target)->LowerLimitPrice=(source)->LowerLimitPrice;	\
	(target)->PreClosePrice=(source)->PreClosePrice;	\
	(target)->ClosePrice=(source)->ClosePrice;	\
	(target)->PreOpenInterest=(source)->PreOpenInterest;	\
	(target)->OpenInterest=(source)->OpenInterest;		\
	(target)->PreDelta=(source)->PreDelta;				\
	(target)->CurrDelta=(source)->CurrDelta;			\
	(target)->PreSettlementPrice=(source)->PreSettlementPrice;	\
	(target)->SettlementPrice=(source)->SettlementPrice;	\
	CopyMarketDataUpdateTimeEntity(target,source);
#endif

#ifndef CopyRspMarketDataEntity
#define CopyRspMarketDataEntity(target,source)	\
	CopyTradingDayEntity(target,source);	\
	CopyMarketDataExchangeIDEntity(target,source);	\
	(target)->InstrumentID=(source)->InstrumentID;	\
	(target)->OpenPrice=(source)->OpenPrice;	\
	(target)->AskPrice1=(source)->AskPrice1;	\
	(target)->AskVolume1=(source)->AskVolume1;	\
	(target)->BidPrice1=(source)->BidPrice1;	\
	(target)->BidVolume1=(source)->BidVolume1;	\
	(target)->HighestPrice=(source)->HighestPrice;	\
	(target)->LowestPrice=(source)->LowestPrice;	\
	(target)->LastPrice=(source)->LastPrice;	\
	(target)->Volume=(source)->Volume;	\
	(target)->Turnover=(source)->Turnover;	\
	(target)->UpperLimitPrice=(source)->UpperLimitPrice;	\
	(target)->LowerLimitPrice=(source)->LowerLimitPrice;	\
	(target)->PreClosePrice=(source)->PreClosePrice;	\
	(target)->PreOpenInterest=(source)->PreOpenInterest;	\
	(target)->OpenInterest=(source)->OpenInterest;		\
	(target)->PreSettlementPrice=(source)->PreSettlementPrice;	\
	(target)->SettlementPrice=(source)->SettlementPrice;	\
	CopyMarketDataUpdateTimeEntity(target,source);
#endif


#ifndef CopyMarketDataBaseEntity
#define CopyMarketDataBaseEntity(target,source)	\
	CopyTradingDayEntity(target,source);	\
	(target)->PreSettlementPrice=(source)->PreSettlementPrice;	\
	(target)->PreClosePrice=(source)->PreClosePrice;	\
	(target)->PreOpenInterest=(source)->PreOpenInterest;	\
	(target)->PreDelta=(source)->PreDelta;
#endif
#ifndef CopyMarketDataStaticEntity
#define CopyMarketDataStaticEntity(target,source)	\
	(target)->OpenPrice=(source)->OpenPrice;	\
	(target)->HighestPrice=(source)->HighestPrice;	\
	(target)->LowestPrice=(source)->LowestPrice;	\
	(target)->ClosePrice=(source)->ClosePrice;	\
	(target)->UpperLimitPrice=(source)->UpperLimitPrice;	\
	(target)->LowerLimitPrice=(source)->LowerLimitPrice;	\
	(target)->SettlementPrice=(source)->SettlementPrice;	\
	(target)->CurrDelta=(source)->CurrDelta;
#endif
#ifndef CopyMarketDataLastMatchEntity
#define CopyMarketDataLastMatchEntity(target,source)	\
	(target)->LastPrice=(source)->LastPrice;	\
	(target)->Volume=(source)->Volume;	\
	(target)->Turnover=(source)->Turnover;	\
	(target)->OpenInterest=(source)->OpenInterest;
#endif

#ifndef CopyMarketDataBestPriceEntity
#define CopyMarketDataBestPriceEntity(target,source)	\
	(target)->BidPrice1=(source)->BidPrice1;	\
	(target)->BidVolume1=(source)->BidVolume1;	\
	(target)->AskPrice1=(source)->AskPrice1;	\
	(target)->AskVolume1=(source)->AskVolume1;
#endif

// #ifndef CopySpecificInstrumentEntity
// #define CopySpecificInstrumentEntity(target,source)	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 
// #endif
// #ifndef CopyTopicSearchEntity
// #define CopyTopicSearchEntity(target,source)	\
// 	(target)->TopicID=(source)->TopicID;	\
// 
// #endif
// #ifndef CopyShmDepthMarketDataEntity
// #define CopyShmDepthMarketDataEntity(target,source)	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 	(target)->IPAddress=(source)->IPAddress;	\
// 	(target)->TopicID=(source)->TopicID;	\
// 
// #endif
// #ifndef CopyClientMarginCombTypeEntity
// #define CopyClientMarginCombTypeEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->ParticipantID=(source)->ParticipantID;	\
// 	(target)->ClientID=(source)->ClientID;	\
// 	(target)->InstrumentGroupID=(source)->InstrumentGroupID;	\
// 	(target)->MarginCombType=(source)->MarginCombType;	\
// 
// #endif
// #ifndef CopyInstrumentGroupEntity
// #define CopyInstrumentGroupEntity(target,source)	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->InstrumentID=(source)->InstrumentID;	\
// 	(target)->InstrumentGroupID=(source)->InstrumentGroupID;	\
// 
// #endif
// #ifndef CopyPartClientInsGroupPositionEntity
// #define CopyPartClientInsGroupPositionEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->ParticipantID=(source)->ParticipantID;	\
// 	(target)->ClientID=(source)->ClientID;	\
// 	(target)->InstrumentGroupID=(source)->InstrumentGroupID;	\
// 	(target)->LongPosition=(source)->LongPosition;	\
// 	(target)->LongMargin=(source)->LongMargin;	\
// 	(target)->ShortPosition=(source)->ShortPosition;	\
// 	(target)->ShortMargin=(source)->ShortMargin;	\
// 	(target)->ActualMargin =(source)->ActualMargin ;	\
// 	(target)->LongFrozenPosition=(source)->LongFrozenPosition;	\
// 	(target)->LongFrozenMargin=(source)->LongFrozenMargin;	\
// 	(target)->ShortFrozenPosition=(source)->ShortFrozenPosition;	\
// 	(target)->ShortFrozenMargin=(source)->ShortFrozenMargin;	\
// 	(target)->ActualFrozenMargin =(source)->ActualFrozenMargin ;	\
// 
// #endif
// #ifndef CopyInterLoginInfoEntity
// #define CopyInterLoginInfoEntity(target,source)	\
// 	(target)->ServerAddr=(source)->ServerAddr;	\
// 	(target)->MaxLocalID=(source)->MaxLocalID;	\
// 
// #endif
// #ifndef CopyAPIMessageNotifyInfoEntity
// #define CopyAPIMessageNotifyInfoEntity(target,source)	\
// 	(target)->BrokerID=(source)->BrokerID;	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 	(target)->ExchangeID=(source)->ExchangeID;	\
// 	(target)->Nums=(source)->Nums;	\
// 	(target)->BusinessType=(source)->BusinessType;	\
// 	(target)->WarnMsg=(source)->WarnMsg;	\
// 
// #endif
// #ifndef CopyMessageNotifyInfoEntity
// #define CopyMessageNotifyInfoEntity(target,source)	\
// 	CopyAPIMessageNotifyInfoEntity(target,source);	\
// 
// #endif
// #ifndef CopyQueryFinishNotifyEntity
// #define CopyQueryFinishNotifyEntity(target,source)	\
// 	CopyExchangeTradingDayEntity(target,source);	\
// 	(target)->MaxOrderLocalID=(source)->MaxOrderLocalID;	\
// 
// #endif
// #ifndef CopyInvestorLinkerEntity
// #define CopyInvestorLinkerEntity(target,source)	\
// 	(target)->UserID=(source)->UserID;	\
// 	(target)->APIID=(source)->APIID;	\
// 	(target)->InvestorID=(source)->InvestorID;	\
// 
// #endif
// #endif
// 
// 

