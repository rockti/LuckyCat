#include "JinChanManager.h"
#include <QBoxLayout.h>
#include <QGridLayout.h>
#include <QValidator.h>
#include <QMessageBox.h>
#include "Config.h"
#include <QEvent.h>
#include <QInputDialog.h>
#include <QGroupBox.h>
#include <QheaderView.h>

#include "Utils.h"
#include "Widget/TitleBar.h"

using namespace ztp;


QTableView* newTableView(QWidget* parent, QStandardItemModel*& model, QStringList const& header) {
	auto* tableView = new QTableView(parent);
	tableView->verticalHeader()->hide();    //隐藏默认显示的行头
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows); //设置选中时整行选中
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	model = new QStandardItemModel();
	model->setColumnCount(header.size());
	model->setHorizontalHeaderLabels(header);
	tableView->setModel(model); 
	tableView->setFocusPolicy(Qt::NoFocus);

	return tableView;
}

void addOneRowOfGridLayout(QGridLayout* layout, int row, std::vector<QWidget*> const& widgets) {
	for (size_t i = 0; i < widgets.size(); i++) {
		layout->addWidget(widgets[i],row, i);

	}
}


InvestorWidget::InvestorWidget(QWidget* parent):QWidget(parent) {
	InvestorLabel = new QGrayLabel(ft("客户编码: "));
	InvestorValueLabel = new QGrayLabel(ft(""));
	NameLabel = new QGrayLabel(ft("客户名称: "));
	NameValueLabel = new QGrayLabel(ft(""));
	StatusLabel = new QGrayLabel(ft("客户状态: "));
	StatusValueLabel = new QGrayLabel(ft(""));
	BalanceLabel = new QGrayLabel(ft("初始权益: "));
	BalanceValueLabel = new QGrayLabel(ft(""));
	AvailableLabel = new QGrayLabel(ft("可用资金: "));
	AvailableValueLabel = new QGrayLabel(ft(""));
	DepositLabel = new QGrayLabel(ft("入金总额: "));
	DepositValueLabel = new QGrayLabel(ft(""));
	WithdrawLabel = new QGrayLabel(ft("出金总额:"));
	WithdrawValueLabel = new QGrayLabel(ft(""));

	MarginLabel = new QGrayLabel(ft("保证金: "));
	MarginValueLabel = new QGrayLabel(ft(""));
	FeeLabel = new QGrayLabel(ft("手续费:"));
	FeeValueLabel = new QGrayLabel(ft(""));
	FreezonMarginLabel = new QGrayLabel(ft("冻结保证金: "));
	FreezonMarginValueLabel = new QGrayLabel(ft(""));
	FreezonFeeLabel = new QGrayLabel(ft("冻结手续费:"));
	FreezonFeeValueLabel = new QGrayLabel(ft(""));


	DepositButton = new QPushButton(ft("入金"), this);
// 	QPixmap depositIcon("safe-in.png");
// 	DepositButton->setIcon(depositIcon);
	WithdrawButton = new QPushButton(ft("出金"), this);
// 	QPixmap withdrawIcon("safe-out.png");
// 	WithdrawButton->setIcon(withdrawIcon);
	mainLayout = new QVBoxLayout(this);
	gridLayout = new QGridLayout(this);
	gridLayout->setMargin(15);
	gridLayout->setSpacing(10);


	addOneRowOfGridLayout(gridLayout, 0,
		{ 
			InvestorLabel,InvestorValueLabel,
			NameLabel,NameValueLabel,
			BalanceLabel,BalanceValueLabel,
			AvailableLabel,AvailableValueLabel
		}
	);

	addOneRowOfGridLayout(gridLayout, 1,
		{
			DepositLabel,DepositValueLabel,
			WithdrawLabel,WithdrawValueLabel,
			MarginLabel,MarginValueLabel,
			FeeLabel,FeeValueLabel
		}
	);

	addOneRowOfGridLayout(gridLayout, 2,
		{
			FreezonMarginLabel,FreezonMarginValueLabel,
			FreezonFeeLabel,FreezonFeeValueLabel
		}
	);


	gridLayout->addWidget(DepositButton, 3, 6);
	gridLayout->addWidget(WithdrawButton, 3, 7);

	gridLayout->setSizeConstraint(QLayout::SetFixedSize);

	positionTable = newTableView(this, posItemModel, { ft("合约"), ft("买卖"),ft("今仓"),ft("今仓价"),ft("昨仓"),ft("昨仓价") });
	balanceChangeTable = newTableView(this, balanceItemModel, { ft("类型"), ft("金额"), ft("序号"), ft("时间") });
	storageChangeTable = newTableView(this,storageItemModel, { ft("合约"),ft("类型"), ft("手数"), ft("序号"), ft("时间") });
	auto* vbox = new QVBoxLayout(this);
	auto* hbox = new QHBoxLayout(this);
	hbox->addWidget(balanceChangeTable);
	hbox->addWidget(storageChangeTable);
	auto* groupBox = new QGroupBox(ft("出入金/出入库"));
	groupBox->setLayout(hbox);


	auto* hboxPosition = new QHBoxLayout(this);
	hboxPosition->addWidget(positionTable);
	auto* groupBoxPos = new QGroupBox(ft("持仓"));
	groupBoxPos->setLayout(hboxPosition);

	vbox->addWidget(groupBox);
	vbox->addWidget(groupBoxPos);
	vbox->setStretch(0, 2);
	vbox->setStretch(1, 3);


	mainLayout->addLayout(gridLayout);
	mainLayout->addLayout(vbox);
	mainLayout->setStretch(0, 1);
	mainLayout->setStretch(1, 6);
}


