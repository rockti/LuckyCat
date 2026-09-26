#pragma once
#include "presenter/StatusPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"

class StatusView : public ui::IStatusView {
public:
	StatusView(JinDou& mainWindow) :mainWindow_(mainWindow) {
	}

	virtual void UpdateExchangeStatus(type::Exchange exchange, ExchangeStatus status) override {
		QString statusName = QString("[%1]").arg(NameService::Name(status));
		switch (exchange) {
		case type::Exchange::SHFE:			
			mainWindow_.m_shfeExchangeStatus->setText(statusName);
			break;
		case type::Exchange::CME:
		case type::Exchange::HKEX:
			mainWindow_.m_esunneyExchangeStatus->setText(statusName);
			break;
		case type::Exchange::SGE:
			mainWindow_.m_sgeExchangeStatus->setText(statusName);
			break;
		default:
			return;
		}
	}

	virtual void UpdatePlatformStatus(TradePlatform platform, SubSystem subSystem, TradeSystemStatus status) override {
		QLabel* pLabel = nullptr;
		switch (platform) {
		case TradePlatform::CTP: {
			pLabel = (subSystem == SubSystem::Quote) ? mainWindow_.m_shfeQuoteStatus : mainWindow_.m_shfeTradeStatus;
			break;
		}
		case TradePlatform::HKEX_ESUNNY: {
			pLabel = (subSystem == SubSystem::Quote) ? mainWindow_.m_esunneyQuoteStatus : mainWindow_.m_esunneyTradeStatus;
			break;
		}
		case TradePlatform::ZTP: {
			pLabel = (subSystem == SubSystem::Quote) ? mainWindow_.m_sgeQuoteStatus : mainWindow_.m_sgeTradeStatus;
			break;
		}
		default:
			return;
		}

		if (status == TradeSystemStatus::Disconnected) {
			pLabel->setStyleSheet("font-size:22px;color:red");
			QMessageBox::critical(&mainWindow_,
				QString::fromLocal8Bit("连接断开"),
				QString::fromLocal8Bit("连接断开，请重启程序"));
		}
		else {
			pLabel->setStyleSheet("font-size:22px;color:green");
		}
	}

	virtual void ShowOrderRequestError(OrderErrorNotification const& orderError) override {
		char const* action = orderError.Type == OrderErrorNotification::Insert ? "报单" : "撤单";
		auto msg = fmt::format("{}失败 - [{}]", action,orderError.Error.Msg);
		mainWindow_.m_errorMsgLabel->setText(QString::fromLocal8Bit(msg.c_str()));
		mainWindow_.m_errorMsgLabel->setStyleSheet("font-size:25px;color:red");
	}

	virtual void ShowTradePlatformError(ErrorNotification const& error) override {
		mainWindow_.m_errorMsgLabel->setText(QString::fromLocal8Bit(error.Error.Msg.value()));
		mainWindow_.m_errorMsgLabel->setStyleSheet("font-size:25px;color:red");
	}

private:
	std::map<TradePlatform, bool> m_platformTradeLogined;
	JinDou& mainWindow_;
};