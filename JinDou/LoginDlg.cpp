#include "LoginDlg.h"
#include <QtGui>
#include <QShortcut.h>
#include "view/LoginView.h"
#include <sstream>
#include "utils/Log.h"
using namespace std;
LoginDlg::LoginDlg(QWidget* parent) 
	:QDialog(parent) {
	setupUi(this);
 	SHFEGroupBox->hide();
	EsunneyGroupBox->hide();	
	gridLayout->setSizeConstraint(QLayout::SetFixedSize);
	auto* showConfig = new QShortcut(this);
	showConfig->setAutoRepeat(false);
	showConfig->setKey(QKeySequence(Qt::CTRL + Qt::ALT + Qt::SHIFT + Qt::Key_P));
	connect(resetButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(moreButton, SIGNAL(clicked()), this, SLOT(showPlatform()));
	connect(showConfig, SIGNAL(activated()), this, SLOT(showConfig()));

	this->startTimer(100, Qt::PreciseTimer);
}

const int LOGIN_TIMEOUT = 10000;
void LoginDlg::timerEvent(QTimerEvent* event) {
	if (elapsedTimer_.isValid() && elapsedTimer_.hasExpired(LOGIN_TIMEOUT) && loginView_->LoginRequested() && !loginView_->IsConnected()) {
		QMessageBox::warning(NULL, "Error", QStringLiteral("登录超时，请检查设置后重试!"), QMessageBox::Ok);
		elapsedTimer_.invalidate();
	}

	if (loginView_->IsLoginSuccess()) {
		Log::Info("!!!!!!!!!Login Success!!!!!!!!!!!!!");
		QDialog::accept();
	}
}
void LoginDlg::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	case Qt::Key_Escape:
		break;
	default:
		QDialog::keyPressEvent(event);
	}
}

void LoginDlg::accept() {
	loginView_->OnLoginClicked();
}
void LoginDlg::clear() {
	SGEAccountEdit->clear();
	SGEPwdEdit->clear();
	if (SHFEGroupBox->isChecked()) {
		SHFEAccountEdit->clear();
		SHFEPwdEdit->clear();
	}
	if (EsunneyGroupBox->isChecked()) {
		EsunneyTradeAccountEdit->clear();
		EsunneyTradePwdEdit->clear();
		EsunneyQuoteAccountEdit->clear();
		EsunneyQuotePwdEdit->clear();
	}
}

void LoginDlg::showPlatform() {
	loginView_->ShowPlatform();
}

void LoginDlg::showConfig() {
	auto const& config = ConfigLoader::Get();
	stringstream ss;
	ss << "License=" << config.License << endl;
	for (auto const& tpConfig : config.TradeSystemConfigs)	{
		ss << "[" << toString(tpConfig.Platform) << "]" << endl;
		ss << "  TradeEndpoint=" << tpConfig.TradeEndpoint << endl;
		ss << "  QuoteEndpoint=" << tpConfig.QuoteEndpoint << endl;
		ss << "  Varieties=" << tpConfig.Varieties << endl;
		ss << "  BrokerID=" << tpConfig.Credential.BrokerID << endl;
		ss << "  TradeAuthCode=" << tpConfig.Credential.TradeAuthCode << endl;
		ss << "  QuoteAuthCode=" << tpConfig.Credential.QuoteAuthCode << endl;
	}
	auto index = QMessageBox::information(this, QString::fromLocal8Bit("配置信息"), QString::fromStdString(ss.str()), 
		QString::fromLocal8Bit("Copy"), QString::fromLocal8Bit("退出"));

	if (index == 0) {
		QClipboard* clipboard = QApplication::clipboard();
		clipboard->setText(QString::fromStdString(ss.str()));
	}

}
