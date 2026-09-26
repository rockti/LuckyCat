#pragma once
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

enum class TradePlatform {
	CTP = 0,			// 上期技术
	FEMAS = 1,			//飞马
	HKEX_ESUNNY = 2,			// HK ESUNNY
//	SGE = 3,			// 上海黄金交易直连
	XSPEED = 4,			// 飞创，大连系统
	ZTP = 5,
};

inline TradePlatform fromString(std::string const& platform) {
	if (platform == "CTP") {
		return TradePlatform::CTP;
	}
	if (platform == "HKEX_ESUNNY") {
		return TradePlatform::HKEX_ESUNNY;
	}
	if (platform == "ZTP") {
		return TradePlatform::ZTP;
	}
	return TradePlatform::XSPEED;
}

inline std::string toString(TradePlatform platform) {
	switch (platform)
	{
	case TradePlatform::CTP:
		return "CTP";
	case TradePlatform::FEMAS:
		return "FEMAS";
	case TradePlatform::HKEX_ESUNNY:
		return "HKEX_ESUNNY";
// 	case TradePlatform::SGE:
// 		return "SGE";
	case TradePlatform::XSPEED:
		return "XSPEED";
	case TradePlatform::ZTP:
		return "ZTP";
	default:
		return "CTP";
	}
}

enum class SubSystem {
	Quote,
	Trade,
};
namespace ztp {

	namespace type {
		using ErrorCode = NumberType<int32_t, struct ErrorCodeTag>;
		using InstID = NumberType<uint16_t, struct InstIDTag>;
		using Money = DoubleType<struct MoneyTag>;
		using Price = DoubleType<struct PriceTag>;
		using Ratio = RatioType<struct RatioTag>;
		using Volume = NumberType<uint32_t, struct VolumeTag>;
		using VolumeMultiple = NumberType<uint32_t, struct VolumeMultipleTag>;
		using UnitPrice = DoubleType<struct UnitPriceTag>;
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
		using VarietyCode = StrongCharArray<9, struct VarietyCodeTag>;
		using YearMonth = StrongCharArray<7, struct YearMonthTag>;
		using TradingDay = Date;
		using OrderLocalID = NumberType<uint32_t, struct OrderLocalIDTag>;
		using UserOrderLocalID = StrongCharArray<16, struct UserOrderLocalIDTag>;
		using OrderUniquID = StrongCharArray<21, struct OrderUniquIDTag>;
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
		using InstrumentCode = StrongCharArray<11, struct InstrumentIDTag>;
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
		using Tick = NumberType<uint32_t, struct TickTag>;

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

		enum struct PositionType : char { All = '0', TodayOnly = '1', HistoryOnly = '2' };

		inline bool IsBuy(Direction d) {
			return d == Direction::Buy;
		}

		inline bool IsSell(Direction d) {
			return d == Direction::Sell;
		}

		template<class T>
		inline bool IsBuy(T const& t) {
			return IsBuy(t.Direction);
		}

		template<class T>
		inline bool IsSell(T const& t) {
			return IsSell(t.Direction);
		}


		template<class T>
		inline bool IsBuy(T const* t) {
			return IsBuy(t->Direction);
		}

		template<class T>
		inline bool IsSell(T const* t) {
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
			Unknown = 'X',				//未知
		};

		inline bool IsUnknown(OffsetFlag f) {
			return f == OffsetFlag::Unknown;
		}

