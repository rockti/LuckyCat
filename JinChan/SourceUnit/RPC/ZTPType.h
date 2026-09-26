#pragma once
#include "IClient.h"
#include "type.hpp"
#include <fmt/format.h>


template<uint32_t size, typename Tag> struct fmt::formatter<StrongCharArray<size,Tag>> {
	formatter<char[size]> char_array_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return char_array_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const StrongCharArray<size, Tag>& s, FormatContext& ctx) {
		return char_array_formatter.format(s.value(), ctx);
	}
};

template<typename Base, typename Tag> struct fmt::formatter<NumberType<Base, Tag>> {
	formatter<Base> number_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return number_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const NumberType<Base, Tag>& s, FormatContext& ctx) {
		return number_formatter.format(s.value(), ctx);
	}
};

template<typename Tag> struct fmt::formatter<DoubleType<Tag>> {
	formatter<double> double_formatter;

	template <typename ParseContext>
	constexpr auto parse(ParseContext& ctx) {
		return double_formatter.parse(ctx);
	}

	template <typename FormatContext>
	auto format(const DoubleType<Tag>& s, FormatContext& ctx) {
		return double_formatter.format(s.value(), ctx);
	}
};


namespace ztp {

	namespace type {

		using ErrorCode = NumberType<int32_t, struct ErrorCodeTag>;
		using Money = DoubleType<struct MoneyTag>;
		using Price = DoubleType<struct PriceTag>;
		using Ratio = RatioType<struct RatioTag>;
		using Volume = NumberType<uint32_t, struct VolumeTag>;
		using VolumeMultiple = NumberType<uint32_t, struct VolumeMultipleTag>;
		using PriceTick = DoubleType<struct PriceTickTag>;
		using LargeVolume = NumberType<uint64_t, struct LargeVolumeTag>;
		using SequenceSeries = NumberType<uint32_t, struct SequenceSeriesTag>;
		using SequenceNo = NumberType<uint32_t, struct SequenceNoTag>;
		using ErrorID = NumberType<int32_t, struct ErrorIDTag>;
		using Millisec = NumberType<uint32_t, struct MillisecTag>;
		using CommFlux = NumberType<uint32_t, struct CommFluxTag>;
		using CommPhaseNo = NumberType<uint32_t, struct CommPhaseNoTag>;
		using SessionID = NumberType<uint32_t, struct SessionIDTag>;
		using RequestID = NumberType<uint32_t, struct RequestIDTag>;
		using Time = StrongCharArray<9, struct TimeTag>;
		using Date = StrongCharArray<9, struct DateTag>;
		using TradingDay = Date;
		using OrderLocalID = NumberType<uint32_t, struct OrderLocalIDTag>;
		using UserOrderLocalID = StrongCharArray<16, struct UserOrderLocalIDTag>;
		using OrderSysID = StrongCharArray<21, struct OrderSysIDTag>;
		using TraderID = StrongCharArray<21, struct TraderIDTag>;
		using TradeID = StrongCharArray<21, struct TradeIDTag>;
		using IPAddress = StrongCharArray<21, struct IPAddressTag>;
		using MacAddress = StrongCharArray<21, struct MacAddressTag>;
		using FrontAddr = StrongCharArray<33, struct FrontAddrTag>;
		using MemTableName = StrongCharArray<61, struct MemTableNameTag>;
		using ProductID = StrongCharArray<11, struct ProductIDTag>;
		using ProductName = StrongCharArray<31, struct ProductNameTag>;
		using AppName = StrongCharArray<31, struct AppNameTag>;
		using InstrumentID = StrongCharArray<11, struct InstrumentIDTag>;
		using InstrumentName = StrongCharArray<31, struct InstrumentNameTag>;
		using ExchangeID = StrongCharArray<11, struct ExchangeIDTag>;
		using ExchangeName = StrongCharArray<31, struct ExchangeNameTag>;
		using ParticipantID = StrongCharArray<11, struct ParticipantIDTag>;
		using BrokerID = StrongCharArray<11, struct BrokerIDTag>;
		using SeatID = StrongCharArray<11, struct SeatIDTag>;
		using AccountID = StrongCharArray<11, struct AccountIDTag>;
		using OperatorID = StrongCharArray<11, struct OperatorIDTag>;
		using LoginID = StrongCharArray<11, struct LoginIDTag>;
		using UserName = StrongCharArray<31, struct UserNameTag>;
		using InvestorID = StrongCharArray<11, struct InvestorIDTag>;
		using InvestorName = StrongCharArray<31, struct InvestorNameTag>;
		using ClientID = StrongCharArray<13, struct ClientIDTag>;
		using Password = StrongCharArray<16, struct PasswordTag>;
		using ProductInfo = StrongCharArray<21, struct ProductInfoTag>;
		using TradingSystemName = StrongCharArray<31, struct TradingSystemNameTag>;
		using Custom = StrongCharArray<21, struct CustomeTag>;
		using ErrorMsg = StrongCharArray<121, struct ErrorMsgTag>;
		using Rights = StrongCharArray<32, struct RightsTag>;
		using Hash = StrongCharArray<129, struct HashTag>;

