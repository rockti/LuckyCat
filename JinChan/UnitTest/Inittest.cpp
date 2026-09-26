#include "stdafx.h"

#include <catch.hpp>
#include "RateAlgorithms.h"
#include "Repository/MemoryRepositoryImpl.h"
#include "ZTPPackage.h"
#include "InitServiceLink.h"
#include <fmt/format.h>
#include "Investor.h"
namespace {
	type::BrokerID BrokerID{ "6666" };
	type::InvestorID InvestorID { "8888" };
	type::InstrumentID InstCode{ "Au(T+D)" };
	type::InstrumentID SpotInstCode{ "Au100g" };
	type::ClientID ClientID{ "9999" };
	std::vector<type::InstrumentID> auInsts = { 
		type::InstrumentID("Au(T+D)"),
		type::InstrumentID("mAu(T+D)"),
		type::InstrumentID("Au(T+N1)"),
		type::InstrumentID("Au(T+N2)"),
		type::InstrumentID("Ag(T+D)") };



	template<class T>
	void init(CInitServiceLink& initLink, CZTPPackage& package, T& t, int TID) {
		package.PreparePublish(TID);
		package.AddField(t);
		package.MakePackage();
		package.ValidPackage();
		initLink.HandleMessage(package);
	}

	void PushSeatInfo(CInitServiceLink& initLink, CZTPPackage& package) {
		data::CSeatInfo seatInfo;
		seatInfo.ExchangeID = "SGE";
		seatInfo.SeatID = "6666";
		seatInfo.SeatStatus = type::SeatStatus::Login;
		seatInfo.TraderID = "9999";
		seatInfo.AppName = "Trade";
		seatInfo.ParticipantID = type::ParticipantID(BrokerID.value());
		init(initLink, package, seatInfo, TID_InitSeatInfo);
	}

	void PushInstrument(CInitServiceLink& initLink, CZTPPackage& package) {
		for (auto const& v : auInsts) {
			data::CInstrument inst;
			inst.ExchangeID = "SGE";
			inst.InstrumentID = v;
			inst.InstrumentName = "AU";
			inst.ProductClass = type::ProductClass::Defer;
			inst.LongPosLimit = 1000;
			inst.ShortPosLimit = 1000;
			inst.MaxOrderVolume = 999;
			inst.MinOrderVolume = 2;
			inst.VolumeMultiple = 1000;
			inst.PriceTick = 0.01;
			init(initLink, package, inst, TID_InitInstrument);
		}
	}


	void PushInvestorInfo(CInitServiceLink& initLink, CZTPPackage& package) {
		data::CInvestorInfo info;
		info.BrokerID = "6666";
		info.InvestorID = "8888";
		info.InvestorName = "Owen Test";
		info.Password = "Owen";
		info.ClientID = "9999";
		info.TradeRight = type::TradeRight::Allow;
		info.IsActive = type::IsActive::Active;
		init(initLink, package, info, TID_InitInvestorInfo);
	}


	void PushInvestorFee(CInitServiceLink& initLink, CZTPPackage& package) {
		for (auto& inst : auInsts) {
			data::CInvestorFee fee;
			fee.BrokerID = BrokerID;
			fee.ExchangeID = "SGE";
			fee.InvestorID = InvestorID;
			fee.InstrumentID = inst;
			fee.OpenFeeRate = 0.0006;
			fee.OpenFeeAmt = 0.0;
			fee.OffsetFeeRate = 0.0006;
			fee.OffsetFeeAmt = 0.0;
			fee.OffsetTodayFeeRate = 0.0006;
			fee.OffsetTodayFeeAmt = 0.0;
			init(initLink, package, fee, TID_InitInvestorFee);
		}
	}

	void PushInvestorMargin(CInitServiceLink& initLink, CZTPPackage& package) {
		for (auto& inst : auInsts) {
			data::CInvestorMargin margin;
			margin.BrokerID = BrokerID;
			margin.ExchangeID = "SGE";
			margin.InvestorID = InvestorID;
			margin.InstrumentID = inst;
			margin.LongMarginRate = 0.1;
			margin.LongMarginAmt = 0.0;
			margin.ShortMarginRate = 0.09;
			margin.ShortMarginAmt = 0.0;
			init(initLink, package, margin, TID_InitInvestorMargin);
		}
	}

	void PushInvestorAccount(CInitServiceLink& initLink, CZTPPackage& package, double margin = 998000) {
		data::CInvestorAccount account;
		account.BrokerID = BrokerID;
		account.InvestorID = InvestorID;
		account.PreBalance = 10'000'000.00;
		account.PreAvailable = 0.0;
		account.Balance = 0.00;
		account.Available = 0.00;
		account.Deposit = 0.00;
		account.Withdraw = 0.00;
		account.Margin = margin;
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
		init(initLink, package, account, TID_InitInvestorAccount);
	}

}

