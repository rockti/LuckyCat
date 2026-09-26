#pragma once
#include "StatusMachine.h"
#include "type/ZTPType.h"
#include "model/dealing/OrderCommand.h"
#include "model/dealing/ExchangeRequest.h"
#include <set>
class Business;
class BusinessDealingStore;

namespace automate {
	using namespace ztp;
	class AutoStrategy;
	

	enum class PriceMode {
		Pending,		//挂单模式
		FAK				//对价模式,现不支持
	};

	enum class StrategyMode {
		TodayOpenClose,
		YesterdayClose,
		OpenClose,
	};

	struct ExistedVolume {
		type::Direction OpenDirection = type::Direction::Buy;
		type::Volume Volume = 0;
	};

	struct AutoArgument {
		StrategyMode Mode;
		PriceMode PriceMode;
		type::InstID Major; //后发，有跳价
		type::InstID Minor; //先发
		type::Volume VolumeMultiple;//量比
		type::Volume MaxVolume;		//最大持仓量
		type::Volume Quantity;		//每次开平量
		type::Price SellOpenSpread;	//卖开价差
		type::Price SellCloseSpread;//卖平价差
		type::Price BuyCloseSpread;	//买平价差
		type::Price BuyOpenSpread;	//买开价差
		type::Tick Tolerance;;		//容忍度, 在minor成交后对价差的容忍度，取两个合约中大的单位价格。比如上期一跳5分钱，黄金一跳是1分钱，取5分
		type::Tick MajorLostTick;	//主力跳价点

		ExistedVolume ExistedVolume; //已有仓位
	};

	struct AvailablePrice {
		type::Direction Directin;
		type::OffsetFlag OffsetFlag;
		type::Price Price;
		type::Volume Volume;
	};

	class IPriceSelector {
	public:
		virtual type::Price GetOpenSpread() const = 0;
		virtual type::Price GetCloseSpread() const = 0;
		virtual type::Price GetOpenSpread(type::Price minorOrderPrice) const = 0;
		virtual type::Price GetCloseSpread(type::Price minorOrderPrice) const = 0;
		virtual bool IsOpenReady(AutoArgument const& arg) const = 0;
		virtual bool IsCloseReady(AutoArgument const& arg) const = 0;
		virtual bool IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const = 0;
		virtual bool IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const = 0;
		virtual AvailablePrice GetMinorOpenPrice(type::Volume maxVolume) const = 0;
		virtual AvailablePrice GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const = 0;
		virtual AvailablePrice GetMinorClosePrice(type::Volume maxVolume) const = 0;
		virtual AvailablePrice GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const = 0;

		virtual Quote const& GetMajorQuote() const = 0;
		virtual Quote const& GetMinorQuote() const = 0;

	};

	//买方向开仓-看涨价差（挂单买)
	class BuyOpenPriceSelector : public IPriceSelector{
	public:
		BuyOpenPriceSelector(Quote const& major, Quote const& minor) :
			majorQuote_(major), minorQuote_(minor) {
		}
		virtual type::Price GetOpenSpread() const override;
		virtual type::Price GetCloseSpread() const override;

		virtual type::Price GetOpenSpread(type::Price minorOrderPrice) const override;
		virtual type::Price GetCloseSpread(type::Price minorOrderPrice) const override;

		virtual bool IsOpenReady(AutoArgument const& arg) const override;
		virtual bool IsCloseReady(AutoArgument const& arg) const override;

		virtual bool IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;
		virtual bool IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;

		virtual AvailablePrice GetMinorOpenPrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		virtual AvailablePrice GetMinorClosePrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		Quote const& GetMajorQuote() const { return majorQuote_; }
		Quote const& GetMinorQuote() const { return minorQuote_; }
	private:
		Quote const& majorQuote_;
		Quote const& minorQuote_;
	};
	//卖方向开仓-看跌价差（挂单卖)
	class SellOpenPriceSelector : public IPriceSelector {
	public:
		SellOpenPriceSelector(Quote const& major, Quote const& minor) :
			majorQuote_(major), minorQuote_(minor) {
		}

