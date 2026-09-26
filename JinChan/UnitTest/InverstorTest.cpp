#include "stdafx.h"

#include <catch.hpp>
#include "Investor.h"
#include "Repository/MemoryRepositoryImpl.h"
#include "InverstorTest.h"
namespace {

	type::BrokerID BrokerID{ "6666" };
	type::InvestorID InvestorID{ "8888" };
	type::InstrumentID InstCode{ "Au(T+D)" };
	type::InstrumentID SpotInstCode{ "Au100g" };
	type::InstrumentID UniInstCode{ "UAu(T+D)" };
	type::InstrumentID UniInstCode2{ "mAu(T+D)" };
	type::ClientID ClientID  { "9999" };

	void InitInvestorInfo(InvestorRepository& repo);

	void InitAccount(InvestorRepository& repo);

	void InitInvestorMargin(InvestorRepository& repo);

	void InitInvestorFee(InvestorRepository& repo);

	void InitPos(InvestorRepository& repo);

	void InitSeatInfo(BaseInfoRepository& baseRepo) {
		data::CSeatInfo seatInfo;
		seatInfo.ExchangeID = "SGE";
		seatInfo.SeatID = "6666";
		seatInfo.SeatStatus = type::SeatStatus::Login;
		seatInfo.TraderID = "9999";

		baseRepo.InitAdd(seatInfo);
	}

	void InitInstrument(InstrumentRepository& repo) {
		{
			data::CInstrument inst;
			inst.ExchangeID = "SGE";
			inst.InstrumentID = InstCode;
			inst.InstrumentName = "Au(T+D)";
			inst.ProductClass = type::ProductClass::Defer;
			inst.LongPosLimit = 1000;
			inst.ShortPosLimit = 1000;
			inst.MaxOrderVolume = 999;
			inst.MinOrderVolume = 2;
			inst.VolumeMultiple = 1000;
			inst.PriceTick = 0.01;
			repo.InitAdd(inst);
		}

		{
			data::CInstrument inst;
			inst.ExchangeID = "SGE";
			inst.InstrumentID = SpotInstCode;
			inst.InstrumentName = "Au100g";
			inst.ProductClass = type::ProductClass::Spot;
			inst.LongPosLimit = 1000;
			inst.ShortPosLimit = 1000;
			inst.MaxOrderVolume = 999;
			inst.MinOrderVolume = 2;
			inst.VolumeMultiple = 1000;
			inst.PriceTick = 0.01;
			repo.InitAdd(inst);
		}

		{
			data::CInstrument inst;
			inst.ExchangeID = "SGE";
			inst.InstrumentID = UniInstCode;
			inst.InstrumentName = "UAu(T+D)";
			inst.ProductClass = type::ProductClass::Defer;
			inst.LongPosLimit = 1000;
			inst.ShortPosLimit = 1000;
			inst.MaxOrderVolume = 999;
			inst.MinOrderVolume = 1;
			inst.VolumeMultiple = 1000;
			inst.PriceTick = 0.01;
			inst.UnilateralGroup = 0;
			inst.UnilateralSubIndex = 0;
			repo.InitAdd(inst);
		}

		{
			data::CInstrument inst;
			inst.ExchangeID = "SGE";
			inst.InstrumentID = UniInstCode2;
			inst.InstrumentName = "UAu(T+D)";
			inst.ProductClass = type::ProductClass::Defer;
			inst.LongPosLimit = 1000;
			inst.ShortPosLimit = 1000;
			inst.MaxOrderVolume = 999;
			inst.MinOrderVolume = 1;
			inst.VolumeMultiple = 1000;
			inst.PriceTick = 0.01;
			inst.UnilateralGroup = 0;
			inst.UnilateralSubIndex = 1;
			repo.InitAdd(inst);
		}
	}

	void InitInvestor(InvestorRepository& repo) {

		InitInvestorInfo(repo);
		InitAccount(repo);
		InitInvestorMargin(repo);
		InitInvestorFee(repo);
		InitPos(repo);
	}

	void InitPos(InvestorRepository& repo) {
		CAlgorithms algo;
		{
			data::CInvestorPosition pos;
			pos.BrokerID = BrokerID;
			pos.InvestorID = InvestorID;
			//	pos.TradingDay
			pos.ExchangeID = "SGE";
			pos.InstrumentID = InstCode;
			pos.ProductClass = type::ProductClass::Defer;
			pos.Direction = type::Direction::Buy;
			pos.Position = 0;
			pos.PositionCost = 1000;
			pos.YdPosition = 10;
			pos.YdPositionPrice = 998;
			pos.FrozenMargin = 0.0;
			pos.FrozenFee = 0.0;
			pos.FrozenPosition = 0;

			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = InstCode;
			margin.LongMarginRate = 0.1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 0.09;
			margin.ShortMarginAmt = 0.0;
			pos.UsedMargin = algo.CalculateMargin(pos.YdPosition, pos.YdPositionPrice,
				pos.Direction, 1000, &margin); // 99800.00
			repo.InitAdd(pos);
		}
		{
			data::CInvestorPosition pos;
			pos.BrokerID = BrokerID;
			pos.InvestorID = InvestorID;
			//	pos.TradingDay
			pos.ExchangeID = "SGE";
			pos.InstrumentID = SpotInstCode;
			pos.ProductClass = type::ProductClass::Spot;
			pos.Direction = type::Direction::Buy;
			pos.Position = 0;
			pos.PositionCost = 1000;
			pos.YdPosition = 10;
			pos.YdPositionPrice = 998;
			pos.FrozenMargin = 0.0;
			pos.FrozenFee = 0.0;
			pos.FrozenPosition = 0;

			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = SpotInstCode;
			margin.LongMarginRate = 0.1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 0.09;
			margin.ShortMarginAmt = 0.0;
			pos.UsedMargin = 0;
			repo.InitAdd(pos);
		}

	}

