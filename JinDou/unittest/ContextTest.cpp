#ifdef UNITTEST

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include "strategy/auto/Context.h"
#include "model/Quote.h"
#include "service/InstrumentService.h"
#include "service/ExchangeService.h"
#include "runner/Framework.h"
#include "runner/Queues.h"
#include "strategy/Business.h"
#include "strategy/auto/AutoStrategy.h"

using namespace std;
using namespace automate;

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
// class PositionObserverMock : public IPositionObserver {
// public:
// 	MAKE_MOCK1(OnAdd, void(InvestorPosition const& position), override);
// 	MAKE_MOCK1(OnChange, void(InvestorPosition const& position), override);
// 	MAKE_MOCK1(OnDelete, void(InvestorPosition const& position), override);
// 	MAKE_MOCK2(OnPositionProfit, void(TradePlatform platform, double positionProfit), override);
// };

TEST_CASE("Context Test") {
	const type::InstrumentCode majorCode{ "Au(T+D)" };
	const type::InstrumentCode minorCode{ "Au99.99" };
	const type::BrokerID brokerID{ "6666" };
	const type::InvestorID investorID{ "88888888" };
	auto& services = ServiceSinglton::Services();
	ExchangeService exchangeService;
	services.Exchange = &exchangeService;

	InstrumentService instService;
	Instrument majorInst;
	majorInst.Code = majorCode;
	majorInst.Exchange = type::Exchange::SGE;
	majorInst.LongPosLimit = 1000;
	majorInst.ShortPosLimit = 1000;
	majorInst.MaxOrderVolume = 100;
	majorInst.MinOrderVolume = 100;
	majorInst.ProductClass = type::ProductClass::Defer;
	majorInst.ProductID = "Au";
	majorInst.UnitPrice = 1;
	majorInst.VolumeMultiple = 1000;
	majorInst.VarityCode = "Au";

	instService.Add(majorInst);
	type::InstID majorID = instService.toID(majorCode);


	Instrument minorInst;
	minorInst.Code = minorCode;
	minorInst.Exchange = type::Exchange::SGE;
	minorInst.LongPosLimit = 1000;
	minorInst.ShortPosLimit = 1000;
	minorInst.MaxOrderVolume = 100;
	minorInst.MinOrderVolume = 100;
	minorInst.ProductClass = type::ProductClass::Spot;
	minorInst.ProductID = "Au";
	minorInst.UnitPrice = 1;
	minorInst.VolumeMultiple = 1000;
	minorInst.VarityCode = "Au";

	instService.Add(minorInst);
	type::InstID minorID = instService.toID(minorCode);

	services.Instrument = &instService;
	AutoArgument arg;
	arg.Mode = StrategyMode::TodayOpenClose;
	arg.PriceMode = PriceMode::Pending;
	arg.Major = majorID;
	arg.Minor = minorID;
	arg.VolumeMultiple = 1;
	arg.MaxVolume = 20;		
	arg.Quantity = 1;		
	arg.SellOpenSpread = 120;	
	arg.BuyCloseSpread = 115;	
	arg.SellCloseSpread = 110;
	arg.BuyOpenSpread = 100;	
	arg.Tolerance = 1;		
	arg.MajorLostTick = 1;	

	SimpleQuoteArray buyArray;
	SimpleQuoteArray sellArray;
	SimpleQuote hidenBuy;
	SimpleQuote hidenSell;
	Quote majorQuote{ majorID,buyArray,sellArray, 10,10,10,10,10,10,10,10,10,false,hidenBuy,hidenSell };
	Quote minorQuote{ minorID,buyArray,sellArray, 10,10,10,10,10,10,10,10,10,false,hidenBuy,hidenSell };;
	auto& publisher = Framework::Instance().GetStrategyPublisher();
	auto& connPublisher = Framework::Instance().GetConnPublisher();
	auto& queues = Framework::Instance().GetQueues();
	BusinessDealingStore dealingStore;
	AutoStrategy strategy{ arg, publisher, dealingStore };


	auto buyOpenSpreadQuote = [&]() {
		buyArray[0] = SimpleQuote{ 380,3 };
		sellArray[0] = SimpleQuote{ 360,3 };
		majorQuote.UpdatePrice(buyArray, sellArray, 10);
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
	};

	auto sellCloseSpreadQuote = [&]() {
		SimpleQuoteArray majorBuyArray;
		SimpleQuoteArray majorSellArray;
		majorBuyArray[0] = SimpleQuote{ 250,3 };
		majorSellArray[0] = SimpleQuote{ 260,3 };
		buyArray[0] = SimpleQuote{ 360,3 };
		sellArray[0] = SimpleQuote{ 371,3 };
		majorQuote.UpdatePrice(majorBuyArray, majorSellArray, 10);
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
	};

	auto sellOpenSpreadQuote = [&]() {
		SimpleQuoteArray majorBuyArray;
		SimpleQuoteArray majorSellArray;
		majorBuyArray[0] = SimpleQuote{ 250,3 };
		majorSellArray[0] = SimpleQuote{ 260,3 };
		buyArray[0] = SimpleQuote{ 360,3 };
		sellArray[0] = SimpleQuote{ 380,3 };
		majorQuote.UpdatePrice(majorBuyArray, majorSellArray, 10);
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
	};

	auto buyCloseSpreadQuote = [&]() {
		SimpleQuoteArray majorBuyArray;
		SimpleQuoteArray majorSellArray;
		majorBuyArray[0] = SimpleQuote{ 245,3 };
		majorSellArray[0] = SimpleQuote{ 260,3 };
		buyArray[0] = SimpleQuote{ 360,3 };
		sellArray[0] = SimpleQuote{ 371,3 };
		majorQuote.UpdatePrice(majorBuyArray, majorSellArray, 10);
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
	};

	auto orderFromRequest = [&](OrderInsertRequest const& req, type::OrderSysID const& sysId = type::OrderSysID("Sys01")) -> auto {
		InvestorOrder order;
		order.Direction = req.Direction;
		order.Exchange = req.Exchange;
		order.InstID = req.InstID;
		order.OffsetFlag = req.OffsetFlag;
		order.OrderPriceType = req.OrderPriceType;
		order.OrderLocalID = req.OrderLocalID;
		order.OrderSysID = sysId;
		order.LimitPrice = req.Price;
		order.Volume = req.Volume;
		order.OrderStatus = type::OrderStatus::NoTradeQueueing;
		return order;
	};

	auto tradeFromOrder = [&] (InvestorOrder const& order, type::Volume tradeVolume) -> auto {
		InvestorTrade trade;
		trade.Direction = order.Direction;
		trade.BrokerID = order.BrokerID;
		trade.Exchange = order.Exchange;
		trade.InstID = order.InstID;
		trade.InstrumentCode = order.InstrumentCode;
		trade.InvestorID = order.InvestorID;
		trade.OffsetFlag = order.OffsetFlag;
		trade.OrderLocalID = order.OrderLocalID;
		trade.OrderSysID = order.OrderSysID;
		trade.ProductClass = order.ProductClass;
		trade.TradeID = "1";
		trade.TradePrice = order.LimitPrice;
		trade.TradeVolume = std::min(order.Volume,tradeVolume);
		return trade;
	};

	SECTION("Context Buy Open Happy Test") {
		Context con{ &strategy, arg, majorQuote, minorQuote };
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Paused);
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Buy);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(381.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::AllTraded;
		order.VolumeRemain = 0;
		order.VolumeTrade = 1;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == majorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));

		Dealing* majorDealing = dealingStore.FindDealing(req.OrderLocalID);

		auto majorOrder = orderFromRequest(req, type::OrderSysID("Sys02"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing->OnRtnOrder(majorOrder);
		con.OnMajorFinished(*majorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);

	}


	SECTION("Context Buy Open Cancel Test") {
		Context con{&strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Buy);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(381.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
//		Dealing minorDealing(&strategy, &con, req);
		InvestorOrder order;
		order.Direction = req.Direction;
		order.Exchange = req.Exchange;
		order.InstID = req.InstID;
		order.OffsetFlag = req.OffsetFlag;
		order.OrderPriceType = req.OrderPriceType;
		order.OrderLocalID = req.OrderLocalID;
		order.OrderSysID = "Sys01";
		order.LimitPrice = req.Price;
		order.Volume = req.Volume;
		order.OrderStatus = type::OrderStatus::NoTradeQueueing;
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		sellCloseSpreadQuote();
		con.OnQuote(minorQuote);
		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderCancelRequest const& cancel = *(OrderCancelRequest*)data.Payload;
		CHECK(cancel.OrderLocalID == req.OrderLocalID);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		order.OrderStatus = type::OrderStatus::Canceled;
		minorDealing->OnRtnOrder(order);
		con.OnMinorCancel(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		CHECK(con.GetRunning().IsNone());
	}

 	SECTION("Context Buy Open Minor Tolerance Test1") {
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(req.Price == Approx(381.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto buyArray = minorQuote.GetBuyPriceArray();
		auto sellArray = minorQuote.GetSellPriceArray();
		buyArray[0] = SimpleQuote{ 381, 10 };
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(minorQuote);
		rtn = queues.OrderQueue.pop(data);
		CHECK(!rtn);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		buyArray[0] = SimpleQuote{ 382, 10 };
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(minorQuote);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderCancelRequest const& cancel = *(OrderCancelRequest*)data.Payload;
		CHECK(cancel.OrderLocalID == req.OrderLocalID);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		order.OrderStatus = type::OrderStatus::Canceled;
		minorDealing->OnRtnOrder(order);
		con.OnMinorCancel(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		CHECK(con.GetRunning().IsNone());

	}

	SECTION("Context BuyOpen Spread Lost Test") {
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(req.Price == Approx(381.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto buyArray = minorQuote.GetBuyPriceArray();
		auto sellArray = minorQuote.GetSellPriceArray();
		//有1的tolerance,价差变成101没问题
		buyArray[0] = SimpleQuote{ 279, 10 };
		majorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(majorQuote);
		rtn = queues.OrderQueue.pop(data);
		CHECK(!rtn);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		//价差到102就需要撤单了
		buyArray[0] = SimpleQuote{ 278, 10 };
		majorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(majorQuote);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderCancelRequest const& cancel = *(OrderCancelRequest*)data.Payload;
		CHECK(cancel.OrderLocalID == req.OrderLocalID);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		order.OrderStatus = type::OrderStatus::Canceled;
		minorDealing->OnRtnOrder(order);
		con.OnMinorCancel(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		CHECK(con.GetRunning().IsNone());
	}

	SECTION("Context Buy Open Complicated Test") {
		arg.MaxVolume = 5;
		arg.Quantity = 3;
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Buy);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 3);
		CHECK(req.Price == Approx(381.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::PartTradedQueueing;
		order.VolumeRemain = 2;
		order.VolumeTrade = 1;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == majorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));
		rtn = queues.OrderQueue.pop(data);
		CHECK_FALSE(rtn);

		Dealing* majorDealing = dealingStore.FindDealing(req.OrderLocalID);

		auto majorOrder = orderFromRequest(req, type::OrderSysID("Sys02"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing->OnRtnOrder(majorOrder);
		auto majorTrade = tradeFromOrder(majorOrder,1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		rtn = queues.OrderQueue.pop(data);
		CHECK_FALSE(rtn);

		// second minor trade

		trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::PartTradedQueueing;
		order.VolumeRemain = 1;
		order.VolumeTrade = 2;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == majorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));

		majorDealing = dealingStore.FindDealing(req.OrderLocalID);

		majorOrder = orderFromRequest(req, type::OrderSysID("Sys03"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		// Third minor trade

		trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::AllTraded;
		order.VolumeRemain = 0;
		order.VolumeTrade = 3;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == majorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));

		majorDealing = dealingStore.FindDealing(req.OrderLocalID);

		// major not trade,fire a new one
		majorOrder = orderFromRequest(req, type::OrderSysID("Sys03"));
		majorOrder.OrderStatus = type::OrderStatus::Canceled;
		majorOrder.VolumeRemain = 1;
		majorOrder.VolumeTrade = 0;
		majorDealing->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		// major traded, all balanced, go to start status
		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == majorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));

		majorDealing = dealingStore.FindDealing(req.OrderLocalID);

		majorOrder = orderFromRequest(req, type::OrderSysID("Sys03"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);

	}


	SECTION("Context Buy Open Mix Trade Test") {
		arg.MaxVolume = 5;
		arg.Quantity = 3;
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Buy);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 3);
		CHECK(req.Price == Approx(381.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::PartTradedQueueing;
		order.VolumeRemain = 2;
		order.VolumeTrade = 1;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::PartTradedQueueing;
		order.VolumeRemain = 1;
		order.VolumeTrade = 2;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest majorReq1 = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(majorReq1.Exchange == type::Exchange::SGE);
		CHECK(majorReq1.InstID == majorID);
		CHECK(majorReq1.Direction == type::Direction::Sell);
		CHECK(majorReq1.OffsetFlag == type::OffsetFlag::Open);
		CHECK(majorReq1.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(majorReq1.Volume == 1);
		CHECK(majorReq1.Price == Approx(379.0));

		Dealing* majorDealing1 = dealingStore.FindDealing(majorReq1.OrderLocalID);

		auto majorOrder = orderFromRequest(majorReq1, type::OrderSysID("Sys02"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing1->OnRtnOrder(majorOrder);
		auto majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing1);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);


		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest majorReq2 = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(majorReq2.Exchange == type::Exchange::SGE);
		CHECK(majorReq2.InstID == majorID);
		CHECK(majorReq2.Direction == type::Direction::Sell);
		CHECK(majorReq2.OffsetFlag == type::OffsetFlag::Open);
		CHECK(majorReq2.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(majorReq2.Volume == 1);
		CHECK(majorReq2.Price == Approx(379.0));
		rtn = queues.OrderQueue.pop(data);
		Dealing* majorDealing2 = dealingStore.FindDealing(majorReq2.OrderLocalID);

		majorOrder = orderFromRequest(majorReq2, type::OrderSysID("Sys03"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing2->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing2);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);


		trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::AllTraded;
		order.VolumeRemain = 0;
		order.VolumeTrade = 3;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest majorReq3 = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(majorReq3.Exchange == type::Exchange::SGE);
		CHECK(majorReq3.InstID == majorID);
		CHECK(majorReq3.Direction == type::Direction::Sell);
		CHECK(majorReq3.OffsetFlag == type::OffsetFlag::Open);
		CHECK(majorReq3.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(majorReq3.Volume == 1);
		CHECK(majorReq3.Price == Approx(379.0));
		rtn = queues.OrderQueue.pop(data);
		CHECK_FALSE(rtn);

		Dealing* majorDealing3 = dealingStore.FindDealing(majorReq3.OrderLocalID);

		majorOrder = orderFromRequest(majorReq3, type::OrderSysID("Sys04"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing3->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing3);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
	}

	SECTION("Context SellClose Spread Lost Test") {
		arg.ExistedVolume = { type::Direction::Buy, 6 };
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		sellCloseSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(req.Price == Approx(370.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto buyArray = minorQuote.GetBuyPriceArray();
		auto sellArray = minorQuote.GetSellPriceArray();
		//有1的tolerance,价差变成110没问题
		sellArray[0] = SimpleQuote{ 370, 10 };
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(minorQuote);
		rtn = queues.OrderQueue.pop(data);
		CHECK(!rtn);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		//价差到108就需要撤单了
		buyArray = majorQuote.GetBuyPriceArray();
		sellArray = majorQuote.GetSellPriceArray();
		sellArray[0] = SimpleQuote{ 263, 10 };
		majorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(majorQuote);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderCancelRequest const& cancel = *(OrderCancelRequest*)data.Payload;
		CHECK(cancel.OrderLocalID == req.OrderLocalID);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		order.OrderStatus = type::OrderStatus::Canceled;
		minorDealing->OnRtnOrder(order);
		con.OnMinorCancel(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		CHECK(con.GetRunning().IsNone());
	}


	SECTION("Context Sell Open Happy Test") {
		Context con{ &strategy, arg, majorQuote, minorQuote };
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Paused);
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		sellOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::AllTraded;
		order.VolumeRemain = 0;
		order.VolumeTrade = 1;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == majorID);
		CHECK(req.Direction == type::Direction::Buy);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(261));

		Dealing* majorDealing = dealingStore.FindDealing(req.OrderLocalID);

		auto majorOrder = orderFromRequest(req, type::OrderSysID("Sys02"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing->OnRtnOrder(majorOrder);
		con.OnMajorFinished(*majorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
	}

	SECTION("Context BuyClose Spread Lost Test") {
		arg.ExistedVolume = { type::Direction::Sell, 6 };
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		buyCloseSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(req.Price == Approx(361.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto buyArray = minorQuote.GetBuyPriceArray();
		auto sellArray = minorQuote.GetSellPriceArray();
		//有1的tolerance,价差变成119没问题
		buyArray[0] = SimpleQuote{ 359, 10 };
		minorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(minorQuote);
		rtn = queues.OrderQueue.pop(data);
		CHECK(!rtn);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		//价差到108就需要撤单了
		buyArray = majorQuote.GetBuyPriceArray();
		sellArray = majorQuote.GetSellPriceArray();
		buyArray[0] = SimpleQuote{ 243, 10 };
		majorQuote.UpdatePrice(buyArray, sellArray, 10);
		con.OnQuote(majorQuote);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderCancelRequest const& cancel = *(OrderCancelRequest*)data.Payload;
		CHECK(cancel.OrderLocalID == req.OrderLocalID);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		order.OrderStatus = type::OrderStatus::Canceled;
		minorDealing->OnRtnOrder(order);
		con.OnMinorCancel(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		CHECK(con.GetRunning().IsNone());
	}


	SECTION("Context Sell Open Mix Trade Test") {
		arg.MaxVolume = 5;
		arg.Quantity = 3;
		Context con{ &strategy, arg, majorQuote, minorQuote };
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		sellOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 3);
		CHECK(req.Price == Approx(379.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);
		auto trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::PartTradedQueueing;
		order.VolumeRemain = 2;
		order.VolumeTrade = 1;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::PartTradedQueueing;
		order.VolumeRemain = 1;
		order.VolumeTrade = 2;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest majorReq1 = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(majorReq1.Exchange == type::Exchange::SGE);
		CHECK(majorReq1.InstID == majorID);
		CHECK(majorReq1.Direction == type::Direction::Buy);
		CHECK(majorReq1.OffsetFlag == type::OffsetFlag::Open);
		CHECK(majorReq1.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(majorReq1.Volume == 1);
		CHECK(majorReq1.Price == Approx(261));

		Dealing* majorDealing1 = dealingStore.FindDealing(majorReq1.OrderLocalID);

		auto majorOrder = orderFromRequest(majorReq1, type::OrderSysID("Sys02"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing1->OnRtnOrder(majorOrder);
		auto majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing1);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);


		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest majorReq2 = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(majorReq2.Exchange == type::Exchange::SGE);
		CHECK(majorReq2.InstID == majorID);
		CHECK(majorReq2.Direction == type::Direction::Buy);
		CHECK(majorReq2.OffsetFlag == type::OffsetFlag::Open);
		CHECK(majorReq2.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(majorReq2.Volume == 1);
		CHECK(majorReq2.Price == Approx(261));
		rtn = queues.OrderQueue.pop(data);
		Dealing* majorDealing2 = dealingStore.FindDealing(majorReq2.OrderLocalID);

		majorOrder = orderFromRequest(majorReq2, type::OrderSysID("Sys03"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing2->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing2);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);


		trade = tradeFromOrder(order, 1);
		order.OrderStatus = type::OrderStatus::AllTraded;
		order.VolumeRemain = 0;
		order.VolumeTrade = 3;
		minorDealing->OnRtnOrder(order);

		con.UpdatePositionByTrade(trade);
		con.OnMinorTrade(*minorDealing, trade);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest majorReq3 = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(majorReq3.Exchange == type::Exchange::SGE);
		CHECK(majorReq3.InstID == majorID);
		CHECK(majorReq3.Direction == type::Direction::Buy);
		CHECK(majorReq3.OffsetFlag == type::OffsetFlag::Open);
		CHECK(majorReq3.OrderPriceType == type::OrderPriceType::FAK);
		CHECK(majorReq3.Volume == 1);
		CHECK(majorReq3.Price == Approx(261));
		rtn = queues.OrderQueue.pop(data);
		CHECK_FALSE(rtn);

		Dealing* majorDealing3 = dealingStore.FindDealing(majorReq3.OrderLocalID);

		majorOrder = orderFromRequest(majorReq3, type::OrderSysID("Sys04"));
		majorOrder.OrderStatus = type::OrderStatus::AllTraded;
		majorOrder.VolumeRemain = 0;
		majorOrder.VolumeTrade = 1;
		majorDealing3->OnRtnOrder(majorOrder);
		majorTrade = tradeFromOrder(majorOrder, 1);
		con.UpdatePositionByTrade(majorTrade);
		con.OnMajorFinished(*majorDealing3);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
	}

	SECTION("Context Pause Test") {
		Context con{ &strategy, arg, majorQuote, minorQuote };
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Paused);
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		con.Pause();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Paused);
		con.Start();
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Start);
		sellOpenSpreadQuote();
		con.OnQuote(minorQuote);
		auto* current = con.GetCurrentStatus();
		CHECK(current->SelfStatus == RunningStatus::MinorSent);
		Data data;
		auto rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		OrderInsertRequest req = *(OrderInsertRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderInsert);
		CHECK(req.Exchange == type::Exchange::SGE);
		CHECK(req.InstID == minorID);
		CHECK(req.Direction == type::Direction::Sell);
		CHECK(req.OffsetFlag == type::OffsetFlag::Open);
		CHECK(req.OrderPriceType == type::OrderPriceType::LimitPrice);
		CHECK(req.Volume == 1);
		CHECK(req.Price == Approx(379.0));
		Dealing* minorDealing = dealingStore.FindDealing(req.OrderLocalID);
		InvestorOrder order = orderFromRequest(req);
		minorDealing->OnRtnOrder(order);
		con.OnMinorOrder(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Balancing);

		con.Pause();
		rtn = queues.OrderQueue.pop(data);
		CHECK(rtn);
		auto cancelReq = *(OrderCancelRequest*)data.Payload;
		CHECK(data.Type == DataType::OrderCancel);
		CHECK(cancelReq.Exchange == type::Exchange::SGE);
		CHECK(cancelReq.OrderSysID == order.OrderSysID);
		CHECK(cancelReq.OrderLocalID == order.OrderLocalID);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Pausing);

		order.OrderStatus = type::OrderStatus::Canceled;
		minorDealing->OnRtnOrder(order);
		con.OnMinorCancel(*minorDealing);
		CHECK(con.GetCurrentStatus()->SelfStatus == RunningStatus::Paused);

	}
}

#endif