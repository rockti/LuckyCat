#include "JinDou.h"
#include "QHeaderView"
#include "QButtonGroup"
#include "Define.h"
#include "service/Service.h"
#include "presenter/AccountPresenter.h"
#include "presenter/OrderPresenter.h"
#include "presenter/StatusPresenter.h"
#include "presenter/PositionPresenter.h"
#include "presenter/StrategyListPresenter.h"
#include "view/AccountView.h"
#include "view/OrderListView.h"
#include "view/StatusView.h"
#include "view/TradeListView.h"
#include "view/PositionView.h"
#include "view/OrderView.h"
#include "view/LoginView.h"
#include "view/StrategyListView.h"
#include "runner/Framework.h"
#include "utils/String.h"
#include "notify/Notify.h"
#include "notify/UIPublisher.h"
#include "LoginDlg.h"
#include "StrategySettingDlg.h"
#include "Dispatcher.h"
#include <QTimer>
#include <QDialogButtonBox>
using namespace ui;
extern const char* Version;
inline QString TT(char const* str) {
	return QString::fromLocal8Bit(str);
}
inline QString TT(std::string const& str) {
	return QString::fromLocal8Bit(str.c_str());
}

class ChangePasswordDialog : public QDialog {
public:
	ChangePasswordDialog(QWidget* parent) : QDialog(parent) {
		QFormLayout* lytMain = new QFormLayout(this);
		QLabel* passwordLabel = new QLabel(TT("现有密码:"), this);
		passwordEdit_ = new QLineEdit(this);
		passwordEdit_->setEchoMode(QLineEdit::Password);
		lytMain->addRow(passwordLabel, passwordEdit_);
		QLabel* newPasswordLabel = new QLabel(TT("新的密码:"), this);
		newPasswordEdit_ = new QLineEdit(this);
		newPasswordEdit_->setEchoMode(QLineEdit::Password);
		lytMain->addRow(newPasswordLabel, newPasswordEdit_);
		QLabel* confirmPasswordLabel = new QLabel(TT("确认密码:"), this);
		confimPasswordEdit_ = new QLineEdit(this);
		confimPasswordEdit_->setEchoMode(QLineEdit::Password);
		lytMain->addRow(confirmPasswordLabel, confimPasswordEdit_);
		QDialogButtonBox* buttonBox = new QDialogButtonBox
		(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
			Qt::Horizontal, this);
		buttonBox->button(QDialogButtonBox::Ok)->setText(TT("修改"));
		buttonBox->button(QDialogButtonBox::Cancel)->setText(TT("取消"));
		lytMain->addWidget(buttonBox);
		bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
			this, &ChangePasswordDialog::accept);
		Q_ASSERT(conn);
		conn = connect(buttonBox, &QDialogButtonBox::rejected,
			this, &ChangePasswordDialog::reject);
		Q_ASSERT(conn);

		this->setWindowTitle(TT("修改黄金系统密码"));

		setLayout(lytMain);
		this->startTimer(100, Qt::PreciseTimer);
	}

	void SetView(AccountView* view) { accountView_ = view; }

protected:
	const int CHANGEPASSWORD_TIMEOUT = 500;
	void timerEvent(QTimerEvent* event) {
		if (changed_ && elapsedTimer_.isValid() && elapsedTimer_.hasExpired(CHANGEPASSWORD_TIMEOUT)) {
			QApplication::restoreOverrideCursor();
			auto error = accountView_->LastPasswordChangeError();
			if (error) {
				QMessageBox::information(this, TT("提醒"), TT("密码修改成功"), TT("确认"));
				QDialog::accept();
			} else {
				changed_ = false;
				auto message = fmt::format("密码修改失败[{}]", error.Msg);
				QMessageBox::warning(this, TT("警告"), TT(message), TT("确认"));
			}
			elapsedTimer_.invalidate();
		}
	}

private slots:
	virtual void accept() {
		auto password = passwordEdit_->text();
		auto newPassword = newPasswordEdit_->text();
		auto confimPassword = confimPasswordEdit_->text();

		if (password.isEmpty()) {
			QMessageBox::information(this, TT("提醒"), TT("密码不能为空"), TT("确认"));
			return;
		}

		if (newPassword.isEmpty()) {
			QMessageBox::information(this, TT("提醒"), TT("新密码不能为空"), TT("确认"));
			return;
		}
		if (confimPassword.isEmpty()) {
			QMessageBox::information(this, TT("提醒"), TT("确认密码不能为空"), TT("确认"));
			return;
		}

		if (confimPassword != newPassword) {
			QMessageBox::information(this, TT("提醒"), TT("密码不一致"), TT("确认"));
			return;
		}
		ServiceSinglton::Services().TradeSystem->ChangePassword(
			TradePlatform::ZTP,
			type::Password{ password.toLocal8Bit() },
			type::Password{ newPassword.toLocal8Bit() });
		changed_ = true;
		elapsedTimer_.start();
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	}
private:
	QLineEdit* passwordEdit_;
	QLineEdit* newPasswordEdit_;
	QLineEdit* confimPasswordEdit_;