void InvestorWidget::clearBalance() {
	balanceItemModel->removeRows(0, balanceItemModel->rowCount());
}

void InvestorWidget::clearStorage() {
	storageItemModel->removeRows(0, storageItemModel->rowCount());
}
void InvestorWidget::clearPosition() {
	posItemModel->removeRows(0, posItemModel->rowCount());
}

void InvestorWidget::clearInfo() {
	InvestorValueLabel->setText("");
	NameValueLabel->setText("");
	StatusValueLabel->setText("");
	BalanceValueLabel->setText("");
	AvailableValueLabel->setText("");
	DepositValueLabel->setText("");
	WithdrawValueLabel->setText("");
	MarginValueLabel->setText("");
	FeeValueLabel->setText("");
	FreezonMarginValueLabel->setText("");
	FreezonFeeValueLabel->setText("");

	clearBalance();
	clearStorage();
	clearPosition();
}

QString desc(type::BalanceChangeFlag changeType) {
	switch (changeType) {
	case ztp::type::BalanceChangeFlag::Unknown:
		return ft("未知");
		break;
	case ztp::type::BalanceChangeFlag::Deposit:
		return ft("入金");
		break;
	case ztp::type::BalanceChangeFlag::Withdraw:
		return ft("出金");
		break;
	default:
		return ft("未知");
		break;
	}
}

QString desc(type::Direction dir) {
	switch (dir) {
	case ztp::type::Direction::Buy:
		return ft("买");
		break;
	case ztp::type::Direction::Sell:
		return ft("卖");
		break;
	default:
		return ft("未知");
		break;
	}
}

QString desc(type::OffsetFlag offsetFlag) {
	switch (offsetFlag) {
	case ztp::type::OffsetFlag::Open:
		return ft("开");
		break;
	case ztp::type::OffsetFlag::Close:
		return ft("平");
		break;
	case ztp::type::OffsetFlag::ForceClose:
		return ft("强平");
		break;
	case ztp::type::OffsetFlag::CloseToday:
		return ft("平今");
		break;
	case ztp::type::OffsetFlag::CloseYesterday:
		return ft("平昨");
		break;
	case ztp::type::OffsetFlag::Unknown:
		return ft("未知");
		break;
	default:
		return ft("未知");
		break;
	}
}

