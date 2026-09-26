#pragma once

const WORD DIALOG_FLOW = 1;		//对话流：直接发给对应Session
const WORD PUBLIC_FLOW = 2;		//公共流：交易系统主动通知流
const WORD USER_FLOW = 3;		//用户流：返回客户的回报信息
const WORD PRIVATE_FLOW = 4;	//会员流：返回操作员下所有客户的回报信息
const WORD QUERY_FLOW = 5;		//查询流：
const WORD FTD_MAX_SEQUENCE_SERIES = 5;	//最大流订阅数

//FTD协议功能号(API用)
namespace FTD_TID_DEFINE {
	//错误应答
	const DWORD FTD_TID_RspError = 0x00008000;
	/************************对话功能************************/
	//用户登录请求
	const DWORD FTD_TID_ReqUserLogin = 0x00008101;
	//用户登录应答
	const DWORD FTD_TID_RspUserLogin = 0x00008102;
	//用户退出请求
	const DWORD FTD_TID_ReqUserLogout = 0x00008103;
	//用户退出应答
	const DWORD FTD_TID_RspUserLogout = 0x00008104;
	//强制用户录出请求(交易员用，暂不提供)
	const DWORD FID_TID_ReqForceLogout = 0x00008105;
	//强制用户录出应答(交易员用，暂不提供，要同时给交易员和用户应答)
	const DWORD FID_TID_RspForceLogout = 0x00008106;

	//用户密码修改请求
	const DWORD FID_TID_ReqUserPasswordUpdate = 0x00008107;
	//用户密码修改应答
	const DWORD FID_TID_RspUserPasswordUpdate = FID_TID_ReqUserPasswordUpdate + 1;

	//报单录入请求
	const DWORD FTD_TID_ReqOrderInsert = 0x00008201;
	//报单录入应答
	const DWORD FTD_TID_RspOrderInsert = 0x00008202;
	//报单操作请求
	const DWORD FTD_TID_ReqOrderAction = 0x00008203;
	//报单操作应答
	const DWORD FTD_TID_RspOrderAction = 0x00008204;
	//订阅行情请求
	const DWORD FTD_TID_ReqSubMarketData = 0x00008205;
	//订阅行情应答
	const DWORD FTD_TID_RspSubMarketData = 0x00008206;
	//退订行情请求
	const DWORD FTD_TID_ReqUnSubMarketData = 0x00008207;
	//退订行情应答
	const DWORD FTD_TID_RspUnSubMarketData = 0x00008208;
	/************************对话功能************************/

	/************************查询功能************************/
	//合约查询请求
	const DWORD FTD_TID_ReqQryInstrument = 0x00008301;
	//合约查询应答
	const DWORD FTD_TID_RspQryInstrument = 0x00008302;
	//投资者资金账户查询请求
	const DWORD FTD_TID_ReqQryInvestorAccount = 0x00008303;
	//投资者资金账户查询应答
	const DWORD FTD_TID_RspQryInvestorAccount = 0x00008304;
	//投资者持仓查询请求
	const DWORD FTD_TID_ReqQryInvestorPosition = 0x00008305;
	//投资者持仓查询应答
	const DWORD FTD_TID_RspQryInvestorPosition = 0x00008306;
	//投资者保证金率查询请求
	const DWORD FTD_TID_ReqQryInvestorMargin = 0x00008307;
	//投资者保证金率查询应答
	const DWORD FTD_TID_RspQryInvestorMargin = 0x00008308;
	//投资者手续费率查询请求
	const DWORD FTD_TID_ReqQryInvestorFee = 0x00008309;
	//投资者手续费率查询应答
	const DWORD FTD_TID_RspQryInvestorFee = 0x0000830A;
	//投资者报单查询请求
	const DWORD FTD_TID_ReqQryInvestorOrder = 0x0000830B;
	//投资者报单查询应答
	const DWORD FTD_TID_RspQryInvestorOrder = 0x0000830C;
	//投资者成交单查询请求
	const DWORD FTD_TID_ReqQryInvestorTrade = 0x0000830D;
	//投资者成交单查询请求
	const DWORD FTD_TID_RspQryInvestorTrade = 0x0000830E;
	/************************查询功能************************/

