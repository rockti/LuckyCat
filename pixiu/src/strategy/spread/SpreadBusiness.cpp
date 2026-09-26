#include "pch.h"
#include "SpreadBusiness.h"
#include "utils/Log.h"

namespace hf::spread {
	SpreadBusiness::SpreadBusiness(SpreadStrategy* parent)
		:m_parent(parent), m_spreadType(parent->GetArg().Type), m_spreadDir(parent->GetArg().Dir)
		,m_machine(this), m_portfolio(parent->GetArg().Instruments, m_spreadDir, parent->GetArg().Multiple)		
		,m_orders(parent->GetArg().Instruments, m_spreadDir), m_multiple(parent->GetArg().Multiple), m_majorPendingPrice() {
			
		m_id = m_parent->GetId();
		m_currentPhase = m_machine.GetPhase(Status::End);
		m_portfolio.InitPortfolio(parent->GetArg().OfrPosition/*负值表示上海今仓*/);	

		InstID minorID = m_parent->GetArg().Instruments.Minor;
		Instrument const& minorInst = InstrumentFactory::GetInstruments().Get(minorID);
		m_minorExchange = InstIdExchange(minorID);
		m_minorUnitPrice = minorInst.GetUnitPrice();
		InstID majorID = m_parent->GetArg().Instruments.Major;
		Instrument const& majorInst = InstrumentFactory::GetInstruments().Get(majorID);
		m_majorExchange = InstIdExchange(majorID);
		m_majorUnitPrice = majorInst.GetUnitPrice();
		m_isUnitPriceEqual = (m_minorUnitPrice == m_majorUnitPrice);		
		m_isDeepthQuote = IsSGE(minorID);
	}

	void SpreadBusiness::OnStart() {
		if (Status::End == m_currentPhase->m_status) {
			m_currentPhase = m_machine.GetPhase(Status::Start);
		}
	}
	void SpreadBusiness::OnPaused() {		
		m_currentPhase = m_currentPhase->OnCommand();
	}
	void SpreadBusiness::OnQuote(Quote const& quote) {
		m_currentPhase = m_currentPhase->OnQuote(quote);
	}
	void SpreadBusiness::OnMinorCancel(int quantity) {
		m_minorLeg.EndTag = true;
		m_currentPhase = m_currentPhase->OnMinorCancel(quantity);
	}
	void SpreadBusiness::OnMajorCancel(int quantity) {
		m_majorLeg.EndTag = true;
		m_currentPhase = m_currentPhase->OnMajorCancel(quantity);
	}
	void SpreadBusiness::OnMinorTrade(Dealing const& dealing, int quantity) {
		m_minorLeg.EndTag = dealing.GetRemainQuantity() ? false : true;
		m_currentPhase = m_currentPhase->OnMinorTrade(dealing, quantity);
	}
	void SpreadBusiness::OnMajorTrade(Dealing const& dealing, int quantity) {
		m_majorLeg.EndTag = dealing.GetRemainQuantity() ? false : true;
		m_currentPhase = m_currentPhase->OnMajorTrade(dealing, quantity);
	}
	void SpreadBusiness::OnMinorFailed(Dealing const& dealing) {
		m_minorLeg.EndTag = true;
		m_currentPhase = m_currentPhase->OnMinorInsertFailed(dealing);
	}
	void SpreadBusiness::OnMajorFailed(Dealing const& dealing) {
		m_majorLeg.EndTag = true;
		m_currentPhase = m_currentPhase->OnMajorInsertFailed(dealing);
	}
	
