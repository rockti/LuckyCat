#pragma once
#include "strategy/PositionManager.h"
namespace hk {
// 	class BalanceHandler {
// 	public:
// 		BalanceHandler(HKMarketingStrategy& parent) :
// 			m_parent(parent), m_arg(parent.GetArg()), m_business(parent.GetQuotingBusiness()),
// 			m_position(parent.GetPosition()), m_detailedPositions(parent.GetDetailedPositions()) {
// 		}
// 
// 		void Reset() {
// 			m_currentBalanceDealing = nullptr;
// 		}
// 
// 		Dealing* GetDealingPtr() const { return m_currentBalanceDealing; }
// 
// 		inline void tryBalance(Quote const& quote) {
// 			if (m_currentBalanceDealing != nullptr) return;
// 			int direct = m_detailedPositions.GetDirection();
// 			if (direct > 0) {
// 				tryBuy(quote);
// 			} else if (direct < 0) {
// 				trySell(quote);
// 			}
// 		}
// 
// 		void trySell(const Quote& quote) {
// 			TickT sellTick = priceToTick(quote.GetSell(), m_arg.UnitPrice);
// 			QtyT sellQuantity = QtyT(quote.GetSell().GetQuantity());
// 			auto [quoteHasChance, needPause] = hasSellQuoteChance(sellTick, sellQuantity);
// 			if (quoteHasChance) {
// 				Log::Info(fmt::format("Inst:[{}] PPause Sell Balance:[{} {} {} {}]", quote.GetInstCode().Str(),sellTick,sellQuantity,m_business.GetQuotingTicks().SellTick, m_business.GetQuotingQuantity()));
// 				if (needPause) {
// 					sellQuantity -= m_business.GetQuotingQuantity();
// 				}
// 				pos::TickPosition quote{ sellTick + m_parent.GetArg().SellGainTick, sellQuantity };
// 				pos::TickPosition balanceOrder = m_detailedPositions.SellChance(quote);
// 				if (balanceOrder.Quantity > 0) {
// 					if (needPause) {
// 						m_parent.BalancePause();
// 					}
// 					balanceOrder.Price = sellTick;
// 					sendSellBalance(balanceOrder);
// 				}
// 			}
// 		}
// 
// 		void tryBuy(const Quote& quote) {
// 			TickT buyTick = priceToTick(quote.GetBuy(), m_arg.UnitPrice);
// 			QtyT buyQuantity = QtyT(quote.GetBuy().GetQuantity());
// 			auto [quoteHasChance, needPause] = hasBuyQuoteChance(buyTick, buyQuantity);
// 			if (quoteHasChance) {
// 				Log::Info(fmt::format("Inst:[{}] PPause Buy Balance:[{} {} {} {}]", quote.GetInstCode().Str(), buyTick, buyQuantity, m_business.GetQuotingTicks().BuyTick, m_business.GetQuotingQuantity()));
// 				if (needPause) {
// 					buyQuantity -= m_business.GetQuotingQuantity();
// 				}
// 				pos::TickPosition position{ buyTick - m_parent.GetArg().BuyGainTick, buyQuantity };
// 				pos::TickPosition balanceOrder = m_detailedPositions.BuyChance(position);
// 				if (balanceOrder.Quantity > 0) {
// 					if (needPause) {
// 						m_parent.BalancePause();
// 					}
// 					balanceOrder.Price = buyTick;
// 					sendBuyBalance(balanceOrder);
// 				}
// 			}
// 		}
// 
// 
// 		inline std::string onBalanceTrade(Trade const& trade) {
// 			TickT price = TickT((trade.GetDealPrice() / m_arg.UnitPrice).value() + 0.1);
// 			std::string posInfo;
// 			if (IsBuy(trade.GetDealing().GetDirection())) {
// 				m_position.BalanceBuy(price, trade.GetDealQuantity());
// 				posInfo = m_detailedPositions.BuyTradeMatch(pos::TickPosition{ price, QtyT(trade.GetDealQuantity()) });
// 			} else {
// 				m_position.BalanceSell(price, trade.GetDealQuantity());
// 				posInfo = m_detailedPositions.SellTradeMatch(pos::TickPosition{ price, QtyT(trade.GetDealQuantity()) });
// 			}
// 			if (trade.GetDealing().IsFinished()) {
// 				m_currentBalanceDealing = nullptr;
// 				m_parent.BalanceStart();
// 			}
// 			return posInfo;
// 		}
// 
// 		inline void onBalanceCancel(Dealing const& dealing) {
// 			if (dealing.IsFinished()) {
// 				m_currentBalanceDealing = nullptr;
// 				m_parent.BalanceStart();
// 			}
// 		}
// 
// 
// 		inline std::pair<bool, bool> hasBuyQuoteChance(TickT buyTick, QtyT quantity) const {
// 			if (m_business.GetQuotingQuantity() != 0) {
// 				TickT quotingBuy = m_business.GetQuotingTicks().BuyTick;
// 				if (quotingBuy < buyTick) {
// 					return std::make_pair(true,false);
// 				}
// 				if (quotingBuy == buyTick && quantity > m_business.GetQuotingQuantity()) {
// 					return std::make_pair(true, true);
// 				}
// 				return std::make_pair(false, false);
// 			}
// 			return std::make_pair(true,false);
// 		}
// 
// 		inline std::pair<bool, bool> hasSellQuoteChance(TickT sellTick, QtyT quantity) const {
// 			if (m_business.GetQuotingQuantity() != 0) {
// 				TickT quotingSell = m_business.GetQuotingTicks().SellTick;
// 				if (quotingSell > sellTick) {
// 					return std::make_pair(true, false);
// 				}
// 				if (quotingSell == sellTick && quantity > m_business.GetQuotingQuantity()) {
// 					return std::make_pair(true, true);
// 				}
// 				return std::make_pair(false, false);
// 			}
// 			return std::make_pair(true, false);
// 		}
// 
// 		inline void sendBuyBalance(pos::TickPosition const& order) {
// 			OrderInfo orderInfo = m_parent.CreateOrder(m_arg.InstIDs.Major, SellFakUnknown, order.Quantity, order.Price * m_arg.UnitPrice);
// 			m_currentBalanceDealing = &m_parent.PushOrder(orderInfo);
// 			m_currentBalanceDealing->SetHookPtr(&m_parent);
// 		}
// 
// 
// 		inline void sendSellBalance(pos::TickPosition const& order) {
// 			OrderInfo orderInfo = m_parent.CreateOrder(m_arg.InstIDs.Major, BuyFakUnknown, order.Quantity, order.Price * m_arg.UnitPrice);
// 			m_currentBalanceDealing = &m_parent.PushOrder(orderInfo);
// 			m_currentBalanceDealing->SetHookPtr(&m_parent);
// 		}
// 
// 	private:
// 		HKMarketingStrategy& m_parent;
// 		Arguments const& m_arg;
// 		QuotingBusiness& m_business;
// 		Position& m_position;
// 		pos::TickPositions<>& m_detailedPositions;
// 		Dealing* m_currentBalanceDealing = nullptr;
// 	};
}