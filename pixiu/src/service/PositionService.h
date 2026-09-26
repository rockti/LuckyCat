#pragma once

#include "account/Positions.h"
#include "notify/Notify.h"
#include <algorithm>
#include <utils/Log.h>

#undef min

struct PlatformAccount {
	ConnPlatform Platform;
	money Balance = 0;		// 权益
	money Available = 0;	// 可用
	money Margin = 0;		// 占用保证金
	money Fee = 0;			// 手续费
	money FrozenMargin = 0;	// 冻结保证金
	money FrozenFee = 0;	// 冻结手续费
	money MaxBalance = 0;
	bool IsMaxBalance = false;

	bool Enough(money amount) const { return Available >= amount; }
	bool Enough(double amount) const { return Available >= amount; }

	void SetBalance(money balance) {
		MaxBalance = balance;
		IsMaxBalance = true;
		ReCalcAvailable();
		Log::Info(fmt::format("SetBalance: Balance[{:.2f}] new avail[{:.2f}]", balance, Available));
	}
	money GetBalance() const { return IsMaxBalance ? MaxBalance : Balance; }

	void ReCalcAvailable() {
		money maxBalance = IsMaxBalance ? MaxBalance : Balance;
		Available = maxBalance - Margin - Fee - FrozenMargin - FrozenFee;
// 		Log::Info(fmt::format("ReCalc by ordered: Avail[{:.3f}] Balance[{:.3f}], Margin[OC:{:.3f}, FZ:{:.3f}] Fee[OC:{:.3f}, FZ:{:.3f}]",
// 			Available, GetBalance(), Margin, FrozenMargin, Fee, FrozenFee));
	}
	void ReCalcAvailable(money margined, money feed) {
		Margin += margined;
		Fee += feed;
// 		Log::Info(fmt::format("ReCalc by traded: Avail[{:.3f}] Balance[{:.3f}], Margin[OC:{:.3f}, FZ:{:.3f}] Fee[OC:{:.3f}, FZ:{:.3f}]",
// 			Available, GetBalance(), Margin, FrozenMargin, Fee, FrozenFee));
	}
	void FreezeMargin(money margin) { this->FrozenMargin += margin; }
	void UnfreezeMargin(money margin) { this->FrozenMargin -= margin; }
	void FreezeFee(money fee) { this->FrozenFee += fee; }
	void UnfreezeFee(money fee) { this->FrozenFee -= fee; }
};

namespace pos {
	struct PairPositionVolume {
		InstID InstId;
		int TodayVolumes[2] = { 0,0 };
		int& TodaySellVolume = TodayVolumes[int(Direction::SELL)];
		int& TodayBuyVolume = TodayVolumes[int(Direction::BUY)];

		int HistoryVolumes[2] = { 0,0 };
		int& HistorySellVolume = HistoryVolumes[int(Direction::SELL)];
		int& HistoryBuyVolume = HistoryVolumes[int(Direction::BUY)];

		int TodayFreeze[2] = { 0,0 };
		int& TodaySellFreeze = TodayFreeze[int(Direction::SELL)];
		int& TodayBuyFreeze = TodayFreeze[int(Direction::BUY)];

		int HistoryFreeze[2] = { 0,0 };
		int& HistorySellFreeze = HistoryFreeze[int(Direction::SELL)];
		int& HistoryBuyFreeze = HistoryFreeze[int(Direction::BUY)];

		PairPositionVolume& operator=(PairPositionVolume const& other) {
			TodayVolumes[0] = other.TodayVolumes[0];
			TodayVolumes[1] = other.TodayVolumes[1];
			HistoryVolumes[0] = other.HistoryVolumes[0];
			HistoryVolumes[1] = other.HistoryVolumes[1];
			TodayFreeze[0] = other.TodayFreeze[0];
			TodayFreeze[1] = other.TodayFreeze[1];
			HistoryFreeze[0] = other.HistoryFreeze[0];
			HistoryFreeze[1] = other.HistoryFreeze[1];
			return *this;
		}

		inline bool operator==(PairPositionVolume const& other) const {
			return memcmp(this,&other, sizeof(PairPositionVolume)) == 0;
		}
		inline bool operator!=(PairPositionVolume const& other) const {
			return !(*this == other);
		}

		void Init(InstID instId) {
			InstId = instId;
		}
		void InitToday(Direction posDir, int volume) {
			TodayVolumes[int(posDir)] = volume;
		}
		void InitHistory(Direction posDir, int volume) {
			HistoryVolumes[int(posDir)] = volume;
		}

		bool IsSHFE() const {
			return InstIdExchange(InstId) == Exchange::SHFE;
		}
		bool IsCFFEX() const {
			return InstIdExchange(InstId) == Exchange::CFFEX;
		}

		void FreezeToday(Direction posDir, int volume);
		void UnfreezeToday(Direction posDir, int volume);
		void FreezeHistory(Direction posDir, int volume);
		void UnfreezeHistory(Direction posDir, int volume);

		inline int TotalSell() const {
			return TodaySellVolume + HistorySellVolume;
		}
		inline int TotalBuy() const {
			return TodayBuyVolume + HistoryBuyVolume;
		}
		inline int Total(Direction posDir) const {
			if (posDir == Direction::BUY) {
				return TotalBuy();
			} else {
				return TotalSell();
			}
		}

