#pragma once
#include "model/dealing/Dealing.h"
#include "model/Quote.h"
#include "config/Config.h"
#include <QString.h>
#include "domain/Error.h"
#include <optional>
#include "notify/Notify.h"
#include "strategy/auto/Context.h"
#include "strategy/auto/StatusMachine.h"

#define BOOST_NO_MT
#include <boost/pool/pool_alloc.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>

#include <tuple>
#include <set>

typedef boost::container::deque_options<boost::container::block_size<128u>>::type block_128_option_t;

template<class T>
using Deque = boost::container::deque<T, void, block_128_option_t>;

template<class Key, class Value>
using MultipleIndex = std::multimap<Key, Value*, std::less<Key>, boost::fast_pool_allocator<std::pair<const Key, Value*>>>;

template<class Key, class Value>
using SingleIndex = std::map<Key, Value*, std::less<Key>, boost::fast_pool_allocator<std::pair<const Key, Value*>>>;



template<class T>
struct Result {
	// default is error
	Result() :Error(SystemError) {}
	// error result, no value
	Result(Error error) : Error(error) {
		if (error == NoError) {
			throw std::logic_error("");
		}
	}
	// success result
	Result(T const& t) : Error(NoError), Value(t) {}

	T* operator->() {
		if (!IsSuccess()) {
			throw std::logic_error("");
		}
		return &Value.value();
	}

	inline bool IsSuccess() const {
		return Error.IsSuccess();
	}

	operator bool() const {
		return IsSuccess();
	}

	Error Error;
	std::optional<T> Value;
};

template<class Key, class Value, class Func>
void IndexForEach(MultipleIndex<Key, Value> const& index, Key const& key, Func func) {
	auto [begin, end] = index.equal_range(key);
	MultipleIndex<Key, Value>::mapped_type const pre = nullptr;

	for (auto iter = begin; iter != end; ++iter) {
		func(iter->second);
	}
}

template<class Key, class Value>
Result<Value> GetValue(SingleIndex<Key, Value> const& index, Key const& key) {
	auto iter = index.find(key);
	if (iter != index.end()) {
		return { *iter->second };
	}
	return { NotFoundError };
}

using OrderResult = Result<InvestorOrder>;
using InstrumentResult = Result<Instrument>;
using FeeResult = Result<InvestorFee>;
using MarginResult = Result<InvestorMargin>;
using PositionResult = Result<InvestorPosition>;
using AccountResult = Result<InvestorAccount>;
using TradeResult = Result<InvestorTrade>;
using QuoteResult = Result<Quote>;

template<class T>
struct QName {
	T Value;
	QString Name;
};

template<class T>
using QNames = std::vector<QName<T>>;

class NameService {
public:

	static QString Name(TradePlatform platform) {
		return "";
	}
	using ExchangeNames = QNames<type::Exchange>;
	static ExchangeNames ExchangeNameList() {
		return ExchangeNames{};
	}

	static QString Name(type::Exchange exchange) {
		return "";
	}
	using OrderStatusNames = QNames<type::OrderStatus>;
	static OrderStatusNames OrderStatusNameList() {
		return OrderStatusNames{};
	}

	static QString Name(type::OrderStatus orderStatus);

	using ExhangeStatusNames = QNames<ExchangeStatus>;
	static ExhangeStatusNames ExchangeStatusNameList() {
		return ExhangeStatusNames{};
	}
	static QString Name(ExchangeStatus exchangestatus);

	using DirectionNames = QNames<type::Direction>;
	static DirectionNames DirectionNameList() {
		return DirectionNames{};
	}
	static QString Name(type::Direction direction);
	static QString Name(type::OrderPriceType type);
	static QString Name(type::OffsetFlag operation);

	static QString Name(automate::PriceMode priceMode);
	static QString Name(automate::StrategyMode strategyMode);
	static QString Name(automate::RunningStatus status);

};

enum class TradeSystemStatus {
	Disconnected,
	Connected,
	Logined,
	Ready,
};