	void SpreadBusiness::InitTags() {
		m_YDOffsetTag = false;
		m_offsetForbiddenTag = false;
		m_portfolio.InitCounter();
	}
	void SpreadBusiness::UpdateBand(bool isOpen) {
		money adjustSpread = m_parent->GetArg().MinQty * m_minorUnitPrice;

		if (Direction::SELL == m_spreadDir) {
			m_parent->m_arg.OfrOpen = isOpen ? m_parent->m_arg.OfrOpen + adjustSpread : m_parent->m_arg.OfrOpen - adjustSpread;
			std::string msg = fmt::format("Business[{}] UpdateBullOpen to [{:.3f}]", m_id, m_parent->GetArg().OfrOpen);
			m_parent->m_publisher.NotifyUI(msg);
			Log::Info(msg.c_str());
		}
		else {
			m_parent->m_arg.BidOpen = isOpen ? m_parent->m_arg.BidOpen - adjustSpread : m_parent->m_arg.BidOpen + adjustSpread;
			std::string msg = fmt::format("Business[{}] UpdateBearOpen to [{:.3f}]", m_id, m_parent->GetArg().BidOpen);
			m_parent->m_publisher.NotifyUI(msg);
			Log::Info(msg.c_str());
		}
	}
	bool SpreadBusiness::BalanceCheck(Direction dir, Operation opt, OrderType orderType, bool isMinorDealing) {
		int balanceQty = m_portfolio.GetBalancingQty();		// MinorCnt * Multiple - MajorCnt - MajorPending
		QtyT majorPending = m_portfolio.GetMajorPending();
		if (0 == majorPending) {
			if (balanceQty > 0) {
				InstID majorID = m_orders.Instruments.Major;
				Direction majorDir = isMinorDealing ? OppositeDirection(dir) : dir;
				Quote const& majorQuote = WritableQuoteService().GetQuote(majorID);
				money sellPrice = majorQuote.GetSell().GetPrice();
				money buyPrice = majorQuote.GetBuy().GetPrice();

				money majorPrice = (Direction::SELL == majorDir) ? buyPrice - m_majorUnitPrice * 2 : sellPrice + m_majorUnitPrice * 2;
				if (OrderType::FAK == orderType) {
					if (Operation::OPEN == opt && false == m_offsetForbiddenTag/*防止是平仓反开*/) {
						openMajor(majorPrice, balanceQty, true);
					}
					else {
						offsetMajor(majorPrice, balanceQty, true);
					}
				}	// FAK配平Major，用盘口价
				else {
					majorPrice = (Direction::SELL == majorDir) ? sellPrice - m_majorUnitPrice * 10 : buyPrice + m_majorUnitPrice * 10;
					if (Operation::OPEN == opt && false == m_offsetForbiddenTag) {
						openMajor(majorPrice, balanceQty, false);			// Normal-Minor开仓多，补开Normal-Major
					}
					else {
						offsetMajor(majorPrice, balanceQty, false);			// Normal-Minor平仓多，补平Normal-Major
					}
				}	// 挂单成交配平Major，直接调价10档
				Log::Info(fmt::format("SpreadBusiness[{}]: DoBalance SendMajor[{} {}: {:.3f}, {}, {}]", m_id, Desc(majorDir), Desc(opt), majorPrice, balanceQty, Desc(orderType)));
				return false;
			}	// Minor 成交多，补发Major
			else if (balanceQty < 0) {
				InstID minorID = m_orders.Instruments.Minor;
				Quote const& minorQuote = WritableQuoteService().GetQuote(minorID);
				Direction minorDir = isMinorDealing ? dir : OppositeDirection(dir);				
				money minorPrice = (Direction::SELL == minorDir) ? minorQuote.GetBuy().GetPrice() - m_minorUnitPrice * 2 : minorQuote.GetSell().GetPrice() + m_minorUnitPrice * 2;
				int minorQty = balanceQty / m_multiple;
				if (Operation::OPEN == opt && false == m_offsetForbiddenTag/*防止是平仓反开*/) {
					openMinor(minorPrice, abs(minorQty), true);				// FAK-Major开仓多，补开FAK-Minor
				}
				else {
					offsetMinor(minorPrice, abs(minorQty), true);			// FAK-Major平仓多，补平FAK-Minor
				}
				Log::Info(fmt::format("SpreadBusiness[{}]: Dobalance SendMinor[{} {}: {:.3f}, {}, {}]", m_id, Desc(minorDir), Desc(opt), minorPrice, balanceQty, Desc(orderType)));
				return false;
			}	// Major 成交多，补发Minor（只可能是FAK）
			else {
				Log::Info(fmt::format("SpreadBusiness[{}]: IsBalanced, Portfolio[{},{}(YD)] Pending[{}]", m_id, m_portfolio.GetMinorVolume(), m_portfolio.GetYDMinorVolume(), majorPending));
				return true;
			}
		}	// Major 没有挂单
		else {
			if (0 != balanceQty) {
				cancelMajor();
				Log::Info(fmt::format("SpreadBusiness[{}]: Dobalance CancelMajor, BalanceQty[{}] MajorPending[{}]", m_id, balanceQty, majorPending));
				return false;
			}
			else {
				Log::Info(fmt::format("SpreadBusiness[{}]: Dobalance WaittingMajor, MajorPending[{}]", m_id, majorPending));
				return false;
			}
		}	// Major 有挂单
	}

