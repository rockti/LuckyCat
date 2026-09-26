#pragma once
#include "model/Instrument.h"
#include "model/dealing/Dealing.h"
#include "model/factory/InstrumentFactory.h"
#include "SpreadStatus.h"
#include "SpreadContext.h"
#include "SpreadStrategy.h"

namespace hf::spread {
	struct BalanceCounter {
		QtyT MinorCnt = 0;			// Minor 成交量（不是持仓量）
		QtyT MajorCnt = 0;			// Major 成交量（Multiple的整数倍，同样不是持仓量）
		QtyT MajorPending = 0;		// Major 挂单量（防止多发）
	};

	struct DirPosition {	
		Direction posDir = Direction::BUY;
		QtyT netVolume = 0;
		money turnover = 0;

		void OnTrade(Direction tradeDir, QtyT qty, money price) {
			if (0 == this->netVolume) {
				this->posDir = tradeDir;
				this->netVolume = qty;
				this->turnover = price * qty;
			}
			else {
				if (this->posDir == tradeDir) {
					this->netVolume += qty;
					turnover += price * qty;
				}
				else {
					this->netVolume -= qty;
					turnover -= price * qty;
					if (turnover < 0) {
						turnover = 0;
					}
				}
			}
			if (this->netVolume < 0) {
				Log::Info(fmt::format("A fatal error has occurred，OnTrade[{}:{}] DirPosition[{}:[{}]", Desc(tradeDir), qty, Desc(posDir), this->netVolume));
			}
		}
		money GetAvgPrice() { 
			if (0 != netVolume) {
				return this->turnover / this->netVolume;
			}
			return 0;
		}
		bool operator==(DirPosition const& other) const {
			return posDir == other.posDir && netVolume == other.netVolume;
		}
		bool operator!=(DirPosition const& other) const {
			return !(*this == other);
		}
	};

	struct Leg {
		Dealing* LegDealing = nullptr;
		bool RtnTag = false;	// Dealing 收到应答
		bool EndTag = false;	// Dealing 已经结束
	};

	class SpreadPortfolio {
	public:
		SpreadPortfolio(SPInstruments instruments, Direction spreadDir, QtyT multiple) 
			:SPInstruments(instruments), SpreadDir(spreadDir), Multiple(multiple) {
			MajorFlags = pos::GlobalPositionStore::Instance().GetOpenedFlags(instruments.Major);
			MinorFlags = pos::GlobalPositionStore::Instance().GetOpenedFlags(instruments.Minor);
		}
		SpreadPortfolio() = default;
		SpreadPortfolio& operator=(SpreadPortfolio const& other) = default;

		inline SPInstruments const& GetSPInstruments() { return SPInstruments; }
		void InitCounter() { memset(&Counter, 0, sizeof(BalanceCounter)); }
		void InitPortfolio(int minorQty) {
			if (minorQty < 0) {
				YDMinorPos.posDir = SPInstruments.IsMajorFarther() ? OppositeDirection(SpreadDir) : SpreadDir;
				YDMinorPos.netVolume = -minorQty;
				YDMajorPos.posDir = OppositeDirection(YDMinorPos.posDir);
				YDMajorPos.netVolume = Multiple * YDMinorPos.netVolume;
			}
			else {
				MinorPos.posDir = SPInstruments.IsMajorFarther() ? OppositeDirection(SpreadDir) : SpreadDir;
				MinorPos.netVolume = minorQty;
				MajorPos.posDir = OppositeDirection(MinorPos.posDir);
				MajorPos.netVolume = Multiple * MinorPos.netVolume;
			}			
		}
	
		void OnMajorPending(QtyT qty) { Counter.MajorPending = qty;  }
		void OnMajorCancel(QtyT qty) { Counter.MajorPending = 0; }
		void OnTrade(Dealing const& dealing, QtyT qty, money price) {
			Exchange exchange = dealing.GetExchange();
			Direction tradeDir = dealing.GetDirection();
			if (SPInstruments.Major == dealing.GetInstID()) {
				Counter.MajorCnt += qty;	// 补单的Dealing与发单的Dealing不是同一个，只能用传入Qty，不能用dealing里的Qty
				Counter.MajorPending = dealing.GetRemainQuantity();

				if ((IsSHFE(exchange) || IsINE(exchange)) && Operation::OFFSET == dealing.GetOperation()) {
					YDMajorPos.OnTrade(tradeDir, qty, price);
				}
				else {
					MajorPos.OnTrade(tradeDir, qty, price);
				}
			}
			else {
				Counter.MinorCnt += qty;	// 补单的Dealing与原始的Dealing不是同一个，只能增加量，不能用dealing里的数据
				if ((IsSHFE(exchange) || IsINE(exchange)) && Operation::OFFSET == dealing.GetOperation()) {
					YDMinorPos.OnTrade(tradeDir, qty, price);
				}
				else {
					MinorPos.OnTrade(tradeDir, qty, price);
				}
			}
		}

		int GetBalancingQty() const { return (Multiple * Counter.MinorCnt) - Counter.MajorCnt - Counter.MajorPending; }
		QtyT GetMajorPending() const { return Counter.MajorPending; }
		money GetSpreadCost(bool isMajorFarther) { return isMajorFarther ? MajorPos.GetAvgPrice() - MinorPos.GetAvgPrice() : MinorPos.GetAvgPrice() - MajorPos.GetAvgPrice(); }