private:
	bool changed_ = false;
	AccountView* accountView_;
	QElapsedTimer elapsedTimer_;
};

JinDou::JinDou(QWidget* parent)
	: QMainWindow(parent) {
    ui.setupUi(this);
	setWindowIcon(QIcon(":/JinDou/JinDou.ico"));
    //删除中央区域
    QWidget* pCenterWidget = takeCentralWidget();
    if (NULL != pCenterWidget) {
        delete pCenterWidget;
        pCenterWidget = NULL;
    }

    InitDocks();
	InitRadios();
	InitWidgets();
	InitComboxItems();
	InitStatusBar();
	InitSlot();
	InitPresenterAndService();
	SetTablesStyle();

	this->startTimer(1, Qt::PreciseTimer);
	localIdSaveTimer_.start(); //保存LocalId到文件的timer,1秒写一次文件
	QTimer::singleShot(100, this, SLOT(login()));
}

void JinDou::login() {
	std::unique_ptr<LoginDlg> dlg(new LoginDlg(this));
	std::unique_ptr<LoginView> loginView(new LoginView(*dlg));
	std::unique_ptr<LoginPresenter> loginPresenter(new LoginPresenter(loginView.get()));
	ServiceSinglton::Services().Init->Register(loginPresenter.get());
	ServiceSinglton::Services().TradeSystem->Register(loginPresenter.get());
	dlg->SetView(loginView.get());

	if (dlg->exec() == QDialog::Accepted) {
		orderView_->AddInstruments();
		auto majorInst = ui.majorInstComboBox->currentText();
		orderView_->AddMinorInstrument(majorInst);
	} else {
		loginFailed_ = true;
	}
	auto const& config = ConfigLoader::Get();
	auto title = fmt::format("JinDou {} - 客户号[{}]", Version, CustomizedPersistService::Instance().GetSavedInvestor(TradePlatform::ZTP).TradeInvestor);
	if (config.UserMode == UserMode::Multiple) {
		auto const& trader = CustomizedPersistService::Instance().GetTraderNumber();
		title = fmt::format("JinDou {} - 客户号[{}] 交易员号[{}]", Version,trader.Investor, trader.TraderNubmer);
	}
	this->setWindowTitle(QString::fromLocal8Bit(title.c_str()));

	ServiceSinglton::Services().Init->Unregister(loginPresenter.get());
	ServiceSinglton::Services().TradeSystem->Unregister(loginPresenter.get());
	setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
}

void JinDou::changePassword() {
	ChangePasswordDialog dlg(this);
	dlg.SetView(accountView_);
	dlg.exec();
}

inline void JinDou::InitPresenterAndService() {
	auto& services = ServiceSinglton::Services();
	orderView_ = new OrderView(*this);
	auto* orderPresenter = new OrderPresenter(orderView_);
	accountView_ = new AccountView(*this);
	auto* accountPresenter = new AccountPresenter(accountView_);
	auto* statusView = new StatusView(*this);
	auto* statusPresenter = new StatusPresenter(statusView);
	orderListView_ = new OrderListView(*this);
	auto* orderListPresenter = new OrderListPresenter(orderListView_);
	tradeListView_ = new TradeListView(*this);
	auto* tradeListPresenter = new TradeListPresenter(tradeListView_);
	positionView_ = new PositionView(*this);
	auto* positionPresenter = new PositionPresenter(positionView_);
	strategyListView_ = new StrategyListView(*this);
	auto* strategyListPresenter = new StrategyListPresenter(strategyListView_);

	ServiceSinglton::InitQuoteService(orderPresenter);
	ServiceSinglton::InitAccountService(accountPresenter);
	ServiceSinglton::InitPositionService(positionPresenter);
	ServiceSinglton::InitOrderService(orderListPresenter);
	ServiceSinglton::InitTradeService(tradeListPresenter);
	ServiceSinglton::InitTradeSystemService(statusPresenter, statusPresenter);
	ServiceSinglton::InitInstrumentService(orderPresenter);
	ServiceSinglton::InitExchangeService();
	ServiceSinglton::InitQueryService();
	ServiceSinglton::InitInitService();
	ServiceSinglton::InitStrategyService(strategyListPresenter);
	ServiceSinglton::Services().TradeSystem->Register((ITradeSystemObserver*)accountPresenter);

	for (auto const& config : ConfigLoader::Get().TradeSystemConfigs) {
		auto codes = split(config.Varieties, ",");
		std::set<type::VarietyCode> vcodes;
		for (auto const& code : codes) {
			vcodes.insert(type::VarietyCode(code.c_str()));
		}
		ServiceSinglton::Services().Instrument->SetVarieties(config.Platform, vcodes);
	}
	Framework::Instance().Start(ConfigLoader::Get());
}

void JinDou::timerEvent(QTimerEvent* event) {
	if (loginFailed_) {
		close();	//构造函数无法触发close信号，只能到这里来做
		this->killTimer(event->timerId());
	}

	// get data from queue, dispatch to services
	static Dispatcher s_d;
	Data data;
	
	while (Framework::Instance().GetUIPublisher().Pop(data)) {
		s_d.Dispatch(data);
	}
	ServiceSinglton::Services().TradeSystem->RunQuery();

	if (localIdSaveTimer_.hasExpired(1000)) {
		LocalIdACC::SaveToFile();
		localIdSaveTimer_.restart();
	}
}

