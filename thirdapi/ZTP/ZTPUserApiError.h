#pragma once
/*
以下为API可能会返回的错误码，JinChan系统的错误码小于10000
*/

#define API_ERROR_ERROR									-1			//系统错误
#define API_ERROR_NONE									0			//成功
#define API_ERROR_SESSION_NOT_FOUND						5			//会话找不到
#define API_ERROR_DUPLICATE_ORDER						13			//重复的报单
#define API_ERROR_ORDER_NOT_FOUND						24			//报单找不到
#define API_ERROR_NOT_ENOUGH_CLIENT_POSITION_TO_CLOSE	31			//平仓时客户持仓不足
#define API_ERROR_INVALID_VOLUME						36			//不合法的数量
#define API_ERROR_INSUFFICIENT_MONEY					37			//可用资金不足
#define API_ERROR_INVALID_SYSTEM_STATUS					44			//当前系统状态不可进行此操作
#define API_ERROR_BROKERID_MISMATCH						59			//BrokerID不匹配
#define API_ERROR_INVALID_INVESTOR						63			//无效投资者
#define API_ERROR_INVALID_PRODUCTCLASS					65			//产品类型错误
#define API_ERROR_NOT_LOGIN_YET							68			//用户未登录
#define API_ERROR_INVALID_LOGIN							143			//不合法的登录
#define API_ERROR_INVALID_REQUEST						999			//不支持的请求
#define API_ERROR_FIELDERROR_OFFSETFLAG					8008		//报单字段检查OFFSETFLAG错误
#define API_ERROR_BROKERID_CANNOT_BE_BLANK				8104		//BrokerID不能为空
#define API_ERROR_OPERATORID_CANNOT_BE_BLANK			8105		//操作员号不能为空
#define API_ERROR_BAD_INSTRUMENT_FIELD					8125		//合约信息有误
#define API_ERROR_ORDERACTION_LOCALID_SYSID_NOTMATCH	8127		//撤单字段本地委托号与系统号不匹配
#define API_ERROR_NO_CLIENT_MARGIN_FEE_DATA				8136		//找不到客户费率信息

/*
超过10000的错误是由SGE返回的
*/