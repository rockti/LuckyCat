#include "AutoStrategy.h"
#include "notify/Notify.h"
#include "runner/Queues.h"
#include "utils/Log.h"

using BusinessSnapshot = automate::AutoSnapshot;

template<>
inline void StrategyPublisher::NotifyUIBusinessSnapshot<BusinessSnapshot>(BusinessSnapshot const& update) {
	Data data;
	data.Type = DataType::BusinessUpdate;
	data.Size = sizeof(BusinessSnapshot);
	memcpy(data.Payload, &update, data.Size);
	m_queues.NotifyUI(data);
}

using SnapshotUpdater = BusinessUpdater<BusinessSnapshot> ;

namespace automate {
	bool AutoStrategy::isMajor(Dealing const& dealing) const {
		return dealing.GetInstID() == m_arg.Major;
	}
	bool AutoStrategy::isMinor(Dealing const& dealing) const {
		return dealing.GetInstID() == m_arg.Minor;
	}

	bool AutoStrategy::isMajor(Quote const& quote) const {
		return quote.GetInstID() == m_arg.Major;
	}
	bool AutoStrategy::isMinor(Quote const& quote) const {
		return quote.GetInstID() == m_arg.Minor;
	}

	bool AutoStrategy::isMajor(InvestorTrade const& trade) const {
		return trade.InstID == m_arg.Major;
	}
	bool AutoStrategy::isMinor(InvestorTrade const& trade) const {
		return trade.InstID == m_arg.Minor;
	}

	void AutoStrategy::DoOnInit() {
		SnapshotUpdater updater(this, snapshot_);
	}

	void AutoStrategy::logArg() {

		auto msg = fmt::format("Argument Business [{}] Inst[{} {}] Mode[{} {}] Spread[{} {} {} {}] Max[{}] Qty[{}] ExistedDir[{}] ExistedQty[{}] Tolerance[{}] N[{}] LostTick[{}]",
			this->m_id, m_arg.Major, m_arg.Minor,
			m_arg.Mode, m_arg.PriceMode,
			m_arg.SellOpenSpread, m_arg.SellCloseSpread, m_arg.BuyCloseSpread, m_arg.BuyOpenSpread,
			m_arg.MaxVolume, m_arg.Quantity, m_arg.ExistedVolume.OpenDirection, m_arg.ExistedVolume.Volume,
			m_arg.Tolerance, m_arg.VolumeMultiple, m_arg.MajorLostTick);
		Log::Info(msg);
	}

	void AutoStrategy::DoOnQuote(Quote const& quote) {
		if (isMajor(quote)) {
			majorQuote_.UpdateQuote(quote);
			context_.OnQuote(quote);
		} else if (isMinor(quote)) {
			minorQuote_.UpdateQuote(quote);
			context_.OnQuote(quote);
		}
	}

	void AutoStrategy::DoOnCommand(StrategyCommand const& command) {
		SnapshotUpdater updater(this, snapshot_);
		onStatusCommand(command);
		switch (command.Type) {
		case StrategyCommand::START: {
			logArg();
			context_.Start();
			break;
		}
		case StrategyCommand::PAUSE: {
			context_.Pause();
			break;
		}
		case StrategyCommand::STOP: {
			context_.Pause();
			break;
		}
		case StrategyCommand::DELETE_BUSINESS: {
			context_.Pause();
			updater.SetDeleted();
			break;
		}
		case StrategyCommand::UPDATE_ARGUMENT: {
			AutoArgument& arg = *(AutoArgument*)command.Data;
			m_arg = arg;
			logArg();
			break;
		}
		default:
			break;
		}
		snapshot_.Status = m_status;
	}

	void AutoStrategy::DoOnExchangeStatus(type::Exchange exchange, ExchangeStatus status) {
	}

	void AutoStrategy::DoOnOrdered(Dealing const& dealing) {
		SnapshotUpdater updater(this, snapshot_);
		if (isMinor(dealing)) {
			context_.OnMinorOrder(dealing);
		}
	}

	void AutoStrategy::DoOnCancelled(Dealing const& dealing) {
		SnapshotUpdater updater(this, snapshot_);
		if (isMajor(dealing)) {
			context_.OnMajorFinished(dealing);
		} else if (isMinor(dealing)) {
			if (dealing.GetHookPtr() != nullptr) {
				if (context_.GetArg().PriceMode == PriceMode::Pending) {
					context_.OnMinorCancel(dealing);
				} else if (context_.GetArg().PriceMode == PriceMode::FAK) {
					context_.OnMinorFinished(dealing);
				}
			} else {
				context_.OnMinorOppositeFinished(dealing);
			}
		}
	}

	void AutoStrategy::DoOnOther(Dealing const& dealing) {

		if (m_arg.PriceMode == PriceMode::FAK) {
			if (isMinor(dealing) && dealing.GetHookPtr() && dealing.IsFinished()) {
				context_.OnMinorFinished(dealing);
				return;
			}
		}

		bool isMajor = dealing.GetInstID() == m_arg.Major &&
			dealing.GetOrderType() == type::OrderPriceType::FAK;
		if (isMajor && dealing.IsFinished()) {
			context_.OnMajorFinished(dealing);
			return;
		}

		bool isMinorOpposite = dealing.GetInstID() == m_arg.Minor
			&& dealing.GetOrderType() == type::OrderPriceType::FAK && dealing.GetHookPtr() == nullptr;
		if (isMinorOpposite && dealing.IsFinished()) {
			context_.OnMinorOppositeFinished(dealing);
			return;
		}
	}

	void AutoStrategy::DoOnInsertFail(Dealing const& dealing) {

		bool isMinor = dealing.GetInstID() == m_arg.Minor && dealing.GetHookPtr();
		if (isMinor) {
			context_.OnMinorInsertFailed(dealing);
			return;
		}

		bool isMajor = dealing.GetInstID() == m_arg.Major &&
			dealing.GetOrderType() == type::OrderPriceType::FAK;
		if (isMajor) {
			context_.OnMajorInsertFailed(dealing);
			return;
		}

		bool isMinorOpposite = dealing.GetInstID() == m_arg.Minor
			&& dealing.GetOrderType() == type::OrderPriceType::FAK && dealing.GetHookPtr() == nullptr;
		if (isMinorOpposite) {
			context_.OnMinorOppositeInsertFailed(dealing);
			return;
		}
	}

	void AutoStrategy::DoOnCancelFail(Dealing const& dealing) {

	}
	void AutoStrategy::DoOnTrade(InvestorTrade const& trade) {
		SnapshotUpdater updater(this, snapshot_);
		context_.UpdatePositionByTrade(trade);
		if (trade.InstID == m_arg.Minor && trade.OrderLocalID == context_.GetMinorDealing().GetLocalId()) {
			context_.OnMinorTrade(context_.GetMinorDealing(), trade);
		}
	}

	void AutoStrategy::DoOnConnectStatus(type::Exchange exchange, ConnectStatus connectStatus) {
	}

	void AutoStrategy::DoUpdateSnapshot(bool deleted) {
		snapshot_.Argument = m_arg;
		snapshot_.Deleted = deleted;
		snapshot_.ID = this->GetId();
		snapshot_.Mode = context_.GetRunning().Mode;
		snapshot_.RunningStatus = context_.GetRunningStatus();
		snapshot_.Status = this->m_status;
		snapshot_.TotalMajor = context_.GetMajorPosition();
		snapshot_.TotalMinor = context_.GetMinorPosition();
	}
}