	/************************私有流************************/
	//内部行情通知
	const DWORD FTD_TID_IntlRtnDepthMarketData = 0x00008500;
	//深度行情通知
	const DWORD FTD_TID_RtnDepthMarketData = 0x00008501;
	//递延费通知
	const DWORD FTD_TID_RtnDeferFee = 0x00008502;
	//合约交易状态通知
	const DWORD FTD_TID_RtnInstrumentStatus = 0x00008503;
	//报单录入错误回报
	const DWORD FTD_TID_ErrRtnOrderInsert = 0x00008504;
	//报单操作错误回报
	const DWORD FTD_TID_ErrRtnOrderAction = 0x00008505;
	//报单回报
	const DWORD FTD_TID_RtnOrder = 0x00008506;
	//成交回报
	const DWORD FTD_TID_RtnTrade = 0x00008507;
	//错误回报
	const DWORD FTD_TID_RtnOrderError = 0x00008508;
	/************************私有流************************/
	
	/************************其他************************/
	//投资者资金帐户出入金请求
	const DWORD FTD_TID_ReqAccountDeposit = 0x00005024;
	//投资者资金帐户出入金应答
	const DWORD FTD_TID_RspAccountDeposit = 0x00005025;
	//查询转账流水请求
	const DWORD FTD_TID_ReqQryTransferSerial = 0x00005069;
	//查询转账流水应答
	const DWORD FTD_TID_RspQryTransferSerial = 0x0000506A;

	//警告消息通知
	const DWORD FTD_TID_RtnMessageNotify = 0x00005038;
	//订阅主题请求
	const DWORD FTD_TID_ReqSubscribeTopic = 0x00005051;
	//订阅主题应答
	const DWORD FTD_TID_RspSubscribeTopic = 0x00005052;
// 	//主题查询请求
// 	const DWORD FTD_TID_ReqQryTopic = 0x00005055;
// 	//主题查询应答
// 	const DWORD FTD_TID_RspQryTopic = 0x00005056;
	//用户查询请求
	const DWORD FTD_TID_ReqQryUser = 0x00005057;
	//用户查询应答
	const DWORD FTD_TID_RspQryUser = 0x00005058;
}

