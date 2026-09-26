#include "pch.h"
#include "SpreadStrategy.h"
#include "SpreadBusiness.h"
#include "notify/Notify.h"
#include "runner/Queues.h"
#include "utils/Log.h"

using BusinessSnapshot = hf::spread::SpreadSnapshot;
template<>
inline void StrategyPublisher::NotifyUIBusinessSnapshot<BusinessSnapshot>(BusinessSnapshot const& update) {
	Data data;
	data.Type = DataType::BusinessUpdate;
	data.Size = sizeof(BusinessSnapshot);
	memcpy(data.Data, &update, data.Size);
	m_queues.NotifyUI(data);
}

namespace hf::spread {
	char const* RunModeDesc(int mod) {
		return (SpreadType::Spread == mod) ? "Spread" : "Ratio";
	}

	SpreadStrategy::SpreadStrategy(UIUpdater* updater, Arguments const& arguments, StrategyPublisher& publisher, DealingRepository& dealings)
		:Strategy<Arguments>(updater, arguments, publisher, dealings), m_pBusiness(new SpreadBusiness(this)) {

		m_snapshot.ID = this->m_id;
		m_snapshot.Status = BusinessStatus::New;	// 这个是界面的Status，不是状态机的
		m_snapshot.UIUpdaterPtr = this->m_updater;
		m_snapshot.MajorPos = m_pBusiness->m_portfolio.GetTotalMajorVolume();
		m_snapshot.MinorPos = m_pBusiness->m_portfolio.GetTotalMinorVolume();
		m_snapshot.CancelAmt = pos::GlobalPositionStore::Instance().GetCancelCount(m_arg.Instruments.Minor);
	}

