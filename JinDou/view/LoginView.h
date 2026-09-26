#pragma once
#include "presenter/LoginPresenter.h"
#include "service/Service.h"
#include "service/TradeSystemService.h"
#include "LoginDlg.h"
#include "ViewUtils.h"
#include <QMessageBox>
#include "service/CustomizingPersistService.h"
#include "service/InstrumentProgressManager.h"

class LoginView : public ui::ILoginView {
public:
	LoginView(LoginDlg& dlg) :dlg_(dlg) {
		dlg_.sgeProgressBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		dlg_.shfeProgressBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		CustomizedPersistService::Instance().Load();
		init();
	}

	void init() {
		auto const& sge = CustomizedPersistService::Instance().GetSavedInvestor(TradePlatform::ZTP);
		dlg_.SGEAccountEdit->setText(QString::fromLocal8Bit(sge.TradeInvestor.value()));
		auto const trader = CustomizedPersistService::Instance().GetTraderNumber();
		if (ConfigLoader::Get().UserMode == UserMode::Multiple) {
			if (trader.Investor == sge.TradeInvestor) {
				dlg_.SGETraderNumberCombo->setCurrentIndex(trader.TraderNubmer - 1);
			}
		} else {
			dlg_.SGETraderNumberCombo->hide();
			dlg_.SGETraderNumberLabel->hide();
		}

		auto const& ctpSI = CustomizedPersistService::Instance().GetSavedInvestor(TradePlatform::CTP);
		dlg_.SHFEGroupBox->setChecked(ctpSI.Checked);
		dlg_.SHFEGroupBox->setVisible(ctpSI.Checked);
		dlg_.SHFEAccountEdit->setText(QString::fromLocal8Bit(ctpSI.TradeInvestor.value()));

		auto const& esunnySI = CustomizedPersistService::Instance().GetSavedInvestor(TradePlatform::HKEX_ESUNNY);
		dlg_.EsunneyGroupBox->setChecked(esunnySI.Checked);
		dlg_.EsunneyGroupBox->setVisible(esunnySI.Checked);
		dlg_.EsunneyTradeAccountEdit->setText(QString::fromLocal8Bit(esunnySI.TradeInvestor.value()));
		dlg_.EsunneyQuoteAccountEdit->setText(QString::fromLocal8Bit(esunnySI.QuoteInvestor.value()));
	}

	void save() {
		SavedInvestor sge;
		sge.Checked = true;
		sge.Platform = TradePlatform::ZTP;
		sge.TradeInvestor = dlg_.SGEAccountEdit->text().toLocal8Bit();
		sge.QuoteInvestor = dlg_.SGEAccountEdit->text().toLocal8Bit();
		CustomizedPersistService::Instance().SaveInvestor(sge);

		ZTPInvestorTrader trader;
		trader.Investor = sge.TradeInvestor;
		trader.TraderNubmer = dlg_.SGETraderNumberCombo->currentIndex() + 1;
		CustomizedPersistService::Instance().SaveTraderNumber(trader);
		LocalIdACC::SetSuffix(trader.TraderNubmer);

		SavedInvestor ctp;
		ctp.Checked = dlg_.SHFEGroupBox->isChecked();
		ctp.Platform = TradePlatform::CTP;
		ctp.TradeInvestor = dlg_.SHFEAccountEdit->text().toLocal8Bit();
		ctp.QuoteInvestor = dlg_.SHFEAccountEdit->text().toLocal8Bit();
		CustomizedPersistService::Instance().SaveInvestor(ctp);


		SavedInvestor esunny;
		esunny.Checked = dlg_.EsunneyGroupBox->isChecked();
		esunny.Platform = TradePlatform::HKEX_ESUNNY;
		esunny.TradeInvestor = dlg_.EsunneyTradeAccountEdit->text().toLocal8Bit();
		esunny.QuoteInvestor = dlg_.EsunneyQuoteAccountEdit->text().toLocal8Bit();
		CustomizedPersistService::Instance().SaveInvestor(esunny);
	}

	void ShowPlatform() {
		isShow_ = !isShow_;
		dlg_.SHFEGroupBox->setVisible(isShow_);
		dlg_.EsunneyGroupBox->setVisible(isShow_);
	}

	bool LoginRequested() const {
		return !tpStatus_.empty();
	}
	bool IsConnected() const {
		for (auto& tpStatus : tpStatus_) {
			if (tpStatus.second.QuoteConnected && tpStatus.second.TradeConnected) {
				continue;
			} else {
				return false;
			}
		}
		return true;
	}
	virtual void OnConnected(TradePlatform platform, SubSystem subSystem) override {
		auto& theStatus = (SubSystem::Quote == subSystem) ? tpStatus_[platform].QuoteConnected : tpStatus_[platform].TradeConnected;
		theStatus = true;
		if (platform == TradePlatform::HKEX_ESUNNY) {
			return;
		}
		//如果行情和交易连接都成功，发起登录请求
		if (tpStatus_[platform].QuoteConnected && tpStatus_[platform].TradeConnected) {
			DoLogin(platform);
		}
	}