//ZTP协议功能号(系统内部用)
namespace ZTP_TID_DEFINE {
	/*
		//数据同步开始
	const DWORD TID_DataSyncStart = 0x00005000;
	//数据同步结束
	const DWORD TID_DataSyncEnd = 0x00005001;
	//初始化席位信息
	const DWORD TID_InitSeatInfo = 0x00005002;
	//初始化操作员信息
	const DWORD TID_InitOperatorInfo = 0x00005003;
	//初始化投资者信息
	const DWORD TID_InitInvestorInfo = 0x00005004;
	//初始化用户投资者关系
	const DWORD TID_InitOperatorInvestor = 0x00005005;
	//初始化投资者资金账户
	const DWORD TID_InitInvestorAccount = 0x00005006;		//通过导入的上日结算资金同步
	//初始化投资者持仓
	const DWORD TID_InitInvestorPosition = 0x00005007;		//通过导入的上日结算持仓同步
	//初始化合约列表
	const DWORD TID_InitInstrument = 0x00005008;
	//初始化投资者保证金率
	const DWORD TID_InitInvestorMargin = 0x00005009;
	//初始化投资者手续费
	const DWORD TID_InitInvestorFee = 0x0000500A;

	//初始化投资者手续费
	const DWORD TID_InitOrder = 0x0000500B;

	//初始化投资者手续费
	const DWORD TID_InitTrade = 0x0000500C;

	//初始化投资者手续费
	const DWORD TID_InitInvestorBalanceChange = 0x0000500D;

	//初始化投资者手续费
	const DWORD TID_InitInvestorPositionChange = 0x0000500E;
	//初始化用户交易权限
// 	const DWORD TID_InitPermissionRight = 0x00005006;
	inline bool IsInitFunction(int funcId) {
		return funcId >= 0x00005000 && funcId <= 0x00005099;
	}

	//错误应答
	const DWORD TID_RspError = 0x00005100;
	//用户登录请求
	const DWORD TID_ReqUserLogin = 0x00005101;
	//用户登录应答
	const DWORD TID_RspUserLogin = 0x00005102;
	//用户录出请求
	const DWORD TID_ReqUserLogout = 0x00005103;
	//用户录出应答
	const DWORD TID_RspUserLogout = 0x00005104;
	//强制用户录出请求（交易员用）
	const DWORD TID_ReqForceLogout = 0x00005105;
	//强制用户录出应答
	const DWORD TID_RspForceLogout = 0x00005106;
	//用户密码修改请求
	const DWORD TID_ReqUserPasswordUpdate = 0x00005107;
	//用户密码修改应答
	const DWORD TID_RspUserPasswordUpdate = 0x00005108;
	inline bool IsLoginFunction(int funcId) {
		return funcId >= 0x00005100 && funcId <= 0x00005199;
	}
	//报单录入请求
	const DWORD TID_ReqOrderInsert = 0x00005201;
	//报单录入应答
	const DWORD TID_RspOrderInsert = 0x00005202;
	//报单操作请求
	const DWORD TID_ReqOrderAction = 0x00005204;
	//报单操作应答
	const DWORD TID_RspOrderAction = 0x00005205;
*/
// 
// 	inline bool IsTradeFunction(DWORD tid) {
// 		return (tid >= 0x00005200 && tid <= 0x00005299) || (tid >= 0x00005500 && tid <= 0x00005599);
// 	}
// 
// 	inline bool IsUnsupportedWhenNoTrading(DWORD tid) {
// 		return tid == TID_ReqOrderInsert || tid == TID_ReqOrderAction;
// 	}
// 
// 	inline DWORD GetRspTID(DWORD tID) {
// 		switch (tID) {
// 		case TID_ReqOrderInsert:	return TID_RspOrderInsert;
// 		case TID_ReqOrderAction:	return TID_RspOrderAction;
// 		default:	return 0;
// 		}
// 	}
// 
// 	//合约交易状态（测试一下，早上9点B332或B062是否早于B182）
// 	const DWORD TID_NotifyInstrumentStatus = 0x00005501;
// 	//合约信息通知
// 	const DWORD TID_NotifyInstrumentInfo = 0x00005502;
// 	//递延费通知
 	const DWORD TID_RtnDeferFee = 0x00005503;
// 	//报单录入错误通知（交易所录入失败）
// 	const DWORD TID_ErrOrderInsert = 0x00005504;
// 	//报单错误通知(交易所撤单失败)
// 	const DWORD TID_ErrOrderAction = 0x00005505;
// 	//报单通知
// 	const DWORD TID_NotifyOrder = 0x00005506;
// 	//成交通知
// 	const DWORD TID_NotifyTrade = 0x00005507;
// 	//席位成功登陆通知
// 	const DWORD TID_NotifySeatLoginSuccess = 0x00005508;
// 	//席位断开状态通知
// 	const DWORD TID_NotifySeatDisconnected = 0x00005509;
// 	//席位流水推送结束
// 	const DWORD TID_NotifyInitFinish = 0x0000550A;
// 	//内存表变化通知
// 	const DWORD TID_NtfMemtableChange = 0x0000550B;
// 	//风险度计算通知
// 	const DWORD TID_NotifyDynamicRiskCalculate = 0x0000550C;
// 	//现货出入库
// 	const DWORD TID_NotifySpotPosition = 0x0000550D;
// 
// 	inline bool IsExchangeStatusFunction(int tid) {
// 		return tid == TID_NotifyInstrumentStatus;
// 	}
// 
// 	inline bool IsOrderSyncNotify(int tid) {
// 		return tid == TID_NotifySeatLoginSuccess || tid == TID_NotifySeatDisconnected || tid == TID_NotifyInitFinish ||
// 			tid == TID_NotifyInstrumentInfo || tid == TID_NotifyOrder || tid == TID_NotifyTrade || tid == TID_NotifySpotPosition;
// 	}
// 	inline bool IsOrderTradeNotify(int tid) {
// 		return tid == TID_NotifyOrder || tid == TID_NotifyTrade;
// 	}
// 	//合约查询请求
// 	const DWORD TID_ReqQryInstrument = 0x00005301;
// 	//合约查询应答
// 	const DWORD TID_RspQryInstrument = 0x00005302;
// 	//投资者资金账户查询请求
// 	const DWORD TID_ReqQryInvestorAccount = 0x00005303;
// 	//投资者资金账户查询应答
// 	const DWORD TID_RspQryInvestorAccount = 0x00005304;
// 	//投资者持仓查询请求
// 	const DWORD TID_ReqQryInvestorPosition = 0x00005305;
// 	//投资者持仓查询应答
// 	const DWORD TID_RspQryInvestorPosition = 0x00005306;
// 	//投资者保证金率查询请求
// 	const DWORD TID_ReqQryInvestorMargin = 0x00005307;
// 	//投资者保证金率查询应答
// 	const DWORD TID_RspQryInvestorMargin = 0x00005308;
// 	//投资者手续费率查询请求
// 	const DWORD TID_ReqQryInvestorFee = 0x00005309;
// 	//投资者手续费率查询应答
// 	const DWORD TID_RspQryInvestorFee = 0x0000530A;
// 	//投资者报单查询请求
// 	const DWORD TID_ReqQryInvestorOrder = 0x0000530B;
// 	//投资者报单查询应答
// 	const DWORD TID_RspQryInvestorOrder = 0x0000530C;
// 	//投资者成交查询请求
// 	const DWORD TID_ReqQryInvestorTrade = 0x0000530D;
// 	//投资者成交查询应答
// 	const DWORD TID_RspQryInvestorTrade = 0x0000530E;
// 	inline bool IsQueryFunction(int tid) {
// 		return (tid >= 0x00005300 && tid <= 0x00005399);	
// 	}
// 
// 	//内部成功登陆通知
// 	const DWORD TID_InterLoginInfo = 0x00001058;
// 	//通讯阶段(交易日)切换通知
// 	const DWORD TID_NtfCommPhaseChange = 0x0000105B;

// 	//投资者资金帐户出入金请求
// 	const DWORD TID_ReqAccountDeposit = 0x00001013;
// 	//投资者资金帐户出入金应答
// 	const DWORD TID_RspAccountDeposit = 0x00001014;
// 	//投资者查询请求
// 	const DWORD TID_ReqQryInvestor = 0x00001021;
// 	//投资者查询应答
// 	const DWORD TID_RspQryInvestor = 0x00001022;

// 	//用户查询请求
// 	const DWORD TID_ReqQryUser = 0x0000102A;
// 	//用户查询应答
// 	const DWORD TID_RspQryUser = 0x0000102B;
// 	//可用投资者账户查询请求
// 	const DWORD TID_ReqQryUserInvestor = 0x0000102E;
// 	//可用投资者账户查询应答
// 	const DWORD TID_RspQryUserInvestor = 0x0000102F;

// 	//查询转账流水请求
// 	const DWORD TID_ReqQryTransferSerial = 0x00001041;
// 	//查询转账流水应答
// 	const DWORD TID_RspQryTransferSerial = 0x00001042;
// 	//增加交易用户
// 	const DWORD TID_DbInsertUser = 0x00001056;
// 	//修改交易用户
// 	const DWORD TID_DbUpdateUser = 0x00001057;
// 	//客户资金帐户出入金
// 	const DWORD TID_InvestorAccountDeposit = 0x00001055;

