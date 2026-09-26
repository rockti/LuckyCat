//对交易所使用FTD（FuturesTradingData）通讯协议结构定义
#pragma once
#include "UserDataType.h"
#include "FieldDescribe.h"

//信息分发，订阅流
class CFTDDisseminationField {
public:
	//流号
	CSequenceSeriesType	SequenceSeries;
	//包号
	CSequenceNoType	SequenceNo;
	TYPE_DESCRIPTOR((TYPE_DESC(SequenceSeries),TYPE_DESC(SequenceNo)));
};
const DWORD FTD_FID_Dissemination = 0x3000;

//响应信息
class CFTDRspInfoField {
public:
	//错误代码
	CErrorIDType	ErrorID;
	//错误信息
	CErrorMsgType	ErrorMsg;
	TYPE_DESCRIPTOR((TYPE_DESC(ErrorID),TYPE_DESC(ErrorMsg)));
};
const DWORD FTD_FID_RspInfo = 0x3001;

//用户登录请求
class CFTDReqUserLoginField
{
public:
	//交易日
	CDateType	TradingDay;
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	//登录用户类型
	CUserTypeType	UserType;
	//密码
	CPasswordType	Password;
	//用户端产品信息
	CProductInfoType	UserProductInfo;
	//IP地址
	CIPAddressType	IPAddress;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(BrokerID),TYPE_DESC(LoginID),TYPE_DESC(UserType),TYPE_DESC(Password),TYPE_DESC(UserProductInfo),TYPE_DESC(IPAddress)));
};
const DWORD FTD_FID_ReqUserLogin = 0x3002;
//用户登录应答
class CFTDRspUserLoginField
{
public:
	//交易日
	CDateType	TradingDay;
	//登录成功时间
	CTimeType	LoginTime;
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	//登录用户类型
	CUserTypeType	UserType;
	//交易系统名称
	CTradingSystemNameType	TradingSystemName;
	//会话编号
	CSessionIDType	SessionID;
	//用户最大本地报单号
	CUserOrderLocalIDType	MaxOrderLocalID;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(LoginTime),TYPE_DESC(BrokerID),TYPE_DESC(LoginID),TYPE_DESC(UserType),TYPE_DESC(TradingSystemName),TYPE_DESC(SessionID),TYPE_DESC(MaxOrderLocalID)));
};
const DWORD FTD_FID_RspUserLogin = 0x3003;
//用户登出请求
class CFTDReqUserLogoutField {
public:
	//经纪公司编号（对应会员号）
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(LoginID)));
};
const DWORD FTD_FID_ReqUserLogout = 0x3004;
//用户登出应答
class CFTDRspUserLogoutField {
public:
	//经纪公司编号（对应会员号）
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(LoginID)));
};
const DWORD FTD_FID_RspUserLogout = 0x3005;
//强制用户退出
class CFTDForceInvestorExitField {
public:
	//经纪公司编号（对应会员号）
	CBrokerIDType	BrokerID;
	//交易用户代码
	CInvestorIDType	InvestorID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID)));
};
const DWORD FTD_FID_ForceUserExit = 0x3006;


//用户密码修改
class CFTDUserPasswordUpdateField {
public:
	//经纪公司编号（对应会员号）
	CBrokerIDType	BrokerID;
	//交易用户代码
	CInvestorIDType	InvestorID;
	//现密码
	CPasswordType OldPassword;
	//新密码
	CPasswordType NewPassword;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(OldPassword), TYPE_DESC(NewPassword)));
};
const DWORD FTD_FID_UserPasswordUpdate = 0x3007;

