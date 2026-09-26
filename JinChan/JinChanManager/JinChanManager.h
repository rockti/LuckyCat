#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_JinChanManager.h"
#include <QLabel.h>
#include <QGridLayout.h>
#include <QDialog.h>
#include <QTableWidget.h>
#include <QTableView.h>
#include <QListWidget.h>
#include <QTimer.h>
#include <QQueue.h>
#include <QDateTime.h>
#include <deque>
#include <QStandardItemModel.h>

#include "ZTPClient.h"
#include "Widget/TipWidget.h"
#include "LoginDialog.h"
#include "Widget/ProgressIndicator.h"

class CommonHelper {
public:
	static void setStyle(const QString& style) {
		QFile qss(style);
		qss.open(QFile::ReadOnly);
		qApp->setStyleSheet(qss.readAll());
		qss.close();
	}
};


void blackTheme();
void whiteTheme();

class InvestorWidget : public QWidget {
	Q_OBJECT
public:
	InvestorWidget(QWidget* parent = nullptr);
	QLabel* InvestorLabel;
	QLabel* InvestorValueLabel;
	QLabel* NameLabel;
	QLabel* NameValueLabel;
	QLabel* StatusLabel;
	QLabel* StatusValueLabel;
	QLabel* BalanceLabel;
	QLabel* BalanceValueLabel;
	QLabel* AvailableLabel;
	QLabel* AvailableValueLabel;
	QLabel* DepositLabel;
	QLabel* DepositValueLabel;
	QLabel* WithdrawLabel;
	QLabel* WithdrawValueLabel;
	QLabel* MarginLabel;
	QLabel* MarginValueLabel;
	QLabel* FeeLabel;
	QLabel* FeeValueLabel;
	QLabel* FreezonMarginLabel;
	QLabel* FreezonMarginValueLabel;
	QLabel* FreezonFeeLabel;
	QLabel* FreezonFeeValueLabel;

	QPushButton* DepositButton;
	QPushButton* WithdrawButton;
	QTableView* positionTable;
	QStandardItemModel* posItemModel;
	QTableView* balanceChangeTable;
	QStandardItemModel* balanceItemModel;
	QTableView* storageChangeTable;
	QStandardItemModel* storageItemModel;
	QVBoxLayout* mainLayout;
	QGridLayout* gridLayout;
//	ProgressIndicator* progressIndicator;
	WaitingSpinnerWidget* progressIndicator;
	void clearInfo();
	void addBalanceChange(ztp::data::CBalanceChange const& change);
	void addStorageChange(ztp::data::CPositionChange const& change);
	void addPosition(ztp::data::CInvestorPosition const& pos);

	void clearBalance();
	void clearPosition();
	void clearStorage();

	void setInvestorID(ztp::type::InvestorID const& investorID) {
		investorID_ = investorID;
	}
	ztp::type::InvestorID const& getInvestorID() const {
		return investorID_;
	}
private:
	ztp::type::InvestorID investorID_;
};

class InvestorStore {
public:
private:
	ztp::data::CInvestorInfo m_info;
	ztp::data::CInvestorAccount m_account;
	std::deque<ztp::data::CInvestorPosition> m_positions;
	std::deque<ztp::data::CBalanceChange> m_balanceChanges;
	std::deque<ztp::data::CPositionChange> m_positionChanges;
};
class BrokerStore {
private:
	ztp::type::BrokerID m_brokerId;
	InvestorStore m_investors;
};

using message = ztp::message;
using RequestID = ztp::type::RequestID;
using TimeoutFunc = std::function<void(RequestID requestId)>;
using ResponseFunc = std::function<bool(message const& msg)>;
inline void DoNothingOfResponse(message const& msg) {

}
struct ResponseMatchCondition {
	RequestID RequestID;
	QDateTime ExpireTime;
	TimeoutFunc TimeoutHandler;
	ResponseFunc ResponseHandler;
};
class ResponseMatcher {
public:
	void Add(RequestID requestId, int timeout, TimeoutFunc timeoutHandler, ResponseFunc responseHandler) {
		auto time = QDateTime::currentDateTime();
		ResponseMatchCondition newCond{
			requestId,time.addSecs(timeout),timeoutHandler,responseHandler
		};
		m_matchers.emplace(requestId,newCond);
	}