		virtual type::Price GetOpenSpread() const override;
		virtual type::Price GetCloseSpread() const override;
		virtual type::Price GetOpenSpread(type::Price minorOrderPrice) const override;
		virtual type::Price GetCloseSpread(type::Price minorOrderPrice) const override;

		virtual bool IsOpenReady(AutoArgument const& arg) const override;
		virtual bool IsCloseReady(AutoArgument const& arg) const override;

		virtual bool IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;
		virtual bool IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;

		virtual AvailablePrice GetMinorOpenPrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		//卖方向平仓-价差跌到平仓价差 获利（买平)
		virtual AvailablePrice GetMinorClosePrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		virtual Quote const& GetMajorQuote() const override { return majorQuote_; }
		virtual Quote const& GetMinorQuote() const override { return minorQuote_; }

	private:
		Quote const& majorQuote_;
		Quote const& minorQuote_;
	};

	//买方向开仓-看涨价差（对价买)
	class FAKBuyOpenPriceSelector : public IPriceSelector{
	public:
		FAKBuyOpenPriceSelector(Quote const& major, Quote const& minor) :
			majorQuote_(major), minorQuote_(minor) {
		}
		virtual type::Price GetOpenSpread() const override;
		virtual type::Price GetCloseSpread() const override;

		virtual type::Price GetOpenSpread(type::Price minorOrderPrice) const override;
		virtual type::Price GetCloseSpread(type::Price minorOrderPrice) const override;

		virtual bool IsOpenReady(AutoArgument const& arg) const override;
		virtual bool IsCloseReady(AutoArgument const& arg) const override;

		virtual bool IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;
		virtual bool IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;

		virtual AvailablePrice GetMinorOpenPrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		virtual AvailablePrice GetMinorClosePrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		virtual Quote const& GetMajorQuote() const override { return majorQuote_; }
		virtual Quote const& GetMinorQuote() const override { return minorQuote_; }
	private:
		Quote const& majorQuote_;
		Quote const& minorQuote_;
	};
	//卖方向开仓-看跌价差（对价卖)
	class FAKSellOpenPriceSelector : public IPriceSelector {
	public:
		FAKSellOpenPriceSelector(Quote const& major, Quote const& minor) :
			majorQuote_(major), minorQuote_(minor) {
		}

		virtual type::Price GetOpenSpread() const override;
		virtual type::Price GetCloseSpread() const override;
		virtual type::Price GetOpenSpread(type::Price minorOrderPrice) const override;
		virtual type::Price GetCloseSpread(type::Price minorOrderPrice) const override;

		virtual bool IsOpenReady(AutoArgument const& arg) const override;
		virtual bool IsCloseReady(AutoArgument const& arg) const override;

		virtual bool IsOpenLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;
		virtual bool IsCloseLost(AutoArgument const& arg, type::Price minorOrderPrice, type::UnitPrice toleranceUnitPrice) const override;

		virtual AvailablePrice GetMinorOpenPrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorOpenPrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		//卖方向平仓-价差跌到平仓价差 获利（买平)
		virtual AvailablePrice GetMinorClosePrice(type::Volume maxVolume) const override;
		virtual AvailablePrice GetMajorClosePrice(type::Volume maxVolume, type::Tick majorLostTick) const override;
		virtual Quote const& GetMajorQuote() const override { return majorQuote_; }
		virtual Quote const& GetMinorQuote() const override { return minorQuote_; }
	private:
		Quote const& majorQuote_;
		Quote const& minorQuote_;
	};
	
	struct VolumeRatio {
		uint32_t MajorMultipler;
	};

	struct BalancingPosition {
		type::Volume Minor;
		type::Volume Major;
		type::Volume OppositeMinor;

		type::Volume MajorFrozen;
		type::Volume OppositeMinorFrozen;
	};

	enum class BalanceMode {
		MajorBalance,
		OppositeMinorBalance,
	};

	struct BalanceVolume {
		BalanceMode Mode;
		type::Volume Volume;
	};

	class IVolumeBalancer {
	public:
		virtual BalanceVolume GetBalancingVolume(Dealing const& minorDealing, BalancingPosition const& position) const = 0;
		virtual bool IsBalanced(BalancingPosition const& position) const = 0;
	};