		enum struct UserType: char { Investor = '1', Operator = '2'};
		enum struct SystemStatus : char { Start = '0', DBLoading = '1', ExchSyncing = '2', Ready = '3' };
		enum struct TradingStatus : char { NotTrading = '0', Trading = '1'};
		enum struct SeatStatus : char { InActive = '0', Login = '1', Logout = '2'};
		enum struct OrderPriceType : char {LimitPrice = '0', FOK = '1', FAK = '2' };
		enum struct HedgeFlag : char { Speculation = '1', Arbitrage = '2', Hedge = '3', MarketMaker = '4' };
		enum struct TradeRight: char { Allow = '0', CloseOnly = '1', Forbidden = '2' };
		enum struct PermissionRight : char { Market = '0', Trade = '1' };
		enum struct InstrumentStatus : char {
			Initting = '0',				//初始化
			Inited = '1',				//初始化完成
			OpenMarket = '2',			//开盘
			AuctionOrdering = '3',		//集合竞价报单
			AuctionMatch = '4',			//集合竞价撮合
			Continous = '5',			//连续交易
			Pause = '6',				//暂停交易
			DeryApp = '7',				//交割申报
			DeryMatch = '8',			//交割申报结束
			MidApp = '9',				//中立仓申报
			MidMatch = 'A',				//中立仓平衡
			Closed = 'B',				//收盘
		};

		enum struct Direction : char { Buy = '0', Sell = '1' };
		enum struct DeferFeeDirection : char { Unknown ='0', LongPay = '1', ShortPay = '2', Equal='3' };

		inline bool IsBuy(Direction d) {
			return d == Direction::Buy;
		}

		inline bool IsSell(Direction d) {
			return d == Direction::Sell;
		}

		template<class T>
		inline bool IsBuy(T& t) {
			return IsBuy(t.Direction);
		}

		template<class T>
		inline bool IsSell(T& t) {
			return IsSell(t.Direction);
		}


		template<class T>
		inline bool IsBuy(T* t) {
			return IsBuy(t->Direction);
		}

		template<class T>
		inline bool IsSell(T* t) {
			return IsSell(t->Direction);
		}

		inline type::Direction negetive(type::Direction direction) {
			if (direction == type::Direction::Buy) {
				return type::Direction::Sell;
			}
			return type::Direction::Buy;
		}


		enum struct OffsetFlag : char {
			Open = '0',					//开仓
			Close = '1',				//平仓
			ForceClose = '2',			//强平
			CloseToday = '3',			//平今
			CloseYesterday = '4',		//平昨
			Unknown = 'X',		//平昨
		};

		inline bool IsUnknown(OffsetFlag f) {
			return f == OffsetFlag::Unknown;
		}

		inline bool IsOpen(OffsetFlag f) {
			return f == OffsetFlag::Open;
		}

		inline bool IsClose(OffsetFlag f) {
			return f == OffsetFlag::Close;
		}

		template<class F>
		inline bool IsUnknown(F& t) {
			return IsUnknown(t.OffsetFlag);
		}
		template<class F>
		inline bool IsOpen(F& t) {
			return IsOpen(t.OffsetFlag);
		}
		template<class F>
		inline bool IsClose(F& t) {
			return IsClose(t.OffsetFlag);
		}