	int SpreadBusiness::TryOpen() {
		QtyT openablePos = m_parent->GetArg().MaxPosition - m_portfolio.GetTotalMinorVolume();			
		if (openablePos <= 0) {
			return 0;
		}	// 持仓已满，不得开仓

		QtyT minorOrderQty = 0;
		money minorPrice = money();
		money majorPrice = money();		
		if (SpreadType::Spread == m_spreadType) {
			QtyT maxVolume = std::min(m_parent->GetArg().MaxQty/*FAK量*/, openablePos/*剩余可开量*/);
			if (CanMarketOpen(maxVolume/*最大可开*/, majorPrice, minorPrice, minorOrderQty/*实际可开*/)) {
				openMajor(majorPrice, minorOrderQty * m_multiple, true);
				openMinor(minorPrice, minorOrderQty, true);
				return 2;
			}

			maxVolume = std::min(m_parent->GetArg().MinQty/*挂单量*/, openablePos);
			if (CanNormalOpen(maxVolume/*最大可开*/, majorPrice, minorPrice, minorOrderQty/*实际可开*/)) {
				openMinor(minorPrice, minorOrderQty, false);
				m_majorPendingPrice = majorPrice;		// 记下对应的MajorPrice（可能是均价）
				return 1;
			}
			else {
				m_majorPendingPrice = money();
			}
		}
		return 0;
	}
	int SpreadBusiness::TryOffset() {
		QtyT offsetablePos = m_portfolio.GetMinorVolume();
		if (offsetablePos <= 0) {
			offsetablePos = m_portfolio.GetYDMinorVolume();	// 没有今仓再看看老仓
			if (offsetablePos <= 0) {
				return 0;
			}
			else {
				if (IsSHFE(m_minorExchange) || IsINE(m_minorExchange)) {
					m_YDOffsetTag = true;	// SHFE或INE平老仓
				}
			}
		}

		QtyT minorOrderQty = 0;		
		money minorPrice = money();
		money majorPrice = money();
		if (SpreadType::Spread == m_spreadType) {
			QtyT maxVolume = std::min(m_parent->GetArg().MaxQty/*FAK量*/, offsetablePos/*剩余可平量*/);
			if (CanMarketOffset(maxVolume/*最大可平*/, majorPrice, minorPrice, minorOrderQty/*实际可平*/)) {
				m_offsetForbiddenTag = m_portfolio.IsSpreadOpenOnly();				
				offsetMajor(majorPrice, minorOrderQty * m_multiple, true);
				offsetMinor(minorPrice, minorOrderQty, true);
				return 2;
			}

			maxVolume = std::min(m_parent->GetArg().MinQty/*挂单量*/, offsetablePos);
			if (CanNormalOffset(maxVolume/*最大可平*/, majorPrice, minorPrice, minorOrderQty/*实际可平*/)) {
				m_offsetForbiddenTag = m_portfolio.IsSpreadOpenOnly();
				offsetMinor(minorPrice, minorOrderQty, false);
				m_majorPendingPrice = majorPrice;		// 记下对应的MajorPrice（可能是均价）
				return 1;
			}
			else {
				m_majorPendingPrice = money();
			}
		}
		return 0;
	}

