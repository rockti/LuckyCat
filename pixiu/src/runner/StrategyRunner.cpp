#include "pch.h"
#include "runner/StrategyRunner.h"
#include "strategy/Business.h"
#include "service/PositionService.h"
#include "service/FundService.h"
#include "service/DealingRepository.h"
#include "service/QuoteService.h"
#include "notify/Notify.h"
#include "notify/StrategyPublisher.h"
#include "model/factory/InstrumentFactory.h"
#include "model/dealing/OrderCommand.h"
#include "utils/Log.h"

extern bool g_breakTag;
extern LocalOrderID const GRAB_PREFIX;
namespace {
	inline void initPosition(PositionNotification const& position) {
		auto& s = pos::GlobalPositionStore::Instance();
		if (position.TodayQty > 0) {
			int YDPosition = position.Quantity - position.TodayQty;			
			if (position.TodayQty >= position.Frozen) {
				s.InitToday(position.InstID, position.Direction, position.TodayQty, position.Frozen);	// 全部冻结今仓
				s.InitHistory(position.InstID, position.Direction, YDPosition, 0);						// 昨仓不用冻结
			}
			else {
				int YDFrozen = position.Frozen - position.TodayQty;
				s.InitToday(position.InstID, position.Direction, position.TodayQty, position.TodayQty);	// 今仓全部冻结
				s.InitHistory(position.InstID, position.Direction, YDPosition, YDFrozen);				// 不足部分冻结昨仓
			}
		}	// GlobalPositionStore是将今仓和昨仓分开管理的，这里要做拆分
		else {
			s.InitHistory(position.InstID, position.Direction, position.Quantity, position.Frozen);
		}
	}
}

void StrategyHandler::Handle(Data const& data) {
	switch (data.Type) {
	case DataType::StrategyCommand: {
		StrategyCommand const& command = *(StrategyCommand*)data.Data;
		onCommand(command);
	}
	break;
	case DataType::Quote: {
		Quote const& quote = *(Quote*)data.Data;
		onQuote(quote);
	}
	break;
	case DataType::Price: {
		Quote const& quote = *(Quote*)data.Data;
		onPrice(quote.GetInstID(), quote.GetBuyPriceArray(), quote.GetSellPriceArray(), quote.GetLastPrice(), quote.GetVolume(), quote.GetOpenInterest(), quote.GetMinuteID());
	}
	break;
	case DataType::Dealing: {
		OrderNotification const& notification = *(OrderNotification*)data.Data;
		onOrderNotification(notification);
	}
	break;
	case DataType::QryFund: {
		FundsNotification const& notification = *(FundsNotification*)data.Data;
		onFundsNotification(notification);
	}
	break;
	case DataType::CleanPosition: {
		auto& service = pos::GlobalPositionStore::Instance();
		service.Clear();
	}
	break;
	case DataType::QryPosition: {
		PositionNotification const& position = *(PositionNotification*)data.Data;
		initPosition(position);
	}
	break;
	case DataType::Exchange: {
		ExchangeNotification const& exchangeNotif = *(ExchangeNotification*)data.Data;
		onExchangeNotification(exchangeNotif);
	}
	break;
	default:
		break;
	}
}

void StrategyHandler::onCommand(StrategyCommand const& command) {
	switch (command.Type) {
	case StrategyCommand::ADD_BUSINESS:
		addBusiness(command.Business);
		break;
	default:
		if (command.Business != nullptr) {
			command.Business->OnCommand(command);
		}
		else {
			for (Business* business : m_businesses) {
				if (business->GetId() == command.BusinessID) {
					business->OnCommand(command);
				}
			}
		}
		break;
	}
}