		template<class F>
		inline bool IsUnknown(F* t) {
			return IsUnknown(t->OffsetFlag);
		}
		template<class F>
		inline bool IsOpen(F* t) {
			return IsOpen(t->OffsetFlag);
		}
		template<class F>
		inline bool IsClose(F* t) {
			return IsClose(t->OffsetFlag);
		}

		enum struct ActionFlag : char {
			Delete = '0',			//删除
			Suspend = '1',			//挂起
			Active = '2',			//激活
			Modify = '3',			//修改
		};
		enum struct OrderStatus : char {
			New = 'n',					//新报单
			InsertQueueing =  'a',		//订单已提交，未有系统号
			NoTradeQueueing = '3',		//交易所成功，返回了系统号
			InsertFail = 'f',			//订单失败
			AllTraded = '0',			//全部成交
			PartTradedQueueing = '1',	//部分成交还在队列中
			CancelQueueing = 'q',		//撤单已提交，未有应答
			CancelFail = 'c',			//取消失败
			Canceled = '5',				//已取消
			NoTradeNotQueueing = '4',	//未使用

		};
		enum struct IsActive : char { InActive = '0', Active = '1' };
		enum struct IsNewAuth : char { Old = '0', New = '1' };
		enum struct ProductClass : char {
			Unknown	= '0',				//未知市场
			Defer = '1',				//延期市场
			Spot = '2',					//现货市场
			Delivery = '3',				//交割申报
			Neutral = '4',				//中立仓
		};

		inline bool IsDefer(ProductClass p) {
			return ProductClass::Defer == p;
		}

		inline bool IsSpot(ProductClass p) {
			return ProductClass::Spot == p;
		}

		template<class T>
		inline bool IsDefer(T& t) {
			return IsDefer(t.ProductClass);
		}

		template<class T>
		inline bool IsDefer(T* t) {
			return IsDefer(t->ProductClass);
		}

		template<class T>
		inline bool IsSpot(T& t) {
			return IsSpot(t.ProductClass);
		}

		template<class T>
		inline bool IsSpot(T* t) {
			return IsSpot(t->ProductClass);
		}

		enum struct MemTableChangeType : char {
			Insert = '0',
			Update = '1',
			Delete = '2',
			Commit = '3',
		};

		inline bool operator ==(LoginID const& loginID, InvestorID const& investor) {
			return loginID == investor.value();
		}
		inline bool operator !=(LoginID const& loginID, InvestorID const& investor) {
			return loginID != investor.value();
		}


		inline TradingStatus convertToTradingStatus(InstrumentStatus status) {
			switch (status) {
			case InstrumentStatus::OpenMarket:				//开盘
			case InstrumentStatus::AuctionOrdering:			//集合竞价报单
			case InstrumentStatus::Continous:				//连续交易
			case InstrumentStatus::DeryApp:					//交割申报
			case InstrumentStatus::MidApp:					//中立仓申报
				return TradingStatus::Trading;
			case InstrumentStatus::Initting:				//初始化
			case InstrumentStatus::Inited:					//初始化完成
			case InstrumentStatus::AuctionMatch:			//集合竞价撮合
			case InstrumentStatus::DeryMatch:				//交割申报结束
			case InstrumentStatus::MidMatch:				//中立仓平衡
			case InstrumentStatus::Closed:					//收盘
			case InstrumentStatus::Pause:					//暂停交易
				return TradingStatus::NotTrading;
			default:
				return TradingStatus::NotTrading;
				break;
			}
		}

		enum struct PositionChangeFlag : char {
			Unknown = '0',
			Trade = '1',
			Spot = '2',
			Yesterday  = '3'
		};

		enum struct BalanceChangeFlag : char {
			Unknown = '0',
			Deposit = '1',
			Withdraw = '2',
		};

		template<class base, class Tag>
		inline auto value(typename NumberType<base, Tag> const& number) {
			return number.value();
		}

		template<class Tag>
		inline auto value(typename DoubleType<Tag> const& number) {
			return number.value();
		}

		template<uint16_t size, class Tag>
		inline auto value(typename StrongCharArray<size, Tag> const& charArray) {
			return charArray.value();
		}

		template<class T>
		inline auto value(T const& v) {
			return v;
		}

	}
	namespace data {
		//系统状态
		const MessageID FID_SystemStatus = 0x3001;
		struct CSystemStatus {
			static const MessageID ID = 0x3001;
			type::Date TradingDay;
			type::SystemStatus SystemStatus;
			type::TradingStatus TradingStatus;
		};