	int const PENDING_COLUMN = 1;
	bool SpreadBusiness::IsMissPendingChance() {
		Direction minorDir = m_minorLeg.LegDealing->GetDirection();
		money minorPrice = m_minorLeg.LegDealing->GetOrderPrice();
		QtyT minorQty = m_minorLeg.LegDealing->GetOrderQuantity();

		if (SpreadType::Spread == m_spreadType) {
			Direction openMinorDir = m_portfolio.GetSPInstruments().IsMajorFarther() ? OppositeDirection(m_spreadDir) : m_spreadDir;			
			money majorPrice = money();
			QtyT majorVolume = 0;
			
			// 价差不符合或者Major盘口量不足，撤单：注意要实时更新Major均价
			if (Direction::SELL == m_spreadDir) {
				if (minorDir == openMinorDir) {
					money openDelta = m_portfolio.GetSPInstruments().GetSellDeltaByMinorPrice(minorPrice, m_multiple, minorQty * m_multiple/*需求*/, majorPrice, majorVolume/*可用*/);
					if (openDelta < m_parent->GetArg().OfrOpen || majorVolume < minorQty * m_multiple * 2/*小于2倍*/) {
// 						Instruments instruments = InstrumentFactory::GetInstruments();
// 						auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 						auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 						Log::Info(fmt::format("Inst[{}-{}] Miss SellOpenChance, Delta[{:.3f}] < UpLine[{:.3f}] or MajorVolume[{}] < MajorQty[{}]",
// 							fartherCode.Str(), closerCode.Str(), openDelta, m_parent->GetArg().OfrOpen, majorVolume, minorQty * m_multiple));
						return true;
					}
				}	// SellOpen
				else {
					money offsetDelta = m_portfolio.GetSPInstruments().GetBuyDeltaByMinorPrice(minorPrice, m_multiple, minorQty * m_multiple/*需求*/, majorPrice, majorVolume/*可用*/);
					if (offsetDelta > m_parent->GetArg().OfrOffset || majorVolume < minorQty * m_multiple * 2/*小于2倍*/) {
// 						Instruments instruments = InstrumentFactory::GetInstruments();
// 						auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 						auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 						Log::Info(fmt::format("Inst[{}-{}] Miss SellOffsetChance, Delta[{:.3f}] > DnLine[{:.3f}] or MajorVolume[{}] < MajorQty[{}]",
// 							fartherCode.Str(), closerCode.Str(), offsetDelta, m_parent->GetArg().OfrOffset, majorVolume, minorQty * m_multiple));
						return true;
					}
				}	// SellOffset	
			}
			else {
				if (minorDir == openMinorDir) {
					money openDelta = m_portfolio.GetSPInstruments().GetBuyDeltaByMinorPrice(minorPrice, m_multiple, minorQty * m_multiple, majorPrice, majorVolume);
					if (openDelta > m_parent->GetArg().BidOpen || majorVolume < minorQty * m_multiple * 2/*小于2倍*/) {
// 						Instruments instruments = InstrumentFactory::GetInstruments();
// 						auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 						auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 						Log::Info(fmt::format("Inst[{}-{}] Miss BuyOpenDelta, Delta[{:.3f}] > DnLine[{:.3f}] or MajorVolume[{}] < MajorQty[{}]",
// 							fartherCode.Str(), closerCode.Str(), openDelta, m_parent->GetArg().BidOpen, majorVolume, minorQty * m_multiple));
						return true;
					}
				}	// BuyOpen
				else {
					money offsetDelta = m_portfolio.GetSPInstruments().GetSellDeltaByMinorPrice(minorPrice, m_multiple, minorQty * m_multiple, majorPrice, majorVolume);
					if (offsetDelta < m_parent->GetArg().BidOffset || majorVolume < minorQty * m_multiple * 2/*小于2倍*/) {
// 						Instruments instruments = InstrumentFactory::GetInstruments();
// 						auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 						auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 						Log::Info(fmt::format("Inst[{}-{}] Miss BuyOffsetDelta, Delta[{:.3f}] < UpLine[{:.3f}] or MajorVolume[{}] < MajorQty[{}]",
// 							fartherCode.Str(), closerCode.Str(), offsetDelta, m_parent->GetArg().BidOffset, majorVolume, minorQty * m_multiple));
						return true;
					}
				}	// BuyOffset
			}
			m_majorPendingPrice = majorPrice;

			// 挂单价格超出两档或者2跳外，撤单
			if (!m_portfolio.GetSPInstruments().IsPriceFit(m_minorLeg.LegDealing->GetInstID(), minorDir, minorPrice, m_isDeepthQuote, PENDING_COLUMN)) {
				Instruments instruments = InstrumentFactory::GetInstruments();
				auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
				auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
				Quote const& minorQuote = WritableQuoteService().GetQuote(m_minorLeg.LegDealing->GetInstID());
				money quotePrice = (Direction::SELL == minorDir) ? minorQuote.GetSell().GetPrice() : minorQuote.GetBuy().GetPrice();
				money quote2Price = (Direction::SELL == minorDir) ? minorQuote.GetSell(1).GetPrice() : minorQuote.GetBuy(1).GetPrice();
// 				Log::Info(fmt::format("Inst[{}-{}] PendingPrice missed, MinorOrder[{} {:.3f} {}] QuotePrice[{:.3f} {:.3f}] isDeep[{}]", fartherCode.Str(),
// 					closerCode.Str(), Desc(minorDir), minorPrice, minorQty, quotePrice, quote2Price, m_isDeepthQuote));
				return true;
			}
		}
		return false;
	}
	bool SpreadBusiness::CanMarketOpen(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty) {
		if (SpreadType::Spread == m_spreadType) {
			QtyT minorVolume = 0;
			QtyT majorVolume = 0;
			if (Direction::SELL == m_spreadDir) {
				money delta = m_portfolio.GetSPInstruments().GetMarketSellDelta(m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/, minorPrice, minorVolume/*可用*/);
				majorVolume = majorVolume / m_multiple;
				minorOrderQty = std::min(maxVolume/*最大量*/, std::min(minorVolume, majorVolume)/*盘口量*/);

				if (minorOrderQty > 0 && delta >= m_parent->GetArg().OfrOpen) {
// 					Instruments instruments = InstrumentFactory::GetInstruments();
// 					auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 					auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 					Log::Info(fmt::format("Inst[{}-{}] MarketDelta[{:.3f}] >= BullOpenLine[{:.3f}] can FAK-SOpen, OrderQty[{}]"
// 						, fartherCode.Str(), closerCode.Str(), delta, m_parent->GetArg().OfrOpen, minorOrderQty));
					return true;
				}	// delta高于OfrOpenLine
			}
			else {
				money delta = m_portfolio.GetSPInstruments().GetMarketBuyDelta(m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/, minorPrice, minorVolume/*可用*/);
				majorVolume = majorVolume / m_multiple;
				minorOrderQty = std::min(maxVolume/*最大量*/, std::min(minorVolume, majorVolume)/*盘口量*/);

				if (minorOrderQty > 0 && delta <= m_parent->GetArg().BidOpen) {
// 					Instruments instruments = InstrumentFactory::GetInstruments();
// 					auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 					auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 					Log::Info(fmt::format("Inst[{}-{}] MarketDelta[{:.3f}] <= BearOpenLine[{:.3f}] can FAK-BOpen, OrderQty[{}]"
// 						, fartherCode.Str(), closerCode.Str(), delta, m_parent->GetArg().BidOpen, minorOrderQty));
					return true;
				}	// delta低于BidOpenLine			
			}
		}
		return false;
	}
	bool SpreadBusiness::CanMarketOffset(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty) {
		if (SpreadType::Spread == m_spreadType) {
			QtyT minorVolume = 0;
			QtyT majorVolume = 0;
			if (Direction::SELL == m_spreadDir) {
				money delta = m_portfolio.GetSPInstruments().GetMarketBuyDelta(m_multiple, maxVolume/*需求*/, majorPrice, majorVolume, minorPrice, minorVolume);
				majorVolume = majorVolume / m_multiple;
				minorOrderQty = std::min(maxVolume/*最大量*/, std::min(minorVolume, majorVolume)/*盘口量*/);

				if (minorOrderQty > 0 && delta <= m_parent->GetArg().OfrOffset) {
// 					Instruments instruments = InstrumentFactory::GetInstruments();
// 					auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 					auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 					Log::Info(fmt::format("Inst[{}-{}] MarketDelta[{:.3f}] <= BullOffsetLine[{:.3f}] can FAK-SOffset, OrderQty[{}]"
// 						, fartherCode.Str(), closerCode.Str(), delta, m_parent->GetArg().OfrOffset, minorOrderQty));
					return true;
				}	// delta低于OfrOffsetLine
			}
			else {
				money delta = m_portfolio.GetSPInstruments().GetMarketSellDelta(m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/, minorPrice, minorVolume/*可用*/);
				majorVolume = majorVolume / m_multiple;
				minorOrderQty = std::min(maxVolume/*最大量*/, std::min(minorVolume, majorVolume)/*盘口量*/);

				if (minorOrderQty > 0 && delta >= m_parent->GetArg().BidOffset) {
// 					Instruments instruments = InstrumentFactory::GetInstruments();
// 					auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 					auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 					Log::Info(fmt::format("Inst[{}-{}] MarketDelta[{:.3f}] >= BearOffsetLine[{:.3f}] can FAK-BOffset, OrderQty[{}]"
// 						, fartherCode.Str(), closerCode.Str(), delta, m_parent->GetArg().BidOffset, minorOrderQty));
					return true;
				}	// delta有效且高于BidOffsetLine
			}
		}
		return false;
	}
	bool SpreadBusiness::CanNormalOpen(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty) {
		if (SpreadType::Spread == m_spreadType) {
			QtyT majorVolume = 0;
			if (Direction::SELL == m_spreadDir) {
				minorPrice = m_portfolio.GetSPInstruments().GetMinorPriceByUpperLine(m_parent->GetArg().OfrOpen, m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/);
			}
			else {
				minorPrice = m_portfolio.GetSPInstruments().GetMinorPriceByLowerLine(m_parent->GetArg().BidOpen, m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/);
			}
			majorVolume = majorVolume / m_multiple;
			minorOrderQty = std::min(maxVolume, majorVolume);
			if (minorOrderQty <= 0) {
				return false;
			}

			// 确保价格不优于0档
			Quote const& quote = WritableQuoteService().GetQuote(m_orders.Instruments.Minor);
			if (Direction::SELL == m_orders.OpenOrders.MinorCmd.Direction) {
				minorPrice = std::max(quote.GetSell().GetPrice() - m_minorUnitPrice * 2, minorPrice);	// 卖单不优于卖0
			}
			else {
				minorPrice = std::min(quote.GetBuy().GetPrice() + m_minorUnitPrice * 2, minorPrice);	// 买单不优于买0
			}

			if (m_portfolio.GetSPInstruments().IsPriceFit(m_orders.Instruments.Minor, m_orders.OpenOrders.MinorCmd.Direction, minorPrice, false, 0)/*位于第一档*/) {
// 				Quote const& majorQuote = WritableQuoteService().GetQuote(m_orders.Instruments.Major);
// 				Instruments instruments = InstrumentFactory::GetInstruments();
// 				auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 				auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 
// 				Log::Info(fmt::format("Inst[{}-{}] Minor[{:.3f}, {:.3f}], can open pending [{}: {:.3f}, {}], Major[{:.3f},{}, {:.3f},{}] majorVolume[{}]"
// 					, fartherCode.Str(), closerCode.Str(), quote.GetSell().GetPrice(), quote.GetBuy().GetPrice(), Desc(m_orders.OpenOrders.MinorCmd.Direction)
// 					, minorPrice, minorOrderQty, majorQuote.GetSell().GetPrice(), majorQuote.GetSell().GetQuantity()
// 					, majorQuote.GetBuy().GetPrice(), majorQuote.GetBuy().GetQuantity(), majorVolume));
				return true;
			}
		}
		return false;
	}
	bool SpreadBusiness::CanNormalOffset(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty) {
		if (SpreadType::Spread == m_spreadType) {
			QtyT majorVolume = 0;
			if (Direction::SELL == m_spreadDir) {
				minorPrice = m_portfolio.GetSPInstruments().GetMinorPriceByLowerLine(m_parent->GetArg().OfrOffset, m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/);
			}
			else {
				minorPrice = m_portfolio.GetSPInstruments().GetMinorPriceByUpperLine(m_parent->GetArg().BidOffset, m_multiple, maxVolume/*需求*/, majorPrice, majorVolume/*可用*/);
			}
			majorVolume = majorVolume / m_multiple;
			minorOrderQty = std::min(maxVolume, majorVolume);
			if (minorOrderQty <= 0) {
				return false;
			}

			// 确保价格不优于0档
			Quote const& quote = WritableQuoteService().GetQuote(m_orders.Instruments.Minor);
			if (Direction::SELL == m_orders.OffsetOrders.MinorCmd.Direction) {
				minorPrice = std::max(quote.GetSell().GetPrice() - m_minorUnitPrice * 2, minorPrice);	// 卖单最低发T0
			}
			else {
				minorPrice = std::min(quote.GetBuy().GetPrice() + m_minorUnitPrice * 2, minorPrice);	// 买单最高发T0
			}

			if (m_portfolio.GetSPInstruments().IsPriceFit(m_orders.Instruments.Minor, m_orders.OffsetOrders.MinorCmd.Direction, minorPrice, false, 0)/*位于第一档*/) {
// 				Quote const& majorQuote = WritableQuoteService().GetQuote(m_orders.Instruments.Major);
// 				Instruments instruments = InstrumentFactory::GetInstruments();
// 				auto fartherCode(instruments.Get(m_orders.Instruments.Farther).GetInstCode());
// 				auto closerCode(instruments.Get(m_orders.Instruments.Closer).GetInstCode());
// 				Quote const& quote = WritableQuoteService().GetQuote(m_orders.Instruments.Minor);
// 				Log::Info(fmt::format("Inst[{}-{}] Minor[{:.3f} {:.3f}], can offset pending [{}: {:.3f}, {}] Major[{:.3f},{}, {:.3f}{}] majorVolume[{}]"
// 					, fartherCode.Str(), closerCode.Str(), quote.GetSell().GetPrice(), quote.GetBuy().GetPrice(), Desc(m_orders.OffsetOrders.MinorCmd.Direction)
// 					, minorPrice, minorOrderQty, majorQuote.GetSell().GetPrice(), majorQuote.GetSell().GetQuantity()
// 					, majorQuote.GetBuy().GetPrice(), majorQuote.GetBuy().GetQuantity(), majorVolume));
				return true;
			}
		}
		return false;
	}