class ITradeSystemObserver {
public:
	virtual void OnConnected(TradePlatform platform, SubSystem subSystem) = 0;
	virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) = 0;

	virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) = 0;
	virtual void OnReady(TradePlatform platform, SubSystem subSystem) = 0;
	virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) = 0;
	virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) = 0;
	virtual void OnOrderError(OrderErrorNotification const& orderError) = 0;
	virtual void OnError(ErrorNotification const& error) = 0;
};


class IOrderObserver {
public:
	virtual void OnAdd(const InvestorOrder& order) = 0;
	virtual void OnChange(const InvestorOrder& dealing, int errorCode = 0) = 0;
};

class ITradeObserver {
public:
	virtual void OnAdd(const InvestorTrade& trade) = 0;
};

class IExchangeStatusObserver {
public:
	virtual void OnExchangeStatus(type::Exchange exchange, ExchangeStatus status) = 0;
};

class IPositionObserver {
public:
	virtual void OnAdd(InvestorPosition const& position) = 0;
	virtual void OnChange(InvestorPosition const& position) = 0;
	virtual void OnDelete(InvestorPosition const& position) = 0;
	virtual void OnPositionProfit(TradePlatform platform, double positionProfit) = 0;
};

class IAccountObserver {
public:
	virtual void OnChange(InvestorAccount const& fund) = 0;
};

class IQuoteObserver {
public:
	virtual void OnQuote(Quote const& quote) = 0;
};

class IInstrumentObserver {
public:
	virtual void OnAdd(Instrument const& inst) = 0;
};

enum class ArbitrageStatus {
	New,
	Start,
	PartialTrade,
	AllTrade,
	Cancelled,
	Failed,
};

using ArbitrageID = int;

struct Arbitrage {
	ArbitrageID Id;
	type::InstID FirstLeg;
	type::InstID SecondLeg;
};

class IArbitrageObserver {
public:
	virtual void OnStatus(ArbitrageID id, ArbitrageStatus status) = 0;
};

class IExchangeService {
public:
	virtual TradePlatform toPlatform(type::Exchange exchange) = 0;
};

using InstrumentResults = std::vector<Instrument>;

class IInstrumentService {
public:
	using Fees = std::vector<InvestorFee>;
	using Margins = std::vector<InvestorMargin>;
	using Varieties = std::set<type::VarietyCode>;

	virtual void Register(IInstrumentObserver* observer) = 0;
	virtual void Unregister(IInstrumentObserver* observer) = 0;

	virtual Instrument const& Add(Instrument const& inst) = 0;
	virtual void AddFee(TradePlatform platform, InvestorFee const& fee) = 0;
	virtual void AddMargin(TradePlatform platform, InvestorMargin const& margin) = 0;

	virtual void AddVarietyFeeByPresentInstrument(TradePlatform platform, type::VarietyCode const& varityCode, InvestorFee const& fee) = 0;
	virtual void AddVarietyMarginByPresentInstrument(TradePlatform platform, type::VarietyCode const& varityCode, InvestorMargin const& fee) = 0;
	virtual void AddVarietyFee(TradePlatform platform, type::VarietyCode const& varityCode, InvestorFee const& fee) = 0;
	virtual void AddVarietyMargin(TradePlatform platform, type::VarietyCode const& varityCode, InvestorMargin const& fee) = 0;

	virtual type::InstID toID(type::InstrumentCode const& instCode) = 0;
	virtual type::InstrumentCode const& toCode(type::InstID instCode) = 0;

	virtual InstrumentResult Get(type::InstID instID) = 0;
	virtual InstrumentResult Get(type::InstrumentCode instCode) = 0;
	virtual InstrumentResults GetAll(TradePlatform platform) = 0;
	virtual InstrumentResults GetAll() = 0;
	virtual InstrumentResults GetAll(type::VarietyCode const& varietyCode) = 0;
	virtual InstrumentResults GetSortedAll() = 0;
	virtual InstrumentResults GetSortedInstrumentsExcept(type::InstrumentCode const& code) = 0;
	virtual InstrumentResults GetHotInstruments(TradePlatform platform) = 0;
	virtual FeeResult GetFee(type::InstID instID) = 0;
	virtual FeeResult GetFee(type::InstrumentCode instCode) = 0;
	virtual MarginResult GetMargin(type::InstID instID) = 0;
	virtual MarginResult GetMargin(type::InstrumentCode instCode) = 0;
	