		//席位信息
		const MessageID FID_SeatInfo = 0x3002;
		struct CSeatInfo {
			static const MessageID ID = 0x3002;
			type::AppName AppName;
			type::ExchangeID ExchangeID;
			type::ParticipantID ParticipantID;
			type::SeatID SeatID;
			type::TraderID TraderID;
			type::Password Password;
			type::SeatStatus SeatStatus;
			type::FrontAddr FrontAddr;
			type::OrderLocalID OrderLocalID;
		};

		//操作员信息
		const MessageID FID_OperatorInfo = 0x3004;
		struct COperatorInfo {
			static const MessageID ID = 0x3004;
			type::BrokerID BrokerID;
			type::OperatorID OperatorID;
			type::UserName UserName;
			type::Password Password;
			type::PermissionRight PermissionRight;
		};

		//投资者信息
		const MessageID FID_InvestorInfo = 0x3005;
		struct CInvestorInfo {
			static const MessageID ID = 0x3005;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::InvestorName InvestorName;
			type::Password Password;		
			type::ClientID ClientID;
			type::TradeRight TradeRight;
			type::IsActive	IsActive;
			type::IsNewAuth IsNewAuth;
			type::Hash Salt;
			type::Hash PasswordHash;
		};

		//用户投资者关系
		const MessageID FID_OperatorInvestor = 0x3006;
		struct COperatorInvestor {
			static const MessageID ID = 0x3006;
			type::BrokerID BrokerID;
			type::OperatorID OperatorID;
			type::InvestorID InvestorID;
		};

		//用户会话
		const MessageID FID_UserSession = 0x3007;
		struct CUserSession {
			static const MessageID ID = 0x3007;
			type::BrokerID BrokerID;
			type::LoginID LoginID;
			type::UserType UserType;
			type::SessionID	SessionID;
			type::Time LoginTime;
			type::IPAddress	IPAddress;
			type::ProductInfo UserProductInfo;
		};

		//投资者资金账户
		const MessageID FID_InvestorAccount = 0x300A;
		struct CInvestorAccount {
			static const MessageID ID = 0x300A;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::TradingDay TradingDay;
			type::Money	PreBalance;				//上日权益	
			type::Money	PreAvailable;			//上日可用
			type::Money	Balance;				//当前权益
			type::Money	Available;				//当前可用
			type::Money	Deposit;				//入金金额
			type::Money	Withdraw;				//出金金额
			type::Money	Margin;					//占用保证金
			type::Money	Fee;					//手续费
			type::Money	OtherFee;				//其他费用
			type::Money	CloseProfit;			//平仓盈亏
			type::Money	PositionProfit;			//持仓盈亏
			type::Money	FrozenMargin;			//冻结的保证金
			type::Money	FrozenFee;				//冻结手续费
			type::Money	LongMargin;				//多头占用保证金
			type::Money	ShortMargin;			//空头占用保证金
			type::Money	LongFrozenMargin;		//多头冻结的保证金
			type::Money	ShortFrozenMargin;		//空头冻结的保证金
			type::Money	DynamicRights;			//动态权益
			type::Ratio	Risk;					//风险度
		};

		//客户保证金率
		const MessageID FID_InvestorMargin = 0x300B;
		struct CInvestorMargin {
			static const MessageID ID = 0x300B;

			type::BrokerID BrokerID;
			type::ExchangeID ExchangeID;
			type::InvestorID InvestorID;
			type::InstrumentID InstrumentID;
			type::Ratio	LongMarginRate;				//多头占用保证金按比例
			type::Money	LongMarginAmt;				//多头保证金按手数
			type::Ratio	ShortMarginRate;			//空头占用保证金按比例
			type::Money	ShortMarginAmt;				//空头保证金按手数
		};

		//投资者手续费
		const MessageID FID_InvestorFee = 0x300C;
		struct CInvestorFee {
			static const MessageID ID = 0x300C;

