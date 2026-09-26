#pragma once
#include "BaseDataType.h"
//系统内部用户数据类型

/////////////////////////////////////////////////////////////////////////
///CMoneyType是一个资金类
/////////////////////////////////////////////////////////////////////////
typedef CDoubleType<16,4> CMoneyType;
typedef const CDoubleType<16,4> CReadOnlyMoneyType;
/////////////////////////////////////////////////////////////////////////
///CPriceType是一个价格类
/////////////////////////////////////////////////////////////////////////
typedef CDoubleType<12,4> CPriceType;
typedef const CDoubleType<12,4> CReadOnlyPriceType;
/////////////////////////////////////////////////////////////////////////
///CRatioType是一个比率类
/////////////////////////////////////////////////////////////////////////
typedef CDoubleType<12,8> CRatioType;
typedef const CDoubleType<12,8> CReadOnlyRatioType;
/////////////////////////////////////////////////////////////////////////
///CPriceTickType是一个最小变动价位类
/////////////////////////////////////////////////////////////////////////
typedef CDoubleType<12,4> CPriceTickType;
typedef const CDoubleType<12,4> CReadOnlyPriceTickType;

/////////////////////////////////////////////////////////////////////////
///CLargeVolumeType是一个大额数量类(持仓、成交)
/////////////////////////////////////////////////////////////////////////
typedef CLongType CLargeVolumeType;
typedef CReadOnlyLongType CReadOnlyLargeVolumeType;

/////////////////////////////////////////////////////////////////////////
///CVolumeType是一个数量类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CVolumeType;
typedef CReadOnlyIntType CReadOnlyVolumeType;
/////////////////////////////////////////////////////////////////////////
///CVolumeMultipleType是一个乘数类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CVolumeMultipleType;
typedef CReadOnlyIntType CReadOnlyVolumeMultipleType;
/////////////////////////////////////////////////////////////////////////
///CSequenceSeriesType是一个序列的系列号类(流类型)
/////////////////////////////////////////////////////////////////////////
typedef CIntType CSequenceSeriesType;
typedef CReadOnlyIntType CReadOnlySequenceSeriesType;
/////////////////////////////////////////////////////////////////////////
///CSequenceNoType是一个序列号类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CSequenceNoType;
typedef CReadOnlyIntType CReadOnlySequenceNoType;
/////////////////////////////////////////////////////////////////////////
///CErrorIDType是一个错误代码类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CErrorIDType;
typedef CReadOnlyIntType CReadOnlyErrorIDType;
/////////////////////////////////////////////////////////////////////////
///CMillisecType是一个最后修改毫秒类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CMillisecType;
typedef CReadOnlyIntType CReadOnlyMillisecType;
/////////////////////////////////////////////////////////////////////////
///CCommFluxType是一个通讯数量类(Session个数)
/////////////////////////////////////////////////////////////////////////
typedef CIntType CCommFluxType;
typedef CReadOnlyIntType CReadOnlyCommFluxType;
/////////////////////////////////////////////////////////////////////////
///CIntType是一个通讯时段号类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CCommPhaseNoType;
typedef CReadOnlyIntType CReadOnlyCommPhaseNoType;
/////////////////////////////////////////////////////////////////////////
///CSessionIDType是一个会话编号类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CSessionIDType;
typedef CReadOnlyIntType CReadOnlySessionIDType;
/////////////////////////////////////////////////////////////////////////
///CRequestIDType是一个报单编号类
/////////////////////////////////////////////////////////////////////////
typedef CIntType CRequestIDType;
typedef CReadOnlyIntType CReadOnlyRequestIDType;


/////////////////////////////////////////////////////////////////////////
///CTimeType是一个时间类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<8> CTimeType;
typedef const CStringType<8> CReadOnlyTimeType;
/////////////////////////////////////////////////////////////////////////
///CDateType是一个日期类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<8> CDateType;
typedef const CStringType<8> CReadOnlyDateType;
/////////////////////////////////////////////////////////////////////////
///CTradingDayType是一个交易日类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<8> CTradingDayType;
typedef const CStringType<8> CReadOnlyTradingDayType;

/////////////////////////////////////////////////////////////////////////
///CMsgTypeType是一个消息类型
/////////////////////////////////////////////////////////////////////////
typedef CStringType<4> CMsgTypeType;
typedef const CStringType<4> CReadOnlyCMsgTypeTypeType;