	void SpreadBusiness::openMinor(money price, QtyT qty, bool isFak) {
		// 如果是平仓惩罚进行反开的时候，校验可平量永远是True，只能在这里进行一下限制	
		int currentOpenable = m_parent->GetArg().MaxPosition/*最大可开*/ - m_portfolio.GetTotalMinorVolume()/*已开*/;
		if (currentOpenable <= 0) {
			Log::Info(fmt::format("Minor position[{}] is overmuch[{}], prevent openMinor", m_portfolio.GetTotalMinorVolume(), currentOpenable));
			return;
		}
		if (qty > currentOpenable) {
			Log::Info(fmt::format("Minor position[{}] is insufficient, change qty[{}]->[{}]", m_portfolio.GetTotalMinorVolume(), qty, currentOpenable));
			qty = currentOpenable;
		}

		InstID minorID = m_parent->GetArg().Instruments.Minor;
		if (false == m_isUnitPriceEqual) {			
			price = tickToPrice(priceToTick(price, m_minorUnitPrice), m_minorUnitPrice);
		}	// 两个合约最小报价单位不同，要转换一下

		TSGEProductClassType productClass = SGE_OC_Unknown;
		if (Exchange::SGE == m_minorExchange) {
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(minorID);
			productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		}

		auto orderInfo = m_parent->CreateOrder(minorID, m_orders.OpenOrders.MinorCmd, qty, price, productClass);
		orderInfo.OrderType = isFak ? OrderType::FAK : OrderType::NORMAL;
		m_minorLeg.LegDealing = m_parent->PushOrderWithHook(orderInfo);
		m_minorLeg.RtnTag = false;
		m_minorLeg.EndTag = false;
// 		m_pricePosition.FreezeFee();	// 只冻结手续费
	}
	void SpreadBusiness::openMajor(money price, QtyT qty, bool isFak) {
		// 如果是平仓惩罚进行反开的时候，校验可平量永远是True，只能在这里进行一下限制	
		int currentOpenable = m_parent->GetArg().MaxPosition * m_multiple - m_portfolio.GetTotalMajorVolume();
		if (currentOpenable <= 0) {
			Log::Info(fmt::format("Major position[{}] is overmuch[{}], prevent openMinor", m_portfolio.GetTotalMajorVolume(), currentOpenable));
			return;
		}
		if (qty > currentOpenable) {
			Log::Info(fmt::format("Major position[{}] is insufficient, change qty[{}]->[{}]", m_portfolio.GetTotalMajorVolume(), qty, currentOpenable));
			qty = currentOpenable;
		}

		InstID majorID = m_parent->GetArg().Instruments.Major;
		TSGEProductClassType productClass = SGE_OC_Unknown;
		if (Exchange::SGE == m_majorExchange) {
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(majorID);
			productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		}
		auto orderInfo = m_parent->CreateOrder(majorID, m_orders.OpenOrders.MajorCmd, qty, price, productClass);
		orderInfo.OrderType = isFak ? OrderType::FAK : OrderType::NORMAL;
		m_majorLeg.LegDealing = m_parent->PushOrderWithHook(orderInfo);
		m_majorLeg.RtnTag = false;
		m_majorLeg.EndTag = false;
// 		m_pricePosition.FreezeFee();	// 只冻结手续费
	}
	void SpreadBusiness::offsetMinor(money price, QtyT qty, bool isFak) {
		InstID minorID = m_parent->GetArg().Instruments.Minor;
		if (false == m_isUnitPriceEqual) {			
			price = tickToPrice(priceToTick(price, m_minorUnitPrice), m_minorUnitPrice);
		}	// 两个合约最小报价单位不同，要转换一下

		TSGEProductClassType productClass = SGE_OC_Unknown;
		if (Exchange::SGE == m_minorExchange) {
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(minorID);
			productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		}
		auto orderInfo = m_parent->CreateOrder(minorID, m_orders.OffsetOrders.MinorCmd, qty, price, productClass);
		orderInfo.OrderType = isFak ? OrderType::FAK : OrderType::NORMAL;
		if (m_offsetForbiddenTag) {
			orderInfo.Operation = Operation::OPEN;
		}
		else if (Exchange::SHFE == m_minorExchange || Exchange::INE == m_minorExchange && false == m_YDOffsetTag) {
			orderInfo.Operation = Operation::OFFSET_TODAY;
		}			

		m_minorLeg.LegDealing = m_parent->PushOrderWithHook(orderInfo);
		m_minorLeg.RtnTag = false;
		m_minorLeg.EndTag = false;
// 		m_pricePosition.FreezeFee();	// 只冻结手续费
	}
	void SpreadBusiness::offsetMajor(money price, QtyT qty, bool isFak) {
		InstID majorID = m_parent->GetArg().Instruments.Major;

		TSGEProductClassType productClass = SGE_OC_Unknown;
		if (Exchange::SGE == m_majorExchange) {
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(majorID);
			productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		}
		auto orderInfo = m_parent->CreateOrder(majorID, m_orders.OffsetOrders.MajorCmd, qty, price, productClass);
		orderInfo.OrderType = isFak ? OrderType::FAK : OrderType::NORMAL;
		if (m_offsetForbiddenTag) {
			orderInfo.Operation = Operation::OPEN;
		}
		else if (Exchange::SHFE == m_majorExchange || Exchange::INE == m_majorExchange && false == m_YDOffsetTag) {
			orderInfo.Operation = Operation::OFFSET_TODAY;
		}

		m_majorLeg.LegDealing = m_parent->PushOrderWithHook(orderInfo);
		m_majorLeg.RtnTag = false;
		m_majorLeg.EndTag = false;
// 		m_pricePosition.FreezeFee();	// 只冻结手续费
	}