//查询合约查询
class CFTDQryInstrumentField {
public:
	//经纪公司编号（对应会员号）
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//产品类型
	CProductClassType	ProductClass;
	//合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID)));
};
const DWORD FTD_FID_Qry_Instrument = 0x3010;
//合约信息应答
class CFTDRspInstrumentField
{
public:
	//交易所代码
	CExchangeIDType	ExchangeID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//合约名称
	CInstrumentNameType	InstrumentName;
	//品种代码
	CProductIDType	ProductID;
	//产品类型
	CProductClassType	ProductClass;
	//多头限仓
	CVolumeType	LongPosLimit;
	//空头限仓
	CVolumeType	ShortPosLimit;
	//限价单最大下单量
	CVolumeType	MaxLimitOrderVolume;
	//限价单最小下单量
	CVolumeType	MinLimitOrderVolume;
	//市价单最大下单量
	CVolumeType	MaxMarketOrderVolume;
	//市价单最小下单量
	CVolumeType	MinMarketOrderVolume;
	//数量乘数
	CVolumeMultipleType	VolumeMultiple;
	//报价单位
	CPriceTickType	PriceTick;
	//合约交易状态
	CInstrumentStatusType	InstrumentStatus;
	TYPE_DESCRIPTOR((TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID),TYPE_DESC(InstrumentName),TYPE_DESC(ProductID),TYPE_DESC(ProductClass)
		,TYPE_DESC(LongPosLimit),TYPE_DESC(ShortPosLimit),TYPE_DESC(MaxLimitOrderVolume),TYPE_DESC(MinLimitOrderVolume),TYPE_DESC(MaxMarketOrderVolume)
		,TYPE_DESC(MinMarketOrderVolume),TYPE_DESC(VolumeMultiple),TYPE_DESC(PriceTick),TYPE_DESC(InstrumentStatus)));
};
const DWORD FTD_FID_Rsp_Instrument = 0x3011;

//投资者资金查询
class CFTDQryInvestorAccountField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID)));
};
const DWORD FTD_FID_Qry_Account = 0x3012;
//投资者资金账户
class CFTDRspInvestorAccountField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易日
	CTradingDayType	TradingDay;
	//上次结算准备金
	CMoneyType	PreBalance;
	//上日可用资金
	CMoneyType	PreAvailable;
	//结算准备金
	CMoneyType	Balance;
	//可用资金
	CMoneyType	Available;
	//入金金额
	CMoneyType	Deposit;
	//出金金额
	CMoneyType	Withdraw;
	//占用保证金
	CMoneyType	Margin;
	//手续费
	CMoneyType	Fee;
	//其他费用
	CMoneyType	OtherFee;
	//平仓盈亏
	CMoneyType	CloseProfit;
	//持仓盈亏
	CMoneyType	PositionProfit;
	//冻结的保证金
	CMoneyType	FrozenMargin;
	//冻结手续费
	CMoneyType	FrozenFee;
	//多头占用保证金
	CMoneyType	LongMargin;
	//空头占用保证金
	CMoneyType	ShortMargin;
	//多头冻结的保证金
	CMoneyType	LongFrozenMargin;
	//空头冻结的保证金
	CMoneyType	ShortFrozenMargin;
	//动态权益
	CMoneyType	DynamicRights;
	//风险度
	CMoneyType	Risk;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(TradingDay),TYPE_DESC(PreBalance),TYPE_DESC(PreAvailable),TYPE_DESC(Balance),TYPE_DESC(Available)
		,TYPE_DESC(Deposit),TYPE_DESC(Withdraw),TYPE_DESC(Margin),TYPE_DESC(Fee),TYPE_DESC(OtherFee),TYPE_DESC(CloseProfit),TYPE_DESC(PositionProfit),TYPE_DESC(FrozenMargin)
		,TYPE_DESC(FrozenFee),TYPE_DESC(LongMargin),TYPE_DESC(ShortMargin),TYPE_DESC(LongFrozenMargin),TYPE_DESC(ShortFrozenMargin),TYPE_DESC(DynamicRights),TYPE_DESC(Risk)));
};
const DWORD FTD_FID_Rsp_Account = 0x3013;

//投资者持仓查询请求
class CFTDQryInvestorPositionField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CInvestorIDType	ExchangeID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID)));
};
const DWORD FTD_FID_Qry_Position = 0x3014;
//投资者持仓信息
class CFTDRspInvestorPositionField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易日期
	CTradingDayType	TradingDay;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//产品类型
	CProductClassType	ProductClass;
	//买卖方向
	CDirectionType	Direction;
	//占用保证金
	CMoneyType	UsedMargin;
	//今持仓量
	CVolumeType	Position;
	//今日持仓成本(复用库存总量)
	CPriceType	PositionCost;
	//昨持仓量
	CVolumeType	YdPosition;
	//昨日持仓结算价
	CMoneyType	YdPositionPrice;
	//冻结的保证金
	CMoneyType	FrozenMargin;
	//开平冻结手续费
	CMoneyType	FrozenFee;
	//平仓冻结持仓
	CVolumeType	FrozenPosition;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(TradingDay),TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID)
		,TYPE_DESC(ProductClass),TYPE_DESC(Direction),TYPE_DESC(UsedMargin),TYPE_DESC(Position),TYPE_DESC(PositionCost),TYPE_DESC(YdPosition)
		,TYPE_DESC(YdPositionPrice), TYPE_DESC(FrozenMargin),TYPE_DESC(FrozenFee),TYPE_DESC(FrozenPosition)));
};
const DWORD FTD_FID_Rsp_Position = 0x3015;

