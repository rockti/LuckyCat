#include "stdafx.h"
#include "Database.h"
#include "Logger.h"
#include "ZtpDataStruct.h"
#include "ZtpType.h"
#include "ZTPPackage.h"
#include "ServiceDef.h"
#include <chrono>
#include "Profile.h"

#undef min
#include <refl.hpp>
using namespace ztp;
using namespace std;

struct Table : refl::attr::usage::type {
	const char* name;

	constexpr Table(const char* name) noexcept
		: name(name) {
	}
};

struct UpdateKey : refl::attr::usage::field {
};

REFL_AUTO(type(data::COrder, Table("t_oper_order")),
	field(BrokerID),
	field(OrderOperatorID),
	field(InvestorID),
	field(ExchangeID),
	field(ProductClass),
	field(InstrumentID),
	field(Direction),
	field(OffsetFlag),
	field(LimitPrice),
	field(Volume),
	field(OrderPriceType),
	field(UserOrderLocalID),
	field(OrderSysID),
	field(UserCustom),
	field(OrderStatus),
	field(TradingDay, UpdateKey{}),
	field(SeatID),
	field(ClientID),
	field(InsertTime),
	field(CancelTime),
	field(CancelOperatorID),
	field(VolumeTrade),
	field(VolumeRemain),
	field(OrderLocalID, UpdateKey{}),
	field(ActionLocalID),
	field(UserActionLocalID),
	field(FrozenMargin),
	field(FrozenFee),
	field(TradeMargin),
	field(TradeFee),
	field(InsertErrorCode),
	field(CancelErrorCode),
	field(SequenceNo)
)

REFL_AUTO(type(data::CTrade, Table("t_oper_trade")),
	field(BrokerID),
	field(InvestorID),
	field(ExchangeID),
	field(ProductClass),
	field(InstrumentID),
	field(Direction),
	field(OffsetFlag),
	field(TradeVolume),
	field(TradePrice),
	field(TradeAmt),
	field(TradeDate, UpdateKey{}),
	field(TradeTime),
	field(TradeID, UpdateKey{}),
	field(OrderSysID, UpdateKey{}),
	field(UserOrderLocalID),
	field(ClientID),
	field(OrderOperatorID),
	field(OrderLocalID),
	field(UsedMargin),
	field(UsedFee),
	field(SequenceNo)
)

REFL_AUTO(type(data::CBalanceChange, Table("t_oper_balancechange")),
	field(BrokerID, UpdateKey{}),
	field(InvestorID, UpdateKey{}),
	field(BalanceChangeType),
	field(Amount),
	field(ExchangeSequenceNo),
	field(BalanceSequenceNo, UpdateKey{}),
	field(ChangeTime, UpdateKey{})
)

REFL_AUTO(type(data::CPositionChange, Table("t_oper_positionchange")),
	field(BrokerID, UpdateKey{}),
	field(InvestorID, UpdateKey{}),
	field(ExchangeID),
	field(TradingDay, UpdateKey{}),
	field(InstrumentID, UpdateKey{}),
	field(ProductClass),
	field(Direction),
	field(OffsetFlag),
	field(ChangeType),
	field(Volume),
	field(OrderLocalID),
	field(TradeID),
	field(OrderSysID),
	field(UserOrderLocalID),
	field(ExchangeSequenceNo),
	field(PositionSequenceNo, UpdateKey{}),
	field(NotifyTime)
)


REFL_AUTO(type(data::CSystemStatus, Table("t_sync_systemstatus")),
	field(TradingDay),
	field(SystemStatus),
	field(TradingStatus)
)

REFL_AUTO(type(data::COperatorInfo, Table("t_sync_operatorinfo")),
	field(BrokerID),
	field(OperatorID),
	field(UserName),
	field(Password),
	field(PermissionRight)
)

REFL_AUTO(type(data::COperatorInvestor, Table("t_sync_operatorinvestor")),
	field(BrokerID),
	field(OperatorID),
	field(InvestorID)
)