/////////////////////////////////////////////////////////////////////////
///COrderLocalIDType是一个本地报单编号类
/////////////////////////////////////////////////////////////////////////
typedef CIntType COrderLocalIDType;
typedef const CIntType CReadOnlyOrderLocalIDType;
/////////////////////////////////////////////////////////////////////////
///CUserOrderLocalIDType是一个用户本地报单号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<15> CUserOrderLocalIDType;
typedef const CStringType<15> CReadOnlyUserOrderLocalIDType;
/////////////////////////////////////////////////////////////////////////
///COrderSysIDType是一个交易所报单编号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<20> COrderSysIDType;
typedef const CStringType<20> CReadOnlyOrderSysIDType;
/////////////////////////////////////////////////////////////////////////
///CTradeIDType是一个交易所成交编号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<20> CTraderIDType;
typedef const CStringType<20> CReadOnlyTradeIDType;

/////////////////////////////////////////////////////////////////////////
///CIPAddressType是一个IP地址类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<20> CIPAddressType;
typedef const CStringType<20> CReadOnlyIPAddressType;
/////////////////////////////////////////////////////////////////////////
///CMacAddressType是一个Mac地址类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<20> CMacAddressType;
typedef const CStringType<20> CReadOnlyMacAddressType;
/////////////////////////////////////////////////////////////////////////
///CFrontAddrType是一个连接地址类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<32> CFrontAddrType;
typedef const CStringType<32> CReadOnlyFrontAddrType;

/////////////////////////////////////////////////////////////////////////
///CAppNameType是一个App名称
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CAppNameType;
typedef const CStringType<30> CReadOnlyAppNameType;

/////////////////////////////////////////////////////////////////////////
///CMemTableNameType是一个内存表名类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<60> CMemTableNameType;
typedef const CStringType<60> CReadOnlyMemTableNameType;
/////////////////////////////////////////////////////////////////////////
///CProductIDType是一个品种编号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CProductIDType;
typedef const CStringType<10> CReadOnlyProductIDType;
/////////////////////////////////////////////////////////////////////////
///CProductNameType是一个品种名称类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CProductNameType;
typedef const CStringType<30> CReadOnlyProductNameType;
/////////////////////////////////////////////////////////////////////////
///CInstrumentIDType是一个合约编号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CInstrumentIDType;
typedef const CStringType<10> CReadOnlyInstrumentIDType;
/////////////////////////////////////////////////////////////////////////
///CInstrumentNameType是一个合约名称类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CInstrumentNameType;
typedef const CStringType<30> CReadOnlyInstrumentNameType;

/////////////////////////////////////////////////////////////////////////
///CExchangeIDType是一个交易(所)通道类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CExchangeIDType;
typedef const CStringType<10> CReadOnlyExchangeIDType;
/////////////////////////////////////////////////////////////////////////
///CExchangeNameType是一个交易(所)所名称类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CExchangeNameType;
typedef const CStringType<30> CReadOnlyExchangeNameType;

/////////////////////////////////////////////////////////////////////////
///CParticipantIDType是一个会员编号类（对交易所）
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CParticipantIDType;
typedef const CStringType<10> CReadOnlyParticipantIDType;
/////////////////////////////////////////////////////////////////////////
///CBrokerIDType是一个经纪公司代码类（对客户）
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CBrokerIDType;
typedef const CStringType<10> CReadOnlyBrokerIDType;
/////////////////////////////////////////////////////////////////////////
///CSeatIDType是一个席位号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CSeatIDType;
typedef const CStringType<10> CReadOnlySeatIDType;

/////////////////////////////////////////////////////////////////////////
///CReadOnlyUserIDType是一个操作员（交易员）号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> COperatorIDType;
typedef const CStringType<10> CReadOnlyOperatorIDType;

/////////////////////////////////////////////////////////////////////////
///CReadOnlyUserIDType是一个操作员（交易员）号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CLoginIDType;
typedef const CStringType<10> CReadOnlyLoginIDType;

/////////////////////////////////////////////////////////////////////////
///CUserNameType是一个操作员名称类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CUserNameType;
typedef const CStringType<30> CReadOnlyUserNameType;
/////////////////////////////////////////////////////////////////////////
///CInvestorIDType是一个投资者编号类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<10> CInvestorIDType;
typedef const CStringType<10> CReadOnlyInvestorIDType;
/////////////////////////////////////////////////////////////////////////
///CInvestorNameType是一个投资者名称类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CInvestorNameType;
typedef const CStringType<30> CReadOnlyInvestorNameType;
/////////////////////////////////////////////////////////////////////////
///CClientIDType是一个交易编码类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<12> CClientIDType;
typedef const CStringType<12> CReadOnlyClientIDType;

/////////////////////////////////////////////////////////////////////////
///CPasswordType是一个密码类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<15> CPasswordType;
typedef const CStringType<15> CReadOnlyPasswordType;
/////////////////////////////////////////////////////////////////////////
///CProductInfoType是一个产品信息类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<20> CProductInfoType;
typedef const CStringType<20> CReadOnlyProductInfoType;
/////////////////////////////////////////////////////////////////////////
///CTradingSystemNameType是一个交易系统名称类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<30> CTradingSystemNameType;
typedef const CStringType<30> CReadOnlyTradingSystemNameType;

