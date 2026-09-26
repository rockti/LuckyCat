#include "StrategySettingDlg.h"
#include <QtGui>
#include <QShortcut.h>
#include <QtWidgets/QMessageBox>
#include "view/StrategyListView.h"

using namespace std;

StrategySettingDlg::StrategySettingDlg(StrategyListView* view_, automate::AutoArgument& argument, bool isModify, QWidget* parent)
	:QDialog(parent), strategyListView_(view_), argument_(argument) {
	setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
	setFixedSize(this->width(), this->height());

	InitSlot();
	InitMajorInstrument();
	InitCtrls();
	if (true == isModify) {
		MajorInstComboBox->setEnabled(false);
		MinorInstComboBox->setEnabled(false);
		ModeComboBox->setEnabled(false);
		OptComboBox->setEnabled(false);
	}
}
void StrategySettingDlg::InitCtrls() {
	auto majorInst = ServiceSinglton::Services().Instrument->toCode(argument_.Major);
	MajorInstComboBox->setCurrentText(QString::fromLocal8Bit(majorInst.value()));
	auto minorInst = ServiceSinglton::Services().Instrument->toCode(argument_.Minor);
	MinorInstComboBox->setCurrentText(QString::fromLocal8Bit(minorInst.value()));

	ModeComboBox->setCurrentIndex(int(argument_.Mode));
	OptComboBox->setCurrentIndex(int(argument_.PriceMode));
	MinOrderQty->setText(QString::number(argument_.Quantity));
	VolumeMultiple->setText(QString::number(argument_.VolumeMultiple));
	MaxPositionQty->setText(QString::number(argument_.MaxVolume));
	int sign = (type::Direction::Buy == argument_.ExistedVolume.OpenDirection) ? 1 : -1;
	int positionValue = argument_.ExistedVolume.Volume * sign;
	CurrentPosition->setText(QString::number(positionValue));
	EndureTickSpin->setValue(argument_.Tolerance);
	GivenTickSpin->setValue(argument_.MajorLostTick);
	SellOpenSpread->setText(QString::number(argument_.SellOpenSpread, 'f', 2));
	SellOffsetSpread->setText(QString::number(argument_.SellCloseSpread, 'f', 2));
	BuyOffsetSpread->setText(QString::number(argument_.BuyCloseSpread, 'f', 2));
	BuyOpenSpread->setText(QString::number(argument_.BuyOpenSpread, 'f', 2));
}
void StrategySettingDlg::InitSlot() {
	connect(ModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnModeComboxChanged(int)));
	connect(OptComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnOptComboxChanged(int)));
	connect(MajorInstComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnMajorComboxChanged(QString)));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}
void StrategySettingDlg::InitMajorInstrument() {
	MajorInstComboBox->clear();
	auto insts = ServiceSinglton::Services().Instrument->GetSortedAll();
	for (auto const& inst : insts) {
		auto majorInst = QString::fromLocal8Bit(inst.Code.value());
		MajorInstComboBox->insertItem(MajorInstComboBox->count(), majorInst);
	}
}

void StrategySettingDlg::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
	case Qt::Key_Escape:
		break;
	default:
		QDialog::keyPressEvent(event);
	}
}

void StrategySettingDlg::accept() {
	if (GetValidsParam()) {
		QDialog::accept();
	}
}

void StrategySettingDlg::OnModeComboxChanged(int index) {
	bool enable = index == 0;
	MaxPositionQty->setText("1");
// 	MaxPositionQty->setEnabled(enable);
}
void StrategySettingDlg::OnOptComboxChanged(int index) {
	bool enable = index == 0;
	EndureTickSpin->setValue(0);
	EndureTickSpin->setEnabled(enable);
}
void StrategySettingDlg::OnMajorComboxChanged(QString const& inst) {
	MinorInstComboBox->clear();
	type::InstrumentCode majorCode(inst.toStdString().c_str());

	auto insts = ServiceSinglton::Services().Instrument->GetSortedInstrumentsExcept(majorCode);
	for (auto const& inst : insts) {
		auto minorInst = QString::fromLocal8Bit(inst.Code.value());
		MinorInstComboBox->insertItem(MinorInstComboBox->count(), minorInst);
	}
}