REFL_AUTO(type(data::CSeatInfo, Table("t_sync_seatinfo")),
	field(AppName),
	field(ExchangeID),
	field(ParticipantID),
	field(SeatID),
	field(TraderID),
	field(Password),
	field(SeatStatus),
	field(FrontAddr),
	field(OrderLocalID)
)

REFL_AUTO(type(data::CInstrument, Table("t_sync_instrument")),
	field(ExchangeID),
	field(InstrumentID),
	field(InstrumentName),
	field(ProductID),
	field(ProductClass),
	field(LongPosLimit),				//多头限仓
	field(ShortPosLimit),				//空头限仓
	field(MaxOrderVolume),				//最大下单量
	field(MinOrderVolume),				//最小下单量
	field(VolumeMultiple),				//数量乘数
	field(PriceTick)					//报价单位		
)

REFL_AUTO(type(data::CInvestorInfo, Table("t_sync_investorinfo")),
	field(BrokerID),
	field(InvestorID),
	field(InvestorName),
	field(Password),
	field(ClientID),
	field(TradeRight),
	field(IsActive),
	field(IsNewAuth),
	field(Salt),
	field(PasswordHash)
)

REFL_AUTO(type(data::CInvestorAccount, Table("t_sync_investoraccount")),
	field(BrokerID),
	field(InvestorID),
	field(TradingDay),
	field(PreBalance),				//上日权益	
	field(PreAvailable),			//上日可用
	field(Balance),					//当前权益
	field(Available),				//当前可用
	field(Deposit),					//入金金额
	field(Withdraw),				//出金金额
	field(Margin),					//占用保证金
	field(Fee),						//手续费
	field(OtherFee),				//其他费用
	field(CloseProfit),				//平仓盈亏
	field(PositionProfit),			//持仓盈亏
	field(FrozenMargin),			//冻结的保证金
	field(FrozenFee),				//冻结手续费
	field(LongMargin),				//多头占用保证金
	field(ShortMargin),				//空头占用保证金
	field(LongFrozenMargin),		//多头冻结的保证金
	field(ShortFrozenMargin),		//空头冻结的保证金
	field(DynamicRights),			//动态权益
	field(Risk)						//风险度)
)

REFL_AUTO(type(data::CInvestorFee, Table("t_sync_investorfee")),
	field(BrokerID),
	field(ExchangeID),
	field(InvestorID),
	field(InstrumentID),
	field(OpenFeeRate),					//开仓手续费按比例
	field(OpenFeeAmt),					//开仓手续费按手数
	field(OffsetFeeRate),				//平仓手续费按比例
	field(OffsetFeeAmt),				//平仓手续费按手数
	field(OffsetTodayFeeRate),			//平今仓手续费按比例
	field(OffsetTodayFeeAmt)			//平今仓手续费按手数
)

REFL_AUTO(type(data::CInvestorMargin, Table("t_sync_investormargin")),
	field(BrokerID),
	field(ExchangeID),
	field(InvestorID),
	field(InstrumentID),
	field(LongMarginRate),				//多头占用保证金按比例
	field(LongMarginAmt),				//多头保证金按手数
	field(ShortMarginRate),				//空头占用保证金按比例
	field(ShortMarginAmt)				//空头保证金按手数
)

REFL_AUTO(type(data::CInvestorPosition, Table("t_sync_investorposition")),
	field(BrokerID),
	field(InvestorID),
	field(TradingDay),
	field(ExchangeID),
	field(InstrumentID),
	field(ProductClass),
	field(Direction),
	field(UsedMargin),				//占用保证金
	field(Position),				//今持仓量
	field(PositionCost),			//今日持仓成本
	field(YdPosition),				//昨持仓量
	field(YdPositionPrice),			//昨日持仓结算价
	field(FrozenMargin),			//冻结的保证金
	field(FrozenFee),				//开平冻结手续费
	field(FrozenPosition)			//平仓冻结持仓
)