		//冻结量
		inline int TotalSellFreeze() const {
			return TodaySellFreeze + HistorySellFreeze;
		}
		inline int TotalBuyFreeze() const {
			return TodayBuyFreeze + HistoryBuyFreeze;
		}
		inline int TotalFreeze(Direction posDir) const {
			if (posDir == Direction::BUY) {
				return TotalBuyFreeze();
			}
			else {
				return TotalSellFreeze();
			}
		}

		//今仓可平量
		inline int TodaySellOffsetable() const {
			return TodaySellVolume - TodaySellFreeze;
		}
		inline int TodayBuyOffsetable() const {
			return TodayBuyVolume - TodayBuyFreeze;
		}
		inline int TodayOffsetable(Direction posDir) const {
			return posDir == Direction::BUY ? TodayBuyOffsetable() : TodaySellOffsetable();
		}
		
		//昨仓可平量
		inline int HisSellOffsetable() const {
			return HistorySellVolume - HistorySellFreeze;
		}
		inline int HisBuyOffsetable() const {
			return HistoryBuyVolume - HistoryBuyFreeze;
		}
		inline int HistoryOffsetable(Direction posDir) const {
			return posDir == Direction::BUY ? HisBuyOffsetable() : HisSellOffsetable();
		}

		//总仓可平量
		inline int SellTotalOffsetable() const {
			return TodaySellOffsetable() + HisSellOffsetable();
		}
		inline int BuyTotalOffsetable() const {
			return TodayBuyOffsetable() + HisBuyOffsetable();
		}
		inline int Offsetable(Direction posDir) const {
			return posDir == Direction::BUY ? BuyTotalOffsetable() : SellTotalOffsetable();
		}

		inline void Reset() {
			TodayVolumes[0] = 0;
			TodayVolumes[1] = 0;

			HistoryVolumes[0] = 0;
			HistoryVolumes[1] = 0;

			TodayFreeze[0] = 0;
			TodayFreeze[1] = 0;

			HistoryFreeze[0] = 0;
			HistoryFreeze[1] = 0;
		}
	};

	//记录某个合约是否有过开仓，对于平今有惩罚的合约，就不能平仓了（上海是不能发平今指令了）
	struct InstrumentDynamicData {
		bool HasOffsetTodayPenalty = false;
		bool Opened[2] = { false, false };
		bool& SellOpened = Opened[int(Direction::SELL)];
		bool& BuyOpened = Opened[int(Direction::BUY)];
		int CancelCount = 0;

		bool IsOpenOnly(Direction dir/*开仓委托方向和持仓方向相同*/) const {
			return HasOffsetTodayPenalty && Opened[int(dir)];	//平今有惩罚且开过今仓
		}
	};

	class GlobalPositionStore {
	public:
		static GlobalPositionStore& Instance();

		PlatformAccount& Account() { return account_; }
		void InitInstrument(InstID inst) { pos_[InstIDIndex(inst)].Init(inst); }
		void InitAccount(FundsNotification const& notif) {
			account_.Platform = notif.Platform;
			account_.Balance = notif.Capital;
			account_.Available = notif.Available;
			account_.Margin = notif.Margin;
			account_.Fee = notif.Fee;
		}

		void Clear();
		void InitToday(InstID inst, Direction posDir, QtyT qty, QtyT frozenQty);
		void InitHistory(InstID inst, Direction posDir, QtyT qty, QtyT frozenQty);
		// OnTrade只处理持仓变化，对于冻结/解冻等操作， 由具体的策略来完成
		void OnTrade(InstID inst, Direction orderDir, Operation opt, QtyT qty, money price);

		PairPositionVolume* Get(InstID inst) {
			return &pos_[InstIDIndex(inst)];
		}
		InstrumentDynamicData* GetOpenedFlags(InstID inst) {
			return &instrumentDynamicData_[InstIDIndex(inst)];
		}

		bool IsOpened(InstID inst, Direction dir) const {
			return instrumentDynamicData_[InstIDIndex(inst)].Opened[int(dir)];
		}
		bool HasTodayOffsetPenalty(InstID inst) const {
			return instrumentDynamicData_[InstIDIndex(inst)].HasOffsetTodayPenalty;
		}

		void Opened(InstID inst, Direction dir) {
			instrumentDynamicData_[InstIDIndex(inst)].Opened[int(dir)] = true;
		}
		void EnableTodayOffsetPenalty(InstID inst) {
			instrumentDynamicData_[InstIDIndex(inst)].HasOffsetTodayPenalty = true;
		}

		int GetCancelCount(InstID inst) const {
			return instrumentDynamicData_[InstIDIndex(inst)].CancelCount;
		}
		int IncCancelCount(InstID inst) {
			return ++instrumentDynamicData_[InstIDIndex(inst)].CancelCount;
		}
	private:
		std::array<PairPositionVolume, 256> pos_ = { 0 };
		std::array<InstrumentDynamicData, 256> instrumentDynamicData_;
		PlatformAccount account_;
	};
}

