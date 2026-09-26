#pragma once

#include "service/Service.h"
#include "PresenterUtils.h"

namespace ui {
	class StatusPresenter;
	class IStatusView : public ViewBase<StatusPresenter> {
	public:
		virtual void UpdateExchangeStatus(type::Exchange exchange, ExchangeStatus status) = 0;
		virtual void UpdatePlatformStatus(TradePlatform platform, SubSystem subSystem, TradeSystemStatus status) = 0;
		virtual void ShowOrderRequestError(OrderErrorNotification const& orderError) = 0;
		virtual void ShowTradePlatformError(ErrorNotification const& error) = 0;
	};

	class StatusPresenter : public IExchangeStatusObserver, public ITradeSystemObserver {
	public:
		StatusPresenter(IStatusView* view) :view_(view) {
			view->SetPresenter(this);
		}

		virtual void OnExchangeStatus(type::Exchange exchange, ExchangeStatus status) override {
			view_->UpdateExchangeStatus(exchange, status);
		}

		virtual void OnConnected(TradePlatform platform, SubSystem subSystem) override {
			view_->UpdatePlatformStatus(platform, subSystem, TradeSystemStatus::Connected);
		}

		virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) override {
			view_->UpdatePlatformStatus(platform, subSystem, TradeSystemStatus::Disconnected);
		}

		virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) override {
			view_->UpdatePlatformStatus(platform, subSystem, TradeSystemStatus::Logined);
		}

		virtual void OnReady(TradePlatform platform, SubSystem subSystem) override {
			view_->UpdatePlatformStatus(platform, subSystem, TradeSystemStatus::Ready);
		}

		virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) override {
		}

		virtual void OnOrderError(OrderErrorNotification const& orderError) override {
			view_->ShowOrderRequestError(orderError);
		}

		virtual void OnError(ErrorNotification const& error) override {
			view_->ShowTradePlatformError(error);
		}

		virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) override {}
	private:
		IStatusView* view_;
	};
}