bool StrategySettingDlg::GetValidsParam() {
	if (0 == ModeComboBox->currentIndex()) {
		argument_.Mode = StrategyMode::TodayOpenClose;
	}
	else if (1 == ModeComboBox->currentIndex()) {
		argument_.Mode = StrategyMode::YesterdayClose;
	}
	else {
		argument_.Mode = StrategyMode::OpenClose;
	}
	argument_.PriceMode = (0 == OptComboBox->currentIndex()) ? PriceMode::Pending : PriceMode::FAK;
	auto majorCode = MajorInstComboBox->currentText();
	auto majorInstResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(majorCode.toStdString().c_str()));
	auto minorCode = MinorInstComboBox->currentText();
	auto minorInstResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(minorCode.toStdString().c_str()));
	argument_.Major = majorInstResult->InstID;
	argument_.Minor = minorInstResult->InstID;

	bool ok = false;
	auto intValue = VolumeMultiple->text().toInt();
	if (intValue < 1) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("主力委托倍数不能小于1"), QStringLiteral("确认"));
		VolumeMultiple->setFocus(Qt::TabFocusReason);
		return false;
	}
	argument_.VolumeMultiple = intValue;

	intValue = MaxPositionQty->text().toInt();	
	if (intValue < 1) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("最大持仓量不能小于1"), QStringLiteral("确认"));
		MaxPositionQty->setFocus(Qt::TabFocusReason);
		return false;
	}
	argument_.MaxVolume = intValue;

	intValue = MinOrderQty->text().toInt();
	if (intValue<1 || intValue>argument_.MaxVolume) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("委托量不能小于1或超过最大持仓量"), QStringLiteral("确认"));
		MinOrderQty->setFocus(Qt::TabFocusReason);
		return false;
	}
	argument_.Quantity = intValue;

	intValue = EndureTickSpin->text().toInt();	
	if (intValue<0 || intValue>5) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("容忍点不能小于0或大于5"), QStringLiteral("确认"));
		EndureTickSpin->setFocus(Qt::TabFocusReason);
		return false;
	}
	argument_.Tolerance = intValue;
	
	intValue = GivenTickSpin->text().toInt();	
	if (intValue <0 || intValue>10) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("主力跳价点不能小于0或大于10"), QStringLiteral("确认"));
		GivenTickSpin->setFocus(Qt::TabFocusReason);
		return false;
	}
	argument_.MajorLostTick = intValue;

	argument_.SellOpenSpread = SellOpenSpread->text().toDouble();
	argument_.SellCloseSpread = SellOffsetSpread->text().toDouble();
	if (argument_.SellOpenSpread <= argument_.SellCloseSpread) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("卖方向开仓价差必须高于平仓价差"), QStringLiteral("确认"));
		SellOpenSpread->setFocus(Qt::TabFocusReason);
		return false;
	}
	argument_.BuyCloseSpread = BuyOffsetSpread->text().toDouble();
	argument_.BuyOpenSpread = BuyOpenSpread->text().toDouble();
	if (argument_.BuyOpenSpread >= argument_.BuyCloseSpread) {
		QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("买方向开仓价差必须低于平仓价差"), QStringLiteral("确认"));
		BuyOpenSpread->setFocus(Qt::TabFocusReason);
		return false;
	}

	auto position = CurrentPosition->text().toInt();
	if (0 != position) {
		argument_.ExistedVolume.OpenDirection = (position > 0) ? type::Direction::Buy : type::Direction::Sell;
		argument_.ExistedVolume.Volume = abs(position);
		auto dirTxt = (position > 0) ? "B" : "S";
		auto message = QString::fromLocal8Bit("现有持仓组合为:[%1]方向 [%2]手?").arg(dirTxt).arg(argument_.ExistedVolume.Volume.value());
		if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("确认"), message, QMessageBox::Yes, QMessageBox::No)) {
			CurrentPosition->setFocus(Qt::TabFocusReason);
			return false;
		}
		if (argument_.ExistedVolume.Volume > argument_.MaxVolume) {
			QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("现有持仓不能超过最大持仓量"), QStringLiteral("确认"));
			MaxPositionQty->setFocus(Qt::TabFocusReason);
			return false;
		}
	}

	return true;
}