//投资者保证金率查询
class CFTDQryInvestorMarginField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID)));
};
const DWORD FTD_FID_Qry_Margin = 0x3016;
//投资者保证金率信息
class CFTDRspInvestorMarginField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//投资者编码
	CInvestorIDType	InvestorID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//多头占用保证金按比例
	CRatioType	LongMarginRate;
	//多头保证金按手数
	CRatioType	LongMarginAmt;
	//空头占用保证金按比例
	CRatioType	ShortMarginRate;
	//空头保证金按手数
	CRatioType	ShortMarginAmt;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(ExchangeID),TYPE_DESC(InvestorID),TYPE_DESC(InstrumentID)
		,TYPE_DESC(LongMarginRate),TYPE_DESC(LongMarginAmt),TYPE_DESC(ShortMarginRate),TYPE_DESC(ShortMarginAmt)));
};
const DWORD FTD_FID_Rsp_Margin = 0x3017;

//投资者手续费率查询
class CFTDQryInvestorFeeField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID)));
};
const DWORD FTD_FID_Qry_Fee = 0x3018;
//投资者手续费率信息
class CFTDRspInvestorFeeField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//投资者编码
	CInvestorIDType	InvestorID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//开仓手续费按比例
	CRatioType	OpenFeeRate;
	//开仓手续费按手数
	CRatioType	OpenFeeAmt;
	//平仓手续费按比例
	CRatioType	OffsetFeeRate;
	//平仓手续费按手数
	CRatioType	OffsetFeeAmt;
	//平今仓手续费按比例
	CRatioType	OffsetTodayFeeRate;
	//平今仓手续费按手数
	CRatioType	OffsetTodayFeeAmt;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(ExchangeID),TYPE_DESC(InvestorID),TYPE_DESC(InstrumentID),TYPE_DESC(OpenFeeRate)
		,TYPE_DESC(OpenFeeAmt),TYPE_DESC(OffsetFeeRate),TYPE_DESC(OffsetFeeAmt),TYPE_DESC(OffsetTodayFeeRate),TYPE_DESC(OffsetTodayFeeAmt)));
};
const DWORD FTD_FID_Rsp_Fee = 0x3019;

//订阅合约的相关信息
class CFTDSpecificInstrumentField {
public:
	///合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(InstrumentID)));
};
const DWORD FTD_FID_SpecificInstrument = 0x3020;
//报单查询
class CFTDQryOrderField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//报单编号
	COrderSysIDType	OrderSysID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(OrderSysID),TYPE_DESC(InstrumentID)));
};
const DWORD FTD_FID_Qry_Order = 0x3021;
//成交查询
class CFTDQryTradeField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//报单编号
	COrderSysIDType	OrderSysID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(OrderSysID),TYPE_DESC(InstrumentID)));
};
const DWORD FTD_FID_Qry_Trade = 0x3022;