	void SpreadBusiness::balanceFAKOpenMinor(Direction minorDir, money price, QtyT qty) {
		InstID minorID = m_parent->GetArg().Instruments.Minor;
		OrderCommand balanceCmd = (Direction::SELL == minorDir) ? SellOpenFAk : BuyOpenFAK;
		TSGEProductClassType productClass = SGE_OC_Unknown;
		if (Exchange::SGE == m_minorExchange) {
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(minorID);
			productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		}
		auto orderInfo = m_parent->CreateOrder(minorID, balanceCmd, qty, price, productClass);
		m_minorLeg.LegDealing = m_parent->PushOrderWithHook(orderInfo);
		m_minorLeg.RtnTag = false;
	}
	void SpreadBusiness::balanceFAKOpenMajor(Direction majorDir, money price, QtyT qty) {
		InstID majorID = m_parent->GetArg().Instruments.Major;
		OrderCommand balanceCmd = (Direction::SELL == majorDir) ? SellOpenFAk : BuyOpenFAK;
		TSGEProductClassType productClass = SGE_OC_Unknown;
		if (Exchange::SGE == m_majorExchange) {
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(majorID);
			productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
		}
		auto orderInfo = m_parent->CreateOrder(majorID, balanceCmd, qty, price, productClass);
		m_majorLeg.LegDealing = m_parent->PushOrderWithHook(orderInfo);
		m_majorLeg.RtnTag = false;
	}