	struct Position {
		type::Direction Direction = type::Direction::Buy;
		type::Volume Volume = 0;

		void OnTrade(type::Direction direction, type::Price price, type::Volume volume) {
			if (this->Volume == 0) {
				this->Direction = direction;
				this->Volume = volume;
			} else {
				if (direction == Direction) {
					this->Volume += volume;
				} else {
					this->Volume -= volume;
					if (this->Volume < 0) {
						this->Direction = direction;
						this->Volume = -Volume;
					}
				}
			}
		}
		bool operator==(Position const& other) const {
			return Direction == other.Direction && Volume == other.Volume;
		}
		bool operator!=(Position const& other) const {
			return !(*this == other);
		}
	};

	enum class RunningMode {
		BuySpreadOpen,
		BuySpreadClose,
		SellSpreadOpen,
		SellSpreadClose,
		None
	};

	class OneToNVolumeBalancer : public IVolumeBalancer {
	public:
		OneToNVolumeBalancer(VolumeRatio ratio, Quote const& majorQuote, Quote const& minorQuote) :
			volumeRatio_(ratio),majorQuote_(majorQuote), minorQuote_(minorQuote) {

		}
		virtual BalanceVolume GetBalancingVolume(Dealing const& minorDealing, BalancingPosition const& position) const override {
			BalanceVolume volume;
			volume.Mode = BalanceMode::MajorBalance; //will support minorOpposite later
			type::Volume minorVolume = position.Minor - position.OppositeMinor - position.OppositeMinorFrozen;
			volume.Volume = minorVolume * volumeRatio_.MajorMultipler - position.Major - position.MajorFrozen;
			return volume;
		}

		virtual bool IsBalanced(BalancingPosition const& position) const override {
			type::Volume minorVolume = position.Minor - position.OppositeMinor;
			return  minorVolume * volumeRatio_.MajorMultipler == position.Major;
		}
	private:
		VolumeRatio volumeRatio_;
		Quote const& majorQuote_;
		Quote const& minorQuote_;
	};

	class Context;
	struct Running {
		Context* ContextPtr = nullptr;
		RunningMode Mode = RunningMode::None;
		AutoArgument const* Arg = nullptr;
		AvailablePrice MinorPrice;
		IPriceSelector* BuySelector;
		IPriceSelector* SellSelector;
		Dealing* MinorDealing = nullptr;
		std::set<type::OrderLocalID> UnfinishedBalanceDealings;
		BalancingPosition Position;

		static Running None(AutoArgument const& arg) {
			return Running{ nullptr, RunningMode::None, &arg};
		}

		bool IsNone() const {
			return this->Mode == RunningMode::None;
		}

		bool IsMinorFinished() const;
		bool IsAllBalanceFinished() const;
		bool IsBalanced() const;
		OrderInsertRequest CreateMajorInsertRequest(type::Volume volume);
		OrderInsertRequest CreateMinorOppositeInsertRequest(type::Volume volume);

		void MinorTrade(type::Volume tradeVolume);
		void FinishMajor(type::OrderLocalID localID, type::Volume orderVolume, type::Volume tradeVolume);

		void FinishMinorOpposite(type::OrderLocalID localID, type::Volume orderVolume, type::Volume tradeVolume);

		void SetMinorDealing(Dealing* dealing) {
			MinorDealing = dealing;
		}

		bool LostOpportunity() const;

		type::OffsetFlag negetive(type::OffsetFlag minorOffsetFlag) const;

		OrderInsertRequest CreateMinorOrderRequest() const;
		OrderCancelRequest CreateMinorCancelRequest() const;

		BalanceVolume GetBalanceVolume() const;

		AvailablePrice getMajorPrice(type::Volume volume) const;
		AvailablePrice getOppsoiteMinorPrice(type::Volume volume) const;

		bool isPendingOrderOverTolerance() const;
		bool isSpreadOverTolerance() const;
	};