	virtual void OnDisconnected(TradePlatform platform, SubSystem subSystem) override {
		auto& theStatus = (SubSystem::Quote == subSystem) ? tpStatus_[platform].QuoteConnected : tpStatus_[platform].TradeConnected;
		theStatus = false;
	}

	virtual void OnLogin(TradePlatform platform, SubSystem subSystem, type::TradingDay const& tradingDay) override {
		auto& status = (SubSystem::Quote == subSystem) ? tpStatus_[platform].QuoteLogined : tpStatus_[platform].TradeLogined;
		status = true;
	}

	virtual void OnLoginFailed(TradePlatform platform, SubSystem subSystem, Error error) override {
		auto& theStatus = (SubSystem::Quote == subSystem) ? tpStatus_[platform].QuoteLogined : tpStatus_[platform].TradeLogined;
		theStatus = false;

		switch (platform) {
		case TradePlatform::CTP: {
			dlg_.SHFEGroupBox->setEnabled(true);
			break;
		}
		case TradePlatform::HKEX_ESUNNY:
			dlg_.EsunneyGroupBox->setEnabled(true);
			break;
		case TradePlatform::ZTP: {
			dlg_.SGEGroupBox->setEnabled(true);
			break;
		}
		default:
			break;
		}
		dlg_.okButton->setEnabled(true);
	}

	virtual void OnProgress(TradePlatform platform, int percent, std::string const& lastMessage) override {
		auto& initService = ServiceSinglton::Services().Init;
		QProgressBar* progressBar = getProgressBar(platform);
		if (progressBar) {
			progressBar->setMaximum(100);
			auto error = initService->GetLastError(platform);
			if (!error) {
				QPalette palette;
				palette.setColor(QPalette::Foreground, Qt::red);
				progressBar->setPalette(palette);
				progressBar->setFormat(QString::fromLocal8Bit(error.Msg.value()));
			} else {
				QPalette palette;
				palette.setColor(QPalette::Foreground, Qt::black);
				progressBar->setPalette(palette);
				progressBar->setFormat(QString::fromLocal8Bit(lastMessage.c_str()));
			}
			progressBar->setValue(percent);
		}
	}

	QProgressBar* getProgressBar(TradePlatform platform)
	{
		switch (platform) {
		case TradePlatform::CTP:
			return dlg_.shfeProgressBar;
			break;
		case TradePlatform::FEMAS:
			break;
		case TradePlatform::HKEX_ESUNNY:
			return dlg_.esunneyProgressBar;
			break;
		case TradePlatform::XSPEED:
			break;
		case TradePlatform::ZTP:
			return dlg_.sgeProgressBar;
			break;
		default:
			break;
		}
		return nullptr;
	}

public:
	void OnLoginClicked() {
		if (!ValidateParam()) {
			return;
		}

		save();

		dlg_.okButton->setEnabled(false);
		dlg_.resetButton->setEnabled(false);
		dlg_.moreButton->setEnabled(false);

		auto platform = ServiceSinglton::Services().Exchange->toPlatform(type::Exchange::SGE);
		ProcessLogin(platform);
		dlg_.SGEGroupBox->setEnabled(false);
		ServiceSinglton::Services().Init->AddPlatform(TradePlatform::ZTP);

		if (dlg_.SHFEGroupBox->isChecked()) {
			platform = ServiceSinglton::Services().Exchange->toPlatform(type::Exchange::SHFE);
			ProcessLogin(platform);
			dlg_.SHFEGroupBox->setEnabled(false);
			ServiceSinglton::Services().Init->AddPlatform(platform);
		} else {
			platform = ServiceSinglton::Services().Exchange->toPlatform(type::Exchange::SHFE);
			ServiceSinglton::Services().Init->DeletePlatform(platform);
		}
		if (dlg_.EsunneyGroupBox->isChecked()) {
			platform = ServiceSinglton::Services().Exchange->toPlatform(type::Exchange::CME);
			ProcessLogin(platform);
			dlg_.EsunneyGroupBox->setEnabled(false);
			ServiceSinglton::Services().Init->AddPlatform(platform);
		} else {
			platform = ServiceSinglton::Services().Exchange->toPlatform(type::Exchange::CME);
			ServiceSinglton::Services().Init->DeletePlatform(platform);
		}
		dlg_.elapsedTimer_.start();
	}
	void ProcessLogin(TradePlatform platform) {
		auto iter = tpStatus_.find(platform);
		if (iter == tpStatus_.end()) {
			tpStatus_.emplace(platform, TPStatus{});
			DoConnect(platform);
		} else {
			DoLogin(platform);
		}
	}
	void DoConnect(TradePlatform platform) {
		presenter_->Connect(platform);
		//ESUNNY是调用登录才连接
		if (platform == TradePlatform::HKEX_ESUNNY) {
			DoLogin(TradePlatform::HKEX_ESUNNY);
		}
	}
	void DoLogin(TradePlatform platform) {
		switch (platform) {
		case TradePlatform::CTP: {
			type::InvestorID investorID{ dlg_.SHFEAccountEdit->text().toStdString().c_str() };
			type::Password pwd{ dlg_.SHFEPwdEdit->text().toStdString().c_str() };
			presenter_->Login(platform, investorID, pwd);
			break;
		}
		case TradePlatform::HKEX_ESUNNY: {
			type::InvestorID tradeInvestorID{ dlg_.EsunneyTradeAccountEdit->text().toStdString().c_str() };
			type::Password tradePwd{ dlg_.EsunneyTradePwdEdit->text().toStdString().c_str() };
			type::InvestorID quoteInvestorID{ dlg_.EsunneyQuoteAccountEdit->text().toStdString().c_str() };
			type::Password quotePwd{ dlg_.EsunneyQuotePwdEdit->text().toStdString().c_str() };
			presenter_->Login(platform, tradeInvestorID, tradePwd, quoteInvestorID, quotePwd);
			break;
		}
		case TradePlatform::ZTP: {
			type::InvestorID investorID{ dlg_.SGEAccountEdit->text().toStdString().c_str() };
			type::Password pwd{ dlg_.SGEPwdEdit->text().toStdString().c_str() };
			presenter_->Login(platform, investorID, pwd);
			break;
		}
		default:
			break;
		}
	}