	void SetVarieties(TradePlatform platform, Varieties const& codes) {
		varieties_[platform] = codes;
	}
	bool HasVariety(TradePlatform platform, type::VarietyCode const& code) const {
		auto iter = varieties_.find(platform);
		if (iter != varieties_.end()) {
			Varieties const& vs = iter->second;
			return vs.find(code) != vs.end();
		}
		return false;
	}
private:

	std::map<TradePlatform, Varieties> varieties_;
};

class ITradeSystemService {
public:
	virtual void Register(IExchangeStatusObserver* observer) = 0;
	virtual void Unregister(IExchangeStatusObserver* observer) = 0;

	virtual void Register(ITradeSystemObserver* observer) = 0;
	virtual void Unregister(ITradeSystemObserver* observer) = 0;

	virtual void Login(TradePlatform platform, type::InvestorID const& investor, type::Password const& password) = 0;
	virtual void Login(TradePlatform platform, type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
		type::InvestorID const& quoteInvestor, type::Password const& quotePassword) = 0;
	virtual void ReLogin(TradePlatform platform, SubSystem subSystem) = 0;
	virtual bool IsConnected(TradePlatform platform, SubSystem subSystem) const = 0;
	virtual bool IsLogined(TradePlatform platform, SubSystem subSystem) const = 0;
	virtual void ChangePassword(TradePlatform platform, type::Password const& oldPassword, type::Password const& newPassword) = 0;

	virtual void Insert(OrderInsertRequest const& order) = 0;
	virtual void Cancel(OrderCancelRequest const& action) = 0;
	virtual void SubscribeQuote(type::Exchange exchange, type::InstrumentCode const& code) = 0;
	virtual void RunQuery() = 0;

	virtual void QueryInstruments(TradePlatform platform) = 0;
	virtual void QueryAccount(TradePlatform platform) = 0;
	virtual void QueryPositions(TradePlatform platform) = 0;
	virtual void QueryOrders(TradePlatform platform) = 0;
	virtual void QueryTrades(TradePlatform platform) = 0;
	virtual void QueryFees(TradePlatform platform) = 0;
	virtual void QueryMargins(TradePlatform platform) = 0;
	virtual void QueryFee(TradePlatform platform, type::InstrumentCode const& code) = 0;
	virtual void QueryMargin(TradePlatform platform, type::InstrumentCode const& code) = 0;
	virtual void OnConnected(TradePlatform platform,SubSystem subSystem) = 0;
	virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) = 0;
	virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) = 0;
	virtual void OnLogout(TradePlatform platform, SubSystem subSystem) = 0;
	virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) = 0;
	virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) = 0;
	virtual void OnStatus(type::Exchange, ExchangeStatus status) = 0;

	virtual void OnOrderError(OrderErrorNotification const& en) = 0;
	virtual void OnError(ErrorNotification const& en) = 0;

	virtual type::TradingDay const& GetTradingDay() const = 0;
	virtual void SetTradingDay(type::TradingDay const& tradingDay) = 0;
};

using Orders = Deque<InvestorOrder>;

class IOrderService {
public:
	virtual void Register(IOrderObserver* observer) = 0;
	virtual void Unregister(IOrderObserver* observer) = 0;
	virtual void InitAdd(InvestorOrder const& order) = 0;
	virtual InvestorOrder const& AddOrUpdate(InvestorOrder const& order) = 0;
	virtual OrderResult GetOrder(type::OrderUniquID localID) const = 0;
	virtual OrderResult GetOrder(type::OrderLocalID localID) const = 0;
	virtual OrderResult GetOrder(type::OrderSysID const& sysNo) const = 0;
	virtual void OnTrade(InvestorTrade const& trade) = 0;
	virtual Orders GetAll() const = 0;
};