QString desc(type::PositionChangeFlag change) {
	switch (change) {
	case ztp::type::PositionChangeFlag::Unknown:
		return ft("未知");
		break;
	case ztp::type::PositionChangeFlag::Trade:
		return ft("成交");
		break;
	case ztp::type::PositionChangeFlag::Spot:
		return ft("出入库");
		break;
	case ztp::type::PositionChangeFlag::Yesterday:
		return ft("昨持仓");
		break;
	default:
		return ft("未知");
		break;
	}
}


template<uint8_t size, class Tag>
QString desc(StrongCharArray<size,Tag> const& v) {
	return ft(v.value());
}


template<class Base, class Tag>
QString desc(NumberType<Base, Tag> const& v) {
	return QString::number(v.value());
}


template<class Tag>
QString desc(DoubleType<Tag> const& v) {
	return QString::number(v.value(),'f',2);
}


void InvestorWidget::addBalanceChange(data::CBalanceChange const& change) {
	int row = balanceItemModel->rowCount();
	balanceItemModel->insertRow(balanceItemModel->rowCount());
	balanceItemModel->setItem(row, 0, new QStandardItem(desc(change.BalanceChangeType)));
	balanceItemModel->setItem(row, 1, new QStandardItem(QString::number(change.Amount, 'f', 2)));
	balanceItemModel->setItem(row, 2, new QStandardItem(QString::number(change.BalanceSequenceNo.value())));
	balanceItemModel->setItem(row, 3, new QStandardItem(ft(change.ChangeTime.value())));
	balanceItemModel->item(row, 2)->setData(change.BalanceSequenceNo.value(), Qt::DisplayRole);
}

void InvestorWidget::addStorageChange(data::CPositionChange const& change) {
	int row = storageItemModel->rowCount();
	storageItemModel->insertRow(storageItemModel->rowCount());
	QString type = type::IsOpen(change) ? ft("入库") : ft("出库");
	storageItemModel->setItem(row, 0, new QStandardItem(desc(change.InstrumentID)));
	storageItemModel->setItem(row, 1, new QStandardItem(type));
	storageItemModel->setItem(row, 2, new QStandardItem(QString::number(change.Volume)));
	storageItemModel->setItem(row, 3, new QStandardItem(QString::number(change.PositionSequenceNo)));
	storageItemModel->setItem(row, 4, new QStandardItem(ft(change.NotifyTime.value())));
	storageItemModel->item(row, 3)->setData(change.PositionSequenceNo.value(), Qt::DisplayRole);
}
void InvestorWidget::addPosition(ztp::data::CInvestorPosition const& pos) {
	int row = posItemModel->rowCount();
	posItemModel->insertRow(posItemModel->rowCount());
	posItemModel->setItem(row, 0, new QStandardItem(desc(pos.InstrumentID)));
	posItemModel->setItem(row, 1, new QStandardItem(desc(pos.Direction)));
	posItemModel->setItem(row, 2, new QStandardItem(desc(pos.Position)));
	posItemModel->setItem(row, 3, new QStandardItem(desc(pos.PositionCost)));
	posItemModel->setItem(row, 4, new QStandardItem(desc(pos.YdPosition)));
	posItemModel->setItem(row, 5, new QStandardItem(desc(pos.YdPositionPrice)));
}


