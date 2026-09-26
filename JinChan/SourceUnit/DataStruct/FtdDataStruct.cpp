#include "stdafx.h"
#include "FtdDataStruct.h"

//0000 最后一位是功能号递增
//0000 第二位是大类功能0-功能类；1-查询类；2-操作类；3-通知类

REGISTER_FIELD(FTD_FID_Dissemination, CFTDDisseminationField, "信息分发");
REGISTER_FIELD(FTD_FID_RspInfo, CFTDRspInfoField, "响应信息");
REGISTER_FIELD(FTD_FID_ReqUserLogin, CFTDReqUserLoginField, "系统用户登录请求");
REGISTER_FIELD(FTD_FID_RspUserLogin, CFTDRspUserLoginField, "系统用户登录应答");
REGISTER_FIELD(FTD_FID_ReqUserLogout, CFTDReqUserLogoutField, "用户录出请求");
REGISTER_FIELD(FTD_FID_RspUserLogout, CFTDRspUserLogoutField, "用户录出应答");
REGISTER_FIELD(FTD_FID_ForceUserExit, CFTDForceInvestorExitField, "强制用户退出");
REGISTER_FIELD(FTD_FID_UserPasswordUpdate, CFTDUserPasswordUpdateField, "用户修改密码请求");

REGISTER_FIELD(FTD_FID_Qry_Instrument, CFTDQryInstrumentField, "合约查询");
REGISTER_FIELD(FTD_FID_Rsp_Instrument, CFTDRspInstrumentField, "合约信息");
REGISTER_FIELD(FTD_FID_Qry_Account, CFTDQryInvestorAccountField, "资金账户查询");
REGISTER_FIELD(FTD_FID_Rsp_Account, CFTDRspInvestorAccountField, "资金账户信息");
REGISTER_FIELD(FTD_FID_Qry_Position, CFTDQryInvestorPositionField, "持仓查询");
REGISTER_FIELD(FTD_FID_Rsp_Position, CFTDRspInvestorPositionField, "持仓信息");
REGISTER_FIELD(FTD_FID_Qry_Margin, CFTDQryInvestorMarginField, "保证金查询");
REGISTER_FIELD(FTD_FID_Rsp_Margin, CFTDRspInvestorMarginField, "保证金信息");
REGISTER_FIELD(FTD_FID_Qry_Fee, CFTDQryInvestorFeeField, "手续费查询");
REGISTER_FIELD(FTD_FID_Rsp_Fee, CFTDRspInvestorFeeField, "手续费信息");

REGISTER_FIELD(FTD_FID_SpecificInstrument, CFTDSpecificInstrumentField, "订阅合约");
REGISTER_FIELD(FTD_FID_Qry_Order, CFTDQryOrderField, "报单查询");
REGISTER_FIELD(FTD_FID_Qry_Trade, CFTDQryTradeField, "成交查询");
REGISTER_FIELD(FTD_FID_InputOrder, CFTDInputOrderField, "输入报单");
REGISTER_FIELD(FTD_FID_OrderAction, CFTDOrderActionField, "报单操作");
REGISTER_FIELD(FTD_FID_OrderRtn, CFTDOrderField, "报单回报");
REGISTER_FIELD(FTD_FID_TradeRtn, CFTDTradeField, "成交回报");
REGISTER_FIELD(FTD_FID_InstrumentStatus, CFTDInstrumentStatusField, "合约状态");
REGISTER_FIELD(FTD_FID_DeferFee, CFTDDeferFeeField, "递延费回报");

REGISTER_FIELD(FTD_FID_DepthMarketData, CFTDDepthMarketDataField, "深度行情");
