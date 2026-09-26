#include "pch.h"

#include "HKMarketingStrategy.h"
#include "model/Quote.h"
#include "model/dealing/OrderCommand.h"
#include "model/dealing/MarketingDealing.h"
#include "model/factory/InstrumentFactory.h"
#include "notify/Notify.h"
#include "runner/Queues.h"
#include "utils/Log.h"

#include "ChangeApplier.h"
#include "BalanceHandler.h"
#include "HedgeManager.h"

using BusinessSnapshot = hk::Snapshot;

template<>
inline void StrategyPublisher::NotifyUIBusinessSnapshot<BusinessSnapshot>(BusinessSnapshot const& update) {
	Data data;
	data.Type = DataType::BusinessUpdate;
	data.Size = sizeof(BusinessSnapshot);
	memcpy(data.Data, &update, data.Size);
	m_queues.NotifyUI(data);
}

namespace hk {

	HKMarketingStrategy::HKMarketingStrategy(UIUpdater* updater, Arguments const& arguments, StrategyPublisher& publisher, DealingRepository& dealings)
		:Strategy<Arguments>(updater, arguments, publisher, dealings), m_business(this),
		m_position(m_arg), m_changeApplier(new QuoteChangeApplier{ *this }),
		/*m_balanceHandler(new BalanceHandler(*this)),*/ m_hedge(*this) {

		m_biasTick = m_position.GetBiasTick();
		initDetailedPosition(arguments);
		initSnapshot();
	}

	void HKMarketingStrategy::initSnapshot() {
		m_snapshot.ID = this->m_id;
		m_snapshot.Status = BusinessStatus::Paused;
		m_snapshot.UIUpdaterPtr = this->m_updater;
		m_snapshot.BiasTick = m_position.GetBiasTick();
		m_snapshot.PosInfo = m_posInfo.c_str();
		m_snapshot.Position = m_position.Pos();
	}

	void HKMarketingStrategy::initDetailedPosition(const hk::Arguments& arguments) {
		if (arguments.InitPosition.Qty > 0) {
			pos::TickPosition p{ m_arg.InitPosition.Price, m_arg.InitPosition.Qty };
			if (arguments.InitPosition.Direct == Direction::BUY) {
				m_posInfo = m_detailedPositions.BuyTradeMatch(p);
			} else {
				m_posInfo = m_detailedPositions.SellTradeMatch(p);
			}
		}
	}

	void HKMarketingStrategy::DoOnInit() {
		m_publisher.SubscribeQuote(m_arg.InstIDs.Major);
		m_publisher.SubscribeQuote(m_arg.InstIDs.Follow);
		m_publisher.SubscribeQuote(m_arg.InstIDs.Hedge);
	}


	void FillTickQuotes(SimpleQuoteArray const& input, TickQuoteArray& output, money unitPrice, Direction direct) {
		for (size_t i = 0; i < input.size(); i++) {
			SimpleQuote const& quote = input[i];
			output[i].Direction = direct;
			output[i].Price = priceToTick(quote.GetPrice(), unitPrice);
			output[i].Quantity = quote.GetQuantity();
		}
	}

	void HKMarketingStrategy::DoOnQuote(Quote const& quote) {
		if (m_arg.InstIDs.Follow == quote.GetInstID()) {
			m_lastFollowQuote = &quote;
			if (!IsRunning()) return;

			BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
			m_changeApplier->Handle(quote);
			updateSnapshot();
		}
		else if (m_arg.InstIDs.Hedge == quote.GetInstID()) {
			m_lastHedgeQuote = &quote;
		}
	}