void JinDou::closeEvent(QCloseEvent* event) {
	if (QMessageBox::question(this, QStringLiteral("确认"), QStringLiteral("确定要退出Jindou吗？"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
		event->accept();
	} else {
		event->ignore();
	}
}

void JinDou::UpdatePositionProfit(TradePlatform platform, double positionProfit) {
	accountView_->UpdatePositionProfit(platform, positionProfit);
}
void JinDou::InitDocks() {
	//去除资金窗口的Title
	QWidget* titleWidget = ui.accountDockWidget->titleBarWidget();
	ui.accountDockWidget->setTitleBarWidget(new QWidget);
	delete titleWidget;

	setDockNestingEnabled(true);	//允许dock嵌套

    m_docksList[FUNDS_DOCK] = ui.accountDockWidget;         //资金窗口
	m_docksList[ORDER_DOCK] = ui.orderDockWidget;           //委托窗口
	m_docksList[TRADE_DOCK] = ui.tradeListDockWidget;       //成交窗口
	m_docksList[DEALING_DOCK] = ui.orderListDockWidget;     //回报窗口
	m_docksList[POSITION_DOCK] = ui.positionDockWidget;     //持仓窗口
  	m_docksList[STRATEGY_DOCK] = ui.strategyDockWidget;     //策略窗口

	addDockWidget(Qt::TopDockWidgetArea, ui.accountDockWidget);	
	tabifyDockWidget(ui.orderListDockWidget, ui.strategyDockWidget);
	ui.orderListDockWidget->raise();
	addDockWidget(Qt::BottomDockWidgetArea, ui.positionDockWidget);
	splitDockWidget(ui.positionDockWidget, ui.tradeListDockWidget, Qt::Horizontal);

// 	ui.positionTableWidget->horizontalHeader()->setSortIndicatorShown(true);		//显示排序图标（默认为上下箭头）
// 	ui.positionTableWidget->horizontalHeader()->setSortIndicator(0, Qt::SortOrder::AscendingOrder);	//设置第0列 升序排序	
// 	ui.positionTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::up-arrow { subcontrol-position: center right; padding-right: 8px;"
// 		"image: url(:/images/sort_up.png);}"
// 		"QHeaderView::down-arrow { subcontrol-position: center right; padding-right: 8px;"
// 		"image: url(:/images/sort_down.png);}");

// 	addDockWidget(Qt::TopDockWidgetArea, ui.accountDockWidget);	
// 	splitDockWidget(ui.accountDockWidget, ui.orderListDockWidget, Qt::Vertical);	
// 	splitDockWidget(ui.strategyDockWidget, ui.orderDockWidget, Qt::Vertical);
// 	splitDockWidget(ui.orderDockWidget, ui.strategyDockWidget, Qt::Horizontal);
// 	addDockWidget(Qt::BottomDockWidgetArea, ui.positionDockWidget);
// 	splitDockWidget(ui.positionDockWidget, ui.tradeListDockWidget, Qt::Horizontal);

    ShowDock();
 }
void JinDou::InitRadios() {
	m_orderRadios = new QButtonGroup(this);
	m_orderRadios->addButton(ui.normalRadioButton, 0);
	m_orderRadios->addButton(ui.combineRadioButton, 1);
	ui.normalRadioButton->setChecked(true);

	m_orderListRadios = new QButtonGroup(this);
	m_orderListRadios->addButton(ui.totalRadioButton, (int)ui::OrderViewModel::All);
	m_orderListRadios->addButton(ui.pendingRadioButton, (int)ui::OrderViewModel::Pending);
	m_orderListRadios->addButton(ui.tradedRadioButton, (int)ui::OrderViewModel::Traded);
	m_orderListRadios->addButton(ui.canceledRadioButton, (int)ui::OrderViewModel::Cancelled);
	ui.totalRadioButton->setChecked(true);

	m_tradeListRadios = new QButtonGroup(this);
	m_tradeListRadios->addButton(ui.tradeDetailRadioButton, 0);
	m_tradeListRadios->addButton(ui.tradeSummaryRadioButton, 1);
	ui.tradeDetailRadioButton->setChecked(true);
}
void JinDou::InitWidgets() {
	ui.minorOrderGroupBox->setEnabled(false);
	ui.minorQuoteTableWidget->setEnabled(false);
	ui.combineOrderPushButton->setEnabled(false);
	ui.directionCheckBox->setEnabled(false);
	ui.operationCheckBox->setEnabled(false);
	ui.spreadTxtLabel->setVisible(false);
	ui.spreadLabel->setVisible(false);
}
void JinDou::InitComboxItems() {
	ui.majorDirComboBox->insertItem(0, QStringLiteral("买入"));
	ui.majorDirComboBox->insertItem(1, QStringLiteral("卖出"));
	ui.minorDirComboBox->insertItem(0, QStringLiteral("买入"));
	ui.minorDirComboBox->insertItem(1, QStringLiteral("卖出"));
	ui.majorOptComboBox->insertItem(0, QStringLiteral("开仓"));
	ui.majorOptComboBox->insertItem(1, QStringLiteral("平仓"));
	ui.minorOptComboBox->insertItem(0, QStringLiteral("开仓"));
	ui.minorOptComboBox->insertItem(1, QStringLiteral("平仓"));

	ui.majorPriceTypeComboBox->insertItem(0, QStringLiteral("普通"));
	ui.majorPriceTypeComboBox->insertItem(1, QStringLiteral("FAK"));
	ui.minorPriceTypeComboBox->insertItem(0, QStringLiteral("普通"));
	ui.minorPriceTypeComboBox->insertItem(1, QStringLiteral("FAK"));
}
void JinDou::InitStatusBar() {
	m_sgeQuoteStatus = new QLabel(this);
	m_sgeQuoteStatus->setText(QStringLiteral("现货行情"));
	m_sgeQuoteStatus->setStyleSheet("font-size:22px;color:red;font-family:幼圆");
	m_sgeTradeStatus = new QLabel(this);
	m_sgeTradeStatus->setText(QStringLiteral("现货交易"));
	m_sgeTradeStatus->setStyleSheet("font-size:22px;color:red;font-family:幼圆");
	m_sgeExchangeStatus = new QLabel(this);
	m_sgeExchangeStatus->setText(QStringLiteral("[未知]	"));
	m_sgeExchangeStatus->setStyleSheet("font-size:22px;color:blue;font-family:幼圆");

	m_shfeQuoteStatus = new QLabel(this);
	m_shfeQuoteStatus->setText(QStringLiteral("期货行情"));
	m_shfeQuoteStatus->setStyleSheet("font-size:22px;color:red;font-family:幼圆");
	m_shfeTradeStatus = new QLabel(this);
	m_shfeTradeStatus->setText(QStringLiteral("期货交易"));
	m_shfeTradeStatus->setStyleSheet("font-size:22px;color:red;font-family:幼圆");
	m_shfeExchangeStatus = new QLabel(this);
	m_shfeExchangeStatus->setText(QStringLiteral("[未知]	"));
	m_shfeExchangeStatus->setStyleSheet("font-size:22px;color:blue;font-family:幼圆");

	m_esunneyQuoteStatus = new QLabel(this);
	m_esunneyQuoteStatus->setText(QStringLiteral("外盘行情"));
	m_esunneyQuoteStatus->setStyleSheet("font-size:22px;color:red;font-family:幼圆");
	m_esunneyTradeStatus = new QLabel(this);
	m_esunneyTradeStatus->setText(QStringLiteral("外盘交易"));
	m_esunneyTradeStatus->setStyleSheet("font-size:22px;color:red;font-family:幼圆");
	m_esunneyExchangeStatus = new QLabel(this);
	m_esunneyExchangeStatus->setText(QStringLiteral("[未知]"));
	m_esunneyExchangeStatus->setStyleSheet("font-size:22px;color:blue;font-family:幼圆");


	ui.statusBar->addPermanentWidget(m_sgeQuoteStatus);
	ui.statusBar->addPermanentWidget(m_sgeTradeStatus);
	ui.statusBar->addPermanentWidget(m_sgeExchangeStatus);
	ui.statusBar->addPermanentWidget(m_shfeQuoteStatus);
	ui.statusBar->addPermanentWidget(m_shfeTradeStatus);
	ui.statusBar->addPermanentWidget(m_shfeExchangeStatus);
	ui.statusBar->addPermanentWidget(m_esunneyQuoteStatus);
	ui.statusBar->addPermanentWidget(m_esunneyTradeStatus);
	ui.statusBar->addPermanentWidget(m_esunneyExchangeStatus);

	m_errorMsgLabel = new QLabel(this);
	m_errorMsgLabel->setIndent(10);
	m_errorMsgLabel->setStyleSheet("font-size:25px;color:blue");
	ui.statusBar->addWidget(m_errorMsgLabel);
	m_errorMsgLabel->setText(QStringLiteral("上海廉道科技有限公司"));
}

void JinDou::RemoveAllDocks() {
    for (std::map<int, QDockWidget*>::iterator iter=m_docksList.begin(); iter!=m_docksList.end(); ++iter) {
        iter->second->hide();
    }
}
void JinDou::ShowDock(int const dockIndex) {
	if (0 == dockIndex) {
		for (std::map<int, QDockWidget*>::iterator iter = m_docksList.begin(); iter != m_docksList.end(); ++iter) {
			iter->second->show();
		}
	}
	else {
		m_docksList[dockIndex]->show();
	}

	auto const& config = ConfigLoader::Get();
	if (UIMode::HasStrategy != config.UIMode) {
		ui.strategyDockWidget->hide();
	}
}
void JinDou::SetTablesStyle() {
    //允许改变列头位置
    ui.accountTableWidget->horizontalHeader()->setSectionsMovable(true);
	ui.strategyTableWidget->horizontalHeader()->setSectionsMovable(true);
	ui.orderListTableWidget->horizontalHeader()->setSectionsMovable(true);
	ui.positionTableWidget->horizontalHeader()->setSectionsMovable(true);
	ui.tradeListTableWidget->horizontalHeader()->setSectionsMovable(true);

    //自动完成列头填空
	ui.accountTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.majorQuoteTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.minorQuoteTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.strategyTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.orderListTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.positionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui.tradeListTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	//删除行号
	ui.accountTableWidget->verticalHeader()->setHidden(true);
	ui.strategyTableWidget->verticalHeader()->setHidden(true);
	ui.orderListTableWidget->verticalHeader()->setHidden(true);
	ui.positionTableWidget->verticalHeader()->setHidden(true);
	ui.tradeListTableWidget->verticalHeader()->setHidden(true);
}

void JinDou::InitSlot() {
	connect(ui.changePasswordMenuAction, SIGNAL(triggered()), this, SLOT(changePassword()));

	connect(ui.normalRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnOrderRadiosToggled()));
	connect(ui.tradeDetailRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnTradeListRadiosToggled()));

	connect(ui.totalRadioButton, SIGNAL(clicked(bool)), this, SLOT(OnOrderListRadiosChecked()));
	connect(ui.pendingRadioButton, SIGNAL(clicked(bool)), this, SLOT(OnOrderListRadiosChecked()));
	connect(ui.tradedRadioButton, SIGNAL(clicked(bool)), this, SLOT(OnOrderListRadiosChecked()));
	connect(ui.canceledRadioButton, SIGNAL(clicked(bool)), this, SLOT(OnOrderListRadiosChecked()));

	connect(ui.priceCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnPriceCheckBoxStateChanged(int)));
	connect(ui.directionCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnDirectionCheckBoxStateChanged(int)));
	connect(ui.operationCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnOperationCheckBoxStateChanged(int)));

	connect(ui.majorInstComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnMajorInstComboBoxChanged(QString)));
	connect(ui.minorInstComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnMinorInstComboBoxChanged(int)));
	connect(ui.majorDirComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDirComboBoxChanged(int)));
	connect(ui.minorDirComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDirComboBoxChanged(int)));
	connect(ui.majorOptComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptComboBoxChanged(int)));
	connect(ui.minorOptComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptComboBoxChanged(int)));

	connect(ui.majorQuoteTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnDoubleClickedMajorTableWidget(int, int)));
	connect(ui.minorQuoteTableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnDoubleClickedMinorTableWidget(int, int)));
	connect(ui.positionTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnDoubleClickedPositionWidget(QTableWidgetItem*)));
	connect(ui.orderListTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(OnDoubleClickedOrderListWidget(QTableWidgetItem*)));
	connect(ui.strategyTableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(OnStrategyItemSelectionChanged()));

	connect(ui.majorOrderPushButton, SIGNAL(clicked()), this, SLOT(OnOrderMajor()));
	connect(ui.minorOrderPushButton, SIGNAL(clicked()), this, SLOT(OnOrderMinor()));
	connect(ui.combineOrderPushButton, SIGNAL(clicked()), this, SLOT(OnOrderCombine()));
	connect(ui.cancelAllpushButton, SIGNAL(clicked()), this, SLOT(OnCancelAll()));
	connect(ui.syncPushButton, SIGNAL(clicked()), this, SLOT(OnSyncPosition()));

	connect(ui.AddPushButton, SIGNAL(clicked()), this, SLOT(OnStrategySetting()));
	connect(ui.ModifyPushButton, SIGNAL(clicked()), this, SLOT(OnStrategyModify()));
	connect(ui.DeletePushButton, SIGNAL(clicked()), this, SLOT(OnStrategyDelete()));
	connect(ui.SwitchPushButton, SIGNAL(clicked()), this, SLOT(OnSwitchStrategy()));

	connect(ui.orderListTableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnOrderListSort(int)));
	connect(ui.tradeListTableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnTradeListSort(int)));
	connect(ui.positionTableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(OnPositionListSort(int)));
}