//输入报单
class CFTDInputOrderField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//报单用户(非操作员下单和客户号相同)
	COperatorIDType	OrderOperatorID;
	//投资者编号(客户号)
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//品种类型
	CProductClassType	ProductClass;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//买卖方向
	CDirectionType	Direction;
	//开平标志
	COffsetFlagType	OffsetFlag;
	//投机套保标志
	CHedgeFlagType	HedgeFlag;
	//价格
	CPriceType	LimitPrice;
	//数量
	CVolumeType	Volume;
	//报单类型
	COrderPriceTypeType	OrderPriceType;
	//用户自定义域
	CCustomType	UserCustom;
	//用户本地报单号
	CUserOrderLocalIDType	UserOrderLocalID;
	//系统报单编号(应答)
	COrderSysIDType	OrderSysID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(OrderOperatorID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass)
		,TYPE_DESC(InstrumentID),TYPE_DESC(Direction),TYPE_DESC(OffsetFlag),TYPE_DESC(HedgeFlag),TYPE_DESC(LimitPrice),TYPE_DESC(Volume)
		,TYPE_DESC(OrderPriceType),TYPE_DESC(UserCustom),TYPE_DESC(UserOrderLocalID),TYPE_DESC(OrderSysID)));
};
const DWORD FTD_FID_InputOrder = 0x3023;
//报单操作
class CFTDOrderActionField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//撤单用户代码
	COperatorIDType	CancelOperatorID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//品种类型
	CProductClassType	ProductClass;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//投保标志（黄金必填）
	CHedgeFlagType	HedgeFlag;;
	//系统报单编号
	COrderSysIDType	OrderSysID;
	//待撤订单的本地报单编号
	CUserOrderLocalIDType	UserOrderLocalID;
	//撤单指令的本地编号
	CUserOrderLocalIDType	UserOrderActionLocalID;
	//报单操作标志
	CActionFlagType	ActionFlag;
	//价格
	CPriceType	LimitPrice;
	//数量变化
	CVolumeType	VolumeChange;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(CancelOperatorID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID),TYPE_DESC(HedgeFlag)
		,TYPE_DESC(OrderSysID),TYPE_DESC(UserOrderLocalID),TYPE_DESC(UserOrderActionLocalID),TYPE_DESC(ActionFlag),TYPE_DESC(LimitPrice),TYPE_DESC(VolumeChange)));
};
const DWORD FTD_FID_OrderAction = 0x3024;
//报单信息
class CFTDOrderField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//报单用户(非操作员下单和客户号相同)
	COperatorIDType	OrderOperatorID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//品种类型
	CProductClassType	ProductClass;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//买卖方向
	CDirectionType	Direction;
	//开平标志
	COffsetFlagType	OffsetFlag;
	//投机套保标志
	CHedgeFlagType	HedgeFlag;
	//价格
	CPriceType	LimitPrice;
	//数量
	CVolumeType	Volume;
	//报单类型
	COrderPriceTypeType	OrderPriceType;
	//用户自定义域
	CCustomType	UserCustom;
	//用户本地报单号
	CUserOrderLocalIDType	UserOrderLocalID;
	//系统报单编号
	COrderSysIDType	OrderSysID;
	/****************************以上是输入报单信息****************************/
	//报单状态
	COrderStatusType	OrderStatus;
	//交易日
	CTradingDayType	TradingDay;
	//下单席位号
	CSeatIDType	SeatID;
	//交易编码
	CClientIDType	ClientID;
	//插入时间
	CTimeType	InsertTime;
	//撤销时间
	CTimeType	CancelTime;
	//撤单用户编号
	COperatorIDType	CancelOperatorID;
	//今成交数量
	CVolumeType	VolumeTrade;
	//剩余数量
	CVolumeType	VolumeRemain;
	//本地报单编号（本系统的）
	COrderLocalIDType	OrderLocalID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(OrderOperatorID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass)
		,TYPE_DESC(InstrumentID),TYPE_DESC(Direction),TYPE_DESC(OffsetFlag),TYPE_DESC(HedgeFlag),TYPE_DESC(LimitPrice),TYPE_DESC(Volume)
		,TYPE_DESC(OrderPriceType),TYPE_DESC(UserCustom),TYPE_DESC(UserOrderLocalID),TYPE_DESC(OrderSysID)
		,TYPE_DESC(OrderStatus),TYPE_DESC(TradingDay),TYPE_DESC(SeatID),TYPE_DESC(ClientID),TYPE_DESC(InsertTime),TYPE_DESC(CancelTime)
		,TYPE_DESC(CancelOperatorID),TYPE_DESC(VolumeTrade),TYPE_DESC(VolumeRemain),TYPE_DESC(OrderLocalID)));
};
const DWORD FTD_FID_OrderRtn = 0x3025;
//成交信息
class CFTDTradeField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//品种类型
	CProductClassType	ProductClass;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//买卖方向
	CDirectionType	Direction;
	//开平标志
	COffsetFlagType	OffsetFlag;
	//投机套保标志
	CHedgeFlagType	HedgeFlag;
	//成交数量
	CVolumeType	TradeVolume;
	//成交价格
	CPriceType	TradePrice;
	//成交金额
	CMoneyType	TradeAmt;
	//成交日期
	CDateType	TradeDate;
	//成交时间
	CTimeType	TradeTime;
	//成交编号
	CTraderIDType	TradeID;
	//报单编号
	COrderSysIDType	OrderSysID;
	//本地报单编号
	CUserOrderLocalIDType	UserOrderLocalID;
	//交易编码
	CClientIDType	ClientID;
	//下单用户编号
	COperatorIDType	OrderOperatorID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID),TYPE_DESC(Direction)
		,TYPE_DESC(OffsetFlag),TYPE_DESC(HedgeFlag),TYPE_DESC(TradeVolume),TYPE_DESC(TradePrice),TYPE_DESC(TradeAmt),TYPE_DESC(TradeDate),TYPE_DESC(TradeTime)
		,TYPE_DESC(TradeID),TYPE_DESC(OrderSysID),TYPE_DESC(UserOrderLocalID),TYPE_DESC(ClientID),TYPE_DESC(OrderOperatorID)));
};
const DWORD FTD_FID_TradeRtn = 0x3026;
//合约交易状态
class CFTDInstrumentStatusField
{
public:
	//交易所代码
	CExchangeIDType ExchangeID;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//合约交易状态
	CInstrumentStatusType	InstrumentStatus;
	TYPE_DESCRIPTOR((TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID),TYPE_DESC(InstrumentStatus)));
};
const DWORD FTD_FID_InstrumentStatus = 0x3027;