template<typename T>
constexpr auto select_all_sql() {
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));


	constexpr auto fields = accumulate(TD::members,
		[](auto acc, auto member) {
			return acc + ", " + member.name;
		}, make_const_string())
		.template substr<1>(); // remove the initial ","

		constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
		// convert the const char* data member to a refl::const_string<N>
		// (necessary to be able to do compile-time string concat)
		constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);
		return "select" + fields + " from " + tbl_name;
}

namespace {
	/* calculate absolute value */
	constexpr int abs_val(int x) {
		return x < 0 ? -x : x;
	}

	/* calculate number of digits needed, including minus sign */
	constexpr int num_digits(int x) {
		return x < 0 ? 1 + num_digits(-x) : x < 10 ? 1 : 1 + num_digits(x / 10);
	}

	/* metaprogramming string type: each different string is a unique type */
	template<char... args>
	struct metastring {
		const char data[sizeof... (args)] = { args... };
	};

	/* recursive number-printing template, general case (for three or more digits) */
	template<int size, int x, char... args>
	struct numeric_builder {
		typedef typename numeric_builder<size - 1, x / 10, '0' + abs_val(x) % 10, args...>::type type;
	};

	/* special case for two digits; minus sign is handled here */
	template<int x, char... args>
	struct numeric_builder<2, x, args...> {
		typedef metastring < x < 0 ? '-' : '0' + x / 10, '0' + abs_val(x) % 10, args...> type;
	};

	/* special case for one digit (positive numbers only) */
	template<int x, char... args>
	struct numeric_builder<1, x, args...> {
		typedef metastring<'0' + x, args...> type;
	};

	/* convenience wrapper for numeric_builder */
	template<int x>
	class numeric_string {
	private:
		/* generate a unique string type representing this number */
		typedef typename numeric_builder<num_digits(x), x, '\0'>::type type;

		/* declare a static string of that type (instantiated later at file scope) */
		static constexpr type value{};

	public:
		/* returns a pointer to the instantiated string */
		static constexpr auto& get() {
			return value.data;
		}
	};

	/* instantiate numeric_string::value as needed for different numbers */
	template<int x>
	constexpr typename numeric_string<x>::type numeric_string<x>::value;

}

template<typename T>
constexpr auto insert_sql() {
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));

	constexpr auto fields = accumulate(TD::members,
		[](auto acc, auto member) {
			return acc + ", " + member.name;
		}, make_const_string())
		.template substr<1>(); // remove the initial ","
	constexpr auto valueParams = accumulate(TD::members,
		[](auto acc, auto member) {
			return acc + ", :" + numeric_string<member.index+1>::get() + " ";
		}, make_const_string())
		.template substr<1>(); // remove the initial ","

	constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
	// convert the const char* data member to a refl::const_string<N>
	// (necessary to be able to do compile-time string concat)
	constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);
	return "insert into " + tbl_name + " (" + fields + ")" + " values(" + valueParams + ")" ;
}


template<uint8_t size, typename Tag>
void fill(typename StrongCharArray<size,Tag>& target, SAField const& column) {
	if (!column.isNull()) {
		target = column.asString();
	}
}

template<class BaseT, typename Tag>
void fill(typename NumberType<BaseT, Tag>& target, SAField const& column) {
	if (!column.isNull()) {
		target = column.asLong();
	}
}

template<typename Tag>
void fill(typename DoubleType<Tag>& target, SAField const& column) {
	if (!column.isNull()) {
		target = column.asDouble();
	}
}

template<typename Tag>
void fill(typename RatioType<Tag>& target, SAField const& column) {
	if (!column.isNull()) {
		target = column.asDouble();
	}
}

template<class E>
void fill(E& target, SAField const& column) {
	if (!column.isNull()) {
		target = E(column.asLongChar()[0]);
	}
}