void StrategyHandler::onQuote(Quote const& quote) {
	Quote& existed = WritableQuoteService().GetQuote(quote.GetInstID());
	existed.UpdateQuote(quote);	
	for (auto business : m_businesses) {
		business->OnQuote(existed);
	}
}
void StrategyHandler::onPrice(InstID instID, SimpleQuoteArray const& buyArray, SimpleQuoteArray const& sellArray, money lastPrice, int volume, int interest, int id) {
	Quote& existed = WritableQuoteService().GetQuote(instID);
	existed.UpdatePrice(instID, buyArray, sellArray, lastPrice, volume, interest, id);
	for (auto business : m_businesses) {
		business->OnQuote(existed);
	}
}

void StrategyHandler::onFundsNotification(FundsNotification const& funds) {
	auto& posService = pos::GlobalPositionStore::Instance();
	posService.InitAccount(funds);	// 先初始化内部资金，再通知界面更新资金(UpdateAvailable)
	m_publisher.NotifyUI(posService.Account());
}
void StrategyHandler::onOrderNotification(OrderNotification const& order) {
	onDealing(order);
}
void StrategyHandler::onExchangeNotification(ExchangeNotification const& notification) {
	ConnectStatus connectStatus = ConnectStatus::None;
	switch (notification.Type) {
	case ExchangeNotification::QUOTE_CONNECTED:
		connectStatus = ConnectStatus::QuoteConnected;
		break;
	case ExchangeNotification::QUOTE_DISCONNECTED:
		connectStatus = ConnectStatus::QuoteDisconnected;
		break;
	case ExchangeNotification::TRADER_CONNECTED:
		connectStatus = ConnectStatus::TradeConnected;
		break;
	case ExchangeNotification::TRADER_DISCONNECTED:
		connectStatus = ConnectStatus::TradeDisconnected;
		break;		
	default:
		break;
	}

	for (auto business : m_businesses) {
		business->OnConnectStatus(notification.Exchange, connectStatus);
		business->OnExchangeStatus(notification.Exchange, notification.Status);
	}
}