		inline bool IsOpen(OffsetFlag f) {
			return f == OffsetFlag::Open;
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
		inline bool IsUnknown(F* t) {
			return IsUnknown(t->OffsetFlag);
		}
		template<class F>
		inline bool IsOpen(F* t) {
			return IsOpen(t->OffsetFlag);
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
			Unknown = 'u',

		};
		enum struct IsActive : char { InActive = '0', Active = '1' };
		enum struct ProductClass : char {
			Unknown	= '0',				//未知市场
			Defer = '1',				//延期市场
			Spot = '2',					//现货市场
			Delivery = '3',				//交割申报
			Neutral = '4',				//中立仓
			Future = '5',				//期货
			Bond = '6',
			Index = '7',
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

		enum struct Exchange : char {
			SHFE = 'S',			// 上海期货
			HKEX = 'H',			// 港交所
			CFFEX = 'C',		// 中金（上海）
			SGE = 'G',			// 上海黄金
			DCE = 'D',			// 大连期货
			CZCE = 'Z',			// 郑州期货
			CME = 'M',			// Chicago
			MT5 = 'T',			// 深圳
		};


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


	struct Error {
		Error() {}
		Error(int code, char const* msg) :Code(code), Msg(msg) {}
		Error(uint32_t code, char const* msg) :Code(code), Msg(msg) {}
		int Code = -1;
		type::ErrorMsg Msg = type::ErrorMsg("系统错误");
		inline bool IsSuccess() const {
			return Code == 0;
		}

		inline operator bool() const {
			return IsSuccess();
		}

		bool operator==(Error const& other) const {
			return Code == other.Code;
		}
		bool operator!=(Error const& other) const {
			return Code != other.Code;
		}
	};
	struct Instrument {
		TradePlatform Platform;
		type::Exchange Exchange;
		type::InstID InstID;
		type::VarietyCode VarityCode;
		type::YearMonth YearMonth;
		type::InstrumentCode Code;
		type::InstrumentName Name;
		type::ProductID ProductID;
		type::ProductClass ProductClass = type::ProductClass::Unknown;
		type::Volume LongPosLimit;				
		type::Volume ShortPosLimit;				
		type::Volume MaxOrderVolume;			
		type::Volume MinOrderVolume;			
		type::VolumeMultiple VolumeMultiple;	
		type::UnitPrice	UnitPrice;				
	};

	struct InvestorMargin {
		TradePlatform Platform;
		type::Exchange Exchange;
		type::BrokerID BrokerID;
		type::InvestorID InvestorID;
		type::VarietyCode VarityCode;
		type::InstID InstID;
		type::InstrumentCode InstrumentCode;
		type::Ratio	LongMarginRate;				//多头占用保证金按比例
		type::Money	LongMarginAmt;				//多头保证金按手数
		type::Ratio	ShortMarginRate;			//空头占用保证金按比例
		type::Money	ShortMarginAmt;				//空头保证金按手数
		bool isOriginal = false;				//是否是原始费（也就是从交易系统查回的，不是使用其它费用代替的）
	};

	struct InvestorFee {
		TradePlatform Platform;
		type::Exchange Exchange;
		type::BrokerID BrokerID;
		type::InvestorID InvestorID;
		type::VarietyCode VarityCode;
		type::InstID InstID;
		type::InstrumentCode InstrumentCode;
		type::Ratio	OpenFeeRate;				//开仓手续费按比例
		type::Money	OpenFeeAmt;					//开仓手续费按手数
		type::Ratio	OffsetFeeRate;				//平仓手续费按比例
		type::Money	OffsetFeeAmt;				//平仓手续费按手数
		type::Ratio	OffsetTodayFeeRate;			//平今仓手续费按比例
		type::Money	OffsetTodayFeeAmt;			//平今仓手续费按手数
		bool isOriginal = false;				//是否是原始费（也就是从交易系统查回的，不是使用其它费用代替的）
	};

	struct InvestorTrade {
		TradePlatform Platform;
		type::Exchange Exchange;
		type::BrokerID BrokerID;
		type::InvestorID InvestorID;
		type::ProductClass ProductClass;
		type::InstID InstID;
		type::InstrumentCode InstrumentCode;
		type::Direction Direction;
		type::OffsetFlag OffsetFlag;
		type::Volume TradeVolume;
		type::Price TradePrice;
		type::Time TradeTime;
		type::TradeID TradeID;
		type::OrderSysID OrderSysID;
		type::OrderLocalID OrderLocalID;
		type::Money	UsedMargin;
		type::Money	UsedFee;

		inline type::Price Total() const {
			return TradePrice * TradeVolume;
		}
	};
	
	struct InvestorOrder {
		TradePlatform Platform;
		type::Exchange Exchange;
		type::BrokerID	BrokerID;
		type::InvestorID InvestorID;
		type::ProductClass ProductClass;
		type::InstID InstID;
		type::InstrumentCode InstrumentCode;
		type::Direction Direction;
		type::OffsetFlag OffsetFlag;
		type::Price	LimitPrice;
		type::Volume Volume;
		type::OrderPriceType OrderPriceType;
		type::OrderSysID OrderSysID;
		type::OrderStatus OrderStatus;
		type::Time InsertTime;
		type::Time CancelTime;
		type::Volume VolumeTrade;
		type::Volume VolumeRemain;
		type::Price TradePrice;
		type::OrderLocalID	OrderLocalID;
		type::OrderLocalID	CancelLocalID;
		type::OrderUniquID UniqueID;
		int Index = 0;
// 		type::Money	FrozenMargin;
// 		type::Money	FrozenFee;
// 		type::Money	TradeMargin;
// 		type::Money	TradeFee;

		Error InsertError;
		Error CancelError;
// 		type::Money Total() const {
// 			return FrozenFee + FrozenMargin + TradeMargin + TradeFee;
// 		}
		type::Volume OnTradeVolume;

		inline bool IsSysIDEmpty() const {
			return OrderSysID.length() == 0;
		}

		inline bool IsNew() const {
			return OrderStatus == type::OrderStatus::New;
		}
		inline bool IsPendingAndNoTrade() {
			return OrderStatus == type::OrderStatus::InsertQueueing
				|| OrderStatus == type::OrderStatus::NoTradeQueueing
				|| OrderStatus == type::OrderStatus::New;

		}
		inline bool IsPending() const {
			return OrderStatus == type::OrderStatus::InsertQueueing
				|| OrderStatus == type::OrderStatus::NoTradeQueueing
				|| OrderStatus == type::OrderStatus::New
				|| OrderStatus == type::OrderStatus::PartTradedQueueing;
		}

		inline bool IsAllTraded() const {
			return OrderStatus == type::OrderStatus::AllTraded;
		}

		inline bool IsCancelled() const{
			return OrderStatus == type::OrderStatus::Canceled ||
				OrderStatus == type::OrderStatus::NoTradeNotQueueing;
		}
		inline bool IsInsertFailed() const {
			return OrderStatus == type::OrderStatus::InsertFail;
		}

		bool OnStatus(type::OrderStatus status) {
			bool result = status != this->OrderStatus;
			if (result) {
				this->OrderStatus = status;
				if (IsCancelled()) {
					this->VolumeRemain = 0;
				}
			}
			return result;
		}

		double TradeTotal() const {
			return this->TradePrice * this->OnTradeVolume;
		}

		void OnTrade(InvestorTrade const& trade) {
			auto oldTotal = TradeTotal();
			this->OnTradeVolume += trade.TradeVolume;
			this->TradePrice = (oldTotal + trade.Total()) / this->OnTradeVolume;
		}
	};


	struct InvestorAccount {
		TradePlatform Platform;
		type::BrokerID BrokerID;
		type::InvestorID InvestorID;
		type::Money	PreBalance;				//上日权益	
		type::Money	PreAvailable;			//上日可用
		type::Money	Balance;				//当前权益
		type::Money	Available;				//当前可用
		type::Money	Margin;					//占用保证金
		type::Money	Fee;					//手续费
		type::Money	CloseProfit;			//平仓盈亏
		type::Money	PositionProfit;			//持仓盈亏
		type::Money	FrozenMargin;			//冻结的保证金
		type::Money	FrozenFee;				//冻结手续费
		type::Money	DynamicRights;			//动态权益
		type::Ratio	Risk;					//风险度

// 		void OnOrder(InvestorOrder const& existedOrder, InvestorOrder const& order) {
// 			FrozenFee -= existedOrder.FrozenFee;
// 			FrozenMargin -= existedOrder.FrozenMargin;
// 			Margin -= existedOrder.TradeMargin;
// 			Fee -= existedOrder.TradeFee;
// 
// 			FrozenFee += order.FrozenFee;
// 			FrozenMargin += order.FrozenMargin;
// 			Margin += order.TradeMargin;
// 			Fee += order.TradeFee;
// 
// 			Available += existedOrder.Total();
// 			Available -= order.Total();
// 			Balance = Available;
// 		}
// 
// 		void OnNewOrder(InvestorOrder const& order) {
// 			FrozenFee += order.FrozenFee;
// 			FrozenMargin += order.FrozenMargin;
// 			Margin += order.TradeMargin;
// 			Fee += order.TradeFee;
// 
// 			Available -= order.Total();
// 			Balance = Available;
// 		}
// 
// 		void OnTrade(InvestorTrade const& trade, type::Money closeProfit) {
// 			CloseProfit += closeProfit;
// 			Available += closeProfit;
// 			Balance = Available;
// 		}
	};

	struct InvestorPosition {
		TradePlatform Platform;
		type::Exchange Exchange;
		type::BrokerID BrokerID;
		type::InvestorID InvestorID;
		type::ProductClass ProductClass;
		type::InstID InstID;
		type::InstrumentCode InstrumentCode;
		type::Direction	Direction;
 		type::Money	UsedMargin;						//占用保证金
		type::Volume Position = 0;					//今持仓量
		type::Price	PositionCost;					//今日持仓成本
		type::Volume YdPosition = 0;				//昨持仓量
		type::Price	YdPositionPrice;				//昨日持仓结算价
// 		type::Money	FrozenMargin;					//开仓冻结保证金
// 		type::Money	FrozenFee;						//开平冻结手续费
 		type::Volume FrozenPosition;				//平今冻结持仓
 		type::Volume FrozenYDPosition;				//平昨冻结持仓
		type::Money PositionProfit;					//持仓盈亏
//		type::Money	CloseProfit;					//平仓盈亏
		type::PositionType PositionType = type::PositionType::All;

		inline bool IsEmpty() const {
			return Position == 0 && YdPosition == 0;
		}
		inline type::Volume TotalVolume() const {
			return Position + YdPosition;
		}

		inline type::Price TotalPositionCost() const {
// 			if (this->Exchange == type::Exchange::SGE) {
// 				return PositionCost;
// 			}
			auto total = Position * PositionCost + YdPosition * YdPositionPrice;
			return total / TotalVolume();
		}
	};
}