TEST_CASE("Init Service Test") {

	SECTION("Seat Init Test") {
		Investors::Clear();
		MemSystemStatusRepository statusRepo;
		MyBaseInfoRepository baseRepo;
		MyInvestorRepository investorRepo;
		MyInstrumentRepository instRepo;
		MyOperatorRepository opRepo;
		MemInitOrderStore orderStore;

		CInitServiceLink link(statusRepo,investorRepo,opRepo,instRepo,baseRepo,orderStore,BrokerID.value());
		CZTPPackage package;
		package.ConstructAllocate(1000, 1000);

		PushSeatInfo(link,package);
		CHECK(baseRepo.Get()->AppName == "Trade");
		PushInstrument(link, package);
		CHECK(instRepo.Get(auInsts[0])->UnilateralGroup == 0);
		CHECK(instRepo.Get(auInsts[0])->UnilateralSubIndex == 0);
		CHECK(instRepo.Get(auInsts[1])->UnilateralGroup == 0);
		CHECK(instRepo.Get(auInsts[1])->UnilateralSubIndex == 1);
		CHECK(instRepo.Get(auInsts[2])->UnilateralGroup == 0);
		CHECK(instRepo.Get(auInsts[2])->UnilateralSubIndex == 2);
		CHECK(instRepo.Get(auInsts[3])->UnilateralGroup == 0);
		CHECK(instRepo.Get(auInsts[3])->UnilateralSubIndex == 3);
		CHECK(instRepo.Get(auInsts[4])->UnilateralGroup == 1);
		CHECK(instRepo.Get(auInsts[4])->UnilateralSubIndex == 0);

		PushInvestorInfo(link, package);
		CHECK(investorRepo.GetInfo(InvestorID) != nullptr);

		PushInvestorFee(link, package);
		CHECK(investorRepo.GetFee(BrokerID, InvestorID, auInsts[0]) != nullptr);
		PushInvestorMargin(link, package);
		CHECK(investorRepo.GetMargin(BrokerID, InvestorID, auInsts[0]) != nullptr);

		PushInvestorAccount(link, package, 998888);

		data::CInvestorPosition pos;
		pos.BrokerID = BrokerID;
		pos.InvestorID = InvestorID;
		pos.ExchangeID = "SGE";
		pos.InstrumentID = auInsts[0];
		pos.ProductClass = type::ProductClass::Defer;
		pos.Direction = type::Direction::Buy;
		pos.Position = 0;
		pos.PositionCost = 0.0;
		pos.YdPosition = 10;
		pos.YdPositionPrice = 998;
		pos.FrozenMargin = 0.0;
		pos.FrozenFee = 0.0;
		pos.FrozenPosition = 0;

		init(link, package, pos, TID_InitInvestorPosition);

		auto investor = Investors::GetInvestor(investorRepo, instRepo, BrokerID, InvestorID);
		auto uniPos = investor->GetUnilateralPositionUsedMargin(0);
		auto expectedMargin = (0.1 * 10 * 998 * 1000);
		CHECK(investorRepo.GetPosition(InvestorID, auInsts[0], type::Direction::Buy)->UsedMargin == expectedMargin);
		CHECK(uniPos.Direction == type::Direction::Buy);
		CHECK(uniPos.UsedMargin == expectedMargin);

		link.InitInvestorUsedMarginByPosition();
		CHECK(investor->GetAccount().LongMargin == Approx(expectedMargin));
		CHECK(investor->GetAccount().ShortMargin == Approx(0.0));
		CHECK(investor->GetAccount().Margin == Approx(expectedMargin));
		CHECK(investor->GetAccount().Withdraw == Approx(998888-expectedMargin));


		pos.YdPosition = 5;
		pos.Direction = type::Direction::Sell;
		pos.YdPositionPrice = 1000;

		init(link, package, pos, TID_InitInvestorPosition);
		uniPos = investor->GetUnilateralPositionUsedMargin(0);
		CHECK(investorRepo.GetPosition(InvestorID, auInsts[0], type::Direction::Sell)->UsedMargin == Approx(0.09 * 5 * 1000 * 1000));
		CHECK(uniPos.Direction == type::Direction::Buy);
		CHECK(uniPos.UsedMargin == expectedMargin);

		link.InitInvestorUsedMarginByPosition();
		CHECK(investor->GetAccount().LongMargin == Approx(expectedMargin));
		CHECK(investor->GetAccount().ShortMargin == Approx(0.0));
		CHECK(investor->GetAccount().Margin == Approx(expectedMargin));
		CHECK(investor->GetAccount().Withdraw == Approx(998888 - expectedMargin));

		pos.InstrumentID = auInsts[1];
		pos.YdPosition = 7;
		pos.Direction = type::Direction::Sell;
		pos.YdPositionPrice = 1000;

		init(link, package, pos, TID_InitInvestorPosition);
		uniPos = investor->GetUnilateralPositionUsedMargin(0);
		CHECK(investorRepo.GetPosition(InvestorID, auInsts[1], type::Direction::Sell)->UsedMargin == (0.09 * 7 * 1000 * 1000));
		CHECK(uniPos.Direction == type::Direction::Sell);
		CHECK(uniPos.UsedMargin == (0.09 * 12 * 1000 * 1000));

		link.InitInvestorUsedMarginByPosition();
		CHECK(investor->GetAccount().LongMargin == Approx(0.0));
		CHECK(investor->GetAccount().ShortMargin == Approx((0.09 * 12 * 1000 * 1000)));
		CHECK(investor->GetAccount().Margin == Approx((0.09 * 12 * 1000 * 1000)));
	}
}