			type::BrokerID BrokerID;
			type::ExchangeID ExchangeID;
			type::InvestorID InvestorID;
			type::InstrumentID InstrumentID;
			type::Ratio	OpenFeeRate;				//开仓手续费按比例
			type::Money	OpenFeeAmt;					//开仓手续费按手数
			type::Ratio	OffsetFeeRate;				//平仓手续费按比例
			type::Money	OffsetFeeAmt;				//平仓手续费按手数
			type::Ratio	OffsetTodayFeeRate;			//平今仓手续费按比例
			type::Money	OffsetTodayFeeAmt;			//平今仓手续费按手数
		};

		//合约信息
		const MessageID FID_Instrument = 0x300D;
		struct CInstrument {
			static const MessageID ID = 0x300D;
			type::ExchangeID ExchangeID;
			type::InstrumentID InstrumentID;
			type::InstrumentName InstrumentName;
			type::ProductID ProductID;
			type::ProductClass ProductClass = type::ProductClass::Unknown;
			type::Volume LongPosLimit;				//多头限仓
			type::Volume ShortPosLimit;				//空头限仓
			type::Volume MaxOrderVolume;			//最大下单量
			type::Volume MinOrderVolume;			//最小下单量
			type::VolumeMultiple VolumeMultiple;	//数量乘数
			type::PriceTick	PriceTick;				//报价单位

			int UnilateralGroup = -1;
			int UnilateralSubIndex = -1;

		};

		//投资者合约持仓
		const MessageID FID_InvestorPosition = 0x300E;
		struct CInvestorPosition {
			static const MessageID ID = 0x300E;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::TradingDay TradingDay;
			type::ExchangeID ExchangeID;
			type::InstrumentID InstrumentID;
			type::ProductClass ProductClass;
			type::Direction	Direction;
			type::Money	UsedMargin;					//占用保证金
			type::Volume Position;				//今持仓量
			type::Price	PositionCost;				//今日持仓成本
			type::Volume YdPosition;			//昨持仓量
			type::Price	YdPositionPrice;			//昨日持仓结算价
			type::Money	FrozenMargin;				//冻结的保证金
			type::Money	FrozenFee;					//开平冻结手续费
			type::Volume FrozenPosition;			//平仓冻结持仓
		};
		//应答信息
		const MessageID FID_RspInfo = 0x3010;
		struct CRspInfo {
			static const MessageID ID = 0x3010;
			type::ErrorID	ErrorID;
			type::ErrorMsg	ErrorMsg;
		};

		//用户登录
		const MessageID FID_UserLogin = 0x3011;

		struct CUserLogin {
			static const MessageID ID = 0x3011;
			type::Date TradingDay;
			type::BrokerID BrokerID;
			type::LoginID LoginID;
			type::UserType UserType;
			type::Password Password;
			type::ProductInfo UserProductInfo;
			type::TradingSystemName TradingSystemName;
			type::IPAddress IPAddress;
			type::Time LoginTime;
			type::UserOrderLocalID MaxOrderLocalID;
		};

		//用户录出
		const MessageID FID_UserLogout = 0x3012;
		struct CUserLogout {
			static const MessageID ID = 0x3012;
			type::BrokerID BrokerID;
			type::LoginID LoginID;
		};

		//强制用户退出
		const MessageID FID_ForceUserExit = 0x3013;
		struct CForceInvestorExit {
			static const MessageID ID = 0x3013;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
		};

		//强制用户退出
		const MessageID FID_UserPasswordUpdate = 0x3014;
		struct CUserPasswordUpdate {
			static const MessageID ID = 0x3014;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::Password OldPassword;
			type::Password NewPassword;
		};

		//合约查询
		const MessageID FID_QryInstrument = 0x3015;
		struct CQryInstrument {
			static const MessageID ID = 0x3015;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::ProductClass ProductClass;
			type::InstrumentID InstrumentID;
		};

		//投资者资金账户查询
		const MessageID FID_QryInvestorAccount = 0x3016;
		struct CQryInvestorAccount {
			static const MessageID ID = 0x3016;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::BrokerID LoginBrokerID;
			type::LoginID LoginUserID;
		};

		//投资者持仓查询
		const MessageID FID_QryInvestorPosition = 0x3017;
		struct CQryInvestorPosition {
			static const MessageID ID = 0x3017;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::BrokerID LoginBrokerID;
			type::LoginID LoginUserID;
		};

		//投资者保证金率查询
		const MessageID FID_QryInvestorMargin = 0x3018;
		struct CQryInvestorMargin {
			static const MessageID ID = 0x3018;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::InstrumentID InstrumentID;
		};