void JinDou::OnOrderRadiosToggled() {
	if (m_orderRadios->checkedId()) {
		ui.minorOrderGroupBox->setEnabled(true);
		ui.minorQuoteTableWidget->setEnabled(true);
		ui.combineOrderPushButton->setEnabled(true);
		ui.spreadLabel->setVisible(true);
		ui.spreadLabel->setStyleSheet("font-size:22px;color:blue;font-family:幼圆");
		ui.spreadTxtLabel->setVisible(true);
		ui.directionCheckBox->setEnabled(true);
		ui.directionCheckBox->setCheckable(true);
		ui.directionCheckBox->setText(TT("买卖(同)"));
		if (ui.majorOptComboBox->isEnabled()) {
			ui.operationCheckBox->setEnabled(true);
			ui.operationCheckBox->setCheckable(true);
			ui.operationCheckBox->setText(TT("开平(同)"));
		}

		auto dirIndex = ui.majorDirComboBox->currentIndex();
		ui.minorDirComboBox->setCurrentIndex(dirIndex);
		auto optIndex = ui.majorOptComboBox->currentIndex();
		ui.minorOptComboBox->setCurrentIndex(optIndex);
	} else {
		ui.minorOrderGroupBox->setEnabled(false);
		ui.minorQuoteTableWidget->setEnabled(false);
		ui.combineOrderPushButton->setEnabled(false);
		ui.spreadLabel->setVisible(false);
		ui.spreadTxtLabel->setVisible(false);
		ui.directionCheckBox->setCheckState(Qt::Unchecked);
		ui.operationCheckBox->setCheckState(Qt::Unchecked);
		ui.directionCheckBox->setEnabled(false);
		ui.operationCheckBox->setEnabled(false);
		ui.directionCheckBox->setText(TT("买卖"));
		ui.operationCheckBox->setText(TT("开平"));
	}
}
void JinDou::OnTradeListRadiosToggled() {
	tradeListView_->ViewModel(TradeViewModel(m_tradeListRadios->checkedId()));
}
void JinDou::OnOrderListRadiosChecked() {
	orderListView_->ViewModel(OrderViewModel(m_orderListRadios->checkedId()));
}