template<typename T>
void print(T const& t) {
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));
	constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
	fmt::print("Table:{} ", tbl.name);
	for_each(TD::members, [&](auto member) {
		fmt::print("{}:[{}]", member.name.str(), member(t));
		});
	fmt::print("\n");
}

template <bool Condition, typename T>
struct if_t {
	using type = T;
};

template <typename T>
struct if_t<true, T> {
	using type = std::underlying_type_t<T>;
};

template <typename T,
	typename X= typename if_t<std::is_enum<T>::value, T>::type>
auto bar(T const& t) {
	return X(t);
}

template<uint8_t size, class Tag>
ostream& operator<<(ostream& os, StrongCharArray<size, Tag> const& v) {
	os << value(v);
	return os;
}

template<class Base, class Tag>
ostream& operator<<(ostream& os, NumberType<Base, Tag> const& v) {
	os << value(v);
	return os;
}


template<class Tag>
ostream& operator<<(ostream& os, DoubleType<Tag> const& v) {
	os << value(v);
	return os;
}


template<typename T>
void log(std::string const& prefix, T const& t, char const* errorMsg = nullptr) {
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));
	constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
	stringstream ss;
	ss << prefix << "Table: " <<  tbl.name;
	for_each(TD::members, [&](auto member) {
		ss <<  member.name.str() << "=[" << bar(member(t)) << "]";
		});
	if (errorMsg) {
		ss << "error msg =[" << errorMsg << "]";
	}
	Log::Info(ss.str());
}

template<typename T, typename F>
void LoadTable(SAConnection& conn, F f) {
	using namespace refl;

	using Td = type_descriptor<T>;
	constexpr auto sql = select_all_sql<T>();

	SACommand cmd(&conn);

	cmd.setCommandText(sql.str().c_str());
	cmd.Execute();
	while (cmd.FetchNext()) {
		T row;
		for_each(refl::reflect(row).members, [&](auto member) {
			auto& value = member(row);
			fill(value, cmd[member.name.str().c_str()]);
			});
		f(row);
	}
}

template<int length>
void Param(SACommand& cmd, int i, CStringType<length> const& v) {
	cmd.Param(i).setAsString() = v;
}

void Param(SACommand& cmd, int i, CIntType const& v) {
	cmd.Param(i).setAsLong() = v;
}

void Param(SACommand& cmd, int i, CLongType const& v) {
	cmd.Param(i).setAsLong() = v;
}

void Param(SACommand& cmd, int i, CCharType const& v) {
	char value[2] = "";
	value[0] = v;
	cmd.Param(i).setAsString() = value;
}

template<int l, int p>
void Param(SACommand& cmd, int i, CDoubleType<l,p> const& v) {
	cmd.Param(i).setAsDouble() = v;
}


template<uint8_t size, class Tag>
void Param(SACommand& cmd, int i, StrongCharArray<size,Tag> const& v) {
	cmd.Param(i).setAsString() = v.value();
}

template<class Base, class Tag>
void Param(SACommand& cmd, int i, NumberType<Base,Tag> const& v) {
	cmd.Param(i).setAsLong() = v.value();
}

template<class T>
void Param(SACommand& cmd, int i, T const& v) {
	bool isenum = std::is_enum<T>::value;
	if (isenum) {
		char value[2] = "";
		value[0] = char(v);
		cmd.Param(i).setAsString() = value;
	} else {
		throw 1;
	}
}

template<class Tag>
void Param(SACommand& cmd, int i, DoubleType<Tag> const& v) {
	cmd.Param(i).setAsDouble() = v.value();
}


template<class T>
void Insert(SAConnection& conn, T const& v) {
	using namespace refl;

	using Td = type_descriptor<T>;
	constexpr auto sql = insert_sql<T>();

	SACommand cmd(&conn);

	cmd.setCommandText(sql.str().c_str());
	int i = 1;
	for_each(Td::members, [&](auto member) {
		Param(cmd, i++, member(v));
		});
//	Log::Info(fmt::format("insert sql =[{}]",sql.str()));
	cmd.Execute();
}