		///投资者手续费率查询
		const MessageID FID_QryInvestorFee = 0x3019;
		struct CQryInvestorFee {
			static const MessageID ID = 0x3019;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::InstrumentID InstrumentID;
		};

		//报单操作
		const MessageID FID_OrderAction = 0x3021;
		struct COrderAction {
			static const MessageID ID = 0x3021;
			type::BrokerID BrokerID;
			type::OperatorID CancelOperatorID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::ProductClass ProductClass;
			type::InstrumentID InstrumentID;
			type::OrderSysID OrderSysID;
			type::UserOrderLocalID UserOrderLocalID;
			type::UserOrderLocalID UserOrderActionLocalID;
			type::ActionFlag ActionFlag;
			type::Price	LimitPrice;
			type::Volume VolumeChange;
			type::SeatID SeatID;
			type::ClientID ClientID;
			type::OrderLocalID OrderLocalID;
			type::OrderLocalID ActionLocalID;
			type::Time CancelTime;
		};
		//席位登录通知
		const MessageID FID_LoginInfo = 0x3023;
		struct CSeatLogin {
			static const MessageID ID = 0x3023;
			type::ExchangeID ExchangeID;
			type::SeatID SeatID;
			type::TraderID TraderID;
			type::TradingDay TradingDay;
			type::OrderLocalID OrderLocalID;
		};

		//合约状态通知
		const MessageID FID_InstrumentStatus = 0x3024;
		struct CInstrumentStatus {
			static const MessageID ID = 0x3024;
			type::ExchangeID ExchangeID;
			type::InstrumentStatus InstrumentStatus;
		};

		//报单信息
		const MessageID FID_Order = 0x3026;
		struct COrder {
			static const MessageID ID = 0x3026;
			type::BrokerID	BrokerID;
			type::OperatorID OrderOperatorID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::ProductClass ProductClass;
			type::InstrumentID InstrumentID;
			type::Direction Direction;
			type::OffsetFlag OffsetFlag;
			type::Price	LimitPrice;
			type::Volume Volume;
			type::OrderPriceType OrderPriceType;
			type::UserOrderLocalID UserOrderLocalID;
			type::OrderSysID OrderSysID;
			type::Custom UserCustom;
			type::OrderStatus OrderStatus;
			type::TradingDay TradingDay;
			type::SeatID SeatID;
			type::ClientID ClientID;
			type::Time InsertTime;
			type::Time CancelTime;
			type::OperatorID CancelOperatorID;
			type::Volume VolumeTrade;
			type::Volume VolumeRemain;
			type::OrderLocalID	OrderLocalID;
			type::OrderLocalID	ActionLocalID;
			type::UserOrderLocalID	UserActionLocalID;
			type::Money	FrozenMargin;
			type::Money	FrozenFee;
			type::Money	TradeMargin;
			type::Money	TradeFee;
			type::ErrorCode InsertErrorCode;
			type::ErrorCode CancelErrorCode;
			type::SequenceNo SequenceNo;


			void linkInstrument(CInstrument* p) { pInstrument = p; }
			void linkInvestorFee(CInvestorFee* p) { pInvestorFee = p; }
			void linkInvestorMargin(CInvestorMargin* p) { pInvestorMargin = p; }
			void linkInvestorPosition(CInvestorPosition* p) { pInvestorPosition = p; }
			void linkInvestorAccount(CInvestorAccount* p) { pInvestorAccount = p; }
			void linkInvestor(CInvestorInfo* p) { pInvestor = p; }

			CInvestorMargin* getInvestorMargin() const { return pInvestorMargin; };
			CInvestorFee* getInvestorFee() const { return pInvestorFee; };
			CInvestorPosition* getInvestorPosition() const { return pInvestorPosition; };
			CInstrument* getInstrument() const { return pInstrument; };
			CInvestorAccount* getInvestorAccount() const { return pInvestorAccount; };
			CInvestorInfo* getInvestor() const { return pInvestor; };