void JinDou::OnDirectionCheckBoxStateChanged(int state) {
	QString showTxt = (state == Qt::Checked) ? TT("买卖(反)") : TT("买卖(同)");
	QColor txtColor = (state == Qt::Checked) ? Qt::red : Qt::black;
	QPalette palette;
	palette.setColor(QPalette::WindowText, txtColor);
	ui.directionCheckBox->setPalette(palette);
	ui.directionCheckBox->setText(showTxt);

	int index = ui.majorDirComboBox->currentIndex();
 	ui.majorDirComboBox->currentIndexChanged(index);		//触发majorDirCombox信号
}
void JinDou::OnOperationCheckBoxStateChanged(int state) {
	if (ui.operationCheckBox->isEnabled()) {
		QString showTxt = (state == Qt::Checked) ? TT("开平(反)") : TT("开平(同)");
		QColor txtColor = (state == Qt::Checked) ? Qt::red : Qt::black;
		QPalette palette;
		palette.setColor(QPalette::WindowText, txtColor);
		ui.operationCheckBox->setPalette(palette);
		ui.operationCheckBox->setText(showTxt);

		int index = ui.majorOptComboBox->currentIndex();
		ui.majorOptComboBox->currentIndexChanged(index);	//触发majorOptCombox信号
	}
}
void JinDou::OnPriceCheckBoxStateChanged(int state) {
	QString showTxt = (state == Qt::Checked) ? TT("跟随盘口") : TT("委托价格");
	QColor txtColor = (state == Qt::Checked) ? Qt::red : Qt::black;
	QPalette palette;
	palette.setColor(QPalette::WindowText, txtColor);
	ui.priceCheckBox->setPalette(palette);
	ui.priceCheckBox->setText(showTxt);
}