template<class T>
std::string const& has_sql(T const& v) {
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));
	constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
	constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);
	static std::string s_sql;

	if (s_sql.empty()) {

		int i = 1;
		stringstream ss;
		ss << "select count(*) from " << tbl_name.str() << " where ";
		bool isFirst = true;
		for_each(TD::members,
			[&](auto member) {
				if (descriptor::has_attribute<UpdateKey>(member)) {
					if (!isFirst) {
						ss << " and ";
					}
					isFirst = false;
					ss << member.name.str() << "=:" << i++;
				}
			}
		);
		s_sql = ss.str();
	}

	return s_sql;
}


template<class T>
bool Has(SAConnection& conn, T const& v) {
	using namespace refl;
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));

	auto sql = has_sql(v);
 	SACommand cmd(&conn);
 	cmd.setCommandText(sql.c_str());
	int i = 1;
	for_each(TD::members, [&](auto member) {
		if (descriptor::has_attribute<UpdateKey>(member)) 
			Param(cmd, i++, member(v));
		});

 	cmd.Execute();

	bool result = cmd.FetchNext();
	auto count = cmd.Field(1).asLong();
//	Log::Info(fmt::format("has=[{}], sql=[{}]", count, sql));
	return count > 0;
}

template<class T>
std::string const& update_sql(T const& v) {
	using namespace refl;
	using TD = type_descriptor<T>;
	static_assert(descriptor::has_attribute<Table>(TD{}));
	constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
	constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);
	static std::string s_sql;
	if (s_sql.empty()) {
		int i = 1;
		stringstream ss;
		ss << "update " << tbl_name.str() + " ";
		bool isFirst = true;
		for_each(TD::members,
			[&](auto member) {
				if (!descriptor::has_attribute<UpdateKey>(member)) {
					if (!isFirst) {
						ss << " , ";
					} else {
						ss << " set ";
					}
					isFirst = false;
					ss << member.name.str() << "=:" << i++;
				}
			}
		);
		ss << " where ";
		isFirst = true;
		for_each(TD::members,
			[&](auto member) {
				if (descriptor::has_attribute<UpdateKey>(member)) {
					if (!isFirst) {
						ss << " and ";
					}
					isFirst = false;
					ss << member.name.str() << "=:" << i++;
				}
			}
		);
		s_sql = ss.str();
	}

	return s_sql;
}


template<class T>
void Update(SAConnection& conn, T const& v) {
	using namespace refl;

	using Td = type_descriptor<T>;
	auto sql = update_sql(v);
	int i = 1;

	SACommand cmd(&conn);
	cmd.setCommandText(sql.c_str());

	for_each(Td::members, [&](auto member) {
		if (!descriptor::has_attribute<UpdateKey>(member)) {
			Param(cmd, i++, member(v));
		}
		});
	for_each(Td::members, [&](auto member) {
		if (descriptor::has_attribute<UpdateKey>(member)) {
			Param(cmd, i++, member(v));
		}
		});
//	Log::Info(fmt::format("update sql=[{}]", sql));
	cmd.Execute();
}

#include "Profile.h"

Database::Database(IFlow* flow, SystemStatusRepository& systemStatusRepo) :
	flow_(flow), systemStatusRepo_(systemStatusRepo) {
	m_flowReader.AttachFlow(flow_,0);
	m_pSeriesFlow = new CSeriesCachedFlow(false, 0x7fffffff, 0x100000);
	m_seriesReader.AttachFlow(m_pSeriesFlow, 0);

}
Profile& theProfile();