		private:
			CInvestorInfo* pInvestor = nullptr;				//对应的投资者
			CInvestorAccount* pInvestorAccount = nullptr;		//对应的帐户	
			CInvestorPosition* pInvestorPosition = nullptr;	//对应的合约持仓信息
			CInstrument* pInstrument = nullptr;				//对应的合约信息	
			CInvestorMargin* pInvestorMargin = nullptr;		//对应的投资者保证金率信息	
			CInvestorFee* pInvestorFee = nullptr;				//对应的投资者手续费率信息

		};

		//成交信息
		const MessageID FID_Trade = 0x3027;
		struct CTrade {
			static const MessageID ID = 0x3027;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::ProductClass ProductClass;
			type::InstrumentID InstrumentID;
			type::Direction Direction;
			type::OffsetFlag OffsetFlag;
			type::Volume TradeVolume;
			type::Price TradePrice;
			type::Money TradeAmt;
			type::Date TradeDate;
			type::Time TradeTime;
			type::TradeID TradeID;
			type::OrderSysID OrderSysID;
			type::UserOrderLocalID UserOrderLocalID;
			type::ClientID ClientID;
			type::OperatorID OrderOperatorID;
			type::OrderLocalID OrderLocalID;
			type::Money	UsedMargin;
			type::Money	UsedFee;
			type::SequenceNo SequenceNo;

		};


		//交易日信息
		const MessageID FID_TradingDay = 0x3061;
		struct CTradingDay {
			static const MessageID ID = 0x3061;
			type::TradingDay TradingDay;
		};

		///报单查询
		const MessageID FID_QryInvestorOrder = 0x3063;
		struct CQryInvestorOrder {
			static const MessageID ID = 0x3063;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::OrderSysID OrderSysID;
			type::InstrumentID InstrumentID;
		};

		///成交查询
		const MessageID FID_QryInvestorTrade = 0x3064;
		struct CQryInvestorTrade {
			static const MessageID ID = 0x3064;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::OrderSysID OrderSysID;
			type::InstrumentID InstrumentID;
		};

		const MessageID FID_OpInvestorBalance = 0x3065;
		struct COpInvestorBalance {
			static const MessageID ID = FID_OpInvestorBalance;
			type::BrokerID BrokerID;
			type::OperatorID OperatorID;
			type::InvestorID InvestorID;
			type::Money Deposit;
			type::Money Withdraw;
		};


		const MessageID FID_SpotPositionNotify = 0x3066;
		struct CSpotPositionNotify {
			static const MessageID ID = FID_SpotPositionNotify;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::InstrumentID InstrumentID;
			type::OffsetFlag OffsetFlag;
			type::Volume Volume;
			type::SequenceNo SequenceNo;
			type::Time NotifyTime;
		};

		const MessageID FID_QryInvestorPositionChange = 0x3067;
		struct CQryPositionChange {
			static const MessageID ID = FID_QryInvestorPositionChange;
			type::BrokerID	BrokerID;
			type::InvestorID	InvestorID;
			type::ExchangeID	ExchangeID;
			type::InstrumentID InstrumentID;
			type::ProductClass	ProductClass;
		};


		const MessageID FID_InvestorPositionChange = 0x3068;
		struct CPositionChange {
			static const MessageID ID = FID_InvestorPositionChange;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::ExchangeID ExchangeID;
			type::TradingDay TradingDay;
			type::InstrumentID InstrumentID;
			type::ProductClass ProductClass;
			type::Direction	Direction;
			type::OffsetFlag OffsetFlag;
			type::PositionChangeFlag ChangeType;
			type::Volume Volume;
			type::OrderLocalID OrderLocalID;
			type::TradeID TradeID;
			type::OrderSysID OrderSysID;
			type::UserOrderLocalID UserOrderLocalID;
			type::SequenceNo ExchangeSequenceNo;
			type::SequenceNo PositionSequenceNo;
			type::Time NotifyTime;
		};

		const MessageID FID_InvestorBalanceChange = 0x3069;
		struct CBalanceChange {
			static const MessageID ID = FID_InvestorBalanceChange;
			type::BrokerID BrokerID;
			type::InvestorID InvestorID;
			type::TradingDay TradingDay;
			type::BalanceChangeFlag BalanceChangeType;
			type::Money Amount;
			type::SequenceNo ExchangeSequenceNo;
			type::SequenceNo BalanceSequenceNo;
			type::Time ChangeTime;
		};