void JinDou::OnMajorInstComboBoxChanged(QString const& inst) {
	orderView_->AddMinorInstrument(inst);
	orderView_->AdjustMajorOptCombox();		//选到上海品种，opt中要有平今指令；选到现货品种，opt要跟随dir
	orderView_->SetMajorPriceSpin();
	spotSpecial();
}
void JinDou::OnMinorInstComboBoxChanged(int index) {
	if (index < 0)	return;
		
	orderView_->AdjustMinorOptCombox();		//选到上海品种，opt中要有平今指令；选到现货品种，opt要跟随dir
	orderView_->SetMinorPriceSpin();
	spotSpecial();
}

void JinDou::OnDirComboBoxChanged(int index) {	
	if (index < 0) return;

	spotSpecial();
	if (false == ui.combineRadioButton->isChecked()) {
		return;		//非组合合约，无对应操作
	}

	int dirIndex = (Qt::Checked == ui.directionCheckBox->checkState()) ? (index + 1) % 2 : index;
	if (sender() == ui.majorDirComboBox) {
		ui.minorDirComboBox->setCurrentIndex(dirIndex);
		orderView_->UpdateQuoteSpread();
	} else {
		ui.majorDirComboBox->setCurrentIndex(dirIndex);
		orderView_->UpdateQuoteSpread();
	}
}
void JinDou::OnOptComboBoxChanged(int index) {
	if (index < 0) return;

	if (false == ui.combineRadioButton->isChecked()) {
		return;		//非组合合约，无对应操作
	}

	if (false == ui.majorOptComboBox->isEnabled() || false == ui.minorOptComboBox->isEnabled()) {
		return;		//組合合約內有現貨，无对应操作
	}

	int optIndex = (Qt::Checked == ui.operationCheckBox->checkState()) ? (index + 1) % 2 : index;
	if (sender() == ui.majorOptComboBox) {
		ui.minorOptComboBox->setCurrentIndex(optIndex);
	} else {
		ui.majorOptComboBox->setCurrentIndex(optIndex);
	}
}
void JinDou::OnOrderListSort(int column) {
	ui.orderListTableWidget->sortByColumn(column);
}
void JinDou::OnTradeListSort(int column) {
	ui.tradeListTableWidget->sortByColumn(column);
}
void JinDou::OnPositionListSort(int column) {
	ui.positionTableWidget->sortByColumn(column);
}

