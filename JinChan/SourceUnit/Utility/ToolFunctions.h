#pragma once

// #include "errorDefine.h"
// #include "CMemorySequence.h"
#include "UserDataType.h"


///所有的管理员功能定义
// #define FuncLead							"1010"
// #define FuncOrderInsert						FuncLead "0001"
// #define FuncOrderAction						FuncLead "0002"
// #define FuncQuoteInsert						FuncLead "0003"
// #define FuncQuoteAction						FuncLead "0004"
// #define FuncOTCOrderInsert					FuncLead "0005"
// #define FuncExecOrderInsert					FuncLead "0006"
// #define FuncExecOrderAction					FuncLead "0007"
// #define FuncAdminOrderInsert				FuncLead "0008"
// #define FuncBulletin						FuncLead "0009"
// #define FuncInstrumentStatusUpdate			FuncLead "0010"
// #define FuncForceUserExit					FuncLead "0011"
// #define FuncForceUserLogout					FuncLead "0012"
// #define FuncExchangeDataSyncStatusUpdate	FuncLead "0013"
// #define FuncSGDataSyncStatusUpdate			FuncLead "0014"
// #define FuncInformation						FuncLead "0015"

// /缺省的会员号
// extern const CParticipantIDType defaultParticipantID;
// 
// /缺省的会员号
// extern const CParticipantIDType defaultClearingParticipantID;
// 
// /缺省的客户号
// extern const CClientIDType defaultClientID;
// 
// /缺省的合约号
// extern const CInstrumentIDType defaultInstrumentID;
// 
// /缺省的产品号
// extern const CProductIDType defaultProductID;

// 

// /缺省的投机套保标志

// extern const CHedgeFlagType defaultHedgeFlag;
// 
// /缺省的用户编码
// extern const CUserIDType defaultUserID;
// 

//交易系统名称
extern CTradingSystemNameType tradingSystemName;
//获取当前时间
CReadOnlyTimeType GetLocalTime();
// 获取今天的交易日期
// CReadOnlyDateType GetTradingDay(CMemoryDB* pDB);
// 获取交易所数据同步状态
// CReadOnlySystemStatusType GetSystemStatus(CMemoryDB* pDB);

// /获取上次时间
// /@param	pDB	指定的内存数据库
// /@return	上次时间
// CReadOnlyTimeType getOldTime(CMemoryDB *pDB);
// 
//获取当前的精确时间
// void getCurrentTime(CMemoryDB* pDB, CTimeType* pTime, CMillisecType* pMillisec);

// /设置当前时间
// /@param	pDB	指定的内存数据库
// /@return	当前时间
// void setCurrentTime(CMemoryDB *pDB, CWriteableCurrentTime *pTime, CTransaction *pTransaction);
// 
// /得到指定时间中的秒钟数
// /@param	theTime	指定时间
// /@return	秒钟数
// int getSecond(CReadOnlyTimeType &theTime);
// 
// /得到时间表示的在今天中的秒钟数，即该时间是今天的第几秒
// /@param	theTime	指定时间
// /@return	今天的秒钟数
// int getSecondOfDay(CReadOnlyTimeType &theTime);
// 
// /获取指定合约的交易状态
// /@param	pInstrument	指定的合约
// /@return	该合约的交易状态
// CInstrumentStatusType getInstrumentStatus(CInstrument *pInstrument);
// 


// /获取相反的买卖方向
// /@param	direction	原买卖方向
// /@return	相反的买卖方向
// CDirectionType getReverseDirection(CReadOnlyDirectionType direction);
// 
// /设置合约是否可以交易的标志
// /@param	pOrder	要设置标志的合约
// void setTradable(CWriteableOrder *pOrder);
// 
// /判断止损单是否应当被激活
// /@param	pOrder	指定的止损单
// /@param	lastPrice	最新价
// /@return	true表示应当激活，false表示不应当激活
// bool needActiveStopOrder(COrder *pOrder, CReadOnlyPriceType &lastPrice);
// 
// /设置止损单状态
// /@param	pOrder	指定的止损单
// /@param	pMarketData	对应的行情
// void setStopOrderStatus(CWriteableOrder *pOrder, CMarketData *pMarketData);
// 
// /设置报单的剩余数量和状态
// /@param	pOrder	指定的报单
// void setOrderStatus(CWriteableOrder *pOrder);
// 
// 
// /获取某个合约的前结算价
// /@param	pInstrument	指定的合约
// CReadOnlyPriceType &getPreSettlementPrice(CInstrument *pInstrument);
// 
// /设置交易系统名称
// /@param	pConfig	系统配置
// void setTradingSystemName(CConfig *pConfig);