		const MessageID FID_OpQryInvestors = 0x3070;
		struct COpQryInvestors {
			static const MessageID ID = FID_OpQryInvestors;
			type::BrokerID	BrokerID;
			type::OperatorID OperatorID;
		};

		const MessageID FID_OpQryInvestorAccount = 0x3071;
		struct COpQryInvestorAccount {
			static const MessageID ID = FID_OpQryInvestorAccount;
			type::BrokerID	BrokerID;
			type::OperatorID OperatorID;
			type::InvestorID InvestorID;
		};

		const MessageID FID_OpQryInvestorPosition = 0x3072;
		struct COpQryInvestorPosition {
			static const MessageID ID = FID_OpQryInvestorPosition;
			type::BrokerID	BrokerID;
			type::OperatorID OperatorID;
			type::InvestorID InvestorID;
		};

		const MessageID FID_OpQryInvestorBalanceChange = 0x3073;
		struct COpQryInvestorBalanceChange {
			static const MessageID ID = FID_OpQryInvestorBalanceChange;
			type::BrokerID	BrokerID;
			type::OperatorID OperatorID;
			type::InvestorID InvestorID;
		};

		const MessageID FID_OpQryInvestorPositionChange = 0x3074;
		struct COpQryInvestorPositionChange {
			static const MessageID ID = FID_OpQryInvestorPositionChange;
			type::BrokerID	BrokerID;
			type::OperatorID OperatorID;
			type::InvestorID InvestorID;
		};

		const MessageID FID_InvestorRights = 0x3075;
		struct CInvestorRights {
			static const MessageID ID = FID_InvestorRights;
			type::BrokerID	BrokerID;
			type::InvestorID InvestorID;
			type::Rights MarketRights; //bit 0 sge, bit 1 comex, if empty means "1"(sge allowed)
			type::Rights TradeRights; // bit 0 sge, bit 1 comex, if empty means "1"(sge allowed)
		};

		const MessageID FID_DeferFee = 0x3076;
		struct CDeferFee {
			static const MessageID ID = FID_DeferFee;
			type::InstrumentID InstrumentID;
			type::DeferFeeDirection Direction;
			type::Ratio FeeRate;
		};


		const MessageID FID_MemTableChange = 0x4073;
		struct CMemTableChange {
			static const MessageID ID = FID_MemTableChange;
			type::MemTableChangeType	MemtableChangeType;
			type::MemTableName	MemTableName;
		};

		//深度行情
		const MessageID FTD_FID_DepthMarketData = 0x3030;
		struct CDepthMarketData {
			static const MessageID ID = FTD_FID_DepthMarketData;
			type::TradingDay	TradingDay;
			type::InstrumentID	InstrumentID;
			type::Price OpenPrice;					//今开盘
			type::Price AskPrice1;
			type::Volume AskVolume1;
			type::Price AskPrice2;
			type::Volume AskVolume2;
			type::Price AskPrice3;
			type::Volume AskVolume3;
			type::Price AskPrice4;
			type::Volume AskVolume4;
			type::Price AskPrice5;
			type::Volume AskVolume5;
			type::Price BidPrice1;
			type::Volume BidVolume1;
			type::Price BidPrice2;
			type::Volume BidVolume2;
			type::Price BidPrice3;
			type::Volume BidVolume3;
			type::Price BidPrice4;
			type::Volume BidVolume4;
			type::Price BidPrice5;
			type::Volume BidVolume5;
			type::Price	HighestPrice;				//最高价
			type::Price	LowestPrice;				//最低价
			type::Price	LastPrice;					//最新价
			type::Price	AvgPrice;					//均价
			type::Price	Weight;						//成交重量
			type::Volume Volume;					//成交量
			type::Money Turnover;					//成交额
			type::Price	LowerLimitPrice;			//跌停板价
			type::Price	UpperLimitPrice;			//涨停板价
			type::LargeVolume OpenInterest;			//今持仓量
			type::LargeVolume PreOpenInterest;		//昨持仓量
			type::Price	PreClosePrice;				//昨收盘
			type::Price	PreSettlementPrice;			//昨结算
			type::Price	ClosePrice;					//今收盘
			type::Price	SettlementPrice;			//今结算
			type::Time	UpdateTime;
			type::Millisec Millisec;
			type::LargeVolume SequenceNo;
		};
	}
}