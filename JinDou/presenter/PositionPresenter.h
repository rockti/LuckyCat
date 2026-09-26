#pragma once

#include "service/Service.h"
#include "PresenterUtils.h"
namespace ui {

	using Positions = std::vector<InvestorPosition>;
	class PositionPresenter;
	class IPositionView : public ViewBase<PositionPresenter>{
	public:
		virtual void AddPosition(InvestorPosition const& position) = 0;
		virtual void UpdatePosition(InvestorPosition const& position) = 0;
		virtual void DeletePosition(type::InstrumentCode instCode, type::Direction direction) = 0;
		virtual void Reload(Positions const& positions) = 0;
		virtual void UpdatePositionProfit(TradePlatform platform, double positionProfit) = 0;
	};

	class PositionPresenter : public IPositionObserver {
	public:
		PositionPresenter(IPositionView* view) :view_(view) {
			view->SetPresenter(this);
		}
		virtual void OnAdd(InvestorPosition const& position) override {
			add(position);
			view_->AddPosition(position);
		}
		virtual void OnChange(InvestorPosition const& position) override {
			update(position);
			view_->UpdatePosition(position);
		}
		virtual void OnDelete(InvestorPosition const& position) override {
			remove(position);
			view_->DeletePosition(position.InstrumentCode, position.Direction);
		}
		virtual void OnPositionProfit(TradePlatform platform, double positionProfit) override {
			view_->UpdatePositionProfit(platform, positionProfit);
		}
		virtual void Reload() {
			view_->Reload(toPositions());
		}
		virtual void Sync() {
			ServiceSinglton::Services().TradeSystem->QueryPositions(TradePlatform::ZTP);
		}

	private:
		void add(InvestorPosition const& position) {
			positions_.emplace(toKey(position), position);
		}
		void update(InvestorPosition const& position) {
			positions_[toKey(position)] = position;
		}
		void remove(InvestorPosition const& position) {
			positions_.erase(positions_.find(toKey(position)));
		}
		std::string toKey(InvestorPosition const& position) {
			return fmt::format("{}:{}", position.InstrumentCode, position.Direction);
		}
		Positions toPositions() {
			Positions result;
			for (auto const& kv : positions_) {
				result.push_back(kv.second);
			}
			return result;
		}

		IPositionView* view_;
		std::map<std::string, InvestorPosition> positions_;
	};
}