Dealing* StrategyHandler::findDealing(LocalOrderID localId) {
	Dealing* pDealing = m_dealingRepository.Get(localId);
	if (nullptr == pDealing) {
		// OnRtnOrder里委托状态已经结束，dealing删除。OnRtnTrade晚来，去Cache里找一下（只保存最近20个）
		pDealing = m_deletedDealings.Get(localId);
// 		if (pDealing == nullptr) {
// 			std::string msg = fmt::format("Fuck Twice ESunny[{}]", localId);
// 			Log::Info(msg);
// 		}
	}
	return pDealing;
}
void StrategyHandler::onDealing(OrderNotification const& order) {
	auto& posService = pos::GlobalPositionStore::Instance();
	
	if (order.Status == OrderNotification::E_CANCELED) {
		auto count = posService.IncCancelCount(order.InstID);
	}	// 统计撤单数

	if (order.IsQueryOrder) {
		return;
	}	// 查询委托，目前不需要处理，直接跳过

	if (order.IsQueryTrade) {
		if (order.Operation == Operation::OPEN) {
			posService.Opened(order.InstID, order.Direction);
		}
		return;
	}	// 查询成交如果是开仓，设置Open标志，以处理平今有惩罚的情况

	Dealing* pDealing = findDealing(order.LocalId);
	if (nullptr != pDealing) {
		handleDealing(order, *pDealing);
		removeFinishedDealing(*pDealing);
	}
	else if (order.LocalId > GRAB_PREFIX && order.LocalId < MAX_LOCALID) {
		handleGrabOrder(order);
	}
	else {
		Log::Info(fmt::format("StrategyHandler: can not find business by LocalId[{}]", order.LocalId));
		handleOtherOrder(order);
	}

	auto& account = posService.Account();
	m_publisher.NotifyUI(account);	// 通过UpdateAvailable更新界面资金 

// 	Log::Info(fmt::format("Funds: Balance[{:.3f}] Avail[{:.3f}] Margin[{:.3f}+{:.3f}] Fee[{:.3f}+{:.3f}]", 
// 		account.GetBalance(), account.Available, account.Margin, account.FrozenMargin, account.Fee, account.FrozenFee));
}
void StrategyHandler::handleDealing(const OrderNotification& order, Dealing& dealing) {
	switch (order.Status) {
	case OrderNotification::E_ACCEPT:
		dealing.OnAccepted();
		break;
	case OrderNotification::E_FLOW_CONTROL:
		dealing.OnFlowControl();
		dealing.GetBusiness()->OnOther(dealing);
		break;
	case OrderNotification::E_SENT_FAILED:
		dealing.OnSendFailed();
		dealing.GetBusiness()->OnOther(dealing);
		break;
	case OrderNotification::E_CANCEL_FAILED:
		dealing.OnCancelFailed();
		dealing.GetBusiness()->OnOther(dealing);
		break;
	case OrderNotification::E_ORDERED:
		dealing.OnOrdered(order.SystemNo);
		dealing.GetBusiness()->OnOrdered(dealing);
		break;
	case OrderNotification::E_CANCELED:
		dealing.OnCanceled();
		dealing.GetBusiness()->OnCanceled(dealing);		
		break;
	case OrderNotification::E_QUANTITY_TRADE:		// 通过委托回报更新的成交(非SGE)
		onQuantityTrade(dealing, order);
		break;
	case OrderNotification::E_FAK_PARTIAL_TRADE:  	// 委托回报内没有成交价，用委托价代替(非SGE)
		onFAKPartial(order, dealing);
		break;
// 	case OrderNotification::E_FUCK_ESUNNY_A_CANCEL: // fuck ESunny 'A',特殊处理
// 		onFuckESunnyA(order, dealing);
// 		break;
	case OrderNotification::E_PARTIAL:
	case OrderNotification::E_TRADE:
		m_publisher.NotifyUI(order);				// 通知界面更新持仓列表
		if (Exchange::SGE == order.Exchange) {
			onTradeDetail(order, dealing);
			break;
		}
		return;
	default:
		break;
	}
	m_publisher.NotifyUI(dealing);					// 通知界面更新委托列表
}
void StrategyHandler::handleGrabOrder(OrderNotification const& order) {
	if (nullptr == order.SystemNo || 0 == strlen(order.SystemNo)) {
		return;
	}	// 系统号为空的废单直接丢弃！OrderNotification没有初始化的话，居然判断不出来Fint！！！

	// 根据有效回报生成Dealing
	auto& instrument = InstrumentFactory::GetInstruments().Get(order.InstID);
	OrderInfo orderInfo;
	orderInfo.InstCode = instrument.GetInstCode();
	orderInfo.Exchange = instrument.GetExchange();
	orderInfo.InstID = order.InstID;
	orderInfo.Direction = order.Direction;
	orderInfo.Operation = order.Operation;
	orderInfo.Quantity = order.OrderQuantity;
	orderInfo.Price = order.OrderPrice;
	orderInfo.OrderType = OrderType::FAK;
	orderInfo.LocalId = order.LocalId;
	Dealing dealing{ orderInfo };

	for (auto business : m_businesses) {
		if (business->IsRunning() && (order.InstID == business->GetGrabInst() || order.InstID == business->GetExGrabInst())) {
			dealing.SetBusiness(business);			// 绑定Dealing到具体的Strategy上			
			m_dealingRepository.Add(dealing);			
			break;
		}
	}
	if (nullptr == dealing.GetBusiness()) {
		Log::Error(fmt::format("HandleGrabOrder: can not find business by LocalId[{}]", dealing.GetLocalId()));
		handleOtherOrder(order);
		return;
	}	// 没有找到对应的Business，是其他客户端的大委托号委托

	g_breakTag = true;					// 停止发单循环

	switch (order.Status) {
	case OrderNotification::E_ORDERED:	// 中立仓或交割申报会有挂单状态
		dealing.OnOrdered(order.SystemNo);
		dealing.GetBusiness()->OnOrdered(dealing);
		break;
	case OrderNotification::E_CANCELED:
		dealing.OnCanceled();
		dealing.GetBusiness()->OnCanceled(dealing);
		break;
	case OrderNotification::E_QUANTITY_TRADE:
		onQuantityTrade(dealing, order);
		break;
	case OrderNotification::E_FAK_PARTIAL_TRADE:
		onFAKPartial(order, dealing);
		break;
	case OrderNotification::E_TRADE:	// SGE 的成交明细还要更新界面委托列表，其他交易所直接更新持仓列表即可
		m_publisher.NotifyUI(order);
		if (Exchange::SGE == order.Exchange) {
			onTradeDetail(order, dealing);
			break;
		}
		return;
	default:	return;					// 其他状态不给回报
	}

	m_publisher.NotifyUI(dealing);		// 更新界面的委托列表
}
void StrategyHandler::handleOtherOrder(OrderNotification const& order) {
	if (nullptr == order.SystemNo || 0 == strlen(order.SystemNo)) {
		return;
	}	// 系统号为空的废单直接丢弃！
		
	// 临时生成一个Dealing专供界面处理
	auto& instrument = InstrumentFactory::GetInstruments().Get(order.InstID);
	OrderInfo orderInfo;
	orderInfo.InstCode = instrument.GetInstCode();
	orderInfo.Exchange = instrument.GetExchange();
	orderInfo.InstID = order.InstID;
	orderInfo.Direction = order.Direction;
	orderInfo.Operation = order.Operation;
	orderInfo.Quantity = order.OrderQuantity;
	orderInfo.Price = (0 == order.DealPrice) ? order.OrderPrice : order.DealPrice;
	orderInfo.OrderType = OrderType::FAK;
	orderInfo.LocalId = order.LocalId;
	Dealing dealing{ orderInfo };
	dealing.OnOrdered(order.SystemNo);	// 填上系统号

	switch (order.Status) {
	case OrderNotification::E_ORDERED:
		if (Exchange::SGE == order.Exchange && order.DealQuantity > 0) {
			// SGE重启之前的成交不会推送，要靠委托回报重放来更新界面委托列表
			dealing.OnQuantityTrade(order.DealQuantity);
		}
		break;
	case OrderNotification::E_CANCELED:
		dealing.OnCanceled();
		break;
	case OrderNotification::E_QUANTITY_TRADE:
		dealing.OnQuantityTrade(order.DealQuantity);
		break;
	case OrderNotification::E_FAK_PARTIAL_TRADE:
		dealing.OnQuantityTrade(order.DealQuantity);
		if (dealing.GetRemainQuantity() > 0) {
			dealing.OnCanceled();
		}
		break;
	case OrderNotification::E_TRADE:
		m_publisher.NotifyUI(order);	// 成交明细，只负责更新界面持仓
		return;
	default:	return;					// 其他状态无需通知界面
	}

	m_publisher.NotifyUI(dealing);		// 更新界面的委托列表
}