bool Database::Connect() {
	std::string dbName = theProfile().GetPrivateProfile("Database", "DbName", "");
	std::string dbUser = theProfile().GetPrivateProfile("Database", "DbUser", "");
	std::string dbPwd = theProfile().GetPrivateProfile("Database", "DbPwd", "");

	try{
		con_.setOption("CharacterSet") = "gbk";
		con_.Connect(_TSA(dbName.c_str()), _TSA(dbUser.c_str()), _TSA(dbPwd.c_str()), SA_MySQL_Client);
		return true;
	}
	catch (SAException& e) {
		println("connect database error, error msg [{}]", e.ErrText().GetMultiByteChars());
		exit(-1);
		return false;
	}
}

void Database::Release() {
	con_.Disconnect();
}

bool Database::IsConnected() const {
	return con_.isConnected();
}

void Database::KeepAlive() {
	SACommand cmd(&con_, "select * from t_sync_systemstatus");
	try {
		cmd.Execute();
		while (cmd.FetchNext());
		printNlog("database keep alive");
	}
	catch (SAException&) {
		con_.Disconnect();
	}
}

template<class T>
void Database::InternalAddOrUpdate(T const& v) {
	if (!IsConnected()) {
		Release();
		Connect();
	}
	try {
		if (!Has(con_, v)) {
			Insert(con_, v);
			log("Add", v);
		} else {
			::Update(con_, v);
			log("Update", v);
		}
	}
	catch (SAException & x) {
		using namespace refl;
		using TD = type_descriptor<T>;
		constexpr auto tbl = descriptor::get_attribute<Table>(TD{});
		constexpr auto tbl_name = REFL_MAKE_CONST_STRING(tbl.name);
		log("AddOrUpdate", v, x.ErrText().GetMultiByteChars());
	}
}


template<class T>
void Database::InternalUpdate(T const& v) {
	if (!IsConnected()) {
		Release();
		Connect();
	}
	try {
		::Update(con_, v);
	}
	catch (SAException & x) {
		log("Update", v, x.ErrText().GetMultiByteChars());
	}
}


template<class T>
void Database::InternalAdd(T const& v) {
	if (!IsConnected()) {
		Release();
		Connect();
	}
	try {
		Insert(con_, v);
	}
	catch (SAException & x) {
		log("Add", v, x.ErrText().GetMultiByteChars());
	}
}


template<typename IP>
void Database::PushData(typename IP::Type const& v) {
	m_loadingPackage.PreparePublish(IP::ID);
	m_loadingPackage.AddField(v);
	m_loadingPackage.MakePackage();
	m_pSeriesFlow->Append(m_loadingPackage.GetAddress(), m_loadingPackage.GetLength());
}

template<uint32_t FuncID, typename Type>
void Database::LoadAndPushTable(SAConnection& conn, internal_protocol<FuncID, Type> p) {
	auto f = [this](Type const& v) {
//		print(v);
		PushData<decltype(p)>(v);
	};
	LoadTable<Type>(conn, f);
}

void Database::Load() {
	try {
		{
			m_loadingPackage.PreparePublish(ztp::TID_DataSyncStart, CHAIN_LAST);
			m_loadingPackage.MakePackage();
			m_pSeriesFlow->Append(m_loadingPackage.GetAddress(), m_loadingPackage.GetLength());
		}

 		LoadTable<data::CSystemStatus>(con_, [&](data::CSystemStatus& v) {
			v.SystemStatus = type::SystemStatus::Start;
 			systemStatusRepo_.Update(v);
 			});
		LoadAndPushTable(con_, Internal::InitSeatInfo{});
		LoadAndPushTable(con_, Internal::InitInstrument{});
		LoadAndPushTable(con_, Internal::InitInvestorInfo{});
		LoadAndPushTable(con_, Internal::InitInvestorAccount{});
		LoadAndPushTable(con_, Internal::InitInvestorFee{});
		LoadAndPushTable(con_, Internal::InitInvestorMargin{});
		LoadAndPushTable(con_, Internal::InitInvestorPosition{});
		LoadAndPushTable(con_, Internal::InitOperatorInfo{});
		LoadAndPushTable(con_, Internal::InitOperatorInvestor{});
		LoadAndPushTable(con_, Internal::InitBalanceChange{});
		LoadAndPushTable(con_, Internal::InitPositionChange{});
		LoadAndPushTable(con_, Internal::InitOrder{});
		LoadAndPushTable(con_, Internal::InitTrade{});

		{
			m_loadingPackage.PreparePublish(ztp::TID_DataSyncEnd, CHAIN_LAST);
			m_loadingPackage.MakePackage();
			m_pSeriesFlow->Append(m_loadingPackage.GetAddress(), m_loadingPackage.GetLength());
		}
	}
	catch (SAException & x) {
		println("load table error, error msg [{}]", x.ErrText().GetMultiByteChars());
		exit(-1);
	}
}

