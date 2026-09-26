#include "pch.h"
#include "service/PositionService.h"
#include "utils/Log.h"
#include "model/factory/InstrumentFactory.h"

namespace pos {
	void PairPositionVolume::FreezeToday(Direction posDir, int volume) {
		TodayFreeze[int(posDir)] += volume;
	}
	void PairPositionVolume::UnfreezeToday(Direction posDir, int volume) {
		TodayFreeze[int(posDir)] -= volume;
// 		Log::Info(fmt::format("UnfreezeToday: Inst[{}] Today[S:{}-{} B:{}-{}]", InstId, TodaySellVolume, TodaySellFreeze, TodayBuyVolume, TodayBuyFreeze));
	}
	void PairPositionVolume::FreezeHistory(Direction posDir, int volume) {
		HistoryFreeze[int(posDir)] += volume;
	}
	void PairPositionVolume::UnfreezeHistory(Direction posDir, int volume) {
		HistoryFreeze[int(posDir)] -= volume;
// 		Log::Info(fmt::format("UnfreezeHistory: Inst[{}] His[S:{}-{} B:{}-{}]", InstId, HistorySellVolume, HistorySellFreeze, HistoryBuyVolume, HistoryBuyFreeze));
	}

	GlobalPositionStore& GlobalPositionStore::Instance() {
		static GlobalPositionStore s_store;
		return s_store;
	}
	void GlobalPositionStore::Clear() {
		for (auto& p : pos_) {
			p.Reset();
		}
	}
	void GlobalPositionStore::InitToday(InstID inst, Direction posDir, QtyT qty, QtyT frozenQty) {
		pos_[InstIDIndex(inst)].InitToday(posDir, qty);
		if (frozenQty > 0) {
			pos_[InstIDIndex(inst)].FreezeToday(posDir, frozenQty);
		}
	}
	void GlobalPositionStore::InitHistory(InstID inst, Direction posDir, QtyT qty, QtyT frozenQty) {
		pos_[InstIDIndex(inst)].InitHistory(posDir, qty);
		if (frozenQty > 0) {
			pos_[InstIDIndex(inst)].FreezeHistory(posDir, frozenQty);
		}
	}
	void GlobalPositionStore::OnTrade(InstID inst, Direction orderDir, Operation opt, QtyT qty, money price) {
		int index = InstIDIndex(inst);
		auto& pos = pos_[index];
		auto const& instrument = InstrumentFactory::GetInstruments().Get(inst);

		Direction posDir = OppositeDirection(orderDir);				/*平仓方向和持仓方向相反*/
		if (opt == Operation::OPEN) {
			int& existedQty = pos_[index].TodayVolumes[int(orderDir)/*开仓方向和持仓方向相同*/];
			existedQty += qty;
			Opened(inst, orderDir);
			account_.ReCalcAvailable(instrument.CalculateMargin(price, qty, Operation::OPEN	/*开仓占用保证金，要用OPEN算*/), instrument.CalculateFee(price, qty, opt));
		} 
		else {
			if (IsSHFE(inst) || IsINE(inst)) {
				int& existedQty = Operation::OFFSET_TODAY == opt ? pos_[index].TodayVolumes[int(posDir)] : pos_[index].HistoryVolumes[int(posDir)];
				existedQty -= qty;
			} else {
				int& existedQty = pos_[index].HistoryVolumes[int(posDir)];
				existedQty -= qty;
			}
			account_.ReCalcAvailable(-instrument.CalculateMargin(price, qty, Operation::OPEN/*平仓释放保证金，要用OPEN算*/).value(), instrument.CalculateFee(price, qty, opt));
		}

// 		bool isToday = !IsSHFE(inst)/*非上海品种一律视为平今*/ || Operation::OFFSET_TODAY == opt;
// 		if (Operation::OPEN == opt) {
// 			Log::Info(fmt::format("!PositionStore OnTrade: Inst[{}] Today[S:{}-{} B:{}-{}] His[S:{}-{} B:{}-{}] [{}]Price[{:.3f} * {}] [{}]",
// 				instrument.GetInstCode().Str(), pos.TodaySellVolume, pos.TodaySellFreeze, pos.TodayBuyVolume, pos.TodayBuyFreeze,
// 				pos.HistorySellVolume, pos.HistorySellFreeze, pos.HistoryBuyVolume, pos.HistoryBuyFreeze, Desc(orderDir), price, qty, Desc(opt)));
// 		}
// 		else if (isToday) {
// 			if (posDir == Direction::SELL) {
// 				Log::Info(fmt::format("!PositionStore OnTrade: Inst[{}] Today[S:{}-{} B:{}-{}] His[S:{}-{} B:{}-{}] [{}]Price[{:.3f} * {}] [{}]",
// 					instrument.GetInstCode().Str(), pos.TodaySellVolume, pos.TodaySellFreeze - qty, pos.TodayBuyVolume, pos.TodayBuyFreeze,
// 					pos.HistorySellVolume, pos.HistorySellFreeze, pos.HistoryBuyVolume, pos.HistoryBuyFreeze, Desc(orderDir), price, qty, Desc(opt)));
// 			}
// 			else {
// 				Log::Info(fmt::format("!PositionStore OnTrade: Inst[{}] Today[S:{}-{} B:{}-{}] His[S:{}-{} B:{}-{}] [{}]Price[{:.3f} * {}] [{}]",
// 					instrument.GetInstCode().Str(), pos.TodaySellVolume, pos.TodaySellFreeze, pos.TodayBuyVolume, pos.TodayBuyFreeze - qty,
// 					pos.HistorySellVolume, pos.HistorySellFreeze, pos.HistoryBuyVolume, pos.HistoryBuyFreeze, Desc(orderDir), price, qty, Desc(opt)));
// 			}
// 		}
// 		else {
// 			if (posDir == Direction::SELL) {
// 				Log::Info(fmt::format("!PositionStore OnTrade: Inst[{}] Today[S:{}-{} B:{}-{}] His[S:{}-{} B:{}-{}] [{}]Price[{:.3f} * {}] [{}]",
// 					instrument.GetInstCode().Str(), pos.TodaySellVolume, pos.TodaySellFreeze, pos.TodayBuyVolume, pos.TodayBuyFreeze,
// 					pos.HistorySellVolume, pos.HistorySellFreeze - qty, pos.HistoryBuyVolume, pos.HistoryBuyFreeze, Desc(orderDir), price, qty, Desc(opt)));
// 			}
// 			else {
// 				Log::Info(fmt::format("!PositionStore OnTrade: Inst[{}] Today[S:{}-{} B:{}-{}] His[S:{}-{} B:{}-{}] [{}]Price[{:.3f} * {}] [{}]",
// 					instrument.GetInstCode().Str(), pos.TodaySellVolume, pos.TodaySellFreeze, pos.TodayBuyVolume, pos.TodayBuyFreeze,
// 					pos.HistorySellVolume, pos.HistorySellFreeze, pos.HistoryBuyVolume, pos.HistoryBuyFreeze - qty, Desc(orderDir), price, qty, Desc(opt)));
// 			}
// 		}
	}
}