	bool IsLoginSuccess() {
		if (tpStatus_.empty()) {
			return false;
		}
		auto* init = ServiceSinglton::Services().Init;
		for (auto const& kv : tpStatus_) {
			auto percent = init->GetPercent(kv.first);
			if (percent != 100) {
				return false;
			}
		}
		return true;
	}
private:
	bool ValidateParam() {
		if (dlg_.SGEAccountEdit->text().isEmpty()) {
			QMessageBox::warning(NULL, "Warning", QStringLiteral("金交所账号不能为空!"), QMessageBox::Ok);
			dlg_.SGEAccountEdit->setFocus(Qt::TabFocusReason);
			return false;
		}
		if (dlg_.SGEPwdEdit->text().isEmpty()) {
			QMessageBox::warning(NULL, "Warning", QStringLiteral("金交所密码不能为空!"), QMessageBox::Ok);
			dlg_.SGEPwdEdit->setFocus(Qt::TabFocusReason);
			return false;
		}
		if (dlg_.SHFEGroupBox->isChecked()) {
			if (dlg_.SHFEAccountEdit->text().isEmpty()) {
				QMessageBox::warning(NULL, "Warning", QStringLiteral("CTP账号不能为空!"), QMessageBox::Ok);
				dlg_.SHFEAccountEdit->setFocus(Qt::TabFocusReason);
				return false;
			}
			if (dlg_.SHFEPwdEdit->text().isEmpty()) {
				QMessageBox::warning(NULL, "Warning", QStringLiteral("CTP密码不能为空!"), QMessageBox::Ok);
				dlg_.SHFEPwdEdit->setFocus(Qt::TabFocusReason);
				return false;
			}
		}
		if (dlg_.EsunneyGroupBox->isChecked()) {
			if (dlg_.EsunneyTradeAccountEdit->text().isEmpty()) {
				QMessageBox::warning(NULL, "Warning", QStringLiteral("易盛交易账号不能为空!"), QMessageBox::Ok);
				dlg_.EsunneyTradeAccountEdit->setFocus(Qt::TabFocusReason);
				return false;
			}
			if (dlg_.EsunneyTradePwdEdit->text().isEmpty()) {
				QMessageBox::warning(NULL, "Warning", QStringLiteral("易盛交易密码不能为空!"), QMessageBox::Ok);
				dlg_.EsunneyTradePwdEdit->setFocus(Qt::TabFocusReason);
				return false;
			}
			if (dlg_.EsunneyQuoteAccountEdit->text().isEmpty()) {
				QMessageBox::warning(NULL, "Warning", QStringLiteral("易盛行情账号不能为空!"), QMessageBox::Ok);
				dlg_.EsunneyQuoteAccountEdit->setFocus(Qt::TabFocusReason);
				return false;
			}
			if (dlg_.EsunneyQuotePwdEdit->text().isEmpty()) {
				QMessageBox::warning(NULL, "Warning", QStringLiteral("易盛行情密码不能为空!"), QMessageBox::Ok);
				dlg_.EsunneyQuotePwdEdit->setFocus(Qt::TabFocusReason);
				return false;
			}
		}
		return true;
	}

private:
	LoginDlg& dlg_;
	std::map<TradePlatform, TPStatus> tpStatus_;
	bool isShow_ = false;
};