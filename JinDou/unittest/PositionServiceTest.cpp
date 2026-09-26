#ifdef UNITTEST

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "service/PositionService.h"
#include "service/ExchangeService.h"
#include "service/InstrumentService.h"

using namespace std;


namespace trompeloeil {
	struct double_equal {
		template <typename X, typename Y>
		auto operator()(X const& x, Y const& y) const -> bool
		{
			::trompeloeil::ignore(x, y);
			return DoubleComparer::equal(x, y);
		}
	};
	template <
		typename T = wildcard,
		typename V,
		typename R = make_matcher_return<T, double_equal, lambdas::equal_printer, V>
	>
	inline auto double_eq(V&& v)
	{
		return make_matcher<T>(double_equal(),
			lambdas::equal_printer(),
			std::forward<V>(v));
	}
}

using namespace trompeloeil;
using trompeloeil::_;
class PositionObserverMock : public IPositionObserver {
public:
	MAKE_MOCK1(OnAdd, void(InvestorPosition const& position), override);
	MAKE_MOCK1(OnChange, void(InvestorPosition const& position), override);
	MAKE_MOCK1(OnDelete, void(InvestorPosition const& position), override);
	MAKE_MOCK2(OnPositionProfit, void(TradePlatform platform, double positionProfit), override);
};
TEST_CASE("PositionService Test") {
	const type::InstrumentCode auCode{ "Au(T+D)" };
	const type::BrokerID brokerID{ "6666" };
	const type::InvestorID investorID{ "88888888" };
	auto& services = ServiceSinglton::Services();
	ExchangeService exchangeService;
	services.Exchange = &exchangeService;

	InstrumentService instService;
	Instrument auInst;
	auInst.Code = auCode;
	auInst.Exchange = type::Exchange::SGE;
	auInst.LongPosLimit = 1000;
	auInst.ShortPosLimit = 1000;
	auInst.MaxOrderVolume = 100;
	auInst.MinOrderVolume = 100;
	auInst.ProductClass = type::ProductClass::Defer;
	auInst.ProductID = "Au";
	auInst.UnitPrice = 0.01;
	auInst.VolumeMultiple = 1000;
	auInst.VarityCode = "Au";

	instService.Add(auInst);
	type::InstID auID = instService.toID(auCode);

	InvestorFee fee;
	fee.BrokerID = brokerID;
	fee.Exchange = type::Exchange::SGE;
	fee.InstrumentCode = auCode;
	fee.InstID = auID;
	fee.InvestorID = investorID;
	fee.OffsetFeeAmt = 0;
	fee.OffsetFeeRate = 0.01;
	fee.OffsetTodayFeeAmt = 0;
	fee.OffsetTodayFeeRate = 0.01;
	fee.OpenFeeAmt = 0;
	fee.OpenFeeRate = 0.01;
	instService.AddFee(TradePlatform::ZTP, fee);

	InvestorMargin margin;
	margin.BrokerID = brokerID;
	margin.InvestorID = investorID;
	margin.Exchange = type::Exchange::SGE;
	margin.InstrumentCode = auCode;
	margin.InstID = auID;
	margin.LongMarginAmt = 0;
	margin.LongMarginRate = 0.10;
	margin.ShortMarginAmt = 0;
	margin.ShortMarginRate = 0.10;
	instService.AddMargin(TradePlatform::ZTP, margin);

	services.Instrument = &instService;
	SECTION("PositionService InitAdd Test") {
		PositionService posService;
		PositionObserverMock observerMock;
		posService.Register(&observerMock);
		InvestorPosition pos;
		pos.Exchange = type::Exchange::SGE;
		pos.BrokerID = brokerID;
		pos.InvestorID = investorID;
		pos.InstrumentCode = auCode;
		pos.InstID = auID;
		pos.ProductClass = type::ProductClass::Defer;
		pos.Direction = type::Direction::Buy;
		pos.Position = 1;
		pos.PositionCost = 345.54;
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.InitAdd(pos);

		CHECK(posService.Get(auID, type::Direction::Buy)->Position == 1);
		CHECK_FALSE(posService.Get(auID, type::Direction::Sell));

		posService.ClearPlatform(TradePlatform::CTP);
		CHECK(posService.Get(auID, type::Direction::Buy));
		REQUIRE_CALL(observerMock, OnDelete(_));
		posService.ClearPlatform(TradePlatform::ZTP);
		CHECK_FALSE(posService.Get(auID, type::Direction::Buy));
	}

	SECTION("PositionService OnTrade Test1") {
		int LocalId = 88888888;
		PositionService posService;
		PositionObserverMock observerMock;
		posService.Register(&observerMock);
		InvestorTrade trade;
		trade.InstID = auID;
		trade.InstrumentCode = auCode;
		trade.Direction = type::Direction::Buy;
		trade.OffsetFlag = type::OffsetFlag::Open;
		trade.OrderLocalID = LocalId;
		trade.OrderSysID = "SydID001";
		trade.TradePrice = 346;
		trade.TradeVolume = 10;
		trade.TradeID = "44";
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.OnTrade(trade);
		
		CHECK(posService.Get(auID, type::Direction::Buy)->Position == 10);
		CHECK(posService.Get(auID, type::Direction::Buy)->PositionCost == 346.00);

		trade.TradeVolume = 5;
		trade.OrderLocalID = LocalId + 1;
		trade.OrderSysID = "SYdID002";
		trade.TradeID = "55";
		trade.OffsetFlag = type::OffsetFlag::Close;
		trade.Direction = type::Direction::Sell;
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.OnTrade(trade);

		auto pos = posService.Get(auID, type::Direction::Buy);
		CHECK(pos);
		CHECK(pos->Position == 5);
		CHECK(pos->PositionCost == 346.00);
		CHECK(pos->UsedMargin == 346 * 5 * 1000 * 0.1);

		REQUIRE_CALL(observerMock, OnDelete(_));
		posService.OnTrade(trade);

		CHECK_FALSE(posService.Get(auID, type::Direction::Buy));
	}

	SECTION("PositionService OnTrade Test2") {
		PositionService posService;
		PositionObserverMock observerMock;
		posService.Register(&observerMock);
		InvestorPosition pos;
		pos.Exchange = type::Exchange::SGE;
		pos.BrokerID = brokerID;
		pos.InvestorID = investorID;
		pos.InstrumentCode = auCode;
		pos.InstID = auID;
		pos.ProductClass = type::ProductClass::Defer;
		pos.Direction = type::Direction::Buy;
		pos.Position = 10;
		pos.PositionCost = 345.54;
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.InitAdd(pos);

		InvestorTrade trade;
		trade.InstID = auID;
		trade.InstrumentCode = auCode;
		trade.Direction = type::Direction::Buy;
		trade.OffsetFlag = type::OffsetFlag::Open;
		trade.OrderLocalID = 88888888;
		trade.OrderSysID = "SydID001";
		trade.TradePrice = 346;
		trade.TradeVolume = 10;
		trade.TradeID = "44";

		REQUIRE_CALL(observerMock, OnChange(_));
		posService.OnTrade(trade);

		CHECK(posService.Get(auID, type::Direction::Buy)->Position == 20);
		CHECK(posService.Get(auID, type::Direction::Buy)->PositionCost == 345.77);
	}

	SECTION("PositionService Quote Test") {
		PositionService posService;
		PositionObserverMock observerMock;
		posService.Register(&observerMock);
		InvestorPosition pos;
		pos.Exchange = type::Exchange::SGE;
		pos.BrokerID = brokerID;
		pos.InvestorID = investorID;
		pos.InstrumentCode = auCode;
		pos.InstID = auID;
		pos.ProductClass = type::ProductClass::Defer;
		pos.Direction = type::Direction::Buy;
		pos.Position = 10;
		pos.PositionCost = 345.54;
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.InitAdd(pos);

		double basePrice = 345.55;
		SimpleQuoteArray buyPrices;
		buyPrices[0] = SimpleQuote(basePrice - 0.5, 30);
		buyPrices[1] = SimpleQuote(basePrice - 0.4, 31);
		buyPrices[2] = SimpleQuote(basePrice - 0.3, 32);
		buyPrices[3] = SimpleQuote(basePrice - 0.2, 33);
		buyPrices[4] = SimpleQuote(basePrice - 0.1, 34);
		SimpleQuoteArray sellPrices;
		sellPrices[0] = SimpleQuote(basePrice + 0.1, 16);
		sellPrices[1] = SimpleQuote(basePrice + 0.2, 17);
		sellPrices[2] = SimpleQuote(basePrice + 0.3, 18);
		sellPrices[3] = SimpleQuote(basePrice + 0.4, 19);
		sellPrices[4] = SimpleQuote(basePrice + 0.5, 20);
		double lastPrice = basePrice;

		Quote quote(auID, buyPrices, sellPrices, 370, 320, 10, 200, 300, 400, 345.55, 500, 600, false, SimpleQuote(), SimpleQuote());


		REQUIRE_CALL(observerMock, OnPositionProfit(TradePlatform::ZTP, double_eq(100)));
		posService.OnQuote(quote);

		CHECK(posService.Get(auID, type::Direction::Buy)->PositionProfit == Approx((lastPrice - pos.PositionCost) * 1000 * pos.Position));
	}

	SECTION("PositionService Quote Test2") {
		PositionService posService;
		PositionObserverMock observerMock;
		posService.Register(&observerMock);
		InvestorPosition pos;
		pos.Exchange = type::Exchange::SGE;
		pos.BrokerID = brokerID;
		pos.InvestorID = investorID;
		pos.InstrumentCode = auCode;
		pos.InstID = auID;
		pos.ProductClass = type::ProductClass::Defer;
		pos.Direction = type::Direction::Buy;
		pos.Position = 10;
		pos.PositionCost = 345.53;
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.InitAdd(pos);

		pos.Direction = type::Direction::Sell;
		pos.PositionCost = 345.54;
		REQUIRE_CALL(observerMock, OnChange(_));
		posService.InitAdd(pos);

		double basePrice = 345.55;
		SimpleQuoteArray buyPrices;
		buyPrices[0] = SimpleQuote(basePrice - 0.5, 30);
		buyPrices[1] = SimpleQuote(basePrice - 0.4, 31);
		buyPrices[2] = SimpleQuote(basePrice - 0.3, 32);
		buyPrices[3] = SimpleQuote(basePrice - 0.2, 33);
		buyPrices[4] = SimpleQuote(basePrice - 0.1, 34);
		SimpleQuoteArray sellPrices;
		sellPrices[0] = SimpleQuote(basePrice + 0.1, 16);
		sellPrices[1] = SimpleQuote(basePrice + 0.2, 17);
		sellPrices[2] = SimpleQuote(basePrice + 0.3, 18);
		sellPrices[3] = SimpleQuote(basePrice + 0.4, 19);
		sellPrices[4] = SimpleQuote(basePrice + 0.5, 20);
		double lastPrice = basePrice;

		Quote quote(auID, buyPrices, sellPrices, 370, 320, 10, 200, 300, 400, 345.55, 500, 600, false, SimpleQuote(), SimpleQuote());


		REQUIRE_CALL(observerMock, OnPositionProfit(TradePlatform::ZTP, double_eq(100.00)));
		posService.OnQuote(quote);

		CHECK(posService.Get(auID, type::Direction::Buy)->PositionProfit == Approx(200));
		CHECK(posService.Get(auID, type::Direction::Sell)->PositionProfit == Approx(-100));
	
		ALLOW_CALL(observerMock, OnPositionProfit(_, _));
 		posService.OnQuote(quote);
// 		CHECK(posService.Get(auID, type::Direction::Buy)->PositionProfit == Approx(200));
// 		CHECK(posService.Get(auID, type::Direction::Sell)->PositionProfit == Approx(-100));

	}
}

#endif