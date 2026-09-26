#include "pch.h"
#include "SpreadStatus.h"
#include "SpreadBusiness.h"
#include "utils/Log.h"


namespace hf::spread {
	BasePhase* StartPhase::OnCommand() {
		m_pBusiness->m_parent->SetPaused();
		return m_pBusiness->GetStatus(Status::End);
	}
	BasePhase* StartPhase::OnQuote(Quote const& quote) {
		m_pBusiness->InitTags();
		int nRtn = m_pBusiness->TryOffset();
		if (0 == nRtn) {
			nRtn = m_pBusiness->TryOpen();
		}

		if (2 == nRtn) {
			return m_pBusiness->GetStatus(Status::Balancing);
		}
		else if (1 == nRtn) {
			return m_pBusiness->GetStatus(Status::MinorPending);
		}
		return this;
	}


	BasePhase* MinorPendingPhase::OnMinorInsertFailed(Dealing const& dealing)	 {
		m_pBusiness->m_parent->SetStop();
		Log::Info(fmt::format("SpreadBusiness[{}]: Pending-OnMinorInsertFailed, goto 'Stop' Phase", m_pBusiness->m_id));
		return m_pBusiness->GetStatus(Status::End);
	}
	BasePhase* MinorPendingPhase::OnCommand() {
		if (m_pBusiness->IsMinorPending()) {
			m_pBusiness->cancelMinor();
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Pending-OnCommand, goto 'Pausing' Phase", m_pBusiness->m_id));
		return m_pBusiness->GetStatus(Status::Pausing);
	}
	BasePhase* MinorPendingPhase::OnQuote(Quote const& quote) {
		if (m_pBusiness->IsMinorPending() && m_pBusiness->IsMissPendingChance()) {
			m_pBusiness->cancelMinor();
		}
		return this;
	}
	BasePhase* MinorPendingPhase::OnMinorCancel(int qty) {
		return m_pBusiness->GetStatus(Status::Start);
	}
	BasePhase* MinorPendingPhase::OnMinorTrade(Dealing const& dealing, int qty) {	
		QtyT majorQty = qty * m_pBusiness->m_multiple;
		Quote const& majorQuote = WritableQuoteService().GetQuote(m_pBusiness->m_parent->GetArg().Instruments.Major);
		money majorPrice = m_pBusiness->m_majorPendingPrice;

		// 挂单套利的Major委托必须用Normal，否则无法区分是挂单配平还是FAK补单（SendFailed）
		if (Operation::OPEN != dealing.GetOperation()) {
			m_pBusiness->offsetMajor(majorPrice, majorQty);
		}
		else {
			m_pBusiness->m_offsetForbiddenTag ? m_pBusiness->offsetMajor(majorPrice, majorQty) : m_pBusiness->openMajor(majorPrice, majorQty);
		}

		if (!dealing.IsAllTrade()) {
			m_pBusiness->cancelMinor();
			Log::Info(fmt::format("SpreadBusiness[{}]: MinorPartialTrade[{}], cancelRemain[{}]", m_pBusiness->m_id, qty, dealing.GetRemainQuantity()));
		}

		return m_pBusiness->GetStatus(Status::Balancing);
	}


	// 结束状态的Dealing会被框架删除，所以不能对m_majorDealing有任何预期值
	BasePhase* BalancingPhase::OnMinorInsertFailed(Dealing const& dealing) {
		int majorQty = m_pBusiness->m_portfolio.GetBalancingQty();
		if (0 == majorQty) {
			if (false == m_pBusiness->m_majorLeg.EndTag) {
				Log::Info(fmt::format("SpreadBusiness[{}]: FAK-Minor InsertFailed but FAK-Major's status is unknown[{}]"
					, m_pBusiness->m_id, m_pBusiness->m_majorLeg.LegDealing->GetStatus()));
				return this;
			}
			else {
				Log::Info(fmt::format("SpreadBusiness[{}]: FAK-Minor InsertFailed and FAK-Major has no trade, goto 'Stop' Phase", m_pBusiness->m_id));
				m_pBusiness->m_parent->SetStop();
				return m_pBusiness->GetStatus(Status::End);
			}
		}

		// FAK-Major 有成交，无论开平，直接反开回来（和Minor方向相同）只发一次，跳20档确保成交
		InstID majorID = m_pBusiness->m_orders.Instruments.Major;
		Quote const& majorQuote = WritableQuoteService().GetQuote(majorID);
		Direction minorDir = dealing.GetDirection();
		money unitPrice = majorQuote.GetUnitPrice();
		money majorPrice = (Direction::SELL == minorDir) ? majorQuote.GetBuy().GetPrice() - unitPrice * 20 : majorQuote.GetSell().GetPrice() + unitPrice * 20;
		m_pBusiness->balanceFAKOpenMajor(minorDir, majorPrice, abs(majorQty));	// MinorFailed，所以BalanceQty一定小于0
		Log::Info(fmt::format("SpreadBusiness[{}]: FAK-Minor InsertFailed and unbalance, balance FAK-Major[{}: {:.3f},{}]"
			, m_pBusiness->m_id, Desc(minorDir), majorPrice, majorQty));
		return this;
	}	
	BasePhase* BalancingPhase::OnMajorInsertFailed(Dealing const& dealing) {
		int minorQty = m_pBusiness->m_portfolio.GetBalancingQty() / m_pBusiness->m_multiple;
		if (0 == minorQty) {	// MajorInsertFailed时MajorPending一定是0，如果持仓平衡则一定是FAK-Minor而非Normal-Minor, 否则不可能平衡
			if (false == m_pBusiness->m_minorLeg.EndTag) {
				Log::Info(fmt::format("SpreadBusiness[{}]: FAK-Major InsertFailed but FAK-Minor's status is unknown[{}]"
					, m_pBusiness->m_id, m_pBusiness->m_minorLeg.LegDealing->GetStatus()));
				return this;
			}	// FAK-Minor 已发，委托回报还没回来，等收到Minor回报后再处理
			else {
				Log::Info(fmt::format("SpreadBusiness[{}]: FAK-Major InsertFailed and FAK-Minor has no trade, goto 'Stop' Phase", m_pBusiness->m_id));
				m_pBusiness->m_parent->SetStop();
				return m_pBusiness->GetStatus(Status::End);
			}
		}
	
		// Minor 有成交，无论类型也无论开平，都要反开回来（和Major方向相同）只发一次，跳20档确保成交
		InstID minorID = m_pBusiness->m_orders.Instruments.Minor;
		Quote const& minorQuote = WritableQuoteService().GetQuote(minorID);
		Direction majorDir = dealing.GetDirection();
		money unitPrice = minorQuote.GetUnitPrice();
		money minorPrice = (Direction::SELL == majorDir) ? minorQuote.GetBuy().GetPrice() - unitPrice * 20 : minorQuote.GetSell().GetPrice() + unitPrice * 20;
		m_pBusiness->balanceFAKOpenMinor(majorDir, minorPrice, minorQty);		// MajorFailed，所以BalanceQty一定大于0
		Log::Info(fmt::format("SpreadBusiness[{}]: Major InsertFailed and unbalance, balance FAK-Minor[{}: {:.3f},{}]"
			, m_pBusiness->m_id, Desc(majorDir), minorPrice, minorQty));
		return this;
	}
	BasePhase* BalancingPhase::OnCommand() {	
		if (m_pBusiness->IsMajorPending()) {
			m_pBusiness->cancelMajor();
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnCommand, goto 'Pausing' Phase", m_pBusiness->m_id));
		return m_pBusiness->GetStatus(Status::Pausing);
	}
	BasePhase* BalancingPhase::OnQuote(Quote const& quote) {
		if (m_pBusiness->IsMajorPending()) {
			Dealing const& majorDealing = *m_pBusiness->m_majorLeg.LegDealing;
			if (!m_pBusiness->m_portfolio.GetSPInstruments().IsPriceFit(majorDealing.GetInstID(), majorDealing.GetDirection(), majorDealing.GetOrderPrice(), false, 0)/*只看第一档*/) {
				m_pBusiness->cancelMajor();
			}
		}
		return this;
	}
	BasePhase* BalancingPhase::OnMinorCancel(int qty) {
		Dealing const& minorDealing = *m_pBusiness->m_minorLeg.LegDealing;
		if (OrderType::FAK == m_pBusiness->m_minorLeg.LegDealing->GetOrderType()) {
			if (false == m_pBusiness->m_minorLeg.EndTag || false == m_pBusiness->m_majorLeg.EndTag) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorCancel[{}], FAK-Minor's EndTag[{}] FAK-Major's EndTag[{}]"
					, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag, m_pBusiness->m_majorLeg.EndTag));
				return this;
			}	// MarketOrder若有任何一腿没有结束，不做配平检查
			if (m_pBusiness->BalanceCheck(minorDealing.GetDirection(), minorDealing.GetOperation(), OrderType::FAK, true)) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorCancel[{}], FAK is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
				return m_pBusiness->GetStatus(Status::Start);
			}
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorCancel[{}], FAK is unbalance and DoBalance", m_pBusiness->m_id, qty));
			return this;
		}

		if (false == m_pBusiness->m_majorLeg.RtnTag/*无回报*/) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorCancel[{}], NML-Minor's EndTag[{}] NML-Major's RtnTag[{}]"
				, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag, m_pBusiness->m_majorLeg.RtnTag));
			return this;
		}	// NML-Minor 必须已经结束；NML-Major 必须收到回报

		if (m_pBusiness->BalanceCheck(minorDealing.GetDirection(), minorDealing.GetOperation(), OrderType::NORMAL, true)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorCancel[{}], NML is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
			return m_pBusiness->GetStatus(Status::Start);
		}	// NML-Minor 已经撤单，持仓平衡且没有挂单
		Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorCancel[{}], NML is unbalance and DoBalance", m_pBusiness->m_id, qty));
		return this;
	}
	BasePhase* BalancingPhase::OnMinorTrade(Dealing const& dealing, int qty) {
		if (OrderType::FAK == dealing.GetOrderType()) {
			if (false == m_pBusiness->m_minorLeg.EndTag || false == m_pBusiness->m_majorLeg.EndTag) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorTrade[{}], FAK-Minor's EndTag[{}] FAK-Major's EndTag[{}]"
					, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag, m_pBusiness->m_majorLeg.EndTag));
				return this;
			}	// MarketOrder若有任何一腿没有结束，不做配平检查
			if (m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::FAK, true)) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorTrade[{}], FAK is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
				m_pBusiness->UpdateBand(dealing.IsOpenOrder());
				return m_pBusiness->GetStatus(Status::Start);
			}
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorTrade[{}], FAK is unbalance and DoBalance", m_pBusiness->m_id, qty));
			return this;
		}

		if (false == m_pBusiness->m_minorLeg.EndTag/*先成交回报再撤单回报*/ || false == m_pBusiness->m_majorLeg.RtnTag/*无回报*/) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorTrade[{}], NML-Minor's EndTag[{}] NML-Major's RtnTag[{}]"
				, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag, m_pBusiness->m_majorLeg.RtnTag));
			return this;
		}	// NML-Minor 必须已经结束；NML-Major 必须收到回报

		if (m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::NORMAL, true)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorTrade[{}], NML is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
			m_pBusiness->UpdateBand(dealing.IsOpenOrder());
			return m_pBusiness->GetStatus(Status::Start);
		}	// NML-Minor 新成交，NML-Major 全部成交会补量；NML-Major 有挂单会撤单（去OnMajorCancel配平）
		Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMinorTrade[{}], NBL is unbalance and DoBalance. Minor's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag));
		return this;
	}
	BasePhase* BalancingPhase::OnMajorCancel(int qty) {
		Dealing const& majorDealing = *m_pBusiness->m_majorLeg.LegDealing;
		if (OrderType::FAK == m_pBusiness->m_majorLeg.LegDealing->GetOrderType()) {
			if (false == m_pBusiness->m_majorLeg.EndTag || false == m_pBusiness->m_minorLeg.EndTag) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorCancel[{}], FAK-Major's EndTag[{}] FAK-Minor's EndTag[{}]"
					, m_pBusiness->m_id, qty, m_pBusiness->m_majorLeg.EndTag, m_pBusiness->m_minorLeg.EndTag));
				return this;
			}	// MarketOrder若有任何一腿没有结束，不做配平检查
			if (m_pBusiness->BalanceCheck(majorDealing.GetDirection(), majorDealing.GetOperation(), OrderType::FAK, false)) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorCancel[{}], FAK is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
				return m_pBusiness->GetStatus(Status::Start);
			}
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorCancel[{}], FAK is unbalance and DoBalance", m_pBusiness->m_id, qty));
			return this;
		}

		if (m_pBusiness->m_minorLeg.EndTag && m_pBusiness->BalanceCheck(majorDealing.GetDirection(), majorDealing.GetOperation(), OrderType::NORMAL, false)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorCancel[{}], NML is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
			return m_pBusiness->GetStatus(Status::Start);
		}	// NML-Major 的撤单：NML-Minor 撤单时有成交引发 或 MajorMissChance引发，都需要补发不可能平衡
		Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorCancel[{}], NML is unbalance and Dobalance. Minor's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag));
		return this;
	}
	BasePhase* BalancingPhase::OnMajorTrade(Dealing const& dealing, int qty) {
		if (OrderType::FAK == dealing.GetOrderType()) {	
			if (false == m_pBusiness->m_majorLeg.EndTag || false == m_pBusiness->m_minorLeg.EndTag) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorTrade[{}], FAK-Major's EndTag[{}] FAK-Minor's EndTag[{}]"
					, m_pBusiness->m_id, qty, m_pBusiness->m_majorLeg.EndTag, m_pBusiness->m_minorLeg.EndTag));
				return this;
			}	// MarketOrder若有任何一腿没有结束，不做配平检查
			if (m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::FAK, false)) {
				Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorTrade[{}], FAK is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
				m_pBusiness->UpdateBand(dealing.IsOpenOrder());
				return m_pBusiness->GetStatus(Status::Start);
			}
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorTrade[{}], FAK is unbalance and DoBalance", m_pBusiness->m_id, qty));
			return this;
		}

		if (m_pBusiness->m_minorLeg.EndTag && m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::NORMAL, false)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorTrade[{}], NML is balanced and goto 'Start' Phase", m_pBusiness->m_id, qty));
			m_pBusiness->UpdateBand(dealing.IsOpenOrder());
			return m_pBusiness->GetStatus(Status::Start);
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Balancing-OnMajorTrade[{}], NML is unbalance and Dobalance. Minor's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag));
		return this;	// Normal-Major 部分成交，挂单平衡
	}


	BasePhase* PausingPhase::OnQuote(Quote const& quote) {
		if (m_pBusiness->m_majorLeg.EndTag && m_pBusiness->m_minorLeg.EndTag && 0 == m_pBusiness->m_portfolio.GetBalancingQty()) {
			m_pBusiness->m_parent->SetPaused();
			return m_pBusiness->GetStatus(Status::End);
		}
		if (m_pBusiness->IsMajorPending()) {
			m_pBusiness->cancelMajor();
		}
		if (m_pBusiness->IsMinorPending()) {
			m_pBusiness->cancelMinor();
		}
		return this;
	}
	BasePhase* PausingPhase::OnMinorCancel(int qty) {
		Dealing const& dealing = *m_pBusiness->m_minorLeg.LegDealing;
		if (m_pBusiness->m_majorLeg.EndTag && m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::FAK, true)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMinorCancel[{}], Portfolio is balanced and goto 'Paused' Phase", m_pBusiness->m_id, qty));
			m_pBusiness->m_parent->SetPaused();
			return m_pBusiness->GetStatus(Status::End);
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMinorCancel[{}], Portfolio is unbalance and Dobalance. Major's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_majorLeg.EndTag));
		return this;
	}
	BasePhase* PausingPhase::OnMinorTrade(Dealing const& dealing, int qty) {
		if (m_pBusiness->m_majorLeg.EndTag && m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::FAK, true)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMinorTrade[{}], Portfolio is balanced and goto 'Paused' Phase", m_pBusiness->m_id, qty));
			m_pBusiness->m_parent->SetPaused();
			return m_pBusiness->GetStatus(Status::End);
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMinorTrade[{}], Portfolio is unbalance and Dobalance. Major's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_majorLeg.EndTag));
		return this;
	}
	BasePhase* PausingPhase::OnMajorCancel(int qty) {
		Dealing const& dealing = *m_pBusiness->m_majorLeg.LegDealing;
		if (m_pBusiness->m_minorLeg.EndTag && m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::FAK, false)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMajorCancel[{}], Portfolio is balanced and goto 'Paused' Phase", m_pBusiness->m_id, qty));
			m_pBusiness->m_parent->SetPaused();
			return m_pBusiness->GetStatus(Status::End);
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMajorCancel[{}], Portfolio is unbalance and DoBalance. Minor's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag));
		return this;
	}
	BasePhase* PausingPhase::OnMajorTrade(Dealing const& dealing, int qty) {
		if (m_pBusiness->m_minorLeg.EndTag && m_pBusiness->BalanceCheck(dealing.GetDirection(), dealing.GetOperation(), OrderType::FAK, false)) {
			Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMajorTrade[{}], Portfolio is balanced and goto 'Paused' Phase", (void*)m_pBusiness, qty));
			m_pBusiness->m_parent->SetPaused();
			return m_pBusiness->GetStatus(Status::End);
		}
		Log::Info(fmt::format("SpreadBusiness[{}]: Pausing-OnMajorTrade[{}], Portfolio is unbalance and DoBalance. Minor's EndTag[{}]"
			, m_pBusiness->m_id, qty, m_pBusiness->m_minorLeg.EndTag));
		return this;
	}
}