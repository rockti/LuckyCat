#include "stdafx.h"
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#undef max
#undef min
#include <catch.hpp>
#include "Investor.h"
#include "Repository/MemoryRepositoryImpl.h"

namespace {
	type::BrokerID BrokerID{ "6666" };
	type::InvestorID InvestorID{ "8888" };
	type::InstrumentID InstCode{ "Au(T+D)" };
	type::InstrumentID SpotInstCode{ "Au100g" };
	type::ClientID ClientID { "9999" };

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

	data::CInstrument g_insts[100];
	data::CInvestorInfo g_investorInfo[30];
	data::CInvestorAccount g_account[30];

	void InitInvestorInfo(InvestorRepository& repo) {
		for (size_t i = 0; i < 30; i++) {
			data::CInvestorInfo& info = g_investorInfo[i];
			info.BrokerID = "6666";
			info.InvestorID = fmt::format("{}", i + 1000).c_str();
			info.InvestorName = "Owen Test";
			info.Password = "Owen";
			info.ClientID = "9999";
			info.TradeRight = type::TradeRight::Allow;
			info.IsActive = type::IsActive::Active;
			repo.InitAdd(info);
		}
	}

	void InitInstrument(InstrumentRepository& repo) {
		for (size_t i = 0; i < 100; ++i) {
			data::CInstrument& inst = g_insts[i];
			inst.ExchangeID = "SGE";
			inst.InstrumentID = fmt::format("INST{}", i).c_str();
			inst.InstrumentName = "AU";
			inst.ProductClass = type::ProductClass::Defer;
			inst.LongPosLimit = 1000;
			inst.ShortPosLimit = 1000;
			inst.MaxOrderVolume = 999;
			inst.MinOrderVolume = 2;
			inst.VolumeMultiple = 1000;
			inst.PriceTick = 0.01;
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
		for (auto& investor : g_investorInfo) {
			for (auto& inst : g_insts) {
				data::CInvestorPosition pos;
				pos.BrokerID = BrokerID;
				pos.InvestorID = investor.InvestorID;
				pos.ExchangeID = "SGE";
				pos.InstrumentID = inst.InstrumentID;
				pos.ProductClass = type::ProductClass::Defer;
				pos.Direction = type::Direction::Buy;
				pos.Position = 0;
				pos.PositionCost = 1000;
				pos.YdPosition = 10;
				pos.YdPositionPrice = 998;
				pos.FrozenMargin = 0.0;
				pos.FrozenFee = 0.0;
				pos.FrozenPosition = 0;
				pos.UsedMargin = 99800;
				repo.InitAdd(pos);
				pos.Direction = type::Direction::Sell;
				repo.InitAdd(pos);
			}
		}
	}

	void InitInvestorFee(InvestorRepository& repo) {
		for (auto& investor : g_investorInfo) {
			for (auto& inst : g_insts) {
				data::CInvestorFee fee;
				fee.BrokerID = BrokerID;
				fee.ExchangeID = "SGE";
				fee.InvestorID = investor.InvestorID;
				fee.InstrumentID = inst.InstrumentID;
				fee.OpenFeeRate = 0.0006;
				fee.OpenFeeAmt = 0.0;
				fee.OffsetFeeRate = 0.0006;
				fee.OffsetFeeAmt = 0.0;
				fee.OffsetTodayFeeRate = 0.0006;
				fee.OffsetTodayFeeAmt = 0.0;
				repo.InitAdd(fee);
			}
		}

	}

	void InitInvestorMargin(InvestorRepository& repo) {

		for (auto& investor : g_investorInfo) {
			for (auto& inst : g_insts) {
				data::CInvestorMargin margin;
				margin.BrokerID = BrokerID;
				margin.ExchangeID = "SGE";
				margin.InvestorID = investor.InvestorID;
				margin.InstrumentID = inst.InstrumentID;
				margin.LongMarginRate = 0.1;
				margin.LongMarginAmt = 0.0;
				margin.ShortMarginRate = 0.09;
				margin.ShortMarginAmt = 0.0;
				repo.InitAdd(margin);
			}
		}
	}

	void InitAccount(InvestorRepository& repo) {
		for (auto& investor : g_investorInfo) {
			data::CInvestorAccount account;
			account.BrokerID = BrokerID;
			account.InvestorID = investor.InvestorID;
			account.PreBalance = 10'000'000.00;
			account.PreAvailable = 0.0;
			account.Balance = 0.00;
			account.Available = 0.00;
			account.Deposit = 0.00;
			account.Withdraw = 0.00;
			account.Margin = 99800.00;
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


	TEST_CASE("Repo", "Repo Test") {

		auto normalOrderField = [&](data::COrder& field, int userLocalID = 1, int orderLocalId = 1, int volume = 3) {
			field.ExchangeID = "SGE";
			field.BrokerID = BrokerID;
// 			field.InvestorID = InvestorID;
// 			field.InstrumentID = InstCode;
			field.ProductClass = type::ProductClass::Defer;
			field.OrderPriceType = type::OrderPriceType::LimitPrice;
			field.Direction = type::Direction::Buy;
			field.OffsetFlag = type::OffsetFlag::Open;
			field.Volume = volume;
			field.LimitPrice = 1000.00;
			field.OrderLocalID = orderLocalId;
			char tmp[32] = "";
			sprintf(tmp, "%d", userLocalID);
			field.UserOrderLocalID = type::UserOrderLocalID(tmp);
			field.OrderSysID = fmt::format("{}", orderLocalId).c_str();
		};

		MyInvestorRepository investorRepo;
		MyInstrumentRepository instRepo;
		InitInstrument(instRepo);
		InitInvestor(investorRepo);
		using Catch::Matchers::VectorContains;
		SECTION("Position ForEach Test") {
			bool last = false;
			std::vector<data::CInvestorPosition const*> poses;
			investorRepo.PositionForEach(type::InvestorID("1000"), [&](data::CInvestorPosition const& pos, bool isLast) {
				poses.push_back(&pos);
				last = isLast;
				});
			CHECK(last);
			CHECK(poses.size() == 100 * 2);

			investorRepo.PositionForEach(type::InvestorID("1001"), [&](data::CInvestorPosition const& pos, bool isLast) {
				poses.push_back(&pos);
				last = isLast;
				});
			CHECK(last);
			CHECK(poses.size() == 100 * 2 + 100 * 2);
		}

		SECTION("Margin ForEach Test") {
			bool last = false;
			std::vector<data::CInvestorMargin const*> poses;
			investorRepo.MarginForEach(type::InvestorID("1000"),type::InstrumentID("*"), [&](data::CInvestorMargin const& pos, bool isLast) {
				poses.push_back(&pos);
				last = isLast;
				});
			CHECK(last);
			CHECK(poses.size() == 100);
		}

		SECTION("Fee ForEach Test") {
			bool last = false;
			std::vector<data::CInvestorFee const*> poses;
			investorRepo.FeeForEach(type::InvestorID("1000"),type::InstrumentID("*"), [&](data::CInvestorFee const& pos, bool isLast) {
				poses.push_back(&pos);
				last = isLast;
				});
			CHECK(last);
			CHECK(poses.size() == 100);
		}

		SECTION("Order/Trade ForEach Test") {
			bool last = false;
			std::vector<data::COrder const*> orders;
			investorRepo.OrderForEach(type::InvestorID("1000"), type::InstrumentID("INST0"), [&](data::COrder const& order, bool isLast) {
				orders.push_back(&order);
				last = isLast;
				});
			CHECK_FALSE(last);
			CHECK(orders.size() == 0);
			data::COrder order;
			order.InvestorID = g_investorInfo[0].InvestorID;
			order.InstrumentID = g_insts[0].InstrumentID;
			normalOrderField(order);
			investorRepo.Add(order);
			investorRepo.AddOrderSysNoIndex(order);
			CHECK(investorRepo.GetOrder(order.InvestorID, order.OrderSysID)->OrderLocalID.value() == order.OrderLocalID.value());
			CHECK(investorRepo.GetOrder(order.InvestorID, order.OrderLocalID)->OrderLocalID.value() == order.OrderLocalID.value());
			CHECK(investorRepo.GetOrder(order.InvestorID, order.UserOrderLocalID)->OrderLocalID.value() == order.OrderLocalID.value());
			investorRepo.OrderForEach(order.InvestorID, order.InstrumentID, [&](data::COrder const& order, bool isLast) {
				orders.push_back(&order);
				last = isLast;
				});
			CHECK(last);
			CHECK(orders.size() == 1);

			auto trade = tradeFromOrder(&order);
			trade.TradeVolume = 2;
			trade.TradeID = "T1";
			investorRepo.Add(trade);
			trade.TradeID = "T2";
			investorRepo.Add(trade);
			trade.TradeID = "T3";
			investorRepo.Add(trade);
			trade.OrderSysID = "nobody";
			trade.TradeID = "T4";
			investorRepo.Add(trade);
			{
				std::vector<data::CTrade const*> trades;
				investorRepo.TradeForEach(order.InvestorID, order.OrderSysID, [&](data::CTrade const& trade, bool isLast) {
					trades.push_back(&trade);
					last = isLast;
					});
				CHECK(last);
				CHECK(trades.size() == 3);
			}
			{
				std::vector<data::CTrade const*> trades;
				investorRepo.TradeForEach(order.InvestorID, order.InstrumentID, [&](data::CTrade const& trade, bool isLast) {
					trades.push_back(&trade);
					last = isLast;
					});
				CHECK(last);
				CHECK(trades.size() == 4);
			}
		}

		for (size_t i = 0; i < 5000; i++) {
			data::COrder order;
			order.InvestorID = g_investorInfo[1].InvestorID;
			order.InstrumentID = g_insts[1].InstrumentID;
			normalOrderField(order,i+2,i+2);
			investorRepo.Add(order);
			investorRepo.AddOrderSysNoIndex(order);

			auto trade = tradeFromOrder(&order);
			trade.TradeVolume = 1;
			trade.TradeID = "T1";
			investorRepo.Add(trade);
			trade.TradeID = "T2";
			investorRepo.Add(trade);
			trade.TradeID = "T3";
			investorRepo.Add(trade);
		}

		int orderCnt = 0;
		investorRepo.OrderForEach(g_investorInfo[1].InvestorID, g_insts[1].InstrumentID, [&](data::COrder const& order, bool isLast) {
			data::COrder field;
			CopyZTPOrderEntity(&field, &order);
			orderCnt++;
			});
		CHECK(orderCnt == 5000);
// 
// 		BENCHMARK("Fee1") {
// 			investorRepo.FeeForEach("1000", [&](CInvestorFee const& fee, bool isLast) {
// 				CInvestorFeeField field;
// 				CopyInvestorFeeEntity(&field, &fee);
// 				});
// 			return;
// 		};
// 
// 		BENCHMARK("Fee2") {
// 			investorRepo.FeeForEach("1001", [&](CInvestorFee const& fee, bool isLast) {
// 				CInvestorFeeField field;
// 				CopyInvestorFeeEntity(&field, &fee);
// 				});
// 			return;
// 		};
// 		BENCHMARK("Order For Each") {
// 			investorRepo.OrderForEach(g_investorInfo[1].InvestorID, g_insts[1].InstrumentID,[&](COrder const& order, bool isLast) {
// 				COrderField field;
// 				CopyZTPOrderEntity(&field, &order);
// 				});
// 			return;
// 		};
// 		COrderSysIDType sysId = "3";
// 		COrderLocalIDType localId = 3;
// 		CUserOrderLocalIDType userLocalID = "3";
// 		BENCHMARK("Order by SysID") {
// 			return investorRepo.GetOrder(g_investorInfo[1].InvestorID, sysId);
// 		};
// 		BENCHMARK("Order by LocalID") {
// 			return investorRepo.GetOrder(g_investorInfo[1].InvestorID,localId);
// 		};
// 		BENCHMARK("Order by UserLocalID") {
// 			return investorRepo.GetOrder(g_investorInfo[1].InvestorID, userLocalID);
// 		};
// 
// 
// 		BENCHMARK("Trade For Each by SysID") {
// 			investorRepo.TradeForEach(g_investorInfo[1].InvestorID, sysId, [&](CTrade const& trade, bool isLast) {
// 				CTradeField field;
// 				CopyTradeEntity(&field, &trade);
// 				});
// 			return;
// 		};
// 
// 		BENCHMARK("Trade For Each by InstrumentID") {
// 			investorRepo.TradeForEach(g_investorInfo[1].InvestorID, g_insts[1].InstrumentID, [&](CTrade const& trade, bool isLast) {
// 				CTradeField field;
// 				CopyTradeEntity(&field, &trade);
// 				});
// 			return;
// 		};

	}
}