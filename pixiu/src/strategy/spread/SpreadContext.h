#pragma once
#include "model/dealing/OrderCommand.h"
#include "model/factory/InstrumentFactory.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class Instrument;

namespace hf::spread {
	struct SPOrderCommands {
		OrderCommand MajorCmd;
		OrderCommand MinorCmd;
		bool operator==(SPOrderCommands const& other) const {
			return other.MinorCmd == MinorCmd && other.MajorCmd == MajorCmd;
		}
	};

	const SPOrderCommands MajorSellOpenOrders{ SellOpenNormal, BuyOpenNormal };	
	const SPOrderCommands MajorSellOffsetOrders{ SellOffsetNormal, BuyOffsetNormal };
	const SPOrderCommands MajorBuyOpenOrders{ BuyOpenNormal, SellOpenNormal };
	const SPOrderCommands MajorBuyOffsetOrders{ BuyOffsetNormal, SellOffsetNormal };

	// 合约组合基本信息：Insts、Orders、Delta、Price
	class SPInstruments {
	public:
		SPInstruments(int major, int minor) :Major(major), Minor(minor) {
			Farther = std::max(major, minor);
			Closer = std::min(major, minor);
			isMajorFarther = (Farther == Major) ? true : false;
			isMajorSpot = InstrumentFactory::GetInstruments().Get(Major).GetVariety().IsSpot();

			MajorExchange = InstrumentFactory::GetInstruments().Get(Major).GetExchange();
			MajorUnitPrice = InstrumentFactory::GetInstruments().Get(Major).GetUnitPrice();
			MinorUnitPrice = InstrumentFactory::GetInstruments().Get(Minor).GetUnitPrice();			
			ValidSpace = InstrumentFactory::GetInstruments().Get(Major).GetVariety().IsSpot() ? MajorUnitPrice * 50/*现货50档*/ : MajorUnitPrice * 10/*10档*/;
		}
		SPInstruments(SPInstruments const& other) = default;
		SPInstruments& operator=(SPInstruments const& other) = default;
		bool operator==(SPInstruments const& other) const {
			return other.Major == Major && other.Minor == Minor;
		}

		// 卖价差Orders：买近抛远->[远月卖Orders，近月买Orders]
		SPOrderCommands GetSellOpenOrders() const { return isMajorFarther ? MajorSellOpenOrders : MajorBuyOpenOrders; }
		SPOrderCommands GetSellOffsetOrders() const { return isMajorFarther ? MajorSellOffsetOrders : MajorBuyOffsetOrders; }

		// 买价差Orders：抛近买远->[远月买开Orders，近月卖开Orders]
		SPOrderCommands GetBuyOpenOrders() const { return isMajorFarther ? MajorBuyOpenOrders : MajorSellOpenOrders; }
		SPOrderCommands GetBuyOffsetOrders() const { return isMajorFarther ? MajorBuyOffsetOrders : MajorSellOffsetOrders; }

		// 根据价差方向选定Orders
		SPOrderCommands GetSpreadOpenOrders(Direction spreadDir) const { return (Direction::SELL == spreadDir) ? GetSellOpenOrders() : GetBuyOpenOrders(); }
		SPOrderCommands GetSpreadOffsetOrders(Direction spreadDir) const { return (Direction::SELL == spreadDir) ? GetSellOffsetOrders() : GetBuyOffsetOrders(); }

		// 根据Major委托方向计算Major均价和挂单量（如果Major挂单量不满足，返回0.0）
		money GetMultipleAvgPrice(Direction majorDir, QtyT majorVolume, QtyT& actualVolume) const;

		// 计算对价价差
		money GetMarketSellDelta(QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume, money& minorPrice, QtyT& minorVolume) const;
		money GetMarketBuyDelta(QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume, money& minorPrice, QtyT& minorVolume) const;

		// 根据设定价差和盘口算出Minor挂单价，同时获取Major的量
		money GetMinorPriceByUpperLine(money ofrLine, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const;
		money GetMinorPriceByLowerLine(money bidLine, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const;

		// 根据MinorPrice计算价差	
		money GetSellDeltaByMinorPrice(money minorPrice, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const;
		money GetBuyDeltaByMinorPrice(money minorPrice, QtyT multiple, QtyT maxVolume, money& majorPrice, QtyT& majorVolume) const;

		// 有等号，判断条件为：rangeTick+1
		bool IsPriceFit(InstID instID, Direction quoteDir, money price, bool isDeepQuote, int rangeTick = 0) const;

		bool IsMajorFarther() const { return isMajorFarther; }

	public:
		InstID Major;
		InstID Minor;
		InstID Farther;
		InstID Closer;
		Exchange MajorExchange;
		money MajorUnitPrice;
		money MinorUnitPrice;
		money ValidSpace = MajorUnitPrice;
	private:
		bool isMajorSpot;
		bool isMajorFarther;
	};

	// 带方向的委托组合
	struct DirectionalOrders {
		DirectionalOrders(SPInstruments const& instruments, Direction spreadDir)
			:Instruments(instruments), SpreadDir(spreadDir) {
			OpenOrders = instruments.GetSpreadOpenOrders(spreadDir);
			OffsetOrders = instruments.GetSpreadOffsetOrders(OppositeDirection(spreadDir));
		}
		DirectionalOrders& operator=(DirectionalOrders const& other) = default;
		bool operator==(DirectionalOrders const& other) const {
			return other.Instruments == Instruments && SpreadDir == other.SpreadDir;
		}

		SPInstruments Instruments;		// 合约组合
		Direction SpreadDir;			// 价差方向
		SPOrderCommands OpenOrders;		// 开仓组合
		SPOrderCommands OffsetOrders;	// 平仓组合

		money TryCalcOpenFreezeTotal(int quantity) const;	// 试算开仓所需资金
		money CalcOpenFreezeMargin(int quantity) const;		// 冻结保证金(大单边)
		money CalcOpenFreezeFee(int quantity) const;		// 冻结手续费
	};
}