	//通知生命周期号
// 	const DWORD TID_NtfLifeCycle = 0x0000105A;
	//会话删除通知
//	const DWORD TID_NtfDelUserSession = 0x0000105C;

// 	//合约和合约组关系
// 	const DWORD TID_InitInstrumentGroup = 0x00001059;
// 	//投资者和通道映射关系
// 	const DWORD TID_InitInvestorLinker = 0x00001057;
// 	//转账流水
// 	const DWORD TID_InitBFTransferInterface = 0x00001059;


// 	//席位状态心跳通知
// 	const DWORD TID_SeatStatusHeartbeat = 0x00001057;
	//最优行情通知
// 	const DWORD TID_NtfMarketDataLastMatch = 0x00001059;
	//投资者资金内部查询请求
// 	const DWORD TID_ReqQryInnerInvestorAccount = 0x00001058;
// 	//投资者资金内部查询应答
// 	const DWORD TID_RspQryInnerInvestorAccount = 0x00001059;
	//投资者持仓内部查询请求
// 	const DWORD TID_ReqQryInnerInvestorPosition = 0x00001056;
// 	//投资者持仓内部查询应答
// 	const DWORD TID_RspQryInnerInvestorPosition = 0x00001057;
// 	//网关信息初始化
// 	const DWORD TID_InitMGateWay = 0x00001056;
// 	//出入金回报通知
// 	const DWORD TID_NtfInvestorAccountDeposit = 0x00001059;
}