void JinDou::spotSpecial() {
	//现货合约的开平方向与买卖一致
	auto majorCode = ui.majorInstComboBox->currentText();
	auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(majorCode.toStdString().c_str()));
	if (type::ProductClass::Spot == instResult->ProductClass) {
		int dirIndex = ui.majorDirComboBox->currentIndex();
		ui.majorOptComboBox->setCurrentIndex(dirIndex);
	}

	if (true == ui.combineRadioButton->isChecked()) {
		auto minorCode = ui.minorInstComboBox->currentText();
		instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(minorCode.toStdString().c_str()));
		if (type::ProductClass::Spot == instResult->ProductClass) {
			int dirIndex = ui.minorDirComboBox->currentIndex();
			ui.minorOptComboBox->setCurrentIndex(dirIndex);
		}
	}
}

void JinDou::OnDoubleClickedMajorTableWidget(int row, int column) {
	if (NULL == ui.majorQuoteTableWidget->item(row, column))
		return;
	
	QString quoteTxt = ui.majorQuoteTableWidget->item(row, column)->text();
	auto price = quoteTxt.split('|')[0];
	ui.majorPriceSpinBox->setValue(price.toDouble());
}
void JinDou::OnDoubleClickedMinorTableWidget(int row, int column) {
	if (NULL == ui.minorQuoteTableWidget->item(row, column))
		return;

	QString quoteTxt = ui.minorQuoteTableWidget->item(row, column)->text();
	auto price = quoteTxt.split('|')[0];
	ui.minorPriceSpinBox->setValue(price.toDouble());
}
void JinDou::OnDoubleClickedPositionWidget(QTableWidgetItem* item) {
	auto currentRow = item->row();
	auto [offset,pos,volume] = positionView_->GetPosition(item);
	if (pos) {
		OrderInsertRequest req;
		req.Direction = type::negetive(pos->Direction);
		req.Exchange = pos->Exchange;
		req.InstCode = pos->InstrumentCode;
		req.InstID = pos->InstID;
		req.OffsetFlag = offset;
		req.Volume = volume;
		auto quote = ServiceSinglton::Services().Quote->Get(pos->InstID);
		req.Price = (req.Direction == type::Direction::Buy) ? quote->GetSell().GetPrice() : quote->GetBuy().GetPrice();
		req.OrderPriceType = type::OrderPriceType::LimitPrice;
		orderView_->FillOrderParm(req);
	}
}
void JinDou::OnDoubleClickedOrderListWidget(QTableWidgetItem* item) {
	auto currentRow = item->row();
	auto orderSysID = ui.orderListTableWidget->item(currentRow, 0)->text();
	orderView_->Cancel(type::OrderSysID(orderSysID.toStdString().c_str()));
}

void JinDou::OnStrategyItemSelectionChanged() {
	auto currentRow = ui.strategyTableWidget->currentRow();
	if (currentRow < 0) { return; }
	auto id = ui.strategyTableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
	bool pausedTag = strategyListView_->GetSnapshot(id).value().RunningStatus == RunningStatus::Paused;
	ui.DeletePushButton->setEnabled(pausedTag);

	QString btnTxt = pausedTag ? QStringLiteral("启动") : QStringLiteral("暂停");
	ui.SwitchPushButton->setText(btnTxt);
}

void JinDou::OnOrderMajor() {
	orderView_->OrderMajor();
}
void JinDou::OnOrderMinor() {
	orderView_->OrderMinor();
}
void JinDou::OnOrderCombine() {
	orderView_->OrderCombine();
}
void JinDou::OnCancelAll() {
	orderListView_->CancelAll(OrderViewModel(m_orderListRadios->checkedId()));
}
void JinDou::OnSyncPosition() {
	positionView_->SyncPosition();
}

void JinDou::OnStrategySetting() {
	AutoArgument argument;
	argument.Mode = StrategyMode::TodayOpenClose;
	argument.PriceMode = PriceMode::Pending;
	argument.Major = 0;
	argument.Minor = 1;
	argument.VolumeMultiple = 1;
	argument.MaxVolume = 1;
	argument.Quantity = 1;
	argument.Tolerance = 0;
	argument.MajorLostTick = 1;
	argument.SellOpenSpread = 666;
	argument.SellCloseSpread = -666;
	argument.BuyCloseSpread = 666;
	argument.BuyOpenSpread = -666;
	argument.ExistedVolume = { type::Direction::Buy , 0 };
	std::unique_ptr<StrategySettingDlg> dlg(new StrategySettingDlg(strategyListView_, argument, false, this));

	if (dlg->exec() == QDialog::Accepted) {
		strategyListView_->AddStrategy(dlg->GetArgument());
	}
}
void JinDou::OnStrategyModify() {
	int row = ui.strategyTableWidget->currentRow();
	if (row < 0) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("请选择需要修改的策略"), QStringLiteral("确认"));
		return;
	}

	auto id = ui.strategyTableWidget->item(row, 0)->data(Qt::UserRole).toInt();
	auto rtnArgument = strategyListView_->GetArgument(id);
	if (rtnArgument) {
		std::unique_ptr<StrategySettingDlg> dlg(new StrategySettingDlg(strategyListView_, rtnArgument.value(), true, this));

		if (dlg->exec() == QDialog::Accepted) {
			strategyListView_->ModifyStrategy(id, dlg->GetArgument());
		}
	}
	else {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("没有找到对应的策略"), QStringLiteral("确认"));
	}
}
void JinDou::OnStrategyDelete() {
	int row = ui.strategyTableWidget->currentRow();
	if (row < 0) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("请选择需要删除的策略"), QStringLiteral("确认"));
		return;
	}

	auto id = ui.strategyTableWidget->item(row, 0)->data(Qt::UserRole).toInt();
	strategyListView_->DeleteStrategy(id);
}
void JinDou::OnSwitchStrategy() {
	int row = ui.strategyTableWidget->currentRow();
	if (row < 0) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("请选择需要启动的策略"), QStringLiteral("确认"));
		return;
	}

	auto id = ui.strategyTableWidget->item(row, 0)->data(Qt::UserRole).toInt();
	bool pausedTag = strategyListView_->GetSnapshot(id).value().RunningStatus == RunningStatus::Paused;
	pausedTag ? strategyListView_->StartStrategy(id) : strategyListView_->PauseStrategy(id);
}