	void SpreadStrategy::DoOnInit() {
		m_publisher.SubscribeQuote(m_arg.Instruments.Major);
		m_publisher.SubscribeQuote(m_arg.Instruments.Minor);
		GetPublisher().NotifyUIBusinessSnapshot(m_snapshot);
	}
	void SpreadStrategy::DoOnQuote(Quote const& quote) {
		if (!IsRunning())	return;
		if (false == m_isQuoteReady) {
			Quote const& minorQuote = WritableQuoteService().GetQuote(m_arg.Instruments.Minor);
			Quote const& majorQuote = WritableQuoteService().GetQuote(m_arg.Instruments.Major);
			m_isQuoteReady = (minorQuote.GetLastPrice() > 10) && (majorQuote.GetLastPrice() > 10);	// 两个合约都有成交
		}
		if (m_isQuoteReady && quote.IsValid() && !quote.IsCloseToLimit()) {
			m_pBusiness->OnQuote(quote);
		}
	}
	void SpreadStrategy::DoOnCommand(StrategyCommand const& command) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);

		OnStatusCommand(command);
		switch (command.Type) {
		case StrategyCommand::UPDATE_ARGUMENT: {
			m_arg = *(Arguments*)command.Data;
			if (BusinessStatus::Running == m_status) {
				logArg();
			}
			break;
		}
		case StrategyCommand::START: {
			m_pBusiness->OnStart();
			logArg();
			break;
		}
		case StrategyCommand::PAUSE: {			
			m_pBusiness->OnPaused();
			logArg();
			break;
		}
		default:
			break;
		}
		m_snapshot.Status = m_status;
	}

	void SpreadStrategy::DoOnOrdered(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (isMinor(dealing.GetInstID())) {
			m_pBusiness->m_minorLeg.RtnTag = true;
		}
		else if (isMajor(dealing.GetInstID())) {
			m_pBusiness->m_majorLeg.RtnTag = true;
			if (OrderType::NORMAL == dealing.GetOrderType()) {
				m_pBusiness->m_portfolio.OnMajorPending(dealing.GetOrderQuantity());	// 增加Major挂单计数
			}
		}
	}
	void SpreadStrategy::DoOnCancelled(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (isMinor(dealing.GetInstID())) {
			m_pBusiness->m_minorLeg.RtnTag = true;
			m_pBusiness->OnMinorCancel(dealing.GetCancelQuantity());
			m_snapshot.CancelAmt = pos::GlobalPositionStore::Instance().GetCancelCount(m_arg.Instruments.Minor);
		}
		else if (isMajor(dealing.GetInstID())) {
			m_pBusiness->m_majorLeg.RtnTag = true;
			m_pBusiness->m_portfolio.OnMajorCancel(dealing.GetCancelQuantity());		// 减少Major挂单计数
			m_pBusiness->OnMajorCancel(dealing.GetCancelQuantity());
		}
		m_snapshot.Status = m_status;
	}
	void SpreadStrategy::DoOnQuantityTrade(Dealing const& dealing, int quantity) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (Exchange::SGE == dealing.GetExchange()) {
			return;
		}	// 是SGE，不处理

		m_pBusiness->m_portfolio.OnTrade(dealing, quantity, dealing.GetOrderPrice()/*委托回报没有成交价用委托价代替*/);
		if (isMinor(dealing.GetInstID())) {
			m_pBusiness->m_minorLeg.RtnTag = true;
// 			Log::Info(fmt::format("SpreadStrategy: Minor[{}] OnQuantityTrade, [{}]:[{:.3f}, {}, {}]"
// 				, dealing.GetInstCode().Str(), Desc(dealing.GetDirection()), dealing.GetOrderPrice(), quantity, Desc(dealing.GetOrderType())));
			m_pBusiness->OnMinorTrade(dealing, quantity);
		}
		else if (isMajor(dealing.GetInstID())) {
			m_pBusiness->m_majorLeg.RtnTag = true;
// 			Log::Info(fmt::format("SpreadStrategy: Major[{}] OnQuantityTrade, [{}]:[{:.3f}, {}, {}]"
// 				, dealing.GetInstCode().Str(), Desc(dealing.GetDirection()), dealing.GetOrderPrice(), quantity, Desc(dealing.GetOrderType())));
			m_pBusiness->OnMajorTrade(dealing, quantity);
		}
		m_snapshot.MinorPos = m_pBusiness->m_portfolio.GetTotalMinorVolume();
		m_snapshot.MajorPos = m_pBusiness->m_portfolio.GetTotalMajorVolume();
		m_snapshot.Spread = m_pBusiness->GetSpread();
		m_snapshot.Status = m_status;
	}
	void SpreadStrategy::DoOnTrade(Trade const& trade) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		Dealing const& dealing = trade.GetDealing();
		if (Exchange::SGE != dealing.GetExchange()) {
			return;
		}	// 非SGE，不处理

		m_pBusiness->m_portfolio.OnTrade(dealing, trade.GetDealQuantity(), trade.GetDealPrice());
		if (isMinor(dealing.GetInstID())) {
			m_pBusiness->m_minorLeg.RtnTag = true;
// 			Log::Info(fmt::format("SpreadStrategy: Minor[{}] DoOnTrade, [{}]:[{:.3f}, {}, {}]"
// 				, dealing.GetInstCode().Str(), Desc(dealing.GetDirection()), dealing.GetOrderPrice(), trade.GetDealQuantity(), Desc(dealing.GetOrderType())));
			m_pBusiness->OnMinorTrade(dealing, trade.GetDealQuantity());
		}
		else if (isMajor(dealing.GetInstID())) {
			m_pBusiness->m_majorLeg.RtnTag = true;
// 			Log::Info(fmt::format("SpreadStrategy: Major[{}] DoOnTrade, [{}]:[{:.3f}, {}, {}]"
// 				, dealing.GetInstCode().Str(), Desc(dealing.GetDirection()), dealing.GetOrderPrice(), trade.GetDealQuantity(), Desc(dealing.GetOrderType())));
			m_pBusiness->OnMajorTrade(dealing, trade.GetDealQuantity());
		}	
		m_snapshot.MinorPos = m_pBusiness->m_portfolio.GetTotalMinorVolume();
		m_snapshot.MajorPos = m_pBusiness->m_portfolio.GetTotalMajorVolume();
		m_snapshot.Spread = m_pBusiness->GetSpread();
		m_snapshot.Status = m_status;
	}
	void SpreadStrategy::DoOnOther(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		if (isMinor(dealing.GetInstID())) {
			m_pBusiness->m_minorLeg.RtnTag = true;
			m_pBusiness->OnMinorFailed(dealing);
		}
		else {
			m_pBusiness->m_majorLeg.RtnTag = true;
			m_pBusiness->OnMajorFailed(dealing);
		}
		auto msg = fmt::format("SpreadStrategy: DoOnOther, Inst[{}] [{} {}:{:.3f},{}] LocalID[{}] DealingStatus[{}]", dealing.GetInstCode().Str(),
			Desc(dealing.GetDirection()), Desc(dealing.GetOperation()), dealing.GetOrderPrice(), dealing.GetOrderQuantity(), dealing.GetLocalId(), dealing.GetStatus());
		m_publisher.NotifyUI(msg);
		Log::Info(msg);
		m_snapshot.Status = m_status;
	}

	Dealing* SpreadStrategy::PushOrderWithHook(OrderInfo const& orderInfo) {
		Dealing& dealing = Strategy::PushOrder(orderInfo);
		dealing.SetHookPtr(m_pBusiness);
		return &dealing;
	}

	bool SpreadStrategy::isMajor(InstID instID) const {
		return instID == m_arg.Instruments.Major;
	}
	bool SpreadStrategy::isMinor(InstID instID) const {
		return instID == m_arg.Instruments.Minor;
	}
	void SpreadStrategy::logArg() {
		Instruments instruments = InstrumentFactory::GetInstruments();
		auto fartherCode(instruments.Get(m_arg.Instruments.Farther).GetInstCode());
		auto closerCode(instruments.Get(m_arg.Instruments.Closer).GetInstCode());
		if (Direction::SELL == m_arg.Dir) {
			Log::Info(fmt::format("Business[{}] Inst[{}-{}] Mode[{}|{}] OfrLine[{:.3f},{:.3f}] Qty[P:{},F:{}] InitPos[{}|{}] ExistPos[{},TD:{}] Status[{}]"
				, this->m_id, fartherCode.Str(), closerCode.Str(), Desc(m_arg.Dir), RunModeDesc(m_arg.Type), m_arg.OfrOpen, m_arg.OfrOffset, m_arg.MinQty
				, m_arg.MaxQty, m_arg.OfrPosition, m_arg.MaxPosition, m_pBusiness->GetPortfolio(), m_pBusiness->GetYDPortfolio(), Desc(m_status)));
		}
		else {
			Log::Info(fmt::format("Business[{}] Inst[{}-{}] Mode[{}|{}] BidLine[{:.3f},{:.3f}] Qty[P:{},F:{}] InitPos[{}|{}] ExistPos[{},TD:{}] Status[{}]"
				, this->m_id, fartherCode.Str(), closerCode.Str(), Desc(m_arg.Dir), RunModeDesc(m_arg.Type), m_arg.BidOpen, m_arg.BidOffset, m_arg.MinQty
				, m_arg.MaxQty, m_arg.OfrPosition, m_arg.MaxPosition, m_pBusiness->GetPortfolio(), m_pBusiness->GetYDPortfolio(), Desc(m_status)));
		}
	}
}