//交易编码查询
class CFTDQryTradingCodeField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//用户代码
	COperatorIDType	OperatorID;
	//投资者编号
	CInvestorIDType	InvestorID;
};

//操作员查询
class CFTDQryUserField
{
public:
	//交易用户代码
	COperatorIDType	StartOperatorID;
	//交易用户代码
	COperatorIDType	EndOperatorID;
};
//操作员信息
class CFTDUserField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//用户代码
	COperatorIDType	OperatorID;
	//用户登录密码
	CPasswordType	Password;
	//是否活跃
	CIsActiveType	IsActive;
	//用户名称
	CUserNameType	UserName;
};

//可用投资者查询
class CFTDQryUserInvestorField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//用户代码
	COperatorIDType	OperatorID;
};
//可用投资者信息
class CFTDRspUserInvestorField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	COperatorIDType	OperatorID;
	//投资者编号
	CInvestorIDType	InvestorID;
};

class CFTDDeferFeeField {
public:
	CInstrumentIDType InstrumentID;
	CDeferFeeDirectionType Direction;
	CRatioType FeeRate;
	TYPE_DESCRIPTOR((TYPE_DESC(InstrumentID), TYPE_DESC(Direction), TYPE_DESC(FeeRate)));
};

const DWORD FTD_FID_DeferFee = 0x3028;

