#pragma once
#include "model/Constants.h"

namespace hk {
	enum class InstrumentModel {
		CME_GDU,
		CME_GDR,
	};

	enum class QuotingPriceModel {
		SellSafe = 0,
		Netural,
		BuySafe,
	};

	struct MarketingInstIDs {
		InstID Major;
		InstID Follow;
		InstID Hedge;
	};

	struct InitPosition {
		Direction Direct = Direction::BUY;
		TickT Price = 0;
		QtyT Qty = 0;
		QtyT BalanceQty = 3;
	};


	struct Arguments {
		InstrumentModel Model;
		MarketingInstIDs InstIDs;
		QuotingPriceModel PriceModel;
		int Position;					//持仓
		int InstCostTick;				//人工偏向加点
		int QualityTick;				//GDU成色加点
		TickT SensitivityTick;			//M参数
		TickT SpreadTick;				//报价间距
		QtyT QuoteQuantity;				//报价量
		QtyT RiskQuantity;				//风险持仓，达到进行价格偏向
		QtyT MaxQuantity;				//最大持仓，达到必须停止报价
		money UnitPrice;
		QtyT BuyGainTick = 1;
		QtyT SellGainTick = 1;
		InitPosition InitPosition;

		bool IsHedge = false;

		inline bool IsMyQuoteInstID(InstID instID) const {
			return instID == InstIDs.Follow || instID == InstIDs.Hedge;
		}
	};
	enum class SpecialCommand {
		Loss,
	};
	struct HKCommand {
		SpecialCommand Command;
	};
}