	void InitInvestorFee(InvestorRepository& repo) {
		{
			data::CInvestorFee fee;
			fee.BrokerID = BrokerID;
			fee.ExchangeID = "SGE";
			fee.InvestorID = InvestorID;
			fee.InstrumentID = InstCode;
			fee.OpenFeeRate = 0.0006;
			fee.OpenFeeAmt = 0.0;
			fee.OffsetFeeRate = 0.0006;
			fee.OffsetFeeAmt = 0.0;
			fee.OffsetTodayFeeRate = 0.0006;
			fee.OffsetTodayFeeAmt = 0.0;
			repo.InitAdd(fee);
		}

		{
			data::CInvestorFee fee;
			fee.BrokerID = BrokerID;
			fee.ExchangeID = "SGE";
			fee.InvestorID = InvestorID;
			fee.InstrumentID = SpotInstCode;
			fee.OpenFeeRate = 0.0006;
			fee.OpenFeeAmt = 0.0;
			fee.OffsetFeeRate = 0.0006;
			fee.OffsetFeeAmt = 0.0;
			fee.OffsetTodayFeeRate = 0.0006;
			fee.OffsetTodayFeeAmt = 0.0;
			repo.InitAdd(fee);
		}

		{
			data::CInvestorFee fee;
			fee.BrokerID = BrokerID;
			fee.ExchangeID = "SGE";
			fee.InvestorID = InvestorID;
			fee.InstrumentID = UniInstCode;
			fee.OpenFeeRate = 0.0006;
			fee.OpenFeeAmt = 0.0;
			fee.OffsetFeeRate = 0.0006;
			fee.OffsetFeeAmt = 0.0;
			fee.OffsetTodayFeeRate = 0.0006;
			fee.OffsetTodayFeeAmt = 0.0;
			repo.InitAdd(fee);
		}

		{
			data::CInvestorFee fee;
			fee.BrokerID = BrokerID;
			fee.ExchangeID = "SGE";
			fee.InvestorID = InvestorID;
			fee.InstrumentID = UniInstCode2;
			fee.OpenFeeRate = 0.0006;
			fee.OpenFeeAmt = 0.0;
			fee.OffsetFeeRate = 0.0006;
			fee.OffsetFeeAmt = 0.0;
			fee.OffsetTodayFeeRate = 0.0006;
			fee.OffsetTodayFeeAmt = 0.0;
			repo.InitAdd(fee);
		}
	}

	void InitInvestorMargin(InvestorRepository& repo) {
		{
			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = InstCode;
			margin.LongMarginRate = 0.1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 0.09;
			margin.ShortMarginAmt = 0.0;
			repo.InitAdd(margin);
		}
		{
			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = SpotInstCode;
			margin.LongMarginRate = 1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 1;
			margin.ShortMarginAmt = 0.0;
			repo.InitAdd(margin);
		}

		{
			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = UniInstCode;
			margin.LongMarginRate = 0.1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 0.1;
			margin.ShortMarginAmt = 0.0;
			repo.InitAdd(margin);
		}

		{
			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = UniInstCode2;
			margin.LongMarginRate = 0.1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 0.1;
			margin.ShortMarginAmt = 0.0;
			repo.InitAdd(margin);
		}
	}

	void InitAccount(InvestorRepository& repo) {
		data::CInvestorAccount account;
		account.BrokerID = BrokerID;
		account.InvestorID = InvestorID;
		//	account.TradingDay
		account.PreBalance = 10'000'000.00;
		account.PreAvailable = 0.0;
		account.Balance = 0.00;
		account.Available = 0.00;
		account.Deposit = 0.00;
		account.Withdraw = 0.00;
		account.Margin = 998000.00;
		account.Fee = 0.00;
		account.OtherFee = 0.00;
		account.CloseProfit = 0.0;
		account.PositionProfit = 0.0;
		account.FrozenMargin = 0.0;
		account.FrozenFee = 0.0;
		account.LongMargin = 998000.00;
		account.ShortMargin = 0.00;
		account.LongFrozenMargin = 0.00;
		account.ShortFrozenMargin = 0.00;
		account.DynamicRights = 0.0;
		account.Risk = 0.0;
		repo.InitAdd(account);
	}

	void InitInvestorInfo(InvestorRepository& repo) {
		data::CInvestorInfo info;
		info.BrokerID = "6666";
		info.InvestorID = "8888";
		info.InvestorName = "Owen Test";
		info.Password = "Owen";
		info.ClientID = "9999";
		info.TradeRight = type::TradeRight::Allow;
		info.IsActive = type::IsActive::Active;
		repo.InitAdd(info);
	}

	data::CTrade tradeFromOrder(data::COrder const* order) {
		data::CTrade trade;
		trade.ExchangeID = order->ExchangeID;
		trade.BrokerID = order->BrokerID;
		trade.InvestorID = order->InvestorID;
		trade.InstrumentID = order->InstrumentID;
		trade.Direction = order->Direction;
		trade.OffsetFlag = order->OffsetFlag;
		trade.ProductClass = order->ProductClass;
		trade.OrderLocalID = order->OrderLocalID;
		trade.OrderSysID = order->OrderSysID;
		trade.TradeVolume = order->Volume;
		trade.TradePrice = order->LimitPrice;

		return trade;
	}