void StrategyHandler::removeDealing(Dealing const& dealing) {
	m_deletedDealings.Add(dealing);
	m_dealingRepository.Remove(dealing.GetLocalId());
}
void StrategyHandler::removeFinishedDealing(Dealing const& dealing) {
	if (dealing.IsFinished()) {
		if (LocalIdACC::IsMarketing(dealing.GetLocalId())) {
			removeMarketingDealings(dealing);
		} 
		else {
			removeDealing(dealing);
		}
	}
}
void StrategyHandler::removeMarketingDealings(Dealing const& dealing) {
	LocalOrderID localId = dealing.GetLocalId();
	LocalOrderID pairID = LocalIdACC::PairID(localId);

	Dealing const* pairDealing = m_dealingRepository.Get(pairID);
	if (pairDealing == nullptr) {
		Log::Warn(fmt::format("can not find pair dealing for LocalId[{}]", dealing.GetLocalId()));
		removeDealing(dealing);
		return;
	} 
	if (pairDealing->IsFinished()) {
		removeDealing(*pairDealing);
		removeDealing(dealing);
	}
}

void StrategyHandler::onTradeDetail(OrderNotification const& order, Dealing& dealing) {	
	dealing.OnOrdered(order.SystemNo);	// 先补一个系统号	

	// SGE通过成交明细来更新持仓与成交（SGE的OnRtnTrade比OnRtnOrder先来）
	auto& posService = pos::GlobalPositionStore::Instance();
	posService.OnTrade(dealing.GetInstID(), dealing.GetDirection(), dealing.GetOperation(), order.DealQuantity, order.DealPrice);

	dealing.OnTradeDetail(order.DealQuantity, order.DealPrice);
	Trade trade(dealing, order.DealPrice, order.DealQuantity, order.SystemNo);
	dealing.GetBusiness()->OnTrade(trade);
}
void StrategyHandler::onFAKPartial(OrderNotification const& order, Dealing& dealing) {
	if (dealing.IsCanceled()) {
		return;
	}

	// 内部补一个委托回报（更新系统号）
	dealing.OnOrdered(order.SystemNo);
	dealing.GetBusiness()->OnOrdered(dealing);

	// 处理持仓
	auto& posService = pos::GlobalPositionStore::Instance();
	posService.OnTrade(dealing.GetInstID(), dealing.GetDirection(), dealing.GetOperation(), order.DealQuantity, order.OrderPrice);
	
	// 处理成交
	dealing.OnQuantityTrade(order.DealQuantity);							// 更新Dealing
	dealing.GetBusiness()->OnQuantityTrade(dealing, order.DealQuantity);	// 通知Business成交

	// 处理撤单
	if (dealing.GetRemainQuantity() > 0) {
		dealing.OnCanceled();							// 更新Dealing
 		dealing.GetBusiness()->OnCanceled(dealing);		// 通知Business撤单
	}
}
void StrategyHandler::onQuantityTrade(Dealing& dealing, const OrderNotification& order) {
	dealing.OnOrdered(order.SystemNo);				// 如果是对价直接成交，是没有Ordered状态的，要在此处补一个，用于更新系统号
	
	int delta = order.DealQuantity/*服务端成交量*/ - dealing.GetQuantityTraded()/*本地成交量*/;	// 成交增量
	if (delta > 0) {
		// 处理持仓
		auto& posService = pos::GlobalPositionStore::Instance();
		posService.OnTrade(dealing.GetInstID(), dealing.GetDirection(), dealing.GetOperation(), delta, order.OrderPrice);

		dealing.OnQuantityTrade(order.DealQuantity);				// 更新Dealing
		dealing.GetBusiness()->OnQuantityTrade(dealing, delta);		// 通知Business

		if (order.Exchange == Exchange::HKEX) {
			m_publisher.NotifyUI(order);	// esunny没有onTrade, 要在这里更新界面持仓
		}
	}
}
void StrategyHandler::onFuckESunnyA(OrderNotification const& order, Dealing& dealing) {
	//先当作quantityTrade处理，然后取消
	int delta = order.DealQuantity - dealing.GetQuantityTraded();
	if (delta > 0) {
		// 处理持仓
		auto& posService = pos::GlobalPositionStore::Instance();
		posService.OnTrade(dealing.GetInstID(), dealing.GetDirection(), dealing.GetOperation(), delta, order.OrderPrice);

		dealing.OnQuantityTrade(order.DealQuantity);
		dealing.GetBusiness()->OnQuantityTrade(dealing, delta);
		Trade trade(dealing, order.OrderPrice, delta, order.SystemNo);
		dealing.OnTradeDetail(trade.GetDealQuantity(), trade.GetDealPrice());
		dealing.GetBusiness()->OnTrade(trade);
	}
	dealing.OnCanceled();
	dealing.GetBusiness()->OnCanceled(dealing);
}
void StrategyHandler::addBusiness(Business* business) {
	business->OnInit();
	m_businesses.push_back(business);
}