	void HKMarketingStrategy::DoOnCommand(StrategyCommand const& command) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);
		OnStatusCommand(command);
		switch (command.Type) {
		case StrategyCommand::PAUSE: {
			Log::Info(fmt::format("Business:[{}] Pause cmd", (void*)this));
			Pause("Pause cmd");
			break;
		}
		case StrategyCommand::STOP: {
			Log::Info(fmt::format("Business:[{}] Stop cmd", (void*)this));
			Stop();
			break;
		}
		case StrategyCommand::START: {
			Log::Info(fmt::format("Business:[{}] Start cmd", (void*)this));
			Start();
			break;
		}
		case StrategyCommand::UPDATE_ARGUMENT: {
			m_arg = *(Arguments*)command.Data;
			break;
		}
		default:
			break;
		}
		m_snapshot.Status = m_status;
	}

	void HKMarketingStrategy::DoOnOrdered(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);

		if (isQuotingDealing(dealing)) {
			onMajorOrder(dealing);
			m_changeApplier->Reapply();
		}
		updateSnapshot();
	}

	void HKMarketingStrategy::DoOnQuantityTrade(Dealing const& dealing, int quantity) {
	}

	void HKMarketingStrategy::DoOnCancelled(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);

		if (dealing.GetInstID() == m_arg.InstIDs.Major) {
			if (isQuotingDealing(dealing)) {
				onMajorCancel(dealing);
				m_changeApplier->Reapply();
			} 
		}
		if (dealing.GetInstID() == m_arg.InstIDs.Follow) {
			m_hedge.OnHedgeCancel(dealing);
		}
		updateSnapshot();
	}


	inline void HKMarketingStrategy::onMajorOrder(Dealing const& dealing) {
		QuotingBusiness* business = (QuotingBusiness*)dealing.GetHookPtr();
		bool isBalance = false;
		if (IsBuy(dealing.GetDirection())) {
			business->OnBuyOrder(isBalance);
		} else {
			business->OnSellOrder(isBalance);
		}
	}

	inline void HKMarketingStrategy::onMajorTrade(Trade const& trade) {
		updatePosition(trade);
		m_hedge.OnMajorTrade(trade);

		Log::Info("1");
		QuotingBusiness* business = (QuotingBusiness*)trade.GetDealing().GetHookPtr();
		Log::Info(fmt::format("2 {}",(void*)business));
		bool isBalance = false;
		auto direction = trade.GetDealing().GetDirection();
		Log::Info("3");
		if (IsBuy(direction)) {
			business->OnBuyTrade(isBalance);
		} else {
			business->OnSellTrade(isBalance);
		}
		Log::Info("4");

		if (m_position.IsOverMaxQuantity()) {
			Pause("Over Max Quantity");
		}
		Log::Info("5");
// 		if (m_detailedPositions.Size() >= 5) {
// 			Pause("Big #####GC##### Pause");
// 		}
//		Log::Info("6");
	}

	inline void HKMarketingStrategy::updatePosition(const Trade& trade) {
		TickT price = priceToTick(trade.GetDealPrice(), m_arg.UnitPrice);
		if (IsBuy(trade.GetDealing().GetDirection())) {
			m_position.Buy(price, trade.GetDealQuantity());
			m_posInfo = m_detailedPositions.BuyTradeMatch(pos::TickPosition{ price, QtyT(trade.GetDealQuantity()) });
		} else {
			m_position.Sell(price, trade.GetDealQuantity());
			m_posInfo = m_detailedPositions.SellTradeMatch(pos::TickPosition{ price, QtyT(trade.GetDealQuantity()) });
		}
		logDetailedPostion();
	}

	inline void HKMarketingStrategy::onMajorCancel(Dealing const& dealing) {

		if (!m_business.IsMyDealing(dealing.GetLocalId())) return; ////为了应对sb的fuck esunny 'A'

		bool isBalance = false;

		if (IsBuy(dealing.GetDirection())) {
			m_business.OnBuyCancel(isBalance);
		} else {
			m_business.OnSellCancel(isBalance);
		}
	}


	void HKMarketingStrategy::Reset() {
	}

	void HKMarketingStrategy::Pause(std::string const& reason) {
		Log::Info(fmt::format("Business:[{}] Pause called, reason[{}]", (void*)this, reason));
		m_business.Pause();
		m_status = BusinessStatus::Paused;
	}

	void HKMarketingStrategy::Stop() {
		Log::Info(fmt::format("Business:[{}] Stop called", (void*)this));
		Pause("Stop cmd");
		m_position.ResetPos(0);
		m_detailedPositions.Reset();
		m_status = BusinessStatus::STOP;
	}

	void HKMarketingStrategy::Start() {
		Log::Info(fmt::format("Business:[{}] Start called", (void*)this));
		m_business.Start();
		m_status = BusinessStatus::Running;
	}

	void HKMarketingStrategy::DoOnTrade(Trade const& trade) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);

		if (isQuotingDealing(trade.GetDealing())) {
			onMajorTrade(trade);
		}
		
		if (trade.GetDealing().GetInstID() == m_arg.InstIDs.Follow) {
			m_hedge.OnHedgeTrade(trade);

			m_hedgeInfo = m_hedge.GetHedgeInfo();
		}
		m_changeApplier->Reapply();
		updateSnapshot();
	}

	void HKMarketingStrategy::logDetailedPostion() {
		char const* instCode = InstrumentFactory::GetInstruments().Get(m_arg.InstIDs.Major).GetInstCode().Str();
		Log::Info(fmt::format("Business:[{}] Inst:[{}] Pos:[{}]", (void*)this, instCode, m_posInfo));
	}

	inline void HKMarketingStrategy::updateSnapshot() {
		m_snapshot.BiasTick = m_position.GetBiasTick();
		m_snapshot.Position = m_position.Pos();
		m_snapshot.Status = m_status;
		m_snapshot.PosInfo = m_posInfo.c_str();
		m_snapshot.HedgeInfo = m_hedgeInfo.c_str();
		m_snapshot.HedgePosition = m_hedge.GetHedgePos();
	}


	bool isEnd(const Dealing& dealing) {
		return dealing.IsFinished() || dealing.IsFailed();
	}

	bool HKMarketingStrategy::isQuotingDealing(const Dealing& dealing) {
		return dealing.GetHookPtr() == &m_business;
	}


	void HKMarketingStrategy::DoOnOther(Dealing const& dealing) {
		BusinessUpdater<BusinessSnapshot> updater(this, m_snapshot);

		if (isQuotingDealing(dealing)) {
			if (dealing.IsFailed()) {
				m_business.Fail();
				m_status = BusinessStatus::Paused;
			}
		} 
		m_changeApplier->Reapply();
		updateSnapshot();
	}



	void HKMarketingStrategy::DoOnConnectStatus(Exchange exchange, ConnectStatus connectStatus) {
		switch (connectStatus) {
		case ConnectStatus::QuoteConnected:
		case ConnectStatus::TradeConnected:
			m_business.OnConnect();
			break;
		case ConnectStatus::QuoteDisconnected:
		case ConnectStatus::TradeDisconnected:
			m_business.OnDisconnect();
		default:
			break;
		}
	}

	void HKMarketingStrategy::DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) {
	}


	OrderInfo HKMarketingStrategy::createHedgeOrder(Direction direct, QtyT qty) {
		auto& instrument = GetInstrument(m_arg.InstIDs.Follow);

		OrderInfo order;
		order.Exchange = instrument.GetExchange();
		order.Operation = Operation::UNKNOWN;
		order.OrderType = OrderType::FAK;
		order.Direction = direct;
		order.InstID = m_arg.InstIDs.Hedge;
		order.InstCode = instrument.GetInstCode();
		order.LocalId = LocalIdACC::NextNormal();
		order.Quantity = qty;
		order.Price = getHedgePrice(direct, instrument);
		return order;
	}

	constexpr QtyT LOST_TICK = 5;

	money HKMarketingStrategy::getHedgePrice(Direction direct, Instrument const& instrument) {
		if (IsBuy(direct)) {
			auto price = m_lastHedgeQuote->GetSell().GetPrice();
			auto hidenPrice = m_lastHedgeQuote->GetHidenSell().GetPrice();
			price = price < hidenPrice ? price : hidenPrice;
			return price += instrument.GetUnitPrice() * LOST_TICK;
		} else {
			auto price = m_lastHedgeQuote->GetBuy().GetPrice();
			auto hidenPrice = m_lastHedgeQuote->GetHidenBuy().GetPrice();
			price = price > hidenPrice ? price : hidenPrice;
			return price -= instrument.GetUnitPrice() * LOST_TICK;
		}
	}
	money HKMarketingStrategy::PushHedgeOrder(Direction direct, QtyT qty) {
		OrderInfo order = createHedgeOrder(direct, qty);
		PushOrder(order);

		return order.Price;
	}
}
