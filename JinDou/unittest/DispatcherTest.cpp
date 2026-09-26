#ifdef UNITTEST

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "Dispatcher.h"
#include "MockService.h"
#include "service/OrderService.h"
#include "service/ExchangeService.h"

using namespace std;
using namespace mock;
using trompeloeil::_;

void attachUniqueID(InvestorOrder& order) {
	char orderRef[16] = "";
	sprintf_s(orderRef,"%12d", order.OrderLocalID.value());
	order.UniqueID = orderRef;
}

TEST_CASE("Dispatcher Test") {
	const type::InstrumentCode auCode{ "Au(T+D)" };
	auto& services = ServiceSinglton::Services();
	ExchangeService exchangeService;
	services.Exchange = &exchangeService;
	SECTION("Init Progress Dispatch Test") {
		InitMockService initMock;
		services.Init = &initMock;
		Dispatcher d;
		InitStatusNotification isn;
		isn.Platform = TradePlatform::CTP;
		isn.Process = InitProcess::QueryAccount;
		isn.Status = InitStatus::Processing;
		Data data;
		data.Type = DataType::InitStatus;
		data.Size = sizeof(isn);
		memcpy(data.Payload, &isn, data.Size);
		REQUIRE_CALL(initMock, OnStatusUpdate(TradePlatform::CTP, InitProcess::QueryAccount, InitStatus::Processing, Error{}));
		d.Dispatch(data);
	}

	SECTION("Quote Dispatcher Test") {
		QuoteMockService quoteMock;
		PositionMockService positionMock;
		services.Quote = &quoteMock;
		services.Position = &positionMock;

		Dispatcher d;

		SimpleQuoteArray buyPriceArray;
		buyPriceArray[0] = SimpleQuote(355.8, 30);
		buyPriceArray[1] = SimpleQuote(355.8, 31);
		buyPriceArray[2] = SimpleQuote(355.8, 32);
		buyPriceArray[3] = SimpleQuote(355.8, 33);
		buyPriceArray[4] = SimpleQuote(355.8, 34);
		SimpleQuoteArray sellPriceArray;
		sellPriceArray[0] = SimpleQuote(356.5, 16);
		sellPriceArray[1] = SimpleQuote(356.4, 17);
		sellPriceArray[2] = SimpleQuote(356.3, 18);
		sellPriceArray[3] = SimpleQuote(356.2, 19);
		sellPriceArray[4] = SimpleQuote(356.1, 20);
		double lastPrice = 356;

		Quote quote(1, buyPriceArray, sellPriceArray, 370, 320, 10, 200, 300, 400,
			lastPrice, 500, 600, false, SimpleQuote(), SimpleQuote());
		Data data;
		data.Type = DataType::Quote;
		data.Size = sizeof(Quote);
		memcpy(data.Payload, &quote, data.Size);
		REQUIRE_CALL(quoteMock, Update(_)).LR_RETURN(std::ref(quote));
		REQUIRE_CALL(positionMock, OnQuote(_));
		d.Dispatch(data);
	}

	SECTION("OrderRtn Dispatcher Test") {
		OrderService orderService;
		PositionMockService positionMock;
		TradeSystemMockService tsMock;
		services.Order = &orderService;
		services.Position = &positionMock;
		services.TradeSystem = &tsMock;

		type::OrderLocalID FirstLocalID = 880000001;

		Dispatcher d;
		InvestorOrder order;
		order.OrderStatus = type::OrderStatus::New;
		order.InstrumentCode = auCode;
		order.InstID = 1;
		order.BrokerID = "6666";
		order.Exchange = type::Exchange::SGE;
		order.InvestorID = "0219999";
		order.Direction = type::Direction::Buy;
		order.OffsetFlag = type::OffsetFlag::Open;
		order.OrderPriceType = type::OrderPriceType::LimitPrice;
		order.ProductClass = type::ProductClass::Defer;
		order.OrderLocalID = FirstLocalID;
		order.Volume = 10;
		order.LimitPrice = 356.56;
		attachUniqueID(order);

		OrderRtnNotification on{ order };
		Data data;
		data.Type = DataType::OrderRtn;
		data.Size = sizeof(OrderRtnNotification);
		memcpy(data.Payload, &on, data.Size);
//		REQUIRE_CALL(positionMock, FreezePosition(_));
		d.Dispatch(data);

		CHECK(orderService.GetOrder(FirstLocalID).IsSuccess());
		CHECK_FALSE(orderService.GetOrder(type::OrderSysID{ "SysID01" }));

		on.Order.OrderStatus = type::OrderStatus::NoTradeQueueing;
		on.Order.OrderSysID = "SysID01"; 
		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);
		CHECK(orderService.GetOrder(FirstLocalID)->OrderStatus == type::OrderStatus::NoTradeQueueing);
		CHECK(orderService.GetOrder(type::OrderSysID{ "SysID01" }));
		CHECK(orderService.GetOrder(type::OrderSysID{ "SysID01" })->OrderLocalID == FirstLocalID);

		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);
		CHECK(orderService.GetOrder(FirstLocalID)->OrderStatus == type::OrderStatus::NoTradeQueueing);

		on.Order.OrderStatus = type::OrderStatus::Canceled;
		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);

		on.Order.OrderStatus = type::OrderStatus::New;
		FirstLocalID = FirstLocalID.value() + 1;
		on.Order.OrderLocalID = FirstLocalID;
		on.Order.OffsetFlag = type::OffsetFlag::Close;
		attachUniqueID(on.Order);

		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(positionMock, FreezePosition(_));
		d.Dispatch(data);
		CHECK(orderService.GetOrder(FirstLocalID).IsSuccess());
		CHECK_FALSE(orderService.GetOrder(type::OrderSysID{ "SysID02" }));

		on.Order.OrderStatus = type::OrderStatus::NoTradeQueueing;
		on.Order.OrderSysID = "SysID02";
		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);
		CHECK(orderService.GetOrder(FirstLocalID).IsSuccess());
		CHECK(orderService.GetOrder(type::OrderSysID{ "SysID02" }));

		on.Order.OrderStatus = type::OrderStatus::Canceled;
		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(positionMock, UnfreezePosition(_));
		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);

		on.Order.OrderStatus = type::OrderStatus::NoTradeQueueing;
		FirstLocalID = FirstLocalID.value() + 1;
		on.Order.OrderLocalID = FirstLocalID;
		on.Order.OffsetFlag = type::OffsetFlag::Close;
		on.Order.OrderSysID = "SysID02";
		attachUniqueID(on.Order);

		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(positionMock, FreezePosition(_));
		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);
		CHECK(orderService.GetOrder(FirstLocalID).IsSuccess());
		CHECK(orderService.GetOrder(type::OrderSysID{ "SysID02" }));

		on.Order.OrderStatus = type::OrderStatus::Canceled;
		memcpy(data.Payload, &on, data.Size);
		REQUIRE_CALL(positionMock, UnfreezePosition(_));
		REQUIRE_CALL(tsMock, QueryAccount(_));
		d.Dispatch(data);

	}
}

#endif
