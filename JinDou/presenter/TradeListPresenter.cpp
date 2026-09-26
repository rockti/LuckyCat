#include "TradeListPresenter.h"

namespace ui {
	void TradeListPresenter::OnAdd(const InvestorTrade& trade) {
		trades_.push_back(trade);
		if (TradeViewModel::Detail == model_) {
			view_->AddTrade(trade);
		} else {
			Reload(TradeViewModel::Total);
		}
	}
	void TradeListPresenter::Reload(TradeViewModel model) {
		auto const& trades = getTrades(model);
		view_->Reload(trades);
	}
	void TradeListPresenter::ViewModel(TradeViewModel model) {
		auto trades = getTrades(model);
		view_->Reload(trades);
	}
	Trades TradeListPresenter::getTrades(TradeViewModel model) {
		return (model == TradeViewModel::Detail) ? getTradesByTradeID() : getTradesByInstID();
	}
	Trades TradeListPresenter::getTradesByTradeID() {
		Trades result;
		std::for_each(trades_.rbegin(), trades_.rend(), [&](auto const& trade) {
			result.push_back(trade);
			});
		return result;
	}
	Trades TradeListPresenter::getTradesByInstID() {
		Trades result;

		using MergeTradeKey = std::tuple<type::InstID, type::Direction, type::OffsetFlag>;
		std::map<MergeTradeKey, InvestorTrade> instTrades;

		std::for_each(trades_.rbegin(), trades_.rend(), [&](auto const& trade) {
			type::InstID id = trade.InstID;
			auto iter = instTrades.find(std::make_tuple(trade.InstID, trade.Direction, trade.OffsetFlag));
			if (iter != instTrades.end()) {
				auto& instTrade = iter->second;
				auto newVolume = trade.TradeVolume + instTrade.TradeVolume;
				auto newTotal = trade.Total() + instTrade.Total();
				auto newAvg = newTotal / newVolume;
				instTrade.TradeVolume = newVolume;
				instTrade.TradePrice = newAvg;
				instTrade.UsedMargin += trade.UsedMargin;
				instTrade.UsedFee += trade.UsedFee;
			} else {
				instTrades.emplace(std::make_tuple(trade.InstID, trade.Direction, trade.OffsetFlag), trade);
			}
			});
		for (auto const& kv : instTrades) {
			result.push_back(kv.second);
		}
		return result;
	}
}