		QtyT GetMinorVolume() {
			Direction minorDir = SPInstruments.IsMajorFarther() ? OppositeDirection(SpreadDir) : SpreadDir;
			return (minorDir == MinorPos.posDir) ? MinorPos.netVolume : 0;
		}
		QtyT GetYDMinorVolume() {
			Direction minorDir = SPInstruments.IsMajorFarther() ? OppositeDirection(SpreadDir) : SpreadDir;
			return (minorDir == YDMinorPos.posDir) ? YDMinorPos.netVolume : 0;
		}
		QtyT GetTotalMinorVolume() { return MinorPos.netVolume + YDMinorPos.netVolume; }
		QtyT GetTotalMajorVolume() { return MajorPos.netVolume + YDMajorPos.netVolume; }

		bool IsMinorOpenOnly(Direction dir) const { return MinorFlags->IsOpenOnly(dir); }
		bool IsMajorOpenOnly(Direction dir) const { return MajorFlags->IsOpenOnly(dir); }
		bool IsSpreadOpenOnly() const {
			Direction minorDir = SPInstruments.IsMajorFarther() ? OppositeDirection(SpreadDir) : SpreadDir;
			Direction majorDir = OppositeDirection(minorDir);			
			return IsMinorOpenOnly(minorDir) || IsMajorOpenOnly(majorDir);
		}

	private:
		SPInstruments SPInstruments;
		Direction SpreadDir;
		QtyT Multiple = 1;
		BalanceCounter Counter = { 0 };	// 平衡计数器(维护单次交易)

		DirPosition MinorPos;
		DirPosition MajorPos;		
		DirPosition YDMinorPos;			// SHFE或INE的老仓（只减不增）
		DirPosition YDMajorPos;			// SHFE或INE的老仓（只减不增）

		// 持仓方向无平仓惩罚就平仓，有平仓惩罚就反开
		pos::InstrumentDynamicData* MajorFlags = nullptr;
		pos::InstrumentDynamicData* MinorFlags = nullptr;
	};

	class SpreadStrategy;
	class SpreadBusiness {
	public:
		SpreadBusiness(SpreadStrategy* parent);
		void OnStart();
		void OnPaused();
		void OnQuote(Quote const& quote);
		void OnMinorCancel(int quantity);
		void OnMajorCancel(int quantity);
		void OnMinorTrade(Dealing const& dealing, int qty);
		void OnMajorTrade(Dealing const& dealing, int qty);
		void OnMinorFailed(Dealing const& dealing);
		void OnMajorFailed(Dealing const& dealing);

		void InitTags();
		void UpdateBand(bool isOpen);

		int TryOpen();		// 0：NoChance 1：Normal 2：FAK
		int TryOffset();	// 0：NoChance 1：Normal 2：FAK

		bool BalanceCheck(Direction dir, Operation opt, OrderType orderType, bool isMinorDealing);
		bool IsMissPendingChance();

		void openMinor(money price, QtyT qty, bool isFak = false);
		void openMajor(money price, QtyT qty, bool isFak = false);
		void offsetMinor(money price, QtyT qty, bool isFak = false);
		void offsetMajor(money price, QtyT qty, bool isFak = false);

		// InsertFailed 的balance单 为了防止平今指令干扰，一律改为反开锁仓
		void balanceFAKOpenMinor(Direction minorDir, money price, QtyT qty);
		void balanceFAKOpenMajor(Direction majorDir, money price, QtyT qty);

		void cancelMinor();
		void cancelMajor();
		
		bool IsMinorPending() const {
			if (m_minorLeg.RtnTag && !m_minorLeg.EndTag) {
				return !m_minorLeg.LegDealing->IsFinished() && m_minorLeg.LegDealing->GetRemainQuantity() > 0;
			}
			return false;
		}
		bool IsMajorPending() const {
			if (m_majorLeg.RtnTag && !m_majorLeg.EndTag) {
				return !m_majorLeg.LegDealing->IsFinished() && m_majorLeg.LegDealing->GetRemainQuantity() > 0;
			}
			return false;
		}
		
		QtyT GetPortfolio() { return m_portfolio.GetMinorVolume(); }
		QtyT GetYDPortfolio() { return m_portfolio.GetYDMinorVolume(); }
		money GetSpread() { return m_portfolio.GetSpreadCost(m_parent->GetArg().Instruments.IsMajorFarther()); }

		BasePhase* GetStatus(Status index) { return m_machine.GetPhase(index); }
	private:
		bool CanMarketOpen(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty);
		bool CanMarketOffset(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty);
		bool CanNormalOpen(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty);
		bool CanNormalOffset(QtyT maxVolume, money& majorPrice, money& minorPrice, QtyT& minorOrderQty);

	public:
		SpreadStrategy* m_parent;
		SpreadType m_spreadType;
		Direction m_spreadDir;
		SpreadMachine m_machine;

		SpreadPortfolio m_portfolio;
		DirectionalOrders m_orders;
		Leg m_minorLeg;
		Leg m_majorLeg;

		QtyT m_multiple;					// 最小是1，不得小于0
		money m_majorPendingPrice;			// 主要为了记录均价

		Exchange m_minorExchange;
		Exchange m_majorExchange;
		money m_minorUnitPrice;
		money m_majorUnitPrice;
		bool m_isDeepthQuote;				// 是否深度行情
		bool m_isUnitPriceEqual;			// 两个合约的最小变动价位是否一致

		int m_id;
		bool m_YDOffsetTag = false;		 	// SHFE或INE平老仓（不支持不同交易所之间的套利）
		bool m_offsetForbiddenTag = false;
		BasePhase* m_currentPhase = nullptr;
	};
}