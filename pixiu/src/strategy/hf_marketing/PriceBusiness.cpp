#include "pch.h"

#include "PriceBusiness.h"
#include "utils/Log.h"
#include "NeturalSpreadStrategy.h"
#include "model/factory/InstrumentFactory.h"

namespace hf::netural {
	bool HFPricePosition::IsNeedBalance() const {		
		return !m_position.IsBothSideNoPending();
	}
	bool HFPricePosition::IsMajorPending() const {
		return m_position.Major.IsPending();
	}
	bool HFPricePosition::IsReversePending() const {
		return m_position.Reverse.IsPending();
	}

	PriceBusiness::PriceBusiness(NeturalSpreadStrategy& parent, Dealing& majorDealing, HFPricePosition const& position, QuoteBias bias, uint32_t selfQty)
		:m_parent(parent), m_pricePosition(position), m_machine(this), m_bias(bias), m_selfQty(selfQty) {
		m_currentPhase = m_machine.GetPhase(Phase::MajorOrdered)/*major委托后才进入此状态机*/;
		m_unitPrice = InstrumentFactory::GetInstruments().Get(m_pricePosition.Spread().Instruments.Major).GetUnitPrice();
		m_DealingStatus.SetMajorLocalId(majorDealing.GetLocalId());
		if (QuoteBias::ATC == m_bias) {
			m_selfQty += m_parent.GetArg().UnitQty;
		}
	}
	
	int const MAX_EXTRA_TICK = 2;	// 最多加两跳（第二跳是防止行情变动太快Major挂单成交跟不上）
	int PriceBusiness::GetReverseTick(Direction majorDir) {
		int gainTick = m_pricePosition.GetGainTick();

		int PFactor = m_parent.GetPFactor();
		int newTick = abs(PFactor) - 1;
		if (-PFactor > 1) {	
			gainTick = (Direction::SELL == majorDir) ? gainTick + std::min(newTick, MAX_EXTRA_TICK) : 0;
		}	// 净买仓多，卖向Gain加点（最多加两跳，发到C3），买向取0
		else if (PFactor > 1) {
			gainTick = (Direction::BUY == majorDir) ? gainTick + std::min(newTick, MAX_EXTRA_TICK) : 0;
		}	// 净卖仓多，买向Gain加点（最多加两跳，发到C3），卖向取0
		return gainTick;
	}
	bool PriceBusiness::IsMajorMissChance() {
		if (m_isOrderQuote) {
			m_isOrderQuote = false;
			return false;
		}	// 第一次来的是触发报价的行情，直接忽略

		InstID majorID = m_pricePosition.Spread().Instruments.Major;
		Quote const& majorQuote = WritableQuoteService().GetQuote(majorID);
		money ofrPrice = majorQuote.GetSell().GetPrice();
		int ofrQty = majorQuote.GetSell().GetQuantity();
		money bidPrice = majorQuote.GetBuy().GetPrice();
		int bidQty = majorQuote.GetBuy().GetQuantity();
		money lastPrice = m_pricePosition.GetLastPrice();

		int PFactor = m_parent.GetPFactor();
		Direction majorDir = GetMajorDirection();
		Operation majorOpt = GetMajorOperation();
		money majorPrice = GetMajorOrderedPrice();
		if (m_parent.m_balanceTag || QuoteBias::BST == m_bias) {
			if (!m_pricePosition.GetLines().IsPriceFit(majorDir, majorPrice, -1/*等同于Best*/)) {
				Log::Info(fmt::format("Business[{}]: Major missChance because of Major is not the best, Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{} {}:{:.3f}] PF[{:2d}]",
					(void*)this, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), Desc(majorOpt), majorPrice, PFactor));
				return true;
			}
			return false;
		}	// 抢价（配平或持仓止损）非最优撤单
		