using Trades = std::vector<InvestorTrade>;

class ITradeService {
public:
	virtual void Register(ITradeObserver* observer) = 0;
	virtual void Unregister(ITradeObserver* observer) = 0;
	virtual InvestorTrade const& Add(InvestorTrade const& trade) = 0;
	virtual Trades GetTrades(type::OrderLocalID localID) const = 0;
	virtual Trades GetTrades(type::OrderSysID const& sysNo) const = 0;
	//TradeID在自成交情况下不唯一，需要加上系统号来标注唯一
	virtual TradeResult GetTrade(type::OrderSysID const& sysNo, type::TradeID const& tradeID) const = 0; 

};

struct InstrumentPosition {
	InvestorPosition BuyPosition;
	InvestorPosition SellPosition;
};


using InstrumentPositionResult = Result<InstrumentPosition>;
bool PositionHasOffset(type::InstID instID, type::Exchange exchange);
class IPositionService {
public:
	using Positions = std::vector<InvestorPosition>;

	virtual void Register(IPositionObserver* observer) = 0;
	virtual void Unregister(IPositionObserver* observer) = 0;

	virtual InvestorPosition const& InitAdd(InvestorPosition const& position) = 0;
	virtual PositionResult Get(type::InstID inst, type::Direction direction) const = 0;
	virtual bool Has(type::InstID inst, type::Direction direction) const = 0;
	virtual InstrumentPositionResult Get(type::InstID inst) = 0;
	virtual Positions All() const = 0;
	virtual void ClearAll() = 0;
	virtual void ClearPlatform(TradePlatform platform) = 0;
	virtual void OnQuote(Quote const& quote) = 0;
	virtual void OnTrade(InvestorTrade const& trade) = 0;
	virtual void FreezePosition(InvestorOrder const& order) = 0;
	virtual void UnfreezePosition(InvestorOrder const& order) = 0;
	virtual void OnOrderRtn(InvestorOrder const& order) = 0 ;
	virtual void OnInitOrder(InvestorOrder const& order) = 0;

};

class IAccountService {
public:
	virtual void Register(IAccountObserver* observer) = 0;
	virtual void Unregister(IAccountObserver* observer) = 0;
	virtual void Update(TradePlatform platform, InvestorAccount const& account) = 0;
	virtual AccountResult Get(TradePlatform platform) = 0;
};

class IQuoteService {
public:
	virtual void Register(IQuoteObserver* observer) = 0;
	virtual void Unregister(IQuoteObserver* observer) = 0;

	virtual Quote const& Update(Quote const& quote) = 0;
	virtual Quote const& UpdatePrice(type::InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, type::Money lastPrice) = 0;
	virtual QuoteResult Get(type::InstID inst) const = 0;
};

class IArbitrageService {
public:
	virtual void Register(IArbitrageObserver* observer) = 0;
	virtual void Unregister(IArbitrageObserver* observer) = 0;

	virtual void AddArbitrage(Arbitrage const& arbitrage) = 0;
};

class ICancellationLimiter {
public:
	virtual void InitLimitation(TradePlatform platform, int max, int start) = 0;
	virtual void CountCancellation(TradePlatform platform) = 0;
};

template<class Observer, class Base>
class ServiceBase : public Base {
public:
	virtual void Register(Observer* observer) override {
		observers_.push_back(observer);
	}

	virtual void Unregister(Observer* observer) override {
		auto iter = observers_.begin();
		for (; iter != observers_.end(); ++iter) {
			if (*iter == observer) {
				break;
			}
		}
		if (iter != observers_.end()) {
			observers_.erase(iter);
		}
	}
protected:
	std::vector<Observer*> observers_;
};