JinChanManager::JinChanManager(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	auto* hbox = new QHBoxLayout(ui.centralWidget);
	BrokerTab = new QTabWidget(ui.centralWidget);
	InvestorList1 = new QListWidget(ui.centralWidget);
	BrokerTab->addTab(InvestorList1,ft("席位1"));
// 	InvestorList2 = new QListWidget(ui.centralWidget);
// 	BrokerTab->addTab(InvestorList2, ft("席位2"));
	investorWidget = new InvestorWidget(ui.centralWidget);
	hbox->addWidget(BrokerTab, 1);
	hbox->addWidget(investorWidget, 4);

	tipWidget = new TTipWidget(this);
	tipWidget->setVisible(false);
	timerId_ = startTimer(100);

//	progressIndicator = new ProgressIndicator(this);
	progressIndicator = new WaitingSpinnerWidget(this);
	investorWidget->progressIndicator = progressIndicator;
	investorWidget->WithdrawButton->setDisabled(true);
	investorWidget->DepositButton->setDisabled(true);
	loginDialog = new LoginDialog(ui.centralWidget);
	loginDialog->setModal(true);


	connect(InvestorList1, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
		this, SLOT(onInvestorList1CurrentItemChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(investorWidget->DepositButton, SIGNAL(clicked()), this, SLOT(on_deposit_button_clicked()));
	connect(investorWidget->WithdrawButton, SIGNAL(clicked()), this, SLOT(on_withdraw_button_clicked()));
	connect(this, SIGNAL(reloginSignal()), this, SLOT(reloginSlot()));
	connect(ui.loginAction, SIGNAL(triggered()), this, SLOT(loginAction()));

}

void JinChanManager::showEvent(QShowEvent* event) {

	QMainWindow::showEvent(event);
	if (event->spontaneous())
		return;

	if (isInited)
		return;
	isInited = true;
	popLogin();
	progressIndicator->setText(ft("测试等待"));
//	progressIndicator->start();
}
void JinChanManager::reloginSlot() {
	popLogin();
}
void JinChanManager::popLogin() {
	loginDialog->show();
//	loginDialog->exec();

}

void JinChanManager::initClient1(QString const& password) {
	auto const& config = Configs::Get();
	type::BrokerID brokerID1;
	brokerID1 = config.Broker1.ID.toStdString();
	data::CUserLogin loginReq;
	loginReq.BrokerID = brokerID1;
	loginReq.LoginID = config.User.toStdString();
	loginReq.UserType = type::UserType::Operator;
	loginReq.Password = password.toStdString();

	if (client1 != nullptr) {
		client1->SetServer(brokerID1, config.Server.toStdString(), config.Broker1.Port);
//		client1->Close();
		client1->Connect();
	} else {
		client1 = new ZTPTradeClient(brokerID1, config.Server.toStdString(), config.Broker1.Port, this);
		client1->Start();
	}
	client1->SetLogin(loginReq);
}

bool JinChanManager::onLogined(message const& msg)
{
	data::CRspInfo rsp;
	data::CUserLogin data;
	msg.retrieve(rsp, data);
	if (rsp.ErrorID == NoError.Code) {
		BrokerTab->setTabText(0, ft(client1->GetBroker()));
		InvestorList1->clear();
		auto id = client1->QueryInvestors();
		m_matcher.Add(id, 10, [&](RequestID id) {}, [&](message const& msg) -> bool {
			return onQueryInvestors(msg);
			});
		ui.statusBar->showMessage(ft("登录成功!正在初始化数据..."), 10000);
		loginDialog->Status = LoginDialog::Logined;
		ui.loginAction->setDisabled(true);
	} else {
		ui.statusBar->showMessage(ft("登录失败"));
		loginDialog->Status = LoginDialog::LoginFailed;
	}
	return true;
}

bool JinChanManager::onQueryInvestors(const message& msg) {
	data::CRspInfo rsp;
	data::CInvestorInfo data;
	msg.retrieve(rsp, data);
	if (rsp.ErrorID == NoError.Code) {
		QListWidgetItem* item = new QListWidgetItem(InvestorList1);
		item->setData(Qt::UserRole, data.InvestorID.value());
		item->setText(ft(data.InvestorID.value()) + "-" + ft(data.InvestorName.value()));
		InvestorList1->addItem(item);
		m_investors1.emplace(data.InvestorID, data);
		return msg.is_last();
	} else {
		ui.statusBar->showMessage(ft("初始化客户失败"));
	}
	return true;
}

void JinChanManager::onInvestorList1CurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous) {
	investorWidget->clearInfo();
	auto data = current->data(Qt::UserRole);
	type::InvestorID investorID;
	investorID = data.toString().toStdString();
	auto iter = m_investors1.find(investorID);
	if (iter != m_investors1.end()) {
		investorWidget->InvestorValueLabel->setText(ft(iter->second.InvestorID));
		investorWidget->NameValueLabel->setText(ft(iter->second.InvestorName));
		investorWidget->setInvestorID(investorID);
		investorWidget->DepositButton->setDisabled(false);
		investorWidget->WithdrawButton->setDisabled(false);
		queryAccount(investorID);
 		queryBalanceChange(investorID);
 		queryStorageChange(investorID);
		queryPosition(investorID);
		return;
	} else {
		investorWidget->clearInfo();
	}
}

void JinChanManager::queryBalanceChange(type::InvestorID& investorID) {
	auto reqId = client1->QueryBalanceChange(investorID);
	investorWidget->clearBalance();

	m_matcher.Add(reqId, 5,
		[&](type::RequestID id) {
			tipWidget->SetMesseage(ft("查询出入金失败"));
		},
		[&](message const& msg)->bool {
			data::CRspInfo rsp;
			Op::QueryInvestorBalanceChange::ResponseType data;
			auto* pData = msg.retrieve(rsp, data);
			tipWidget->setParent(investorWidget);
			tipWidget->setVisible(false);
			if (rsp.ErrorID == NoError.Code ) {
				if (pData != nullptr) investorWidget->addBalanceChange(data);
				if (msg.is_last()) {
					investorWidget->balanceItemModel->sort(2, Qt::DescendingOrder);
					investorWidget->balanceChangeTable->resizeColumnsToContents();
				}
			} else {
				tipWidget->SetMesseage(ft("查询客户[%1]资金失败 - 原因[%2-%3]").arg(investorID.value(), QString::number(rsp.ErrorID), ft(rsp.ErrorMsg)));;  //提示信息
			}
			return msg.is_last();

		});
}

void JinChanManager::queryStorageChange(type::InvestorID& investorID) {
	auto reqId = client1->QueryPositionChange(investorID);
	investorWidget->clearStorage();

	m_matcher.Add(reqId, 5,
		[&](type::RequestID id) {
			tipWidget->SetMesseage(ft("查询出入库失败"));
		},
		[&](message const& msg)->bool {
			data::CRspInfo rsp;
			Op::QueryInvestorPositionChange::ResponseType data;
			auto* pData = msg.retrieve(rsp, data);
			tipWidget->setParent(investorWidget);
			tipWidget->setVisible(false);
			if (rsp.ErrorID == NoError.Code) {
				if (pData != nullptr) investorWidget->addStorageChange(data);
				if (msg.is_last()) {
					investorWidget->storageItemModel->sort(3, Qt::DescendingOrder);
					investorWidget->storageChangeTable->resizeColumnsToContents();
				}
			} else {
				tipWidget->SetMesseage(ft("查询客户[%1]出入库失败 - 原因[%2-%3]").arg(investorID.value(), QString::number(rsp.ErrorID), ft(rsp.ErrorMsg)));;  //提示信息
			}
			return msg.is_last();

		});
}

void JinChanManager::queryPosition(type::InvestorID& investorID) {
	auto reqId = client1->QueryPosition(investorID);
	m_matcher.Add(reqId, 5,
		[&](type::RequestID id) {
			tipWidget->SetMesseage(ft("查询持仓失败"));
		},
		[&](message const& msg)->bool {
			data::CRspInfo rsp;
			Op::QueryInvestorPosition::ResponseType data;
			auto* pData = msg.retrieve(rsp, data);
			tipWidget->setParent(investorWidget);
			tipWidget->setVisible(false);
			if (rsp.ErrorID == NoError.Code) {
				if (pData) investorWidget->addPosition(data);
			} else {
				tipWidget->SetMesseage(ft("查询客户[%1]资金失败 - 原因[%2-%3]").arg(investorID.value(), QString::number(rsp.ErrorID), ft(rsp.ErrorMsg)));;  //提示信息
			}
			return msg.is_last();

		});
}

void JinChanManager::queryAccount(type::InvestorID& investorID) {
	auto reqId = client1->QueryAccount(investorID);
	m_matcher.Add(reqId, 2,
		[&](type::RequestID id) {
			tipWidget->SetMesseage(ft("查询资金失败"));
		},
		[&](message const& msg)->bool {
			data::CRspInfo rsp;
			Op::QueryInvestorAccount::ResponseType data;
			msg.retrieve(rsp, data);
			tipWidget->setParent(investorWidget);
			tipWidget->setVisible(false);
			if (rsp.ErrorID == NoError.Code) {
				updateAccount(data);
			} else {
				tipWidget->SetMesseage(ft("查询客户[%1]资金失败 - 原因[%2-%3]").arg(investorID.value(), QString::number(rsp.ErrorID), ft(rsp.ErrorMsg)));;  //提示信息
			}
			return true;

		});
}

void JinChanManager::updateAccount(ztp::data::CInvestorAccount& data) {
	investorWidget->BalanceValueLabel->setText(QString::number(data.PreBalance.value(), 'f', 2));
	investorWidget->AvailableValueLabel->setText(QString::number(data.Available.value(), 'f', 2));
	investorWidget->DepositValueLabel->setText(QString::number(data.Deposit.value(), 'f', 2));
	investorWidget->WithdrawValueLabel->setText(QString::number(data.Withdraw.value(), 'f', 2));
	investorWidget->MarginValueLabel->setText(QString::number(data.Margin.value(), 'f', 2));
	investorWidget->FeeValueLabel->setText(QString::number(data.Fee.value(), 'f', 2));
	investorWidget->FreezonMarginValueLabel->setText(QString::number(data.FrozenMargin.value(), 'f', 2));
	investorWidget->FreezonFeeValueLabel->setText(QString::number(data.FrozenFee.value(), 'f', 2));
}

void JinChanManager::on_deposit_button_clicked() {
	onBalanceChange(type::BalanceChangeFlag::Deposit);
}

void JinChanManager::onBalanceChange(ztp::type::BalanceChangeFlag changeType) {
	auto name = investorWidget->NameValueLabel->text();
	auto investor = investorWidget->InvestorValueLabel->text();
	bool ok = false;
	int amount = QInputDialog::getInt(investorWidget,
		ft("客户[%1]%2").arg(name, desc(changeType)),
		ft("客户[%1-%2]%3金额").arg(investor, name, desc(changeType)),
		0, 0, 2000000000, 2, &ok);
	if (ok) {
		if (amount > 1) {
			balanceChangeConfirm(investor, name, amount, changeType);
		} else {
			QMessageBox::warning(this, desc(changeType), ft("%1金额[%2]不正确").arg(desc(changeType), QString::number(amount)));
		}

	}
}

void JinChanManager::balanceChangeConfirm(QString& investor, QString& name, int amount, type::BalanceChangeFlag changeType) {
 	DigitalToChinese dtc;
 	QString cnNum = dtc.toChinese(amount);
	auto result = QMessageBox::question(this, 
		ft("%1确认").arg(desc(changeType)), 
		ft("确定对客户[%1] %2[%3] \n %4").arg(investor + "-" + name, desc(changeType),QString::number(amount, 'f', 2), cnNum));
	if (result == QMessageBox::Yes) {
		progressIndicator->start(ft("正在%s...").arg(desc(changeType)));
		auto reqId = changeType == type::BalanceChangeFlag::Deposit ? 
			client1->Deposit(investorWidget->getInvestorID(), amount) :
			client1->Withdraw(investorWidget->getInvestorID(), amount);

		m_matcher.Add(reqId, 10, 
			[&,changeType](type::RequestID reqId) {
				progressIndicator->stop();
				tipWidget->setParent(investorWidget);
				tipWidget->SetMesseage(ft("%1失败，超时").arg(desc(changeType)));
			},
			[&,changeType](message const& msg)->bool { 
				progressIndicator->stop();
				onChangeResponse(msg,changeType);
				return true;
			}
		);
	}
}

void JinChanManager::onChangeResponse(const ztp::message& msg, type::BalanceChangeFlag changeType) {
	data::CRspInfo rsp;
	Op::ChangeInvestorBalance::ResponseType data;
	msg.retrieve(rsp, data);
	tipWidget->setParent(investorWidget);
	tipWidget->setVisible(false);
	if (rsp.ErrorID == NoError.Code) {
		tipWidget->SetMesseage(ft("向客户[%1]出%2成功").arg(ft(data.InvestorID.value()),desc(changeType)));  //提示信息
		updateAccount(data);
	} else {
		tipWidget->SetMesseage(ft("向客户[%1]出入金失败 - 原因[%2-%3]").arg(QString::number(rsp.ErrorID), ft(rsp.ErrorMsg)));;  //提示信息
	}
	queryBalanceChange(data.InvestorID);
}

void JinChanManager::on_withdraw_button_clicked() {
	onBalanceChange(type::BalanceChangeFlag::Withdraw);
}


void JinChanManager::timerEvent(QTimerEvent* event) {
	if (event->timerId() != timerId_) return;

	for (size_t i = 0; i < 20; i++) {
		if (!rspQueue_.isEmpty()) {
			message msg = rspQueue_.head();
			rspQueue_.pop_front();
			m_matcher.OnRsp(msg);
		}
	}
	m_matcher.CheckTimeout();

	if (loginDialog != nullptr) {
		auto result = loginDialog->result();
		if (loginDialog->Status == LoginDialog::StartLogin) {
			initClient1(loginDialog->Password);
			auto requestId = client1->Login();
			loginDialog->Status = LoginDialog::Logining;
			ui.statusBar->showMessage(ft("正在登录..."));

			m_matcher.Add(requestId, 10, [&](type::RequestID requestID) {
				ui.statusBar->showMessage(ft("登录超时，请检查网络及服务配置"));
				loginDialog->Status = LoginDialog::LoginFailed;
			},
			[&](message const& msg)->bool {
				return onLogined(msg);
			});
		}
	}
}

void JinChanManager::loginAction() {
//	CommonHelper::setStyle("style.qss");
	reloginSlot();
}

void JinChanManager::OnConnected(type::BrokerID const& brokerID) {
	ui.statusBar->showMessage(ft("已连接..."));
//	ui.menuBar->setDisabled(true);
}

void JinChanManager::OnConnectTimeout(type::BrokerID const& brokerID) {
	ui.statusBar->showMessage(ft("连接超时"));
	//	ui.menuBar->setDisabled(true);
}

void JinChanManager::OnDisconnected(type::BrokerID const& brokerID) {
	ui.statusBar->showMessage(ft("连接已断开，请检查服务器并重新登录"));
//	ui.menuBar->setDisabled(false);
}

void JinChanManager::OnRspLogin(data::CRspInfo const& rsp, data::CUserLogin const* data) {
}

void JinChanManager::OnRspQueryInvestors(data::CRspInfo const& rsp, data::CInvestorInfo const* data, bool isLast) {

}

void JinChanManager::OnRspQueryInvestorAccount(data::CRspInfo const& rsp, data::CInvestorAccount const* data) {
}

void JinChanManager::OnRspQueryInvestorPosition(data::CRspInfo const& rsp, data::CInvestorPosition const* data, bool isLast) {
}

void JinChanManager::OnRspChangeBalance(data::CRspInfo const& rsp, data::CInvestorAccount const* data) {
}

void JinChanManager::OnRspQueryBalanceChange(data::CRspInfo const& rsp, data::CBalanceChange const* data, bool isLast) {
}

void JinChanManager::OnOpRsp(message const& msg) {
	rspQueue_.append(msg);
}

void JinChanManager::OnAuthRsp(message const& msg) {
	rspQueue_.append(msg);
}


void blackTheme() {
	CommonHelper::setStyle("style.qss");
}

void whiteTheme() {
	CommonHelper::setStyle("style.white.qss");
}