	void SpreadBusiness::cancelMinor() {
		if (0 != m_minorLeg.LegDealing->GetSystemNo().Length()) {
			InstID minorID = m_parent->GetArg().Instruments.Minor;
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(minorID);
			InstCodeT const& instCode = instrument.GetInstCode();			
			TSGEProductClassType productClass = SGE_OC_Unknown;
			if (Exchange::SGE == m_minorExchange) {
				productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
			}
			OrderCancel action(m_minorExchange, m_minorLeg.LegDealing->GetSystemNo(), instCode, minorID, m_minorLeg.LegDealing->GetLocalId(), LocalIdACC::NextNormal(), productClass);
			m_parent->PushAction(action);
		}
	}
	void SpreadBusiness::cancelMajor() {
		if (0 != m_minorLeg.LegDealing->GetSystemNo().Length()) {
			InstID majorID = m_parent->GetArg().Instruments.Major;
			Instrument const& instrument = InstrumentFactory::GetInstruments().Get(majorID);
			InstCodeT const& instCode = instrument.GetInstCode();
			TSGEProductClassType productClass = SGE_OC_Unknown;
			if (Exchange::SGE == m_majorExchange) {
				productClass = instrument.GetVariety().IsSpot() ? SGE_OC_Spot : SGE_OC_Defer;
			}
			OrderCancel action(m_majorExchange, m_majorLeg.LegDealing->GetSystemNo(), instCode, majorID, m_majorLeg.LegDealing->GetLocalId(), LocalIdACC::NextNormal(), productClass);
			m_parent->PushAction(action);
		}
	}
}