/////////////////////////////////////////////////////////////////////////
///CCustomType是一个用户自定义域类型类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<20> CCustomType;
typedef const CStringType<20> CReadOnlyCustomType;
/////////////////////////////////////////////////////////////////////////
///CErrorMsgType是一个错误信息类
/////////////////////////////////////////////////////////////////////////
typedef CStringType<120> CErrorMsgType;
typedef const CStringType<120> CReadOnlyErrorMsgType;


//const char* ProductInfo = "JinChan 1.2.1";


/////////////////////////////////////////////////////////////////////////
///CUserTypeType是一个登录用户类型
/////////////////////////////////////////////////////////////////////////
//客户
#define UT_Person '1'
//管理员
#define UT_Manager '2'
class CUserTypeType : public CCharType					
{														
public:													
	CUserTypeType(void)	{}													
	CUserTypeType(CUserTypeType const& v)	{ setValue(v.getValue()); }													
	CUserTypeType(char const v)	{ setValue(v); }													
	CUserTypeType(CCharType const& v)	{ setValue(v.getValue()); }													
	CUserTypeType& operator=(CUserTypeType const& c)	
	{
		setValue(c.getValue());							
		return *this;									
	}													
	CUserTypeType& operator=(CCharType const& c)		
	{			
		setValue(c.getValue());							
		return *this;									
	}													
	char const operator =(char const c)					
	{					
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}													
	void clear(void)									
	{													
		setValue("12"[0]);			
	}													
	void setValue(char const v)	{ value = v; } 													
	bool isNull(void) const	{ return value==EnumNull; }													
	bool isValid(void) const
	{													
		if (value=='\0')								
			return false;								
		if (strchr("12",value))					
			return true;								
		else											
			return false;								
	} 													
	static bool isValid(char const ch)					
	{													
		if (strchr("12",ch))						
			return true;								
		else											
			return false;								
	}													
};														
typedef const CUserTypeType CReadOnlyUserTypeType;		

/////////////////////////////////////////////////////////////////////////
///CSystemStatusType是一个系统状态类
/////////////////////////////////////////////////////////////////////////
//起始状态
#define ZTP_SS_Start		'0'
//导入初始数据
#define ZTP_SS_DBLoading	'1'
//同步交易所数据
#define ZTP_SS_ExchSyncing	'2'
//系统已经准备好
#define ZTP_SS_Ready		'3'
//交易结束
//#define ZTP_SS_NoTrading	'4'
class CSystemStatusType : public CCharType					
{														
public:													
	CSystemStatusType(void)	{ }													
	CSystemStatusType(CSystemStatusType const& v)	{ setValue(v.getValue()); }													
	CSystemStatusType(char const v)	{ setValue(v); }													
	CSystemStatusType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c) {													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v) {													
		setValue(*v);									
		return v;										
	}	
	CSystemStatusType& operator=(const CSystemStatusType &c) {													
		setValue(c.getValue());							
		return *this;									
	}													
	CSystemStatusType& operator=(const CCharType &c) {													
		setValue(c.getValue());							
		return *this;									
	}																								
	void clear(void) {													
		setValue("012345"[0]);						
	}													
	void setValue(const char v) {													
		value = v;										
	} 													
	bool isNull(void) const {													
		return value==EnumNull;							
	}													
	bool isValid(void) const {													
		if (value=='\0')								
			return false;								
		if (strchr("012345",value))					
			return true;								
		else											
			return false;								
	} 																									
};														
typedef const CSystemStatusType CReadOnlySystemStatusType;

/////////////////////////////////////////////////////////////////////////
///CSeatStatusType是一个席位状态类
/////////////////////////////////////////////////////////////////////////
//不可用
#define ZTP_TS_NotTrading '0'
//已登录
#define ZTP_TS_Trading '1'
class CTradingStatusType : public CCharType {
public:
	CTradingStatusType(void) {}
	CTradingStatusType(CTradingStatusType const& v) { setValue(v.getValue()); }
	CTradingStatusType(char const v) { setValue(v); }
	CTradingStatusType(CCharType const& v) { setValue(v.getValue()); }
	char const operator =(char const c) {
		setValue(c);
		return c;
	}
	char const* operator =(char const* v) {
		setValue(*v);
		return v;
	}
	CTradingStatusType& operator=(CTradingStatusType const& c) {
		setValue(c.getValue());
		return *this;
	}
	CTradingStatusType& operator=(CCharType const& c) {
		setValue(c.getValue());
		return *this;
	}
	void clear(void) {
		setValue("012"[0]);
	}
	void setValue(const char v) {
		value = v;
	}
	bool isNull(void) const {
		return value == EnumNull;
	}
	bool isValid(void) const {
		if (value == '\0')
			return false;
		if (strchr("012", value))
			return true;
		else
			return false;
	}
	static bool isValid(const char ch) {
		if (strchr("012", ch))
			return true;
		else
			return false;
	}
};
typedef const CTradingStatusType CReadOnlyTradeingStatusType;

