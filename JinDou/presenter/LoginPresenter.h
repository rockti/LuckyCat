#pragma once

#include "service/Service.h"
#include "PresenterUtils.h"
#include "runner/Framework.h"
#include "service/CustomizingPersistService.h"
namespace ui {
	class LoginPresenter;


	class ILoginView : public ViewBase<LoginPresenter> {
	public:
		virtual void OnConnected(TradePlatform platform, SubSystem subSystem) = 0;
		virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) = 0;
		virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) = 0;
		virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) = 0;
		virtual void OnProgress(TradePlatform platform, int percent, std::string const& lastMessage) = 0;
	};

	class LoginPresenter : public ITradeSystemObserver, public IInitProgressObserver {
		enum PlatformStatus {
			New,
			Requesting,
			LoginSuccess,
			LoginFailed,
		};
	public:
		LoginPresenter(ILoginView* view) :view_(view) {
			view->SetPresenter(this);
		}

		void Connect(TradePlatform platform) {
			bool isInited = Framework::Instance().IsInited(platform);
			if (!isInited) {
				Framework::Instance().InitConnect(platform);
			}
		}
		void Login(TradePlatform platform, type::InvestorID const& investor, type::Password const& password) {
			auto status = getStatus(platform);
			if (status == New || status == LoginFailed) {
				ServiceSinglton::Services().TradeSystem->Login(platform, investor, password);
			}
		}

		void Login(TradePlatform platform, type::InvestorID const& tradeInvestor, type::Password const& tradePassword,
			type::InvestorID const& quoteInvestor, type::Password const& quotePassword) {
			auto status = getStatus(platform);
			if (status == New || status == LoginFailed) {
				ServiceSinglton::Services().TradeSystem->Login(platform, tradeInvestor, tradePassword, quoteInvestor, quotePassword);
			}
		}

		type::InvestorID GetSavedInvestorID(TradePlatform platform, SubSystem subSystem) {
			return type::InvestorID("");
		}
		virtual void OnConnected(TradePlatform platform, SubSystem subSystem) override {
			view_->OnConnected(platform,subSystem);
		}
		virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) override {
			setStatus(platform, LoginFailed);
			view_->OnDisconnected(platform,subSystem);
		}
		virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) override {
			setStatus(platform, LoginSuccess);
			view_->OnLogin(platform,subSystem, tradingDay);
		}
		virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) override {
			setStatus(platform, LoginFailed);
			view_->OnLoginFailed(platform, subSystem, error);
		}
		virtual void OnReady(TradePlatform platform, SubSystem subSystem) override {
		}
		virtual void OnProgress(TradePlatform platform, int percent, std::string const& lastMessage) override {
			view_->OnProgress(platform, percent, lastMessage);
		}
		virtual void OnOrderError(OrderErrorNotification const& orderError) override {}
		virtual void OnError(ErrorNotification const& error) override {}
		virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) override {}

	private:
		PlatformStatus getStatus(TradePlatform platform) const {
			return platformStatus_[int(platform)];
		}
		void setStatus(TradePlatform platform, PlatformStatus status) {
			platformStatus_[int(platform)] = status;
		}
		std::array<PlatformStatus, 10> platformStatus_ = { New };
		ILoginView* view_;
	};
}
