//内部ZTP通讯协议结构定义
#pragma once
/*

#include "UserDataType.h"
#include "FieldDescribe.h"

//系统状态
class CSystemStatusField {
public:
	//交易日
	CDateType	TradingDay;
	//系统状态
	CSystemStatusType	SystemStatus;
	//交易所交易状态
	CTradingStatusType TradingStatus;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(SystemStatus),TYPE_DESC(TradingStatus)));
};
const DWORD FID_SystemStatus = 0x3001;

//席位信息
class CSeatInfoField {
public:
	CAppNameType AppName;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//会员编号
	CParticipantIDType	ParticipantID;
	//席位号
	CSeatIDType	SeatID;
	//登录交易员号
	CTraderIDType TraderID;
	//席位密码（金交所对应交易员密码）
	CPasswordType	Password;
	//席位状态
	CSeatStatusType SeatStatus;
	//连接地址
	CFrontAddrType	FrontAddr;
	//本地报单编号
	COrderLocalIDType	OrderLocalID;
	TYPE_DESCRIPTOR((TYPE_DESC(AppName), TYPE_DESC(ExchangeID),TYPE_DESC(ParticipantID),TYPE_DESC(SeatID),TYPE_DESC(TraderID),TYPE_DESC(Password),
		TYPE_DESC(SeatStatus),TYPE_DESC(FrontAddr),TYPE_DESC(OrderLocalID)));
};
const DWORD FID_SeatInfo = 0x3002;

//操作员信息
class COperatorInfoField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//用户代码
	COperatorIDType	OperatorID;
	//用户名称
	CUserNameType	UserName;
	//用户登录密码
	CPasswordType	Password;
	//用户权限
	CPermissionRightType	PermissionRight;
	COperatorInfoField& operator=(COperatorInfoField const& other) = default;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(OperatorID),TYPE_DESC(UserName),TYPE_DESC(Password),TYPE_DESC(PermissionRight)));
};
const DWORD FID_OperatorInfo = 0x3004;

//投资者信息
class CInvestorInfoField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者帐号
	CInvestorIDType	InvestorID;
	//投资者名称
	CInvestorNameType InvestorName;
	//资者密码
	CPasswordType	Password;
	// SGE Client ID
	CClientIDType ClientID;
	//交易权限	
	CTradeRightType	TradeRight;
	//是否可用
	CIsActiveType	IsActive;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(InvestorName),TYPE_DESC(Password),TYPE_DESC(ClientID),TYPE_DESC(TradeRight),TYPE_DESC(IsActive)));
};
const DWORD FID_InvestorInfo = 0x3005;

//用户投资者关系
class COperatorInvestorField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	COperatorIDType	OperatorID;
	//投资者编号
	CInvestorIDType	InvestorID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(OperatorID),TYPE_DESC(InvestorID)));
};
const DWORD FID_OperatorInvestor = 0x3006;

//用户会话
class CUserSessionField {
public:
	//经纪公司代码
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	//用户类型
	CUserTypeType	UserType;
	//会话编号
	CSessionIDType	SessionID;
	//登录时间
	CTimeType	LoginTime;
	//IP地址
	CIPAddressType	IPAddress;
	//用户端产品信息
	CProductInfoType	UserProductInfo;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(LoginID),TYPE_DESC(UserType),TYPE_DESC(SessionID),TYPE_DESC(LoginTime),TYPE_DESC(IPAddress),TYPE_DESC(UserProductInfo)));
};
const DWORD FID_UserSession = 0x3007;

//投资者资金账户
class CInvestorAccountField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易日
	CTradingDayType	TradingDay;
	//上日权益
	CMoneyType	PreBalance;
	//上日可用
	CMoneyType	PreAvailable;
	//当前权益
	CMoneyType	Balance;
	//当前可用
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
const DWORD FID_InvestorAccount = 0x300A;

//客户保证金率
class CInvestorMarginField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//投资者编号
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
const DWORD FID_InvestorMargin = 0x300B;

//投资者手续费
class CInvestorFeeField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//投资者编号
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
const DWORD FID_InvestorFee = 0x300C;

//合约信息
class CInstrumentField {
public:
	//交易所代码
	CExchangeIDType	ExchangeID;
	//合约编号
	CInstrumentIDType	InstrumentID;
	//合约名称
	CInstrumentNameType	InstrumentName;
	//品种代码
	CProductIDType	ProductID;
	//品种类型
	CProductClassType	ProductClass;
	//多头限仓
	CVolumeType	LongPosLimit;
	//空头限仓
	CVolumeType	ShortPosLimit;
	//最大下单量
	CVolumeType	MaxOrderVolume;
	//最小下单量
	CVolumeType	MinOrderVolume;
	//数量乘数
	CVolumeMultipleType	VolumeMultiple;
	//报价单位
	CPriceTickType	PriceTick;
	TYPE_DESCRIPTOR((TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID),TYPE_DESC(InstrumentName),TYPE_DESC(ProductID), TYPE_DESC(ProductClass),TYPE_DESC(LongPosLimit),
		TYPE_DESC(ShortPosLimit),TYPE_DESC(MaxOrderVolume),TYPE_DESC(MinOrderVolume),TYPE_DESC(VolumeMultiple),TYPE_DESC(PriceTick)));
};
const DWORD FID_Instrument = 0x300D;

//投资者合约持仓
class CInvestorPositionField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易日
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
	CVolumeType Position;
	//今日持仓成本
	CPriceType	PositionCost;
	//昨持仓量
	CVolumeType YdPosition;
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
const DWORD FID_InvestorPosition = 0x300E;
//应答信息
class CRspInfoField {
public:
	//错误代码
	CErrorIDType	ErrorID;
	//错误信息
	CErrorMsgType	ErrorMsg;
	TYPE_DESCRIPTOR((TYPE_DESC(ErrorID),TYPE_DESC(ErrorMsg)));
};
const DWORD FID_RspInfo = 0x3010;

//用户登录
class CUserLoginField {
public:
	//交易日
	CDateType	TradingDay;
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	//用户类型
	CUserTypeType	UserType;
	//密码
	CPasswordType	Password;
	//用户端产品信息(用户填写)
	CProductInfoType	UserProductInfo;
	//接口端产品信息(交易系统返回)
	CTradingSystemNameType	TradingSystemName;
	//登录IP
	CIPAddressType	IPAddress;
	//登录成功时间
	CTimeType	LoginTime;
	//最大报单本地编号
	CUserOrderLocalIDType	MaxOrderLocalID;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(BrokerID),TYPE_DESC(LoginID),TYPE_DESC(UserType),TYPE_DESC(Password)
		,TYPE_DESC(UserProductInfo),TYPE_DESC(TradingSystemName),TYPE_DESC(IPAddress),TYPE_DESC(LoginTime),TYPE_DESC(MaxOrderLocalID)));
};
const DWORD FID_UserLogin = 0x3011;

//用户录出
class CUserLogoutField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	CLoginIDType	LoginID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(LoginID)));
};
const DWORD FID_UserLogout = 0x3012;

//强制用户退出
class CForceInvestorExitField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//交易用户代码
	CInvestorIDType	InvestorID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID)));
};
const DWORD FID_ForceUserExit = 0x3013;

//合约查询
class CQryInstrumentField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//产品类型
	CProductClassType	ProductClass;
	//合约代码(*代表查询所有合约)
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID)));
};
const DWORD FID_QryInstrument = 0x3015;

//投资者资金账户查询
class CQryInvestorAccountField
{
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//登录的经纪公司编号
	CBrokerIDType	LoginBrokerID;
	//登录用户代码(可能是操作员)
	CLoginIDType	LoginUserID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(LoginBrokerID),TYPE_DESC(LoginUserID)));
};
const DWORD FID_QryInvestorAccount = 0x3016;

//投资者持仓查询
class CQryInvestorPositionField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType ExchangeID;
	//登录的经纪公司编号
	CBrokerIDType	LoginBrokerID;
	//登录用户代码(可能是操作员)
	CLoginIDType	LoginUserID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(LoginBrokerID),TYPE_DESC(LoginUserID)));
};
const DWORD FID_QryInvestorPosition = 0x3017;

//投资者保证金率查询
class CQryInvestorMarginField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//合约代码, *表示查询所有
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentID)));
};
const DWORD FID_QryInvestorMargin = 0x3018;

///投资者手续费率查询
class CQryInvestorFeeField
{
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
const DWORD FID_QryInvestorFee = 0x3019;

//报单操作
class COrderActionField {
public:
	//经纪公司编号
	CBrokerIDType	BrokerID;
	//用户代码
	COperatorIDType	CancelOperatorID;
	//投资者编号
	CInvestorIDType	InvestorID;
	//交易所代码
	CExchangeIDType	ExchangeID;
	//品种类型
	CProductClassType	ProductClass;
	//合约代码
	CInstrumentIDType	InstrumentID;
	//报单编号
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
	//下单席位号	
	CSeatIDType SeatID;
	//交易编码	
	CClientIDType ClientID;
	//待撤本地报单号(本系统的)
	COrderLocalIDType OrderLocalID;
	//撤单指令本地操作编号(本系统的)	
	COrderLocalIDType ActionLocalID;
	//撤销时间
	CTimeType CancelTime;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(CancelOperatorID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID)
		,TYPE_DESC(OrderSysID),TYPE_DESC(UserOrderLocalID),TYPE_DESC(UserOrderActionLocalID),TYPE_DESC(ActionFlag),TYPE_DESC(LimitPrice),TYPE_DESC(VolumeChange)
		,TYPE_DESC(SeatID),TYPE_DESC(ClientID),TYPE_DESC(OrderLocalID),TYPE_DESC(ActionLocalID),TYPE_DESC(CancelTime)));

};
const DWORD FID_OrderAction = 0x3021;
//席位登录通知
class CSeatLoginField {
public:
	//交易所代码
	CExchangeIDType	ExchangeID;
	//席位号
	CSeatIDType	SeatID;
	//交易员号
	CTraderIDType	TraderID;
	//交易日
	CTradingDayType	TradingDay;
	//最大本地报单号
	COrderLocalIDType	OrderLocalID;
	TYPE_DESCRIPTOR((TYPE_DESC(ExchangeID),TYPE_DESC(SeatID),TYPE_DESC(TraderID),TYPE_DESC(TradingDay),TYPE_DESC(OrderLocalID)));
};
const DWORD FID_LoginInfo = 0x3023;

//合约状态通知
class CInstrumentStatusField {
public:
	//交易所代码
	CExchangeIDType ExchangeID;
	//合约交易状态
	CInstrumentStatusType	InstrumentStatus;
	TYPE_DESCRIPTOR((TYPE_DESC(ExchangeID),TYPE_DESC(InstrumentStatus)));
};
const DWORD FID_InstrumentStatus = 0x3024;


//递延费率通知
class CDeferFeeField {
public:
	//合约代码
	CInstrumentIDType	InstrumentID;
	//支付方向(复用持仓方向，净0-表示平-3)
	CPosiDirectionType	Direction;
	//递延费率
	CRatioType	FeeRate;
	TYPE_DESCRIPTOR((TYPE_DESC(InstrumentID), TYPE_DESC(Direction), TYPE_DESC(FeeRate)));
};
const DWORD FID_DeferFee = 0x3025;

//报单信息
class COrderField {
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
	//价格
	CPriceType	LimitPrice;
	//数量
	CVolumeType	Volume;
	//报单类型
	COrderPriceTypeType	OrderPriceType;
	//用户本地报单号
	CUserOrderLocalIDType	UserOrderLocalID;
	//系统报单编号(应答)
	COrderSysIDType	OrderSysID;
	//用户自定义域
	CCustomType	UserCustom;
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
	//本地报单编号（本系统的）
	COrderLocalIDType	ActionLocalID;
	//用户本地报单号
	CUserOrderLocalIDType	UserActionLocalID;
	//冻结的保证金
	CMoneyType	FrozenMargin;
	//冻结手续费
	CMoneyType	FrozenFee;
	//成交手续费
	CMoneyType	TradeMargin;
	//成交手续费
	CMoneyType	TradeFee;
	//委托错误码
	CIntType InsertErrorCode;
	//撤单错误码
	CIntType CancelErrorCode;
	//回报序号
	CSequenceNoType SequenceNo;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(OrderOperatorID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID),
		TYPE_DESC(Direction),TYPE_DESC(OffsetFlag),TYPE_DESC(LimitPrice),TYPE_DESC(Volume),TYPE_DESC(OrderPriceType),
		TYPE_DESC(UserOrderLocalID),TYPE_DESC(OrderSysID), TYPE_DESC(UserCustom), TYPE_DESC(OrderStatus),TYPE_DESC(TradingDay),TYPE_DESC(SeatID),TYPE_DESC(ClientID),
		TYPE_DESC(InsertTime),TYPE_DESC(CancelTime),TYPE_DESC(CancelOperatorID),TYPE_DESC(VolumeTrade),TYPE_DESC(VolumeRemain),TYPE_DESC(OrderLocalID), TYPE_DESC(ActionLocalID),TYPE_DESC(UserActionLocalID),
		TYPE_DESC(FrozenMargin),TYPE_DESC(FrozenFee),TYPE_DESC(TradeMargin),TYPE_DESC(TradeFee),TYPE_DESC(InsertErrorCode),TYPE_DESC(CancelErrorCode), TYPE_DESC(SequenceNo)));
};
const DWORD FID_Order = 0x3026;

//成交信息
class CTradeField {
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
	//下单用户
	COperatorIDType	OrderOperatorID;
	//本地报单编号
	COrderLocalIDType	OrderLocalID;
	//占用的保证金
	CMoneyType	UsedMargin;
	//占用手续费
	CMoneyType	UsedFee;
	//回报序号
	CSequenceNoType SequenceNo;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID),TYPE_DESC(InvestorID),TYPE_DESC(ExchangeID),TYPE_DESC(ProductClass),TYPE_DESC(InstrumentID)
		,TYPE_DESC(Direction),TYPE_DESC(OffsetFlag),TYPE_DESC(TradeVolume),TYPE_DESC(TradePrice),TYPE_DESC(TradeAmt)
		,TYPE_DESC(TradeDate),TYPE_DESC(TradeTime),TYPE_DESC(TradeID),TYPE_DESC(OrderSysID),TYPE_DESC(UserOrderLocalID),TYPE_DESC(ClientID)
		,TYPE_DESC(OrderOperatorID),TYPE_DESC(OrderLocalID),TYPE_DESC(UsedMargin),TYPE_DESC(UsedFee), TYPE_DESC(SequenceNo)));
};
const DWORD FID_Trade = 0x3027;



//内存表变化
class CMemTableChangeField
{
public:
	//内存表变化类型
	CMemTableChangeTypeType	MemtableChangeType;
	//内存表名
	CMemTableNameType	MemTableName;
	TYPE_DESCRIPTOR((TYPE_DESC(MemtableChangeType),TYPE_DESC(MemTableName)));
};
const DWORD FID_MemTableChange = 0x3073;

//内部登录信息
class CInterLoginInfoField
{
public:
	//IP地址
	CFrontAddrType	ServerAddr;
	//最大本地报单号
	CSequenceNoType	MaxLocalID;
	TYPE_DESCRIPTOR((TYPE_DESC(ServerAddr),TYPE_DESC(MaxLocalID)));
};
const DWORD FID_InterLoginInfo = 0x3096;

//通讯阶段
class CCommPhaseField
{
public:
	//交易日
	CDateType	TradingDay;
	//通讯时段号
	CCommPhaseNoType	CommPhaseNo;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay),TYPE_DESC(CommPhaseNo)));
};
const DWORD FID_CommPhase = 0x3020;

//生命周期号
class CLifeCycleIDField
{
public:
	//生命周期号
	CSequenceNoType	LifeCycleID;
	TYPE_DESCRIPTOR((TYPE_DESC(LifeCycleID)));
};
const DWORD FID_LifeCycleID = 0x3063;

//交易日信息
class CTradingDayField
{
public:
	//交易日
	CTradingDayType	TradingDay;
	TYPE_DESCRIPTOR((TYPE_DESC(TradingDay)));
};
const DWORD FID_TradingDay = 0x3061;


///报单查询
class CQryOrderField
{
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///投资者编号
	CInvestorIDType	InvestorID;
	///交易所代码
	CExchangeIDType	ExchangeID;
	///报单编号
	COrderSysIDType	OrderSysID;
	///合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(OrderSysID), TYPE_DESC(InstrumentID)));


};
const DWORD FID_QryInvestorOrder = 0x3063;

///成交查询
class CQryTradeField
{
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///投资者编号
	CInvestorIDType	InvestorID;
	///交易所代码
	CExchangeIDType	ExchangeID;
	///报单编号
	COrderSysIDType	OrderSysID;
	///合约代码
	CInstrumentIDType	InstrumentID;
	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(OrderSysID), TYPE_DESC(InstrumentID)));
};

const DWORD FID_QryInvestorTrade = 0x3064;

///OP资金出入金
class COpInvestorBalanceField {
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///操作员编号
	COperatorIDType	OperatorID;
	///投资者编号
	CInvestorIDType	InvestorID;
	//入金
	CMoneyType Deposit;
	//出金 
	CMoneyType Withdraw;

	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(OperatorID), TYPE_DESC(InvestorID), TYPE_DESC(Deposit), TYPE_DESC(Withdraw)));
};

const DWORD FID_OpInvestorBalance = 0x3065;


///现货持仓出入金
class CSpotPositionNotifyField {
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///投资者编号
	CInvestorIDType	InvestorID;
	///交易所代码
	CExchangeIDType	ExchangeID;
	///现货合约
	CInstrumentIDType	InstrumentID;
	//开平方向
	COffsetFlagType OffsetFlag;
	//量
	CVolumeType Volume;
	//序列号
	CSequenceNoType SequenceNo;
	// Time
	CTimeType NotifyTime;


	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(InstrumentID),
		TYPE_DESC(OffsetFlag), TYPE_DESC(Volume), TYPE_DESC(SequenceNo), TYPE_DESC(NotifyTime)));
};

const DWORD FID_SpotPositionNotify = 0x3066;

///查询持仓变化
class CQryPositionChangeField {
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///投资者编号
	CInvestorIDType	InvestorID;
	///交易所代码
	CExchangeIDType	ExchangeID;
	///合约代码(*表示查询所有合约
	CInstrumentIDType InstrumentID;
	//产品类型(ZTP_PCT_Unknown 0 表示所有)
	CProductClassType	ProductClass;

	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(InstrumentID), TYPE_DESC(ProductClass)));
};

const DWORD FID_QryInvestorPositionChange = 0x3067;

///持仓变化
class CPositionChangeField {
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///投资者编号
	CInvestorIDType	InvestorID;
	///交易所代码
	CExchangeIDType	ExchangeID;
	///交易日
	CTradingDayType	TradingDay;
	///合约代码
	CInstrumentIDType InstrumentID;
	//产品类型
	CProductClassType	ProductClass;
	//买卖方向
	CDirectionType	Direction;
	//开平标志
	COffsetFlagType	OffsetFlag;
	//变化类型
	CPositionChangeType ChangeType;
	//变化量
	CVolumeType Volume;
	//本地报单编号
	COrderLocalIDType OrderLocalID;
	//成交编号
	CTraderIDType TradeID;
	//报单编号
	COrderSysIDType	OrderSysID;
	//客户报单编号
	CUserOrderLocalIDType UserOrderLocalID;
	//序列号
	CSequenceNoType ExchangeSequenceNo;
	//序列号
	CSequenceNoType PositionSequenceNo;

	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(ExchangeID), TYPE_DESC(TradingDay),
		TYPE_DESC(InstrumentID), TYPE_DESC(ProductClass), TYPE_DESC(Direction), TYPE_DESC(OffsetFlag), 
		TYPE_DESC(ChangeType), TYPE_DESC(Volume),  TYPE_DESC(OrderLocalID), TYPE_DESC(TradeID),
		TYPE_DESC(OrderSysID), TYPE_DESC(UserOrderLocalID), TYPE_DESC(ExchangeSequenceNo), TYPE_DESC(PositionSequenceNo)));
};

const DWORD FID_InvestorPositionChange = 0x3068;


///资金变化
class CBalanceChangeField {
public:
	///经纪公司编号
	CBrokerIDType	BrokerID;
	///投资者编号
	CInvestorIDType	InvestorID;
	//交易日
	CTradingDayType TradingDay;
	//资金变化
	CBalanceChangeType BalanceChangeType;
	//金额
	CMoneyType Amount;
	//序列号
	CSequenceNoType ExchangeSequenceNo;
	//序列号
	CSequenceNoType BalanceSequenceNo;

	TYPE_DESCRIPTOR((TYPE_DESC(BrokerID), TYPE_DESC(InvestorID), TYPE_DESC(TradingDay), TYPE_DESC(BalanceChangeType), TYPE_DESC(Amount),
		TYPE_DESC(ExchangeSequenceNo), TYPE_DESC(BalanceSequenceNo)));
};

const DWORD FID_InvestorBalanceChange = 0x3069;

*/