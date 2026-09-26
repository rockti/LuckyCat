#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_JinDou.h"
#include "type/ZTPType.h"
#include <QElapsedTimer>
#include <QCloseEvent>

class OrderView;
class PositionView;
class OrderListView;
class TradeListView;
class AccountView;
class StrategyListView;

struct Data;
struct TradePlatformNotification;
struct OrderErrorNotification;
struct OrderRtnNotification;
class JinDou : public QMainWindow
{
    Q_OBJECT

public:
    JinDou(QWidget *parent = Q_NULLPTR);

    void InitPresenterAndService();

	Ui::JinDouClass& UI() {
        return ui;
    }
protected:
    void timerEvent(QTimerEvent* event);
    void closeEvent(QCloseEvent* event);

private:
    Ui::JinDouClass ui;
	std::map<int, QDockWidget*> m_docksList;
    QButtonGroup* m_orderRadios;
    QButtonGroup* m_orderListRadios;
	QButtonGroup* m_tradeListRadios;

public:
    QLabel* m_sgeQuoteStatus;
	QLabel* m_sgeTradeStatus;
    QLabel* m_sgeExchangeStatus;
	QLabel* m_shfeQuoteStatus; 
	QLabel* m_shfeTradeStatus;
    QLabel* m_shfeExchangeStatus;
	QLabel* m_esunneyQuoteStatus;
	QLabel* m_esunneyTradeStatus;
    QLabel* m_esunneyExchangeStatus;
    QLabel* m_errorMsgLabel;
    void UpdatePositionProfit(TradePlatform platform, double positionProfit);

private:
    void InitDocks();
    void InitRadios();
    void InitWidgets();
	void InitComboxItems();
    void InitStatusBar();
    void RemoveAllDocks();
	void ShowDock(int const dockIndex = 0);

    void SetTablesStyle();
    void InitSlot();
 
private slots:
    void OnOrderRadiosToggled();
	void OnTradeListRadiosToggled();
    void OnOrderListRadiosChecked();
	void OnDirectionCheckBoxStateChanged(int state);
	void OnOperationCheckBoxStateChanged(int state);
    void OnPriceCheckBoxStateChanged(int state);
	void OnMajorInstComboBoxChanged(QString const& inst);
	void OnMinorInstComboBoxChanged(int index);
    void OnDirComboBoxChanged(int index);
    void OnOptComboBoxChanged(int index);
	void OnOrderListSort(int column);
	void OnTradeListSort(int column);
	void OnPositionListSort(int column);
    void OnDoubleClickedMajorTableWidget(int row, int column);
    void OnDoubleClickedMinorTableWidget(int row, int column);
	void OnDoubleClickedPositionWidget(QTableWidgetItem* item);
    void OnDoubleClickedOrderListWidget(QTableWidgetItem* item);
    void OnStrategyItemSelectionChanged();
    void OnOrderMajor();
    void OnOrderMinor();
    void OnOrderCombine();
    void OnCancelAll();
    void OnSyncPosition();
    void OnStrategySetting();
    void OnStrategyModify();
    void OnStrategyDelete();
    void OnSwitchStrategy();

	void login();
    void changePassword();
    void spotSpecial();

private:
    OrderView* orderView_;
	PositionView* positionView_;
    OrderListView* orderListView_;
    TradeListView* tradeListView_;
    AccountView* accountView_;
    StrategyListView* strategyListView_;

    bool loginFailed_ = false;
    QElapsedTimer localIdSaveTimer_;
};