template<class Observer, class Observer2, class Base>
class ServiceBase2 : public Base {
public:
	virtual void Register(Observer* observer) override {
		observers_.push_back(observer);
	}
	virtual void Unregister(Observer* observer) override {
		auto iter = observers_.begin();
		for (; iter != observers_.end(); ++iter) {
			if (*iter == observer) {
				break;
			}
		}
		if (iter != observers_.end()) {
			observers_.erase(iter);
		}
	}
	virtual void Register(Observer2* observer) override {
		observers2_.push_back(observer);
	}
	virtual void Unregister(Observer2* observer) override {
		auto iter = observers2_.begin();
		for (; iter != observers2_.end(); ++iter) {
			if (*iter == observer) {
				break;
			}
		}
		if (iter != observers2_.end()) {
			observers2_.erase(iter);
		}
	}
protected:
	std::vector<Observer*> observers_;
	std::vector<Observer2*> observers2_;
};

template<class Observer, class Observer2, class Observer3, class Base>
class ServiceBase3 : public ServiceBase2<Observer,Observer2, Base> {
public:
	virtual void Register(Observer3* observer) override {
		observers3_.push_back(observer);
	}
	virtual void Unregister(Observer3* observer) override {
		auto iter = observers3_.begin();
		for (; iter != observers3_.end(); ++iter) {
			if (*iter == observer) {
				break;
			}
		}
		if (iter != observers3_.end()) {
			observers3_.erase(iter);
		}
	}

protected:
	std::vector<Observer3*> observers3_;
};

// CTP has rate limit for query requests. Only can query every seconds. This may change later
class IQueryService {
public:
	virtual void TriggerQueryAccount(TradePlatform platform) = 0;
	virtual void TriggerQueryPosition(TradePlatform platform) = 0;
};


class IInitProgressObserver {
public:
	virtual void OnProgress(TradePlatform platform, int percent, std::string const& lastMessage) = 0;
};

class IInitService {
public:
	virtual void Register(IInitProgressObserver* observer) = 0;
	virtual void Unregister(IInitProgressObserver* observer) = 0;

	virtual void AddPlatform(TradePlatform platform) = 0;
	virtual void DeletePlatform(TradePlatform platform) = 0;
	virtual void ResetPlatform(TradePlatform platform) = 0;
	virtual void OnStatusUpdate(TradePlatform platform, InitProcess process, InitStatus status, Error error) = 0;
	virtual int GetPercent(TradePlatform platform) = 0;;
	virtual std::string GetLastMessage(TradePlatform platform) = 0;
	virtual Error GetLastError(TradePlatform platform) = 0;
	virtual bool Has(TradePlatform platform) const = 0;
};

namespace automate {
	struct AutoSnapshot;

}

class IStrategyObserver {
public:
	virtual void OnUpdate(int id, automate::AutoSnapshot const& snapshot) = 0;
};

class IStrategyService {
public:
	virtual void Register(IStrategyObserver* observer) = 0;
	virtual void Unregister(IStrategyObserver* observer) = 0;

	virtual void OnUpdate(int id, automate::AutoSnapshot const& snapshot) = 0;

};

struct ServiceStore {
	IAccountService* Account = nullptr;
	IPositionService* Position = nullptr;
	ITradeService* Trade = nullptr;
	IOrderService* Order = nullptr;
	ITradeSystemService* TradeSystem = nullptr;
	IInstrumentService* Instrument = nullptr;
	IExchangeService* Exchange = nullptr;
	IQuoteService* Quote = nullptr;
	IQueryService* Query = nullptr;
	IInitService* Init = nullptr;
	IStrategyService* Strategy = nullptr;
};

class ServiceSinglton {
public:
	static void InitAccountService(IAccountObserver* observer);
	static void InitPositionService(IPositionObserver* observer);
	static void InitTradeService(ITradeObserver* observer);
	static void InitOrderService(IOrderObserver* observer);
	static void InitTradeSystemService(IExchangeStatusObserver* observer, ITradeSystemObserver* tradeSystemObserver);
	static void InitInstrumentService(IInstrumentObserver* observer);
	static void InitExchangeService();
	static void InitQuoteService(IQuoteObserver* observer);
	static void InitQueryService();
	static void InitInitService();
	static void InitStrategyService(IStrategyObserver* observer);

	static ServiceStore& Services();
};