//quote test
// 	auto& initService = ServiceSinglton::Services().Init;
// 	if (initService->Has(TradePlatform::ZTP) && initService->GetPercent(TradePlatform::ZTP) >= 100) {
// 
// 		static int count = 0;
// 		count++;
// 		if (count % 500 == 0) {
// 			static double basePrice = 356;
// 			basePrice += 0.1;
// 
// 			SimpleQuoteArray buyPriceArray;
// 			buyPriceArray[0] = SimpleQuote(basePrice - 0.5, 30);
// 			buyPriceArray[1] = SimpleQuote(basePrice - 0.4, 31);
// 			buyPriceArray[2] = SimpleQuote(basePrice - 0.3, 32);
// 			buyPriceArray[3] = SimpleQuote(basePrice - 0.2, 33);
// 			buyPriceArray[4] = SimpleQuote(basePrice - 0.1, 34);
// 			SimpleQuoteArray sellPriceArray;
// 			sellPriceArray[0] = SimpleQuote(basePrice + 0.1, 16);
// 			sellPriceArray[1] = SimpleQuote(basePrice + 0.2, 17);
// 			sellPriceArray[2] = SimpleQuote(basePrice + 0.3, 18);
// 			sellPriceArray[3] = SimpleQuote(basePrice + 0.4, 19);
// 			sellPriceArray[4] = SimpleQuote(basePrice + 0.5, 20);
// 			double lastPrice = buyPriceArray[0].GetPrice();
// 
// 			type::InstrumentCode instCode("Au(T+D)");
// 			auto instID = ServiceSinglton::Services().Instrumnet->toID(instCode);
// 			Quote quote(instID, buyPriceArray, sellPriceArray, 370, 320, 10, 200, 300, 400,
// 				lastPrice, 500, 600, false, SimpleQuote(), SimpleQuote());
// 			data.Type = DataType::Quote;
// 			data.Size = sizeof(Quote);
// 			memcpy(data.Payload, &quote, data.Size);
// 			s_d.Dispatch(data);
// 
// 		}
// 	}

// 	if (ServiceSinglton::Services().Init->GetPercent(TradePlatform::ZTP) == 100) {
// 		static int count = 0;
// 		count++;
// 
// 		if (count % 10000 == 0) {
// 			static int c = 0;
// 			c++;
// 			InvestorTrade trade;
// 			trade.InstrumentCode = "Au(T+D)";
// 			trade.InstID = ServiceSinglton::Services().Instrumnet->toID(trade.InstrumentCode);
// 			trade.Direction = type::Direction::Buy;
// 			trade.OffsetFlag = type::OffsetFlag::Open;
// 			trade.OrderLocalID = 88888888;
// 			trade.OrderSysID = "SydID001";
// 			trade.TradePrice = 346;
// 			trade.TradeVolume = 10;
// 			trade.TradeID = "44";
// 			data.Type = DataType::TradeRtn;
// 			data.Size = sizeof(InvestorTrade);
// 			memcpy(data.Payload, &trade, data.Size);
// 			if (c == 1) {
// 				s_d.Dispatch(data);
// 			}
// 			if (c == 2) {
// 				trade.Direction = type::Direction::Sell;
// 				trade.OffsetFlag = type::OffsetFlag::CloseToday;
// 				trade.OrderLocalID = 88888889;
// 				trade.OrderSysID = "SydID002";
// 				trade.TradeVolume = 5;
// 				trade.TradeID = "55";
// 				memcpy(data.Payload, &trade, data.Size);
// 				s_d.Dispatch(data);
// 			}
// 			if (c == 3) {
// 				trade.Direction = type::Direction::Sell;
// 				trade.OffsetFlag = type::OffsetFlag::CloseToday;
// 				trade.OrderLocalID = 88888899;
// 				trade.OrderSysID = "SydID003";
// 				trade.TradeVolume = 5;
// 				trade.TradeID = "56";
// 				memcpy(data.Payload, &trade, data.Size);
// 				s_d.Dispatch(data);
// 			}
// 		}
// 	}