	void OnRsp(message const& msg) {
		auto iter = m_matchers.find(msg.request_id());
		if (iter != m_matchers.end()) {
			bool pop = iter->second.ResponseHandler(msg);
			if (pop) {
				m_matchers.erase(iter);
			}
		}
	}

	void CheckTimeout() {
		auto time = QDateTime::currentDateTime();

		for (auto iter = m_matchers.begin(); iter != m_matchers.end();) {
			if (time > iter->second.ExpireTime) {
				iter->second.TimeoutHandler(iter->second.RequestID);
				iter = m_matchers.erase(iter);
			} else {
				++iter;
			}
		}
	}
private:
	std::map<RequestID, ResponseMatchCondition> m_matchers;
};

class QGrayLabel : public QLabel {
	Q_OBJECT
public:
	explicit QGrayLabel(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QLabel(parent, f) {
		setObjectName("grayLabel");
	};
	explicit QGrayLabel(const QString& text, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()) : QLabel(text, parent, f) {
		setObjectName("grayLabel");
	};
};

class JinChanManager : public QMainWindow, public ztp::TradeHandler
{
	Q_OBJECT

public:
	JinChanManager(QWidget *parent = Q_NULLPTR);

	QTabWidget* BrokerTab;
	QListWidget* InvestorList1;
	QListWidget* InvestorList2;
	InvestorWidget* investorWidget;
	TTipWidget* tipWidget;
//	ProgressIndicator* progressIndicator;
	WaitingSpinnerWidget* progressIndicator;
	LoginDialog* loginDialog = nullptr;


	ztp::ZTPTradeClient* client1 = nullptr;
	ztp::ZTPTradeClient* client2 = nullptr;

	virtual void OnConnected(ztp::type::BrokerID const& brokerID) override;
	virtual void OnConnectTimeout(ztp::type::BrokerID const& brokerID) override;
	virtual void OnDisconnected(ztp::type::BrokerID const& brokerID) override;
	virtual void OnRspLogin(ztp::data::CRspInfo const& rsp, ztp::data::CUserLogin const* data = nullptr) override;
	virtual void OnRspQueryInvestors(ztp::data::CRspInfo const& rsp, ztp::data::CInvestorInfo const* data = nullptr, bool isLast = true) override;
	virtual void OnRspQueryInvestorAccount(ztp::data::CRspInfo const& rsp, ztp::data::CInvestorAccount const* data = nullptr) override;
	virtual void OnRspQueryInvestorPosition(ztp::data::CRspInfo const& rsp, ztp::data::CInvestorPosition const* data = nullptr, bool isLast = true) override;
	virtual void OnRspChangeBalance(ztp::data::CRspInfo const& rsp, ztp::data::CInvestorAccount const* data = nullptr) override;
	virtual void OnRspQueryBalanceChange(ztp::data::CRspInfo const& rsp, ztp::data::CBalanceChange const* data = nullptr, bool isLast = true) override;
	virtual void OnOpRsp(message const& msg) override;
	virtual void OnAuthRsp(message const& msg) override;
private:
	Ui::JinChanManagerClass ui;
	QQueue<message> rspQueue_;
	int timerId_;
	ResponseMatcher m_matcher;
signals:
	void reloginSignal();
private:
	void balanceChangeConfirm(QString& investor, QString& name, int amount, ztp::type::BalanceChangeFlag changeType);
	void onChangeResponse(const ztp::message& msg, ztp::type::BalanceChangeFlag changeType);
	void popLogin();

	void initClient1(QString const& password);

	bool onLogined(message const& msg);

	bool onQueryInvestors(const message& msg);

	bool isInited = false;
	std::map<ztp::type::InvestorID, ztp::data::CInvestorInfo> m_investors1;
	void queryAccount(ztp::type::InvestorID& investorID);
	void updateAccount(ztp::data::CInvestorAccount& data);
	void queryBalanceChange(ztp::type::InvestorID& investorID);
	void queryStorageChange(ztp::type::InvestorID& investorID);
	void queryPosition(ztp::type::InvestorID& investorID);
private slots:
	void showEvent(QShowEvent* event);
	void onInvestorList1CurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);


	void on_deposit_button_clicked();
	void onBalanceChange(ztp::type::BalanceChangeFlag changeType);
	void on_withdraw_button_clicked();
	void timerEvent(QTimerEvent* event);
	void loginAction();
	void reloginSlot();
};