//深度行情
class CFTDDepthMarketDataField {
public:
	//交易日
	CTradingDayType	TradingDay;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//今开盘
	CPriceType	OpenPrice;
	//申卖价一
	CPriceType	AskPrice1;
	//申卖量一
	CVolumeType	AskVolume1;
	//申卖价二
	CPriceType	AskPrice2;
	//申卖量二
	CVolumeType	AskVolume2;
	//申卖价三
	CPriceType	AskPrice3;
	//申卖量三
	CVolumeType	AskVolume3;
	//申卖价四
	CPriceType	AskPrice4;
	//申卖量四
	CVolumeType	AskVolume4;
	//申卖价五
	CPriceType	AskPrice5;
	//申卖量五
	CVolumeType	AskVolume5;
	//申买价一
	CPriceType	BidPrice1;
	//申买量一
	CVolumeType	BidVolume1;
	//申买价二
	CPriceType	BidPrice2;
	//申买量二
	CVolumeType	BidVolume2;
	//申买价三
	CPriceType	BidPrice3;
	//申买量三
	CVolumeType	BidVolume3;
	//申买价四
	CPriceType	BidPrice4;
	//申买量四
	CVolumeType	BidVolume4;
	//申买价五
	CPriceType	BidPrice5;
	//申买量五
	CVolumeType	BidVolume5;
	//最高价
	CPriceType	HighestPrice;
	//最低价
	CPriceType	LowestPrice;
	//最新价
	CPriceType	LastPrice;
	//均价
	CPriceType	AvgPrice;
	//成交重量
	CPriceType	Weight;
	//成交量
	CVolumeType	Volume;
	//成交额
	CMoneyType	Turnover;
	//跌停板价
	CPriceType	LowerLimitPrice;
	//涨停板价
	CPriceType	UpperLimitPrice;
	//今持仓量
	CLargeVolumeType	OpenInterest;
	//昨持仓量
	CLargeVolumeType	PreOpenInterest;
	//昨收盘
	CPriceType	PreClosePrice;
	//昨结算
	CPriceType	PreSettlementPrice;
	//今收盘
	CPriceType	ClosePrice;
	//今结算
	CPriceType	SettlementPrice;
	//最后修改时间
	CTimeType	UpdateTime;
	//最后修改毫秒
	CMillisecType	Millisec;
	//行情序号
	CLargeVolumeType	SequenceNo;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(InstrumentID),TYPE_DESC(OpenPrice)
		,TYPE_DESC(AskPrice1),TYPE_DESC(AskVolume1),TYPE_DESC(BidPrice1),TYPE_DESC(BidVolume1)
		,TYPE_DESC(AskPrice2),TYPE_DESC(AskVolume2),TYPE_DESC(BidPrice2),TYPE_DESC(BidVolume2)
		,TYPE_DESC(AskPrice3),TYPE_DESC(AskVolume3),TYPE_DESC(BidPrice3),TYPE_DESC(BidVolume3)
		,TYPE_DESC(AskPrice4),TYPE_DESC(AskVolume4),TYPE_DESC(BidPrice4),TYPE_DESC(BidVolume4)
		,TYPE_DESC(AskPrice5),TYPE_DESC(AskVolume5),TYPE_DESC(BidPrice5),TYPE_DESC(BidVolume5)
		,TYPE_DESC(HighestPrice),TYPE_DESC(LowestPrice),TYPE_DESC(LastPrice),TYPE_DESC(AvgPrice),TYPE_DESC(Weight),TYPE_DESC(Volume),TYPE_DESC(Turnover)
		,TYPE_DESC(LowerLimitPrice),TYPE_DESC(UpperLimitPrice),TYPE_DESC(OpenInterest),TYPE_DESC(PreClosePrice),TYPE_DESC(PreOpenInterest)
		,TYPE_DESC(PreSettlementPrice),TYPE_DESC(ClosePrice),TYPE_DESC(SettlementPrice),TYPE_DESC(UpdateTime),TYPE_DESC(Millisec),TYPE_DESC(SequenceNo)));
};
const DWORD FTD_FID_DepthMarketData = 0x3030;


///实时行情查询应答
class CFTDRspMarketDataField
{
public:
	///交易日
	CTradingDayType	TradingDay;
	///交易所代码
	CExchangeIDType	ExchangeID;
	///合约代码
	CInstrumentIDType	InstrumentID;
	///今开盘
	CPriceType	OpenPrice;
	///申卖价一
	CPriceType	AskPrice1;
	///申卖量一
	CVolumeType	AskVolume1;
	///申买价一
	CPriceType	BidPrice1;
	///申买量一
	CVolumeType	BidVolume1;
	///最高价
	CPriceType	HighestPrice;
	///最低价
	CPriceType	LowestPrice;
	///最新价
	CPriceType	LastPrice;
	///数量
	CVolumeType	Volume;
	///成交金额
	CMoneyType	Turnover;
	///涨停板价
	CPriceType	UpperLimitPrice;
	///跌停板价
	CPriceType	LowerLimitPrice;
	///昨收盘
	CPriceType	PreClosePrice;
	///昨持仓量
	CLargeVolumeType	PreOpenInterest;
	///持仓量
	CLargeVolumeType	OpenInterest;
	///昨结算
	CPriceType	PreSettlementPrice;
	///今结算
	CPriceType	SettlementPrice;
	///最后修改毫秒
	CMillisecType	Millisec;
	///最后修改时间
	CTimeType	UpdateTime;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID),TYPE_DESC(OpenPrice)
		,TYPE_DESC(AskPrice1),TYPE_DESC(AskVolume1),TYPE_DESC(BidPrice1),TYPE_DESC(BidVolume1),TYPE_DESC(HighestPrice),TYPE_DESC(LowestPrice)
		,TYPE_DESC(LastPrice),TYPE_DESC(Volume),TYPE_DESC(Turnover),TYPE_DESC(UpperLimitPrice),TYPE_DESC(LowerLimitPrice),TYPE_DESC(PreClosePrice)
		,TYPE_DESC(PreOpenInterest),TYPE_DESC(OpenInterest),TYPE_DESC(PreSettlementPrice),TYPE_DESC(SettlementPrice),TYPE_DESC(Millisec),TYPE_DESC(UpdateTime)));
};
const DWORD FTD_FID_MarketData=0x3053;