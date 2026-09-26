#pragma once

#include <QDialog>
#include "ui_StrategySetting.h"
#include "presenter/StrategyListPresenter.h"

class StrategyListView;
class StrategySettingDlg : public QDialog, public Ui::StrategySettingDlg
{
	Q_OBJECT
public:
	StrategySettingDlg(StrategyListView* view_, automate::AutoArgument& argument, bool isModify = false, QWidget* parent = 0);
	AutoArgument const& GetArgument() const { return argument_; }

private:
	void InitSlot();
	void InitCtrls();
	void InitMajorInstrument();
	bool GetValidsParam();

protected:
	void keyPressEvent(QKeyEvent* event);	//¹ýÂËesc°´Å¥²Ù×÷

private slots:
	void accept();
	void OnModeComboxChanged(int index);
	void OnOptComboxChanged(int index);
	void OnMajorComboxChanged(QString const& inst);

private:
	StrategyListView* strategyListView_;
	automate::AutoArgument& argument_;
};