/////////////////////////////////////////////////////////////////////////
///CSeatStatusType是一个席位状态类
/////////////////////////////////////////////////////////////////////////
//不可用
#define ZTP_SS_InActive '0'
//已登录
#define ZTP_SS_Login '1'
//已登出
#define ZTP_SS_Logout '2'
class CSeatStatusType : public CCharType					
{														
public:													
	CSeatStatusType(void)	{ }													
	CSeatStatusType(CSeatStatusType const& v)	{ setValue(v.getValue()); }													
	CSeatStatusType(char const v)	{ setValue(v); }													
	CSeatStatusType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}
	CSeatStatusType& operator=(CSeatStatusType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CSeatStatusType& operator=(CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																								
	void clear(void)									
	{													
		setValue("012"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("012",value))					
			return true;								
		else											
			return false;								
	} 													
	static bool isValid(const char ch)					
	{													
		if (strchr("012",ch))						
			return true;								
		else											
			return false;								
	}													
};														
typedef const CSeatStatusType CReadOnlySeatStatusType;	

/////////////////////////////////////////////////////////////
////////////
///CMemTableActionTypeType是一个内存表变化类型类(CScheduler)
/////////////////////////////////////////////////////////////////////////
///插入
#define ZTP_MTA_INSERT '0'
///修改
#define ZTP_MTA_UPDATE '1'
///删除
#define ZTP_MTA_DELETE '2'
///提交
#define ZTP_MTA_COMMIT '3'
class CMemTableChangeTypeType : public CCharType					
{														
public:													
	CMemTableChangeTypeType(void)	{ }													
	CMemTableChangeTypeType(CMemTableChangeTypeType const& v) { setValue(v.getValue()); }													
	CMemTableChangeTypeType(char const v)	{ setValue(v); }													
	CMemTableChangeTypeType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}													
	CMemTableChangeTypeType& operator = (CMemTableChangeTypeType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CMemTableChangeTypeType& operator = (CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	void clear(void)									
	{													
		setValue("0123"[0]);						
	}													
	void setValue(char const v)							
	{													
		value = v;										
	} 													
	bool isNull(void) const								
	{													
		return value == EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("0123",value))					
			return true;								
		else											
			return false;								
	} 													
	static bool isValid(char const ch)					
	{													
		if (strchr("0123",ch))						
			return true;								
		else											
			return false;								
	}													
};														
typedef const CMemTableChangeTypeType CReadOnlyMemTableActionTypeType;	

/////////////////////////////////////////////////////////////////////////
///COrderPriceTypeType是一个报单价格条件类
/////////////////////////////////////////////////////////////////////////
//限价
#define ZTP_OPT_LimitPrice '0'
//FOK
#define ZTP_OPT_FOKPrice '1'
//FAK
#define ZTP_OPT_FAKPrice '2'
class COrderPriceTypeType : public CCharType					
{														
public:													
	COrderPriceTypeType(void)	{ }													
	COrderPriceTypeType(COrderPriceTypeType const& v) { setValue(v.getValue()); }													
	COrderPriceTypeType(char const v)	{ setValue(v); }													
	COrderPriceTypeType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator = (char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator = (char const* v)				
	{													
		setValue(*v);									
		return v;										
	}													
	COrderPriceTypeType& operator = (COrderPriceTypeType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	COrderPriceTypeType& operator = (CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	void clear(void)									
	{													
		setValue("123"[0]);						
	}													
	void setValue(char const v)	{ value = v; } 													
	bool isNull(void) const	{ return value == EnumNull; }													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("123", value))					
			return true;								
		else											
			return false;								
	} 													
};														
typedef const COrderPriceTypeType CReadOnlyOrderPriceTypeType;		

/////////////////////////////////////////////////////////////////////////
///CHedgeFlagType是一个投机套保标志类
/////////////////////////////////////////////////////////////////////////
///投机（延期）
#define ZTP_HF_Speculation '1'
///套利（中立仓）
#define ZTP_HF_Arbitrage '2'
///套保（交割）
#define ZTP_HF_Hedge '3'
///做市商（现货）
#define ZTP_HF_MarketMaker '4'
class CHedgeFlagType : public CCharType					
{														
public:													
	CHedgeFlagType(void)	{ }													
	CHedgeFlagType(CHedgeFlagType const& v)	{ setValue(v.getValue()); }													
	CHedgeFlagType(char const v)	{ setValue(v); }													
	CHedgeFlagType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const *v)				
	{													
		setValue(*v);									
		return v;										
	}													
	CHedgeFlagType& operator=(CHedgeFlagType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CHedgeFlagType& operator=(CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	void clear(void)									
	{													
		setValue("1234"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("1234",value))					
			return true;								
		else											
			return false;								
	} 													
};														
typedef const CHedgeFlagType CReadOnlyHedgeFlagType;		

/////////////////////////////////////////////////////////////////////////
///CTradingRightType是一个交易权限类
/////////////////////////////////////////////////////////////////////////
///可以交易
#define ZTP_TR_Allow '0'
///只能平仓
#define ZTP_TR_CloseOnly '1'
///不能交易
#define ZTP_TR_Forbidden '2'
class CTradeRightType : public CCharType					
{														
public:													
	CTradeRightType(void)	{ }													
	CTradeRightType(CTradeRightType const& v)	{ setValue(v.getValue()); }													
	CTradeRightType(char const v)	{ setValue(v); }													
	CTradeRightType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}													
	CTradeRightType& operator=(const CTradeRightType &c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CTradeRightType& operator=(const CCharType &c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	void clear(void)									
	{													
		setValue("012"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("012",value))					
			return true;								
		else											
			return false;								
	} 																									
};														
typedef const CTradeRightType CReadOnlyTradeRightType;	

////////////////////////////////////////////////////////////////////////
//CPersmissionRightType是一个功能权限类
////////////////////////////////////////////////////////////////////////
///只有行情
#define ZTP_PR_MARKET '0'
///完整交易
#define ZTP_PR_TRADE '1'
class CPermissionRightType : public CCharType					
{														
public:													
	CPermissionRightType(void)	{ }													
	CPermissionRightType(CPermissionRightType const& v)	{ setValue(v.getValue()); }													
	CPermissionRightType(char const v)	{ setValue(v); }													
	CPermissionRightType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}	
	CPermissionRightType& operator=(CPermissionRightType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CPermissionRightType& operator=(CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																									
	void clear(void)									
	{													
		setValue("01"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("01",value))					
			return true;								
		else											
			return false;								
	} 													
};														
typedef const CPermissionRightType CReadOnlyPermissionRightType;	

/////////////////////////////////////////////////////////////////////////
///CStringType是一个授权功能号类0000行情	0001交易	000
/////////////////////////////////////////////////////////////////////////
// typedef CUFStringType<4> CGrantFuncSetType;
// typedef const CUFStringType<4> CReadOnlyGrantFuncSetType;

/////////////////////////////////////////////////////////////////////////
///type::InstrumentStatus是一个合约交易状态类
/////////////////////////////////////////////////////////////////////////
//初始化
#define ZTP_IS_Initting '0'
//初始化完成
#define ZTP_IS_Inited '1'
//开盘
#define ZTP_IS_OpenMarket '2'
//集合竞价报单
#define ZTP_IS_AuctionOrdering '3'
//集合竞价撮合
#define ZTP_IS_AuctionMatch '4'
//连续交易
#define ZTP_IS_Continous '5'
//暂停交易
#define ZTP_IS_Pause	'6'
//交割申报
#define ZTP_IS_DeryApp	'7'
//交割申报结束
#define ZTP_IS_DeryMatch	'8'
//中立仓申报
#define ZTP_IS_MidApp	'9'
//中立仓平衡
#define ZTP_IS_MidMatch	'A'
//收盘
#define ZTP_IS_Closed 'B'
class CInstrumentStatusType : public CCharType					
{														
public:													
	CInstrumentStatusType(void)	{ }
	CInstrumentStatusType(CInstrumentStatusType const& v)	{ setValue(v.getValue()); }
	CInstrumentStatusType(char const v)	{ setValue(v); }
	CInstrumentStatusType(CCharType const& v)	{ setValue(v.getValue()); }
	char const operator = (char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator = (char const* v)				
	{													
		setValue(*v);									
		return v;										
	}	
	CInstrumentStatusType& operator = (CInstrumentStatusType const& c)
	{					
		setValue(c.getValue());							
		return *this;									
	}													
	CInstrumentStatusType& operator = (CCharType const& c)
	{													
		setValue(c.getValue());							
		return *this;									
	}													
												
	void clear(void)	{ setValue("01234567DEFG"[0]); }													
	void setValue(const char v)	{ value = v; } 													
	bool isNull(void) const	{ return value == EnumNull; }													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("01234567DEFG", value))					
			return true;								
		else											
			return false;								
	} 																										
};														

/////////////////////////////////////////////////////////////////////////
///CDirectionType是一个买卖方向类
/////////////////////////////////////////////////////////////////////////
//买
#define ZTP_D_Buy '0'
//卖
#define ZTP_D_Sell '1'
class CDirectionType : public CCharType					
{														
public:													
	CDirectionType(void)	{ }													
	CDirectionType(CDirectionType const& v)	{ setValue(v.getValue()); }													
	CDirectionType(char const v)	{ setValue(v); }													
	CDirectionType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}	
	CDirectionType& operator=(const CDirectionType &c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CDirectionType& operator=(const CCharType &c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																									
	void clear(void)									
	{													
		setValue("01"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("01",value))					
			return true;								
		else											
			return false;								
	} 													
};														
typedef const CDirectionType CReadOnlyDirectionType;		

/////////////////////////////////////////////////////////////////////////
///CDeferFeeDirectionType是一个递延费方向类
/////////////////////////////////////////////////////////////////////////
//Unknown
#define ZTP_DFD_Unknown '0'
//多付空
#define ZTP_DFD_LongPay '1'
//空付多
#define ZTP_DFD_ShortPay '2'
//平
#define ZTP_DFD_EQUAL '3'

class CDeferFeeDirectionType : public CCharType {
public:
	CDeferFeeDirectionType(void) {}
	CDeferFeeDirectionType(CDeferFeeDirectionType const& v) { setValue(v.getValue()); }
	CDeferFeeDirectionType(char const v) { setValue(v); }
	CDeferFeeDirectionType(CCharType const& v) { setValue(v.getValue()); }
	char const operator =(char const c) {
		setValue(c);
		return c;
	}
	char const* operator =(char const* v) {
		setValue(*v);
		return v;
	}
	CDeferFeeDirectionType& operator=(const CDeferFeeDirectionType& c) {
		setValue(c.getValue());
		return *this;
	}
	CDeferFeeDirectionType& operator=(const CCharType& c) {
		setValue(c.getValue());
		return *this;
	}
	void clear(void) {
		setValue("01"[0]);
	}
	void setValue(const char v) {
		value = v;
	}
	bool isNull(void) const {
		return value == EnumNull;
	}
	bool isValid(void) const {
		if (value == '\0')
			return false;
		if (strchr("01", value))
			return true;
		else
			return false;
	}
};
typedef const CDeferFeeDirectionType CReadOnlyDeferFeeDirectionType;


/////////////////////////////////////////////////////////////////////////
///COffsetFlagType是一个开平标志类
/////////////////////////////////////////////////////////////////////////
//开仓
#define ZTP_OF_Open '0'
//平仓
#define ZTP_OF_Close '1'
//平今
#define ZTP_OF_CloseToday '2'
//强平
#define ZTP_OF_ForceClose '3'
//平昨
#define ZTP_OF_CloseYesterday '4'

//未知
#define ZTP_OF_Unknown 'X'
class COffsetFlagType : public CCharType
{														
public:													
	COffsetFlagType(void)	{ }													
	COffsetFlagType(COffsetFlagType const& v)	{ setValue(v.getValue()); }													
	COffsetFlagType(char const v)	{ setValue(v); }													
	COffsetFlagType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator = (char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	const char * operator = (char const* v)				
	{													
		setValue(*v);									
		return v;										
	}	
	COffsetFlagType& operator = (COffsetFlagType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	COffsetFlagType& operator = (CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																									
	void clear(void)									
	{													
		setValue("01234"[0]);						
	}													
	void setValue(char const v)							
	{													
		value = v;										
	} 													
	bool isNull(void) const	{ return value == EnumNull; }													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("01234", value))					
			return true;								
		else											
			return false;								
	} 																							
};														
typedef const COffsetFlagType CReadOnlyOffsetFlagType;		

/////////////////////////////////////////////////////////////////////////
///CActionFlagType是一个操作标志类
/////////////////////////////////////////////////////////////////////////
//删除
#define ZTP_AF_Delete '0'
//挂起
#define ZTP_AF_Suspend '1'
//激活
#define ZTP_AF_Active '2'
//修改
#define ZTP_AF_Modify '3'
class CActionFlagType : public CCharType					
{														
public:													
	CActionFlagType(void)									
	{													
	}													
	CActionFlagType(CActionFlagType const& v)	{ setValue(v.getValue()); }													
	CActionFlagType(char const v)	{ setValue(v); }													
	CActionFlagType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const * v)				
	{													
		setValue(*v);									
		return v;										
	}	
	CActionFlagType& operator=(CActionFlagType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CActionFlagType& operator=(CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																									
	void clear(void)									
	{													
		setValue("0123"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("0123",value))					
			return true;								
		else											
			return false;								
	} 																										
};														
typedef const CActionFlagType CReadOnlyActionFlagType;	

/////////////////////////////////////////////////////////////////////////
///COrderStatusType是一个报单状态类
/////////////////////////////////////////////////////////////////////////
#define ZTP_OS_New	'n'
#define ZTP_OS_InsertQueueing 'a'			//订单已提交，未有系统号
#define ZTP_OS_NoTradeQueueing '3'			//交易所成功，返回了系统号
#define ZTP_OS_InsertFail 'f'				//订单失败
#define ZTP_OS_AllTraded '0'				//全部成交
#define ZTP_OS_PartTradedQueueing '1'		//部分成交还在队列中
#define ZTP_OS_CancelQueueing 'q'			//撤单已提交，未有应答
#define ZTP_OS_CancelFail 'c'				//取消失败
#define ZTP_OS_Canceled '5'					//已取消
#define ZTP_OS_NoTradeNotQueueing '4'		//未使用


///撤单
///无效报单
///新报单
class COrderStatusType : public CCharType
{														
public:													
	COrderStatusType(void)	{ }													
	COrderStatusType(COrderStatusType const& v)	{ setValue(v.getValue()); }													
	COrderStatusType(char const v)	{ setValue(v); }													
	COrderStatusType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator = (const char c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator = (char const* v)				
	{													
		setValue(*v);									
		return v;										
	}
	COrderStatusType& operator = (COrderStatusType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	COrderStatusType& operator = (CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																										
	void clear(void)									
	{													
		setValue("012345ab"[0]);						
	}													
	void setValue(char const v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("012345ab",value))					
			return true;								
		else											
			return false;								
	} 													
};														
typedef const COrderStatusType CReadOnlyOrderStatusType;		

/////////////////////////////////////////////////////////////////////////
///CPosiDirectionType是一个持仓多空方向类
/////////////////////////////////////////////////////////////////////////
//净
#define ZTP_PD_Net '0'
//多头
#define ZTP_PD_Long '1'
//空头
#define ZTP_PD_Short '2'
class CPosiDirectionType : public CCharType					
{														
public:													
	CPosiDirectionType(void)	{ }													
	CPosiDirectionType(CPosiDirectionType const& v)	{ setValue(v.getValue()); }													
	CPosiDirectionType(char const v)	{ setValue(v); }													
	CPosiDirectionType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator = (char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator = (char const* v)				
	{													
		setValue(*v);									
		return v;										
	}	
	CPosiDirectionType& operator = (CPosiDirectionType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CPosiDirectionType& operator = (CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																									
	void clear(void)									
	{													
		setValue("012"[0]);						
	}													
	void setValue(char const v)							
	{													
		value = v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("012", value))					
			return true;								
		else											
			return false;								
	} 																									
};														
typedef const CPosiDirectionType CReadOnlyPosiDirectionType;		

/////////////////////////////////////////////////////////////////////////
///CIsActiveType是一个是激活状态类
/////////////////////////////////////////////////////////////////////////
//未激活
#define ZTP_UIA_InActive '0'
//已激活
#define ZTP_UIA_Active '1'
class CIsActiveType : public CCharType					
{														
public:													
	CIsActiveType(void)	{ }													
	CIsActiveType(CIsActiveType const& v)	{ setValue(v.getValue()); }													
	CIsActiveType(char const v)	{ setValue(v); }													
	CIsActiveType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}
	CIsActiveType& operator=(CIsActiveType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CIsActiveType& operator=(CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																										
	void clear(void)									
	{													
		setValue("01"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("01",value))					
			return true;								
		else											
			return false;								
	} 																									
};														
typedef const CIsActiveType CReadOnlyIsActiveType;		

/////////////////////////////////////////////////////////////////////////
///CProductClassType是一个品种类型
/////////////////////////////////////////////////////////////////////////
//未知市场
#define ZTP_PCT_Unknown	'0'
//延期市场
#define ZTP_PCT_Defer	'1'
//现货市场
#define ZTP_PCT_Spot	'2'
//交割申报
#define ZTP_PCT_Delivery '3'
//中立仓
#define ZTP_PCT_Neutral '4'

class CProductClassType : public CCharType					
{														
public:													
	CProductClassType(void)	{ }													
	CProductClassType(CProductClassType const& v)	{ setValue(v.getValue()); }
	CProductClassType(char const v)	{ setValue(v); }													
	CProductClassType(CCharType const& v)	{ setValue(v.getValue()); }													
	char const operator =(char const c)					
	{													
		setValue(c);									
		return c;										
	}													
	char const* operator =(char const* v)				
	{													
		setValue(*v);									
		return v;										
	}	
	CProductClassType& operator=(CProductClassType const& c)	
	{													
		setValue(c.getValue());							
		return *this;									
	}													
	CProductClassType& operator=(CCharType const& c)		
	{													
		setValue(c.getValue());							
		return *this;									
	}																								
	void clear(void)									
	{													
		setValue("012345"[0]);						
	}													
	void setValue(const char v)							
	{													
		value=v;										
	} 													
	bool isNull(void) const								
	{													
		return value==EnumNull;							
	}													
	bool isValid(void) const							
	{													
		if (value=='\0')								
			return false;								
		if (strchr("12345",value))					
			return true;								
		else											
			return false;								
	} 																										
};														
typedef const CProductClassType CReadOnlyProductClassType;	

/////////////////////////////////////////////////////////////////////////
///CPositionChangeType
/////////////////////////////////////////////////////////////////////////
//未知
#define ZTP_PostionChange_Unknown '0'
//成交
#define ZTP_PostionChange_Trade '1'
//现货出入金
#define ZTP_PostionChange_Spot '2'
//初始化昨仓
#define ZTP_PostionChange_Yesterday '3'

class CPositionChangeType : public CCharType {
public:
	CPositionChangeType(void) {
	}
	CPositionChangeType(CPositionChangeType const& v) { setValue(v.getValue()); }
	CPositionChangeType(char const v) { setValue(v); }
	CPositionChangeType(CCharType const& v) { setValue(v.getValue()); }
	char const operator =(char const c) {
		setValue(c);
		return c;
	}
	char const* operator =(char const* v) {
		setValue(*v);
		return v;
	}
	CPositionChangeType& operator=(CPositionChangeType const& c) {
		setValue(c.getValue());
		return *this;
	}
	CPositionChangeType& operator=(CCharType const& c) {
		setValue(c.getValue());
		return *this;
	}
	void clear(void) {
		setValue("0123"[0]);
	}
	void setValue(const char v) {
		value = v;
	}
	bool isNull(void) const {
		return value == EnumNull;
	}
	bool isValid(void) const {
		if (value == '\0')
			return false;
		if (strchr("0123", value))
			return true;
		else
			return false;
	}
};
typedef const CPositionChangeType CReadOnlyPositionChangeType;


/////////////////////////////////////////////////////////////////////////
///CBalanceChangeType
/////////////////////////////////////////////////////////////////////////
//未知
#define ZTP_BalanceChange_Unknown '0'
//入金
#define ZTP_BalanceChange_Deposit '1'
//出金
#define ZTP_BalanceChange_Withdraw '2'

class CBalanceChangeType : public CCharType {
public:
	CBalanceChangeType(void) {
	}
	CBalanceChangeType(CBalanceChangeType const& v) { setValue(v.getValue()); }
	CBalanceChangeType(char const v) { setValue(v); }
	CBalanceChangeType(CCharType const& v) { setValue(v.getValue()); }
	char const operator =(char const c) {
		setValue(c);
		return c;
	}
	char const* operator =(char const* v) {
		setValue(*v);
		return v;
	}
	CBalanceChangeType& operator=(CBalanceChangeType const& c) {
		setValue(c.getValue());
		return *this;
	}
	CBalanceChangeType& operator=(CCharType const& c) {
		setValue(c.getValue());
		return *this;
	}
	void clear(void) {
		setValue("0123"[0]);
	}
	void setValue(const char v) {
		value = v;
	}
	bool isNull(void) const {
		return value == EnumNull;
	}
	bool isValid(void) const {
		if (value == '\0')
			return false;
		if (strchr("0123", value))
			return true;
		else
			return false;
	}
};
typedef const CBalanceChangeType CReadOnlyBalanceChangeType;

template<uint32_t size> 
struct fmt::formatter<typename CStringType<size>> {
	formatter<char[size+1]> char_array_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return char_array_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const CStringType<size>& s, FormatContext& ctx) {
		return char_array_formatter.format(s.getValue(), ctx);
	}
};

template<>
struct fmt::formatter<CIntType> {
	formatter<int> number_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return number_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const CIntType& s, FormatContext& ctx) {
		return number_formatter.format(s.getValue(), ctx);
	}
};

template<>
struct fmt::formatter<CLongType> {
	formatter<long> number_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return number_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const CLongType& s, FormatContext& ctx) {
		return number_formatter.format(s.getValue(), ctx);
	}
};

template<>
struct fmt::formatter<CCharType> {
	formatter<char> number_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return number_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const CCharType& s, FormatContext& ctx) {
		return number_formatter.format(s.getValue(), ctx);
	}
};

template<int l, int p> 
struct fmt::formatter<typename CDoubleType<l,p> > {
	formatter<double> double_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return double_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const CDoubleType<l, p>& s, FormatContext& ctx) {
		return double_formatter.format(s.getValue(), ctx);
	}
};
