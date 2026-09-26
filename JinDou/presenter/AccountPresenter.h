#pragma once

#include "service/Service.h"
#include "PresenterUtils.h"
namespace ui {
	class AccountPresenter;
	class IAccountView : public ViewBase<AccountPresenter> {
	public:
		virtual void UpdateAccount(InvestorAccount const& account) = 0;
		virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) = 0;
	};

	class AccountPresenter : public IAccountObserver, public ITradeSystemObserver {
	public:
		AccountPresenter(IAccountView* view) :view_(view) {
			view->SetPresenter(this);
		}
		virtual void OnChange(InvestorAccount const& account) override {
			update(account);
			view_->UpdateAccount(account);
		}	

		virtual void OnConnected(TradePlatform platform, SubSystem subSystem) override {}
		virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) override {}
		virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) override {}
		virtual void OnReady(TradePlatform platform, SubSystem subSystem) override {}
		virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) override {}
		virtual void OnOrderError(OrderErrorNotification const& orderError) override {}
		virtual void OnError(ErrorNotification const& error) override {}
		virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) override {
			view_->OnPasswordChange(platform, subSystem, error);
		}

	private:
		void update(InvestorAccount const& account) {
			accounts_[account.InvestorID] = account;
		}

		IAccountView* view_;
		std::map<type::InvestorID, InvestorAccount> accounts_;
	};
}