void Database::Add(data::COrder const& order) {
	InternalAddOrUpdate(order);
}

void Database::AddOrUpdate(data::COrder const& order) {
	InternalAddOrUpdate(order);
}

void Database::Update(data::COrder const& order) {
	InternalAddOrUpdate(order);
}

void Database::Add(data::CTrade const& trade) {
	InternalAddOrUpdate(trade);
}

void Database::Add(data::CBalanceChange const& balanceChange) {
	InternalAddOrUpdate(balanceChange);
}

void Database::Add(data::CPositionChange const& positionChange) {
	InternalAddOrUpdate(positionChange);
}

void Database::HandleMemberDBMsg(CZTPPackage* pPackage) {
	CFieldTypeIterator iter = pPackage->GetFieldTypeIterator();
	while (!iter.IsEnd()) {
		data::CMemTableChange field;
		iter.Retrieve(field);
		iter.Next();

		TFieldHeader fieldHead;
		iter.RetrieveHeader(fieldHead);
		ProcField(fieldHead.FieldID, iter, field.MemtableChangeType);
		iter.Next();
	}
}

void Database::ProcField(WORD fid, CFieldTypeIterator& iter, type::MemTableChangeType actType) {
	switch (fid) {
	case data::COrder::ID: {
		data::COrder field;
		iter.Retrieve(field);
		if (actType == type::MemTableChangeType::Insert) {
			Add(field);
		} else if (actType == type::MemTableChangeType::Update) {
			Update(field);
		}
		break;
	}
	case data::CTrade::ID: {
		data::CTrade field;
		iter.Retrieve(field);
		Add(field);
		break;
	}
	case data::CBalanceChange::ID:{
		data::CBalanceChange field;
		iter.Retrieve(field);
		Add(field);
		break;
	}
	case data::CPositionChange::ID: {
		data::CPositionChange field;
		iter.Retrieve(field);
		Add(field);
		break;
	}
	default:
		break;
	}
}

bool Database::ChangePassword(type::InvestorID investor, type::Hash newHash) {
	auto sql = fmt::format("update t_sync_investorinfo set PasswordHash = '{}', IsNewAuth = '1' where InvestorID = '{}'",
		newHash, investor);
	SACommand cmd(&con_);
	cmd.setCommandText(sql.c_str());

	cmd.Execute();
	return true;
}

bool Database::InitInstance() {
	Connect();
	m_publishPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
	m_loadingPackage.ConstructAllocate(ZTP_PACKAGE_MAX_SIZE, 1000);
	Load();
	m_last = chrono::system_clock::now();;
	return true;
}

void Database::ExitInstance() {
	Release();
}

bool Database::HandleOtherTask() {
	for (int i = 0; i < 100; ++i) {
		if (!m_flowReader.GetNextUseNoBuf(&m_publishPackage)) {
			auto now = chrono::system_clock::now();
			if (chrono::duration_cast<chrono::minutes>(now - m_last) >= chrono::minutes(10)) {
				KeepAlive();
				m_last = now;
			} 
			return CSelectThread::HandleOtherTask();
		}
		HandleMemberDBMsg(&m_publishPackage);
	}
	return true;
}
