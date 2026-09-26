#include "pch.h"
#include "MCTStrategy.h"
#include "utils/Log.h"
#include "notify/Notify.h"
#include "service/PositionService.h"
#include "model/factory/InstrumentFactory.h"

namespace mct {

	void MCTStrategy::DoOnInit() {
		Log::Info(fmt::format("MCTBusiness[{}] is added", GetId()));
	}
	void MCTStrategy::DoOnQuote(Quote const& market) {
		UNREFERENCED_PARAMETER(market);
	}
	void MCTStrategy::DoOnCommand(StrategyCommand const& command) {
		if (command.Type == StrategyCommand::MANUAL_COMMAND) {
			MCTCommand const& cmd = *(MCTCommand*)command.Data;
			if (cmd.Type == MCTCommand::CommandType::Order) {
				MCTOrder const& order = cmd.mctOrder;
				Order(order.InstID, order.OrderCommand, order.Quantity, order.Price);
			} else {
				Action(cmd.mctCancel.actionCommand);
			}
		}
	}

	void MCTStrategy::DoOnOrdered(Dealing const& dealing) {
		
	}
	void MCTStrategy::DoOnCancelled(Dealing const& dealing) {

	}
	void MCTStrategy::DoOnQuantityTrade(Dealing const& dealing, int quantity) {

	}
	void MCTStrategy::DoOnOther(Dealing const& dealing) {

	}
	void MCTStrategy::DoOnTrade(Trade const& trade) {

	}
	void MCTStrategy::DoOnExchangeStatus(Exchange exchange, ExchangeStatus status) {
		UNREFERENCED_PARAMETER(exchange);
		UNREFERENCED_PARAMETER(status);
	}

	void MCTStrategy::DoOnConnectStatus(Exchange exchange, ConnectStatus status) {
		UNREFERENCED_PARAMETER(exchange);
		UNREFERENCED_PARAMETER(status);
	}

	const int INDEX_AMT = 20;
	const int RATE_AMT = 50;
	void MCTStrategy::Order(InstID instID, OrderCommand const& order, int quantity, money price) {
		Instrument instrument = InstrumentFactory::GetInstruments().Get(instID);
		// 香港品种的平仓复用为双边委托
		auto& account = pos::GlobalPositionStore::Instance().Account();
		if (Exchange::HKEX == instrument.GetExchange() && Operation::OFFSET == order.Operation) {
			money unitPrice = instrument.GetUnitPrice();
			money freezeAmt = instrument.CalculateTotal(price*2, quantity*2, Operation::OPEN);
			if (account.Enough(freezeAmt)) {
				if (Direction::BUY == order.Direction) {
					money sellPrice = price + 3 * unitPrice;
					PushMarketingOrder(CreateMarketingOrder(instID, quantity, price, sellPrice));
				} else {
					money buyPrice = price - 3 * unitPrice;
					PushMarketingOrder(CreateMarketingOrder(instID, quantity, buyPrice, price));
				}
			} else {
				m_publisher.NotifyUI(fmt::format("MCTStrategy:lack of funds[{:.3f}], order failed", freezeAmt.value()));
			}
		}
		else {
			money freezeAmt = instrument.CalculateTotal(price, quantity, order.Operation);
			if (account.Enough(freezeAmt)) {
				TSGEProductClassType productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
				if (0 == instrument.GetVariety().GetCode().compare("T") || 0 == instrument.GetVariety().GetCode().compare("TF") ||
					0 == instrument.GetVariety().GetCode().compare("TL") || 0 == instrument.GetVariety().GetCode().compare("TS")) {
					while (quantity > RATE_AMT) {
						PushOrder(CreateOrder(instID, order, RATE_AMT, price, productClass));
						quantity -= RATE_AMT;
					}
				}	// 国债期货委托超过50手拆单
				else if (0 == instrument.GetVariety().GetCode().compare("IC") || 0 == instrument.GetVariety().GetCode().compare("IF") ||
					0 == instrument.GetVariety().GetCode().compare("IH")) {
					while (quantity > INDEX_AMT) {
						PushOrder(CreateOrder(instID, order, INDEX_AMT, price, productClass));
						quantity -= INDEX_AMT;
					}
				}	// 股指期货委托超过20手拆单
				PushOrder(CreateOrder(instID, order, quantity, price, productClass));
			} else {
				m_publisher.NotifyUI(fmt::format("MCTStrategy:lack of funds[{:.3f}], order failed", freezeAmt.value()));
			}
		}
	}
	void MCTStrategy::Action(OrderCancel const& action) {
		PushAction(action);
	}
}