	TEST_CASE("Investor", "Investor Test") {

		auto normalOrderField = [&](data::COrder& field, int userLocalID = 1, int orderLocalId = 1, int volume = 10) {
			field.ExchangeID = "SGE";
			field.BrokerID = BrokerID;
			field.InvestorID = InvestorID;
			field.InstrumentID = InstCode;
			field.ProductClass = type::ProductClass::Defer;
			field.OrderPriceType = type::OrderPriceType::LimitPrice;
			field.Direction = type::Direction::Buy;
			field.OffsetFlag = type::OffsetFlag::Open;
			field.Volume = volume;
			field.LimitPrice = 1000.00;
			field.OrderLocalID = orderLocalId;
			char tmp[32] = "";
			sprintf_s(tmp, "%d", userLocalID);
			field.UserOrderLocalID = type::UserOrderLocalID(tmp);
		};

		auto spotOrderField = [&](data::COrder& field, int userLocalID = 1, int orderLocalId = 1, int volume = 10) {
			field.ExchangeID = "SGE";
			field.BrokerID = BrokerID;
			field.InvestorID = InvestorID;
			field.InstrumentID = SpotInstCode;
			field.ProductClass = type::ProductClass::Spot;
			field.OrderPriceType = type::OrderPriceType::LimitPrice;
			field.Direction = type::Direction::Buy;
			field.OffsetFlag = type::OffsetFlag::Open;
			field.Volume = volume;
			field.LimitPrice = 100.00;
			field.OrderLocalID = orderLocalId;
			char tmp[32] = "";
			sprintf_s(tmp, "%d", userLocalID);
			field.UserOrderLocalID = type::UserOrderLocalID(tmp);
		};

		auto unilateralOrderField = [&](data::COrder& field, int userLocalID = 1, int orderLocalId = 1, int volume = 5, bool isTwo = false) {
			field.ExchangeID = "SGE";
			field.BrokerID = BrokerID;
			field.InvestorID = InvestorID;
			field.InstrumentID = isTwo ? UniInstCode2 : UniInstCode;
			field.ProductClass = type::ProductClass::Defer;
			field.OrderPriceType = type::OrderPriceType::LimitPrice;
			field.Direction = type::Direction::Buy;
			field.OffsetFlag = type::OffsetFlag::Open;
			field.Volume = volume;
			field.LimitPrice = 1000.00;
			field.OrderLocalID = orderLocalId;
			char tmp[32] = "";
			sprintf_s(tmp, "%d", userLocalID);
			field.UserOrderLocalID = type::UserOrderLocalID(tmp);
		};

		using Catch::Matchers::VectorContains;
		SECTION("position test") {
			MyInvestorRepository investorRepo;
			data::CInvestorPosition pos;
			pos.BrokerID = BrokerID;
			pos.InvestorID = InvestorID;
			//	pos.TradingDay
			pos.ExchangeID = "SGE";
			pos.InstrumentID = InstCode;
			pos.ProductClass = type::ProductClass::Defer;
			pos.Direction = type::Direction::Buy;
			pos.Position = 0;
			pos.PositionCost = 1000;
			pos.YdPosition = 10;
			pos.YdPositionPrice = 998;
			pos.FrozenMargin = 0.0;
			pos.FrozenFee = 0.0;
			pos.FrozenPosition = 0;
			investorRepo.InitAdd(pos);
			investorRepo.InitAdd(pos);

		}

		SECTION("Create test") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, type::InvestorID("hahaha"));
				CHECK_FALSE(investor.IsSuccess());
			}
			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, type::BrokerID("hehehe"), type::InvestorID("hahaha"));
				CHECK_FALSE(investor.IsSuccess());
				CHECK(investor.Error == InvalidInvestorError);
			}
			{
				// 			auto investor = Investors::GetInvestor(investorRepo, instRepo, "hehehe", InvestorID);
				// 			CHECK_FALSE(investor.IsSuccess());
				// 			CHECK(investor.Error == InvalidInvestorError);
			}
			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				CHECK(investor.IsSuccess());
			}
			{
				// test cache
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				CHECK(investor.IsSuccess());
			}
		}

		/*
		#define type::OrderStatus::New	'n'
		#define type::OrderStatus::InsertQueueing 'a'			//订单已提交，未有系统号
		#define type::OrderStatus::NoTradeQueueing '3'			//交易所成功，返回了系统号
		#define type::OrderStatus::InsertFail 'f'				//订单失败
		#define type::OrderStatus::AllTraded '0'				//全部成交
		#define type::OrderStatus::PartTradedQueueing '1'		//部分成交还在队列中
		#define type::OrderStatus::CancelQueueing 'q'			//撤单已提交，未有应答
		#define type::OrderStatus::CancelFail 'c'				//取消失败
		#define type::OrderStatus::Canceled '5'					//已取消
		#define type::OrderStatus::NoTradeNotQueueing '4'		//未使用

		//限价
		#define type::OrderPriceType::LimitPrice '0'
		//FOK
		#define ZTP_OPT_FOKPrice '1'
		//FAK
		#define ZTP_OPT_FAKPrice '2'

		//未知市场
		#define ZTP_PCT_Unknown	'0'
		//延期市场
		#define type::ProductClass::Defer	'1'
		//现货市场
		#define type::ProductClass::Spot	'2'
		//交割申报
		#define ZTP_PCT_Delivery '3'
		//中立仓
		#define ZTP_PCT_Neutral '4'
		*/
		SECTION("Create Order Test") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				data::COrder field;
				normalOrderField(field);

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				CHECK(order->GetStatus() == type::OrderStatus::New);
				CHECK(order->GetFreezeFee() == Approx(10 * 1000 * 1000 * 0.0006));
				CHECK(order->GetFreezeMargin() == Approx(0.1 * 10 * 1000 * 1000));
				CHECK(order->GetInvestorPosition()->Position == 0);
				CHECK(investor->GetAvailable() == Approx(7996000.0));
			}

			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				data::COrder field;
				normalOrderField(field);

				auto order = investor->CreateOrder(field);
				CHECK(!order.IsSuccess());
				CHECK(order.Error == DuplicateOrderError);
			}
			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				data::COrder field;
				normalOrderField(field, 2, 2, 666);

				auto order = investor->CreateOrder(field);
				CHECK(!order.IsSuccess());
				CHECK(order.Error == NotEnoughMoneyError);
			}

			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				data::COrder field;
				normalOrderField(field, 3, 3, 3);

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
			}
			// position check

			{
				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				data::COrder field;
				field.ExchangeID = "SGE";
				field.BrokerID = BrokerID;
				field.InvestorID = InvestorID;
				field.InstrumentID = InstCode;
				field.ProductClass = type::ProductClass::Defer;
				field.OrderPriceType = type::OrderPriceType::LimitPrice;
				field.Direction = type::Direction::Sell;
				field.OffsetFlag = type::OffsetFlag::Close;
				field.Volume = 20;
				field.LimitPrice = 1000.00;
				field.OrderLocalID = 5;
				field.UserOrderLocalID = type::UserOrderLocalID("5");

				auto order = investor->CreateOrder(field);
				CHECK(!order.IsSuccess());
				CHECK(order.Error == NotEnoughPositoinError);
			}
		}

		SECTION("Order/Trade Happy Path") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			data::COrder field;
			normalOrderField(field);

			auto order = investor->CreateOrder(field);
			CHECK(order.IsSuccess());
			CHECK(order->GetStatus() == type::OrderStatus::New);
			CHECK(order->GetFreezeFee() == Approx(10 * 1000 * 1000 * 0.0006));
			CHECK(order->GetFreezeMargin() == Approx(0.1 * 10 * 1000 * 1000));
			CHECK(investor->GetAvailable() == Approx(7996000));
			CHECK(investor->GetAccount().LongFrozenMargin == Approx(0.1 * 10 * 1000 * 1000));
			CHECK(investor->GetAccount().ShortFrozenMargin == Approx(0.0));

			CHECK(order->GetInvestorPosition()->Position == 0);
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);


			//部分成交
			order->OnAccepted("SysNo1");
			auto tradeField = tradeFromOrder(order->GetOriginalOrder());
			tradeField.TradeVolume = 5;
			auto trade = order->CreateTrade(tradeField);
			order->OnTrade(trade);

			CHECK(order->GetOriginalOrder()->VolumeRemain == 5);
			CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
			CHECK(order->GetStatus() == type::OrderStatus::PartTradedQueueing);
			CHECK(order->GetFreezeFee() == Approx(3000));
			CHECK(order->GetFreezeMargin() == Approx(500000));
			CHECK(investor->GetAvailable() == Approx(7996000));
			CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(500000));
			CHECK(investor->GetAccount().LongMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 998000));
			CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().Fee.value() == Approx(5 * 1000 * 1000 * 0.0006));
			CHECK(investor->GetAccount().Margin.value() == Approx(0.1 * 5 * 1000 * 1000 + 998000));
			CHECK(order->GetInvestorPosition()->Position == 5);
			CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
			CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(998000 + 500000));
			{
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 4;
				tradeField.TradePrice = 1010.0;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 1);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 9);
				CHECK(order->GetStatus() == type::OrderStatus::PartTradedQueueing);
				CHECK(order->GetFreezeFee() == Approx(1 * 1000 * 1000 * 0.0006));
				CHECK(order->GetFreezeMargin() == Approx(0.1 * 1 * 1000 * 1000));
				CHECK(investor->GetAvailable() == Approx(7996000 - 0.1 * 4 * 10 * 1000 - 4 * 10 * 1000 * 0.0006));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0.1 * 1 * 1000 * 1000));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 0.1 * 4 * 1010 * 1000 + 998000));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().Fee.value() == Approx(5 * 1000 * 1000 * 0.0006 + 4 * 1010 * 1000 * 0.0006));
				CHECK(investor->GetAccount().Margin.value() == Approx(0.1 * 5 * 1000 * 1000 + 0.1 * 4 * 1010 * 1000 + 998000));
				CHECK(order->GetInvestorPosition()->Position == 9);
				CHECK(order->GetInvestorPosition()->PositionCost == Approx(1004.44));
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 0.1 * 4 * 1010 * 1000 + 998000));
			}

			order->OnCancelSending(type::UserOrderLocalID("3"), 3);
			CHECK(order->GetStatus() == type::OrderStatus::CancelQueueing);
			order->OnCancelled();
			CHECK(order->GetOriginalOrder()->VolumeRemain == 1);
			CHECK(order->GetOriginalOrder()->VolumeTrade == 9);
			CHECK(order->GetStatus() == type::OrderStatus::Canceled);
			CHECK(order->GetFreezeFee() == Approx(0));
			CHECK(order->GetFreezeMargin() == Approx(0));
			CHECK(investor->GetAvailable() == Approx(7991976.0 + 0.1 * 1 * 1000 * 1000 + 1 * 1000 * 1000 * 0.0006));
			CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
			CHECK(investor->GetAccount().LongMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 0.1 * 4 * 1010 * 1000 + 998000));
			CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().Fee.value() == Approx(5 * 1000 * 1000 * 0.0006 + 4 * 1010 * 1000 * 0.0006));
			CHECK(investor->GetAccount().Margin.value() == Approx(0.1 * 5 * 1000 * 1000 + 0.1 * 4 * 1010 * 1000 + 998000));
			CHECK(order->GetInvestorPosition()->Position == 9);
			CHECK(order->GetInvestorPosition()->PositionCost == Approx(1004.44));
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
			CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 0.1 * 4 * 1010 * 1000 + 998000));
		}


		SECTION("Order/Trade YDPostion Happy Path") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			data::COrder field;
			normalOrderField(field);
			field.Volume = 5;

			auto order = investor->CreateOrder(field);
			CHECK(order.IsSuccess());
			order->OnAccepted("SysNo1");
			auto tradeField = tradeFromOrder(order->GetOriginalOrder());
			tradeField.TradeVolume = 5;
			auto trade = order->CreateTrade(tradeField);
			order->OnTrade(trade);
			CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
			CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
			CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
			CHECK(order->GetFreezeFee() == Approx(0));
			CHECK(order->GetFreezeMargin() == Approx(0));
			CHECK(investor->GetAvailable() == Approx(8499000.0));
			CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
			CHECK(investor->GetAccount().LongMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 998000));
			CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().Fee.value() == Approx(5 * 1000 * 1000 * 0.0006));
			CHECK(investor->GetAccount().Margin.value() == Approx(0.1 * 5 * 1000 * 1000 + 998000));
			CHECK(order->GetInvestorPosition()->Position == 5);
			CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
			CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(998000 + 500000));


			{
				data::COrder field;
				normalOrderField(field, 2, 2);
				field.Volume = 12;
				field.OffsetFlag = type::OffsetFlag::Close;
				field.Direction = type::Direction::Sell;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				CHECK(order->GetOriginalOrder()->VolumeRemain == 12);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 0);
				CHECK(order->GetStatus() == type::OrderStatus::New);
				CHECK(order->GetFreezeFee() == Approx(12 * 1000 * 1000 * 0.0006));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(8491800));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(0.1 * 5 * 1000 * 1000 + 998000));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().Fee.value() == Approx(5 * 1000 * 1000 * 0.0006));
				CHECK(investor->GetAccount().Margin.value() == Approx(0.1 * 5 * 1000 * 1000 + 998000));
				CHECK(order->GetInvestorPosition()->Position == 5);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 12);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(998000 + 500000));



				order->OnAccepted("SysNo2");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 5;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);

				double preAvailable = 8491800.0;
				double tradeFreeMargin = 0.1 * 5 * 998 * 1000; // 按持仓也就是0.1来算返还的margin
				double closeProfit = (1000.0 - 998) * 1000 * 5;
				double todayPositionMargin = 0.1 * 5 * 1000 * 1000;
				double preYdPositionMargin = 998000;

				CHECK(order->GetOriginalOrder()->VolumeRemain == 7);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
				CHECK(order->GetStatus() == type::OrderStatus::PartTradedQueueing);
				CHECK(order->GetFreezeFee() == Approx(7 * 1000 * 1000 * 0.0006));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable + tradeFreeMargin + closeProfit));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(todayPositionMargin + preYdPositionMargin - tradeFreeMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(10 * 1000 * 1000 * 0.0006));
				CHECK(investor->GetAccount().Margin.value() == Approx(todayPositionMargin + preYdPositionMargin - tradeFreeMargin));
				CHECK(investor->GetAccount().CloseProfit.value() == Approx(closeProfit));
				CHECK(order->GetInvestorPosition()->Position == 5);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == (12 - 5));
				CHECK(order->GetInvestorPosition()->YdPosition == (10 - 5));
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(todayPositionMargin + preYdPositionMargin - tradeFreeMargin));
				auto tradeField2 = tradeFromOrder(order->GetOriginalOrder());
				tradeField2.TradeVolume = 7;
				auto trade2 = order->CreateTrade(tradeField2);
				order->OnTrade(trade2);

				preAvailable = 9000800;
				tradeFreeMargin = 0.1 * 5 * 998 * 1000 + 0.1 * 2 * 1000 * 1000; // 按持仓也就是0.1来算返还的margin
				double preCloseProfit = (1000.0 - 998) * 1000 * 5;
				closeProfit = (1000.0 - 998) * 1000 * 5;
				todayPositionMargin = 0.1 * 3 * 1000 * 1000;

				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 12);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0.0));
				CHECK(order->GetFreezeMargin() == Approx(0.0));
				CHECK(investor->GetAvailable() == Approx(preAvailable + tradeFreeMargin + closeProfit));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(todayPositionMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(17 * 1000 * 1000 * 0.0006));
				CHECK(investor->GetAccount().Margin.value() == Approx(todayPositionMargin));
				CHECK(investor->GetAccount().CloseProfit.value() == Approx(preCloseProfit + closeProfit));
				CHECK(order->GetInvestorPosition()->Position == 3);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->YdPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(todayPositionMargin));
			}
		}


		SECTION("Spot Order/Trade YDPostion Happy Path") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			data::COrder field;
			spotOrderField(field);
			field.Volume = 5;

			double preAvailable = 10'000'000.00;
			double preMargin = 998000;
			double tradeMargin = 1 * 5 * 100 * 1000;
			double tradeFee = 5 * 100 * 1000 * 0.0006;

			auto order = investor->CreateOrder(field);
			CHECK(order.IsSuccess());
			order->OnAccepted("SysNo1");
			auto tradeField = tradeFromOrder(order->GetOriginalOrder());
			tradeField.TradeVolume = 5;
			auto trade = order->CreateTrade(tradeField);
			order->OnTrade(trade);
			CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
			CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
			CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
			CHECK(order->GetFreezeFee() == Approx(0));
			CHECK(order->GetFreezeMargin() == Approx(0));
			CHECK(investor->GetAvailable() == Approx(preAvailable - preMargin - tradeMargin - tradeFee));
			CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
			CHECK(investor->GetAccount().LongMargin.value() == Approx(tradeMargin + preMargin));
			CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().Fee.value() == Approx(tradeFee));
			CHECK(investor->GetAccount().Margin.value() == Approx(tradeMargin + preMargin));
			CHECK(order->GetInvestorPosition()->Position == 5);
			CHECK(order->GetInvestorPosition()->PositionCost == 100.0);
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
			CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(tradeMargin));

			CHECK(order->GetInvestorPosition()->YdPosition == 10);


			const double SPOT_MARGIN_PERCENT = 0.9;
			preAvailable = preAvailable - preMargin - tradeMargin - tradeFee;
			double preLongMargin = tradeMargin + preMargin; 
			double preUsedMargin = tradeMargin;
			double preFee = tradeFee;
			{
				data::COrder field;
				spotOrderField(field, 2, 2);
				field.Volume = 12;
				field.LimitPrice = 98.8;
				field.OffsetFlag = type::OffsetFlag::Close;
				field.Direction = type::Direction::Sell;


				double freezeFee = 12 * 98.8 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				CHECK(order->GetOriginalOrder()->VolumeRemain == 12);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 0);
				CHECK(order->GetStatus() == type::OrderStatus::New);
				CHECK(order->GetFreezeFee() == Approx(freezeFee));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - freezeFee));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(preLongMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(preLongMargin));
				CHECK(order->GetInvestorPosition()->Position == 5);
				CHECK(order->GetInvestorPosition()->PositionCost == 100.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 12);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(preUsedMargin));

				preAvailable = preAvailable - freezeFee;

				order->OnAccepted("SysNo2");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 5;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);


				double tradeFreeMargin = 1 * 5 * trade.GetPrice() * 1000 * SPOT_MARGIN_PERCENT; 
				double closeProfit = 0;// (1000.0 - 998) * 1000 * 5; 昨仓没有CloseProfit
				double tradeFee = 5 * trade.GetPrice() * 1000 * 0.0006;
				double unfreezeFee = tradeFee;

				CHECK(order->GetOriginalOrder()->VolumeRemain == 7);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
				CHECK(order->GetStatus() == type::OrderStatus::PartTradedQueueing);
				CHECK(order->GetFreezeFee() == Approx(freezeFee - unfreezeFee));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable + tradeFreeMargin + closeProfit));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(preLongMargin - tradeFreeMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(preLongMargin - tradeFreeMargin));
				CHECK(investor->GetAccount().CloseProfit.value() == Approx(closeProfit));
				CHECK(order->GetInvestorPosition()->Position == 5);
				CHECK(order->GetInvestorPosition()->PositionCost == 100.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == (12 - 5));
				CHECK(order->GetInvestorPosition()->YdPosition == (10 - 5));
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(preUsedMargin - tradeFreeMargin));

				freezeFee = freezeFee - unfreezeFee;
				preAvailable = preAvailable + tradeFreeMargin + closeProfit;
				preLongMargin = preLongMargin - tradeFreeMargin;
				double preCloseProfit = closeProfit;
				preUsedMargin = preUsedMargin - tradeFreeMargin;

				auto tradeField2 = tradeFromOrder(order->GetOriginalOrder());
				tradeField2.TradeVolume = 7;
				auto trade2 = order->CreateTrade(tradeField2);
				order->OnTrade(trade2);

				tradeFreeMargin = (1 * 5 * trade.GetPrice() * 1000 )* SPOT_MARGIN_PERCENT + 1 * 2 * 100.00 * 1000 * SPOT_MARGIN_PERCENT;
				closeProfit = (98.8 - 100.0) * 1000 * 2;
				tradeFee = 7 * 98.8 * 1000 * 0.0006;
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 12);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0.0));
				CHECK(order->GetFreezeMargin() == Approx(0.0));
				CHECK(investor->GetAvailable() == Approx(preAvailable + tradeFreeMargin + closeProfit));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(preLongMargin - tradeFreeMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(5 * 100 * 1000 * 0.0006 + 12 * 98.8 * 1000 * 0.0006));
				CHECK(investor->GetAccount().Margin.value() == Approx(preLongMargin - tradeFreeMargin));
				CHECK(investor->GetAccount().CloseProfit.value() == Approx(preCloseProfit + closeProfit));
				CHECK(order->GetInvestorPosition()->Position == 3);
				CHECK(order->GetInvestorPosition()->PositionCost == 100.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->YdPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(preUsedMargin - tradeFreeMargin));
			}
		}


		SECTION("Unilateral Order/Trade Happy Path") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			data::COrder field;
			unilateralOrderField(field);
			field.Direction = type::Direction::Sell;

			double preAvailable = 10'000'000.00;
			double initMargin = 998000;
			double tradeMargin = 0.1 * 5 * 1000 * 1000;
			double tradeFee = 5 * 1000 * 1000 * 0.0006;

			auto order = investor->CreateOrder(field);
			CHECK(order.IsSuccess());
			order->OnAccepted("SysNo1");
			auto tradeField = tradeFromOrder(order->GetOriginalOrder());
			auto trade = order->CreateTrade(tradeField);
			order->OnTrade(trade);
			CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
			CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
			CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
			CHECK(order->GetFreezeFee() == Approx(0));
			CHECK(order->GetFreezeMargin() == Approx(0));
			CHECK(investor->GetAvailable() == Approx(preAvailable - initMargin - tradeMargin - tradeFee));
			CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
			CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin));
			CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().ShortMargin.value() == Approx(tradeMargin));
			CHECK(investor->GetAccount().Fee.value() == Approx(tradeFee));
			CHECK(investor->GetAccount().Margin.value() == Approx(tradeMargin + initMargin));
			CHECK(order->GetInvestorPosition()->Position == 5);
			CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
			CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(tradeMargin));
			CHECK(order->GetInvestorPosition()->Position == 5);
			preAvailable = preAvailable - initMargin - tradeMargin - tradeFee;
			double preLongMargin = tradeMargin + initMargin;
			double preUsedMargin = tradeMargin;
			double preFee = tradeFee;
			double preShortMargin = tradeMargin;
			{
				data::COrder field;

				unilateralOrderField(field, 2, 2 ,2);

				tradeMargin = 0.1 * 2 * 1000 * 1000;
				tradeFee = 2 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo2");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 2;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 2);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(preShortMargin));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin + preShortMargin));
				CHECK(order->GetInvestorPosition()->Position == 2);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(tradeMargin));
			}
			preFee += tradeFee;
			preAvailable -= tradeFee;

			{
				data::COrder field;

				unilateralOrderField(field, 3, 3, 3, true);

				tradeMargin = 0.1 * 3 * 1000 * 1000;
				tradeFee = 3 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo3");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 3;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 3);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0).margin(0.01));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin + preShortMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin + preShortMargin));
				CHECK(order->GetInvestorPosition()->Position == 3);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(tradeMargin));
			}

			preFee = investor->GetAccount().Fee.value();
			preAvailable = investor->GetAvailable();

			{
				data::COrder field;

				unilateralOrderField(field, 4, 4, 3, true);

				double unitMargin = 0.1 * 1000 * 1000;
				tradeMargin = unitMargin * 3;
				tradeFee = 3 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo4");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 3;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 3);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0.0).margin(0.01));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee - unitMargin*3));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin + unitMargin*8));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin + unitMargin*8));
				CHECK(order->GetInvestorPosition()->Position == 6);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(unitMargin * 6));
			}


			preFee = investor->GetAccount().Fee.value();
			preAvailable = investor->GetAvailable();

			{
				data::COrder field;

				unilateralOrderField(field, 5, 5, 3, true);
				field.OffsetFlag = type::OffsetFlag::Close;

				double unitMargin = 0.1 * 1000 * 1000;
				tradeMargin = 0;
				tradeFee = 3 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo5");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 3;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 3);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0.0).margin(0.01));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee + unitMargin * 3));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin + unitMargin * 5));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin + unitMargin * 5));
				CHECK(order->GetInvestorPosition()->Position == 3);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(unitMargin * 3));
			}

			preFee = investor->GetAccount().Fee.value();
			preAvailable = investor->GetAvailable();

			{
				data::COrder field;

				unilateralOrderField(field, 6, 6, 1 , true);
				field.OffsetFlag = type::OffsetFlag::Close;

				double unitMargin = 0.1 * 1000 * 1000;
				tradeMargin = 0;
				tradeFee = 1 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo6");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 1);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(unitMargin * 5));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin + unitMargin * 5));
				CHECK(order->GetInvestorPosition()->Position == 2);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(unitMargin * 2));
			}
		}


		SECTION("Unilateral Order/Trade Zero Test") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);
			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			data::COrder field;
			unilateralOrderField(field);
			field.Direction = type::Direction::Sell;

			double preAvailable = 10'000'000.00;
			double initMargin = 998000;
			double tradeMargin = 0.1 * 5 * 1000 * 1000;
			double tradeFee = 5 * 1000 * 1000 * 0.0006;

			auto order = investor->CreateOrder(field);
			CHECK(order.IsSuccess());
			order->OnAccepted("SysNo1");
			auto tradeField = tradeFromOrder(order->GetOriginalOrder());
			auto trade = order->CreateTrade(tradeField);
			order->OnTrade(trade);
			CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
			CHECK(order->GetOriginalOrder()->VolumeTrade == 5);
			CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
			CHECK(order->GetFreezeFee() == Approx(0).margin(0.01));
			CHECK(order->GetFreezeMargin() == Approx(0));
			CHECK(investor->GetAvailable() == Approx(preAvailable - initMargin - tradeMargin - tradeFee));
			CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
			CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin));
			CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
			CHECK(investor->GetAccount().ShortMargin.value() == Approx(tradeMargin));
			CHECK(investor->GetAccount().Fee.value() == Approx(tradeFee));
			CHECK(investor->GetAccount().Margin.value() == Approx(tradeMargin + initMargin));
			CHECK(order->GetInvestorPosition()->Position == 5);
			CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
			CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
			CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(tradeMargin));
			CHECK(order->GetInvestorPosition()->Position == 5);
			preAvailable = preAvailable - initMargin - tradeMargin - tradeFee;
			double preUsedMargin = tradeMargin;
			double preFee = tradeFee;
			double preShortMargin = tradeMargin;
			double returnMargin = 0.0;
			{
				data::COrder field;

				unilateralOrderField(field, 2, 2, 2);
				field.OffsetFlag = type::OffsetFlag::Close;

				returnMargin = 0.1 * 2 * 1000 * 1000;
				tradeFee = 2 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo2");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 2;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 2);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee + returnMargin));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(preShortMargin - returnMargin));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin + preShortMargin - returnMargin));
				CHECK(order->GetInvestorPosition()->Position == 3);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(preShortMargin - returnMargin));
			}
			preFee += tradeFee;
			preAvailable = preAvailable + returnMargin - tradeFee;

			{
				data::COrder field;

				unilateralOrderField(field, 3, 3, 3);
				field.OffsetFlag = type::OffsetFlag::Close;

				returnMargin = 0.1 * 3 * 1000 * 1000;
				tradeFee = 3 * 1000 * 1000 * 0.0006;

				auto order = investor->CreateOrder(field);
				CHECK(order.IsSuccess());
				order->OnAccepted("SysNo3");
				auto tradeField = tradeFromOrder(order->GetOriginalOrder());
				tradeField.TradeVolume = 3;
				auto trade = order->CreateTrade(tradeField);
				order->OnTrade(trade);
				CHECK(order->GetOriginalOrder()->VolumeRemain == 0);
				CHECK(order->GetOriginalOrder()->VolumeTrade == 3);
				CHECK(order->GetStatus() == type::OrderStatus::AllTraded);
				CHECK(order->GetFreezeFee() == Approx(0).margin(0.01));
				CHECK(order->GetFreezeMargin() == Approx(0));
				CHECK(investor->GetAvailable() == Approx(preAvailable - tradeFee + returnMargin));
				CHECK(investor->GetAccount().LongFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().LongMargin.value() == Approx(initMargin));
				CHECK(investor->GetAccount().ShortFrozenMargin.value() == Approx(0.0));
				CHECK(investor->GetAccount().ShortMargin.value() == Approx(0));
				CHECK(investor->GetAccount().Fee.value() == Approx(preFee + tradeFee));
				CHECK(investor->GetAccount().Margin.value() == Approx(initMargin));
				CHECK(order->GetInvestorPosition()->Position == 0);
				CHECK(order->GetInvestorPosition()->PositionCost == 1000.0);
				CHECK(order->GetInvestorPosition()->FrozenPosition == 0);
				CHECK(order->GetInvestorPosition()->UsedMargin.value() == Approx(0.0));
			}
		}

		SECTION("Unilateral Position init test") {
			Investors::Clear();
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitInstrument(instRepo);
			InitInvestor(investorRepo);


			CAlgorithms algo;
			{
				data::CInvestorPosition pos;
				pos.BrokerID = BrokerID;
				pos.InvestorID = InvestorID;
				//	pos.TradingDay
				pos.ExchangeID = "SGE";
				pos.InstrumentID = UniInstCode;
				pos.ProductClass = type::ProductClass::Defer;
				pos.Direction = type::Direction::Buy;
				pos.Position = 0;
				pos.PositionCost = 1000;
				pos.YdPosition = 100;
				pos.YdPositionPrice = 998;
				pos.FrozenMargin = 0.0;
				pos.FrozenFee = 0.0;
				pos.FrozenPosition = 0;

				data::CInvestorMargin margin;
				margin.BrokerID = BrokerID;
				margin.ExchangeID = "SGE";
				margin.InvestorID = InvestorID;
				margin.InstrumentID = UniInstCode;
				margin.LongMarginRate = 0.1;
				margin.LongMarginAmt = 0.0;
				margin.ShortMarginRate = 0.1;
				margin.ShortMarginAmt = 0.0;
				pos.UsedMargin = algo.CalculateMargin(pos.YdPosition, pos.YdPositionPrice,
					pos.Direction, 1000, &margin); // 99800.00
				auto* pos1 = investorRepo.InitAdd(pos);

				auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
				CHECK(investor.IsSuccess());
				investor->InitUnilateralPosition(*instRepo.Get(UniInstCode), pos1);

				pos.Direction = type::Direction::Sell;
				pos.YdPosition = 100;
				pos.YdPositionPrice = 999;
				pos.UsedMargin = algo.CalculateMargin(pos.YdPosition, pos.YdPositionPrice,
					pos.Direction, 1000, &margin); // 99900.00
				pos1 = investorRepo.InitAdd(pos);

				investor->InitUnilateralPosition(*instRepo.Get(UniInstCode), pos1);

				investor->InitAccountMarginByPositions();
				CHECK(investor->GetAccount().Margin == Approx(998000 + 9990000));
			}

		}


		SECTION("Balance Deposit/Withdraw test") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);

			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			CHECK(investor.IsSuccess());
			data::CBalanceChange change;
			change.BrokerID = BrokerID;
			change.InvestorID = InvestorID;
			change.BalanceChangeType = type::BalanceChangeFlag::Deposit;
			change.ExchangeSequenceNo = 0;
			change.BalanceSequenceNo = 0;
			change.Amount = 10000;
			investor->OnBalanceChange(change);
			auto& account = investor->GetAccount();
			CHECK(account.Deposit == change.Amount);
			CHECK(account.Available == 10'000'000 - 998000 + 10000);
			change.BalanceChangeType = type::BalanceChangeFlag::Withdraw;
			investor->OnBalanceChange(change);
			CHECK(account.Deposit == change.Amount);
			CHECK(account.Withdraw == change.Amount);
			CHECK(account.Available == 10'000'000 - 998000);
		}

		SECTION("Spot Deposit/Withdraw test") {
			Investors::Clear();
			MemSystemStatusRepository statusRepo;
			MyBaseInfoRepository baseRepo;
			MyInvestorRepository investorRepo;
			MyInstrumentRepository instRepo;

			InitSeatInfo(baseRepo);
			InitInstrument(instRepo);
			InitInvestor(investorRepo);

			auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
			CHECK(investor.IsSuccess());
			data::CPositionChange change;
			change.BrokerID = BrokerID;
			change.InvestorID = InvestorID;
			change.InstrumentID = SpotInstCode;
			change.ChangeType = type::PositionChangeFlag::Spot;
			change.ProductClass = type::ProductClass::Spot;
			change.ExchangeSequenceNo = 0;
			change.Direction = type::Direction::Buy;
			change.OffsetFlag = type::OffsetFlag::Open;
			change.Volume = 10;
			investor->OnPosition(change);

			data::CInvestorPosition* pPosition = investorRepo.GetPosition(change.InvestorID, change.InstrumentID, change.Direction);
			CHECK(pPosition != nullptr);
			CHECK(pPosition->YdPosition == 20);

			change.OffsetFlag = type::OffsetFlag::Close;
			change.Volume = 5;
			investor->OnPosition(change);
			CHECK(pPosition->YdPosition == 15);

		}
	}
}