		QuoteBias latestBias = m_parent.GetLatestBias(majorDir);
		if (QuoteBias::FBD == latestBias) {
			Log::Info(fmt::format("Business[{}]: Major missChance because of the dir is forbidden, Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{} {}:{:.3f}] PF[{:2d}]",
				(void*)this, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), Desc(majorOpt), majorPrice, PFactor));
			return true;
		}	// 禁价方向直接撤单

		money latestQuote = m_parent.GetLatestQuote(majorDir);
		if (latestQuote.value() != majorPrice.value()) {
// 			Log::Info(fmt::format("Business[{}]: Major missChance because of the price is changed, Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{}:{:.3f}->{:.3f}]  PF[{:2d}]",
// 				(void*)this, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), majorPrice, latestQuote, PFactor));
			return true;
		}	// 报价有变化直接撤单

		// FST不看盘口数量，价格不变就等成交
		if (latestBias > QuoteBias::FST ) {
			Quote const lastQuote = m_parent.GetLastQuote();
			QtyT increaseVol = majorQuote.GetVolume() - lastQuote.GetVolume();	// 成交增量
			if (Direction::SELL == majorDir) {
				int outSideVol = (lastPrice >= lastQuote.GetSell().GetPrice()) ? increaseVol : 0;	// 向外成交增量
				if (outSideVol > 0) {
					m_selfQty -= outSideVol;
					if (ofrQty <= m_selfQty) {
// 						Log::Info(fmt::format("Business[{}]: Major missChance because of trade, ofrQty[{}] <= selfQty[{}], Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{} {}:{:.3f}] PF[{:2d}]",
// 							(void*)this, ofrQty, m_selfQty, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), Desc(majorOpt), majorPrice, PFactor));
						return true;
					}
				}
				else if (lastQuote.GetSell().GetPrice() == ofrPrice && ofrQty < m_selfQty) {
// 					Log::Info(fmt::format("Business[{}]: Major missChance because of ofrQty[{}] < selfQty[{}], Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{} {}:{:.3f}] PF[{:2d}]",
// 						(void*)this, ofrQty, m_selfQty, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), Desc(majorOpt), majorPrice, PFactor));
					return true;
				}
			}
			else {
				int inSideVol = (lastPrice <= lastQuote.GetBuy().GetPrice()) ? increaseVol : 0;		// 向内成交增量
				if (inSideVol > 0) {
					m_selfQty -= inSideVol;
					if (bidQty <= m_selfQty) {
// 						Log::Info(fmt::format("Business[{}]: Major missChance because of trade, bidQty[{}] <= selfQty[{}], Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{} {}:{:.3f}] PF[{:2d}]",
// 							(void*)this, bidQty, m_selfQty, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), Desc(majorOpt), majorPrice, PFactor));
						return true;
					}
				}
				else if (lastQuote.GetBuy().GetPrice() == bidPrice && bidQty < m_selfQty) {
// 					Log::Info(fmt::format("Business[{}]: Major missChance because of bidQty[{}] < selfQty[{}], Quote[{:.3f} {:3d}, {:.3f} {:3d}]		Major[{} {}:{:.3f}] PF[{:2d}]",
// 						(void*)this, bidQty, m_selfQty, ofrPrice, ofrQty, bidPrice, bidQty, Desc(majorDir), Desc(majorOpt), majorPrice, PFactor));
					return true;
				}
			}
		}

		return false;
	}	
	bool PriceBusiness::IsReverseNeedRetry() {	
		Direction majorDir = GetMajorDirection();
		Direction reverseDir = GetReverseDirection();
		Operation reverseOpt = GetReverseOperation();
		money majorPrice = GetMajorOrderedPrice();
		money reversePrice = GetReverseOrderedPrice();

		int PFactor = m_parent.GetPFactor();		
		int rvsCnt = m_parent.GetReverseCnt(majorDir);
		if (QuoteBias::FST == m_bias) {
			Variance var = m_parent.GetMacroVariance();
			money retryLine = (Direction::SELL == reverseDir) ? tickToPrice(var.AVGTick + 2 * var.STDevTick, m_unitPrice) : tickToPrice(var.AVGTick - 2 * var.STDevTick, m_unitPrice);
			if ((Direction::SELL == reverseDir && retryLine < reversePrice) || (Direction::BUY == reverseDir && retryLine > reversePrice)) {
				Log::Info(fmt::format("PriceBusiness[{}]: special reverse[{} {}：{:.3f}] is unfit, retryLine[{:.3f}] PFactor[{}] needRetry",
					(void*)this, Desc(reverseDir), Desc(reverseOpt), reversePrice, retryLine, PFactor));
				return true;
			}
		}	// 超发单，特殊处理，不看持仓

		if (1 == m_parent.GetReverseCnt(majorDir)/*本方向仅1手*/ && m_parent.GetReverseCnt(reverseDir) > 1/*反方向超1手*/) {
			if (m_pricePosition.GetLines().IsPriceFit(reverseDir, reversePrice, GetGainTick())) {
// 				Log::Info(fmt::format("PriceBusiness[{}]: the lonely reverse[{} {}：{:.3f}] is within quoteRange[{}], PFactor[{}] RC[{}:{}] needEscape",
// 					(void*)this, Desc(reverseDir), Desc(reverseOpt), reversePrice, GetGainTick(), PFactor, Desc(majorDir), rvsCnt));
				m_escapeTag = true;	// 逃离标记
				return true;
			}	// 进入盈利窗口
		}	// RVS数量不平衡的情况：只有1手的需要重新报价，防止被击穿

		int retryTick = (Direction::SELL == reverseDir) ? m_parent.GetArg().Ticks.BidBaseTick : m_parent.GetArg().Ticks.OfrBaseTick;
		if (m_parent.IsPositionExcessive(majorDir) && m_parent.IsTheWorstAlterableRvs(majorDir, this)/*最差且有空间*/) {
			if (!m_pricePosition.GetLines().IsPriceFit(reverseDir, reversePrice, retryTick * MAX_EXTRA_TICK)) {
// 				Log::Info(fmt::format("PriceBusiness[{}]: the worst Reverse[{} {}：{:.3f}] is over quoteRange[{}], PFactor[{}] RC[{}:{}] needRetry",
// 					(void*)this, Desc(reverseDir), Desc(reverseOpt), reversePrice, retryTick, PFactor, Desc(majorDir), rvsCnt));
				return true;
			}	// 超出改价范围（两个报价窗口）
		}	// 持仓溢出方向的最差RVS委托

		if (!m_pricePosition.GetLines().IsPriceFit(reverseDir, reversePrice, m_parent.GetArg().WaitingTick)) {
			m_stopTag = true;		// 标记止损
			Log::Info(fmt::format("PriceBusiness[{}]: reverse[{} {}：{:.3f}] is over stopRange[{}], PFactor[{}] needStop",
				(void*)this, Desc(reverseDir), Desc(reverseOpt), reversePrice, m_parent.GetArg().WaitingTick, PFactor));
			return true;
		}	// 止损

		return false;
	}

	void PriceBusiness::OnQuote() {
		m_currentPhase = m_currentPhase->OnQuote();
	}
	void PriceBusiness::OnMajorTrade(money orderPrice, int quantity) {
		m_pricePosition.OnMajorTrade(quantity);
		m_currentPhase = m_currentPhase->OnMajorTrade(quantity);
		m_parent.RemovePendingPrice(this, "Major Trade", m_pricePosition.Spread().Orders.Major.Direction, orderPrice, quantity);
// 		logCurrentPosition("Major Trade end");
	}
	void PriceBusiness::OnMajorCancel(money orderPrice, int quantity) {
		m_pricePosition.OnMajorCancel(quantity);
		m_currentPhase = m_currentPhase->OnMajorCancel(quantity);
		m_parent.RemovePendingPrice(this, "Major Cancel", m_pricePosition.Spread().Orders.Major.Direction, orderPrice, quantity);
// 		logCurrentPosition("Major Cancel end");
	}
	void PriceBusiness::OnReverseTrade(money reversePrice, int quantity) {
		m_pricePosition.OnReverseTrade(quantity);
		m_currentPhase = m_currentPhase->OnReverseTrade(quantity);
		m_parent.RemovePendingPrice(this, "Reverse Trade", GetReverseDirection(), reversePrice, quantity);
// 		logCurrentPosition("Reverse Trade end");
	} 
	void PriceBusiness::OnReverseCancel(money orderPrice, int quantity) {
		m_pricePosition.OnReverseCancel(quantity);
		m_currentPhase = m_currentPhase->OnReverseCancel(quantity);
		m_parent.RemovePendingPrice(this, "Reverse Cancel", GetReverseDirection(), orderPrice, quantity);
// 	 	logCurrentPosition("Reverse Cancel end");
	}

	void PriceBusiness::pushCancel(DealingStatus const& dealingStatus) {
		if (!dealingStatus.IsFinished) {
			InstID instID = m_pricePosition.Spread().Instruments.Major;
			InstCodeT const& instCode = InstrumentFactory::GetInstruments().Get(instID).GetInstCode();
			OrderCancel action(InstIdExchange(instID), dealingStatus.SystemNo, instCode, instID, dealingStatus.LocalId, LocalIdACC::NextNormal());
			m_parent.PushAction(action);
		}
	}
	void PriceBusiness::cancelMajor() {
		if (m_pricePosition.Position().Major.IsPending()) {
			pushCancel(m_DealingStatus.MajorStatus());
		}
	}
	void PriceBusiness::sendReverse(money price, int qty) {	
		InstID majorID = m_pricePosition.Spread().Instruments.Major;
		auto const& inst = InstrumentFactory::GetInstruments().Get(majorID);
		Direction majorDir = m_pricePosition.Spread().GetMajorDirection();
		Operation majorOpt = m_pricePosition.Spread().GetMajorOperation();
		Direction reverseDir = OppositeDirection(m_pricePosition.Spread().GetMajorDirection());
		auto orderInfo = m_parent.CreateOrder(majorID, m_pricePosition.Spread().Orders.Reverse, qty, price);

		if (Operation::OPEN != orderInfo.Operation) {
			if (!IsSHFE(majorID)/*非上海品种一律视为平今*/ || Operation::OFFSET_TODAY == orderInfo.Operation) {
				if (!m_parent.TryFreezeTDRvs(majorDir, qty)) {
					orderInfo.Operation = Operation::OPEN;	// 冻结失败改成开仓
				}
			}
			else if (Operation::OFFSET == orderInfo.Operation) {
				if (!m_parent.TryFreezeHisRvs(majorDir, qty)) {
					orderInfo.Operation = Operation::OPEN;	// 冻结失败改成开仓
				}
			}
		}

		Dealing& dealing = m_parent.PushOrderWithHook(orderInfo, this);
		DealingStatus status;
		status.LocalId = dealing.GetLocalId();
		m_DealingStatus.AddReverse(status);
		m_pricePosition.OnReverseOrder(price, qty);
		m_parent.AddPendingPrice(this, "Reverse Order", dealing.GetDirection(), price, qty);
		
		m_pricePosition.FreezeFee();	// 只冻结手续费
		m_parent.SetReverseCnt(majorDir, true);

		money majorPrice = GetMajorOrderedPrice();
		Log::Info(fmt::format("!!Reverse[{}]: LocalID[{}] Inst[{}]			Major[{} {}: {:.3f}] -> Rvs[{} {}: {:.3f}]",
			(void*)this, status.LocalId, inst.GetInstCode().Str(), Desc(majorDir), Desc(majorOpt), majorPrice,
			Desc(reverseDir), Desc(dealing.GetOperation()), price));
		m_retryTag = false;
		m_escapeTag = false;
// 		logCurrentPosition("Reverse Send end");
	}
	void PriceBusiness::cancelReverse() {
		if (m_pricePosition.IsReversePending()) {
			for (auto const& kv: m_DealingStatus.ReverseStatuses()) {
				pushCancel(kv.second);
			}
		}
	}
	void PriceBusiness::Pause() {
		m_currentPhase = m_currentPhase->OnPauseCommand();
	}

	inline std::string positionStr(Position const& position) {
		return fmt::format("O:{},D:{},C:{}", position.Ordered, position.Dealed, position.Cancelled);
	}
	void PriceBusiness::logCurrentPosition(char const* prefix) {
		std::string major = positionStr(m_pricePosition.Position().Major);
		std::string reverse = positionStr(m_pricePosition.Position().Reverse);
		Log::Info(fmt::format("PriceBusiness[{}]: [{}], Major[{}] Reverse[{}]", (void*)this, prefix, major, reverse));
	}
}