	struct Context : public StatusMachine {
		Context(Business* business, AutoArgument const& arg, Quote const& majorQuote, Quote const& minorQuote);
		bool QuoteCloseToLimit() const;
		bool InvalidQuote() const;
		Running ReadyToOpen();
		Running ReadyToClose();

		Running TryRunning();

		void FireMinor(Running const& running);

		type::OffsetFlag GetRightOffsetFlag(Instrument const& instrument) const {
			if (arg_.Mode == StrategyMode::TodayOpenClose && instrument.Exchange == type::Exchange::SHFE) {
				return type::OffsetFlag::CloseToday;
			}
			return type::OffsetFlag::Close;
		}
		BusinessDealingStore& GetDealingStore();
		bool IsBuyOpenPosition() const {
			return type::IsBuy(minorPosition_) && minorPosition_.Volume > 0;
		}
		bool IsSellOpenPosition() const {
			return type::IsSell(minorPosition_) && minorPosition_.Volume > 0;
		}

		void CancelMinor();

		void OppositeMinor(type::Volume volume);

		void OpenMajor(type::Volume volume);

		bool LostOpportunity() const {
			return running_.LostOpportunity();
		}

		void Reset() {
			running_ = Running::None(arg_);
		}

// 		BaseStatus* GetStatus(RunningStatus status) const {
// 			return this->GetStatus(status);
// 		}

		bool IsBalanced() const {
			return running_.IsBalanced();
		}

		bool IsAllFinished() const {
			return IsMinorFinished() && IsAllBalanceFinished();
		}

		bool IsMinorFinished() const {
			return running_.IsMinorFinished();
		}

		bool IsAllBalanceFinished() const {
			return running_.IsAllBalanceFinished();
		}

		bool IsDone() const {
			return IsAllFinished() && IsBalanced();
		}

		bool IsOverFailedLimit() const {
			return true;
//			return running_.FailedCount > maxFailCount_;
		}

		bool IsOverCancelLimit() const;

		void UpdatePositionByTrade(InvestorTrade const& trade) {
			if (trade.InstID == arg_.Minor) {
				minorPosition_.OnTrade(trade.Direction, trade.TradePrice, trade.TradeVolume);
			} else {
				majorPosition_.OnTrade(trade.Direction, trade.TradePrice, trade.TradeVolume);
			}
		}

		Running& GetRunning() {
			return running_;
		}
		RunningStatus GetRunningStatus() const {
			return GetCurrentStatus()->SelfStatus;
		}

		Position const& GetMajorPosition() const {
			return majorPosition_;
		}

		Position const& GetMinorPosition() const {
			return minorPosition_;
		}
		BalanceVolume GetBalanceVolume() const {
			return running_.GetBalanceVolume();
		}

		Dealing const& GetMinorDealing() const {
			return *running_.MinorDealing;
		}

		Dealing const* GetMinorDealingPtr() const {
			return running_.MinorDealing;
		}

		Instrument const& GetMajorInst() const {
			return majorInst_;
		}

		Instrument const& GetMinorInst() const {
			return minorInst_;
		}

		type::UnitPrice GetToleranceUnitPrice() const {
			return toleranceUnitPrice_;
		}

		IVolumeBalancer const& VolumeBalancer() const {
			return balancer_;
		}

		Quote const& GetMajorQuote() const {
			return buySelector_.GetMajorQuote();
		}

		Quote const& GetMinorQuote() const {
			return buySelector_.GetMinorQuote();
		}

		Business* GetBusiness() {
			return business_;
		}

		AutoArgument const& GetArg() const { return arg_; }

	private:

		AutoArgument const& arg_;
		Instrument majorInst_;
		Instrument minorInst_;
		type::UnitPrice toleranceUnitPrice_;
		BuyOpenPriceSelector buySelector_;
		SellOpenPriceSelector sellSelector_;
		FAKBuyOpenPriceSelector fakBuySelector_;
		FAKSellOpenPriceSelector fakSellSelector_;
		Position majorPosition_;
		Position minorPosition_;
		Running running_;
		int maxFailCount_ = 1;
		Business* business_ = nullptr;
		OneToNVolumeBalancer balancer_;
	};
}