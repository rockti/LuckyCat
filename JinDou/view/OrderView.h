#pragma once
#include "presenter/OrderPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"
#include <QMessageBox>

class OrderView : public ui::IOrderView {
public:
	OrderView(JinDou& mainWindow) :mainWindow_(mainWindow) {
	}

	virtual void UpdateQuote(Quote const& quote) override {
		auto result = ServiceSinglton::Services().Instrument->Get(quote.GetInstID());
		auto majorCode = mainWindow_.UI().majorInstComboBox->currentText();
		if (majorCode == QString::fromLocal8Bit(result->Code.value())) {
			bool isInteger = majorCode.contains("Ag");
			updateMajorQuote(quote, isInteger);	//更新行情
			UpdateQuoteSpread();				//更新价差
			if (mainWindow_.UI().priceCheckBox->isChecked()) {
				updateMajorPriceBox(quote);		//更新委托价格
			}
		}

		if (mainWindow_.UI().combineRadioButton->isChecked()) {
			auto minorCode = mainWindow_.UI().minorInstComboBox->currentText();
			bool isInteger = minorCode.contains("Ag");
			if (minorCode == QString::fromLocal8Bit(result->Code.value())) {
				updateMinorQuote(quote, isInteger);
				UpdateQuoteSpread();
				if (mainWindow_.UI().priceCheckBox->isChecked()) {
					updateMinorPriceBox(quote);
				}
			}
		}	
	}

	virtual void AddInstrument(Instrument const& inst) override {
		return;
// 		mainWindow_.UI().majorInstComboBox->clear();
// 		auto insts = ServiceSinglton::Services().Instrument->GetSortedAll();
// 		for (auto const& inst : insts) {
// 			auto majorInst = QString::fromLocal8Bit(inst.Code.value());
// 			mainWindow_.UI().majorInstComboBox->insertItem(inst.InstID, majorInst);
// 		}
	}

	void AddInstruments() {
		mainWindow_.UI().majorInstComboBox->clear();
		auto insts = ServiceSinglton::Services().Instrument->GetSortedAll();
		for (auto const& inst : insts) {
			auto majorInst = QString::fromLocal8Bit(inst.Code.value());
			mainWindow_.UI().majorInstComboBox->insertItem(mainWindow_.UI().majorInstComboBox->count(), majorInst);
		}
	}
	void AddMinorInstrument(QString const& majorInst) {
		mainWindow_.UI().minorInstComboBox->clear();
		type::InstrumentCode majorCode(majorInst.toStdString().c_str());	
		auto insts = ServiceSinglton::Services().Instrument->GetSortedInstrumentsExcept(majorCode);
		for (auto const& inst : insts) {
			auto minorInst = QString::fromLocal8Bit(inst.Code.value());
			mainWindow_.UI().minorInstComboBox->insertItem(mainWindow_.UI().minorInstComboBox->count(), minorInst);
		}
	}

	void SetMajorPriceSpin() {
		auto majorCode = mainWindow_.UI().majorInstComboBox->currentText();
		auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(majorCode.toStdString().c_str()));
		mainWindow_.UI().majorPriceSpinBox->setSingleStep(instResult->UnitPrice.value());
	}
	void SetMinorPriceSpin() {
		auto minorCode = mainWindow_.UI().minorInstComboBox->currentText();
		auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(minorCode.toStdString().c_str()));
		if (!instResult) {
			return;
		}
		mainWindow_.UI().minorPriceSpinBox->setSingleStep(instResult->UnitPrice.value());
	}
	void AdjustMajorOptCombox() {
		auto majorCode = mainWindow_.UI().majorInstComboBox->currentText();
		auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(majorCode.toStdString().c_str()));
		resetOptComboxItem(mainWindow_.UI().majorOptComboBox, instResult);
		isMajorSpot = type::ProductClass::Spot == instResult->ProductClass;
		mainWindow_.UI().majorOptComboBox->setEnabled(!isMajorSpot);		//现货的开平跟随买卖，不能主动修改
		resetOptCheckBoxStatus();
	}
	void AdjustMinorOptCombox() {
		auto minorCode = mainWindow_.UI().minorInstComboBox->currentText();
		auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(minorCode.toStdString().c_str()));
		if (!instResult) {
			return;
		}
		resetOptComboxItem(mainWindow_.UI().minorOptComboBox, instResult);		
		isMinorSpot = type::ProductClass::Spot == instResult->ProductClass;
		mainWindow_.UI().minorOptComboBox->setEnabled(!isMinorSpot);		//现货的开平跟随买卖，不能主动修改
		resetOptCheckBoxStatus();
	}

	void FillOrderParm(OrderInsertRequest const& request) {
		auto majorInst = ServiceSinglton::Services().Instrument->toCode(request.InstID);
		mainWindow_.UI().majorInstComboBox->setCurrentText(QString::fromLocal8Bit(majorInst.value()));
		int dirIndex = (type::Direction::Buy == request.Direction) ? 0 : 1;
		mainWindow_.UI().majorDirComboBox->setCurrentIndex(dirIndex);
		int optIndex = (type::OffsetFlag::CloseToday == request.OffsetFlag) ? 2 : 1;
		mainWindow_.UI().majorOptComboBox->setCurrentIndex(optIndex);
		mainWindow_.UI().majorQtySpinBox->setValue(request.Volume);
		int typeIndex = (type::OrderPriceType::LimitPrice == request.OrderPriceType) ? 0 : 1;
		mainWindow_.UI().majorPriceTypeComboBox->setCurrentIndex(typeIndex);
		mainWindow_.UI().majorPriceSpinBox->setValue(request.Price);
	}
	void OrderMajor() {
		OrderInsertRequest order;
		auto instCode = mainWindow_.UI().majorInstComboBox->currentText();
		auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(instCode.toStdString().c_str()));
		order.Exchange = instResult->Exchange;
		order.InstCode = instResult->Code;
		order.InstID = instResult->InstID;
		double tickPrice = mainWindow_.UI().majorTickSpinBox->text().toInt() * instResult->UnitPrice;
		order.Price = mainWindow_.UI().majorPriceSpinBox->text().toDouble() + tickPrice;
		order.Volume = mainWindow_.UI().majorQtySpinBox->text().toInt();
		order.OrderLocalID = LocalIdACC::NextNormal();
		order.Direction = getDirection();
		bool isSpot = instResult->ProductClass == type::ProductClass::Spot;
		order.OffsetFlag = getOperation(isSpot);
		order.OrderPriceType = getOrderType();

		if (ValidateParam(order)) {
			presenter_->InsertOrder(order);
		}		
	}
	void OrderMinor() {
		OrderInsertRequest order;
		auto instCode = mainWindow_.UI().minorInstComboBox->currentText();
		auto instResult = ServiceSinglton::Services().Instrument->Get(type::InstrumentCode(instCode.toStdString().c_str()));
		order.Exchange = instResult->Exchange;
		order.InstCode = instResult->Code;
		order.InstID = instResult->InstID;
		double tickPrice = mainWindow_.UI().minorTickSpinBox->text().toInt() * instResult->UnitPrice;
		order.Price = mainWindow_.UI().minorPriceSpinBox->text().toDouble() + tickPrice;
		order.Volume = mainWindow_.UI().minorQtySpinBox->text().toInt();
		order.OrderLocalID = LocalIdACC::NextNormal();
		order.Direction = getDirection(false);
		bool isSpot = instResult->ProductClass == type::ProductClass::Spot;
		order.OffsetFlag = getOperation(isSpot, false);
		order.OrderPriceType = getOrderType(false);

		if (ValidateParam(order)) {
			presenter_->InsertOrder(order);
		}
	}
	void OrderCombine() {
		OrderMajor();
		OrderMinor();
	}
	void Cancel(type::OrderSysID const& orderSysID) {
		if (orderSysID.empty()) {
			return;
		}
		auto order = ServiceSinglton::Services().Order->GetOrder(orderSysID);
		if (order && order->IsPending()) {
			presenter_->CancelOrder(order.Value.value());
		}
	}
	void UpdateQuoteSpread() {
		auto majorCode = mainWindow_.UI().majorInstComboBox->currentText();
		auto minorCode = mainWindow_.UI().minorInstComboBox->currentText();
		auto majorID = ServiceSinglton::Services().Instrument->toID(type::InstrumentCode{ majorCode.toStdString().c_str() });
		auto minorID = ServiceSinglton::Services().Instrument->toID(type::InstrumentCode{ minorCode.toStdString().c_str() });
		if (majorID != INVALID_INSTRUMENT_ID && minorID != INVALID_INSTRUMENT_ID) {
			auto majorQuote = ServiceSinglton::Services().Quote->Get(majorID);
			auto minorQuote = ServiceSinglton::Services().Quote->Get(minorID);

			auto majorPrice = (type::Direction::Buy == getDirection()) ? majorQuote->GetSell().GetPrice() : majorQuote->GetBuy().GetPrice();
			auto majorOther = (type::Direction::Buy == getDirection()) ? majorQuote->GetBuy().GetPrice() : majorQuote->GetSell().GetPrice();
			auto minorPrice = (type::Direction::Buy == getDirection(false)) ? minorQuote->GetSell().GetPrice() : minorQuote->GetBuy().GetPrice();
			auto minorOther = (type::Direction::Buy == getDirection(false)) ? minorQuote->GetBuy().GetPrice() : minorQuote->GetSell().GetPrice();
			auto spread = minorPrice - majorPrice;
			auto another = minorOther - majorOther;
			auto spreadMsg = fmt::format("{:.2f} | {:.2f}", spread, another);
			mainWindow_.UI().spreadLabel->setText(QString::fromLocal8Bit(spreadMsg.c_str()));
// 			mainWindow_.UI().spreadLabel->setText(QString::number(spread));
		}
	}

private:
	void updateMajorQuote(Quote const& quote, bool isInteger) {
		int row = 0;
		for_each(quote.GetSellPriceArray().rbegin(), quote.GetSellPriceArray().rend(), [&](auto const& v) {
			auto simpleQuote = formatQuote(v, isInteger);
 			mainWindow_.UI().majorQuoteTableWidget->setItem(row, 0, itemFrom(simpleQuote));
 			mainWindow_.UI().majorQuoteTableWidget->item(row++, 0)->setTextAlignment(Qt::AlignLeft);
			});

		QString lastPrice = isInteger ? QString("%1").arg(quote.GetLastPrice().value()) : QString("%1").arg(quote.GetLastPrice().value(), 1, 'f', 2);
		mainWindow_.UI().majorQuoteTableWidget->setItem(row, 0, itemFrom(lastPrice));
		mainWindow_.UI().majorQuoteTableWidget->item(row++, 0)->setTextAlignment(Qt::AlignLeft);

		for_each(quote.GetBuyPriceArray().begin(), quote.GetBuyPriceArray().end(), [&](auto const& v) {
			auto simpleQuote = formatQuote(v, isInteger);
 			mainWindow_.UI().majorQuoteTableWidget->setItem(row, 0, itemFrom(simpleQuote));
 			mainWindow_.UI().majorQuoteTableWidget->item(row++, 0)->setTextAlignment(Qt::AlignLeft);
			});
	}
	void updateMajorPriceBox(Quote const& quote) {
		auto majorPrice = (type::Direction::Buy == getDirection()) ? quote.GetSell().GetPrice() : quote.GetBuy().GetPrice();
		mainWindow_.UI().majorPriceSpinBox->setValue(majorPrice.value());
	}
	void updateMinorQuote(Quote const& quote, bool isInteger) {
		int row = 0;
		for_each(quote.GetSellPriceArray().rbegin(), quote.GetSellPriceArray().rend(), [&](auto const& v) {
			auto simpleQuote = formatQuote(v, isInteger);
			mainWindow_.UI().minorQuoteTableWidget->setItem(row, 0, itemFrom(simpleQuote));
			mainWindow_.UI().minorQuoteTableWidget->item(row++, 0)->setTextAlignment(Qt::AlignLeft);
			});

		QString lastPrice = isInteger ? QString("%1").arg(quote.GetLastPrice().value()) : QString("%1").arg(quote.GetLastPrice().value(), 1, 'f', 2);
		mainWindow_.UI().minorQuoteTableWidget->setItem(row, 0, itemFrom(lastPrice));
 		mainWindow_.UI().minorQuoteTableWidget->item(row++, 0)->setTextAlignment(Qt::AlignLeft);

		for_each(quote.GetBuyPriceArray().begin(), quote.GetBuyPriceArray().end(), [&](auto const& v) {		
			auto simpleQuote = formatQuote(v, isInteger);
			mainWindow_.UI().minorQuoteTableWidget->setItem(row, 0, itemFrom(simpleQuote));
 			mainWindow_.UI().minorQuoteTableWidget->item(row++, 0)->setTextAlignment(Qt::AlignLeft);
			});
	}
	void updateMinorPriceBox(Quote const& quote) {
		auto minorPrice = (type::Direction::Buy == getDirection(false)) ? quote.GetSell().GetPrice() : quote.GetBuy().GetPrice();	
		mainWindow_.UI().minorPriceSpinBox->setValue(minorPrice.value());
	}
	void resetOptComboxItem(QComboBox* combox, InstrumentResult result) {
		auto itemIndex = combox->currentIndex();
		combox->clear();
		combox->insertItem(0, QStringLiteral("开仓"));
		combox->insertItem(1, QStringLiteral("平仓"));
		if (result->Exchange == type::Exchange::SHFE) {
			combox->insertItem(2, QStringLiteral("平今"));
		}
		auto currentIndex = (itemIndex > 1) ? 1 : itemIndex;
		combox->setCurrentIndex(currentIndex);
	}
	void resetOptCheckBoxStatus() {
		if (mainWindow_.UI().combineRadioButton->isChecked()) {
			if (isMajorSpot || isMinorSpot) {	//either major or minor is spot			
				mainWindow_.UI().operationCheckBox->setEnabled(false);
				mainWindow_.UI().operationCheckBox->setCheckable(false);
				mainWindow_.UI().operationCheckBox->setText(QStringLiteral("开平"));
				mainWindow_.UI().operationCheckBox->setCheckState(Qt::Unchecked);
				QPalette palette;
				palette.setColor(QPalette::WindowText, Qt::black);
				mainWindow_.UI().operationCheckBox->setPalette(palette);
			} else {
				mainWindow_.UI().operationCheckBox->setEnabled(true);
				mainWindow_.UI().operationCheckBox->setCheckable(true);
				mainWindow_.UI().operationCheckBox->setText(QStringLiteral("开平(同)"));
				int index = mainWindow_.UI().majorOptComboBox->currentIndex();
				mainWindow_.UI().majorOptComboBox->currentIndexChanged(index);		//触发majorOptCombox信号
			}
		}
	}

	bool ValidateParam(OrderInsertRequest const& order) {
		QMessageBox msgBox;
		if (order.Volume <= 0 || order.Volume > 10000) {
			QMessageBox::warning(NULL, "Warning", QStringLiteral("委托数量不正确!"), QMessageBox::Ok);
			return false;
		}
		if (order.Price <= 0 ) {
			QMessageBox::warning(NULL, "Warning", QStringLiteral("委托价格不正确!"), QMessageBox::Ok);
			return false;
		}
		return true;
	}
	type::Direction getDirection(bool isMajor = true) {
		int index = 0;
		if (isMajor) {
			index = mainWindow_.UI().majorDirComboBox->currentIndex();
		} else {
			index = mainWindow_.UI().minorDirComboBox->currentIndex();
		}
		return (0 == index) ? type::Direction::Buy : type::Direction::Sell;
	}
	type::OffsetFlag getOperation(bool isSpot, bool isMajor = true) {
		int index = 0;
		if (isSpot) {	//現貨的Operator跟隨Direction
			if (isMajor) {
				index = mainWindow_.UI().majorDirComboBox->currentIndex();
			} else {
				index = mainWindow_.UI().minorDirComboBox->currentIndex();
			}
			return (0 == index) ? type::OffsetFlag::Open : type::OffsetFlag::Close;
		}

		if (isMajor) {
			index = mainWindow_.UI().majorOptComboBox->currentIndex();
		} else {
			index = mainWindow_.UI().minorOptComboBox->currentIndex();
		}

		if (0 == index) {
			return type::OffsetFlag::Open;
		} 
		else if (1 == index) {
			return type::OffsetFlag::Close;
		}
		else {
			return type::OffsetFlag::CloseToday;
		}
	}
	type::OrderPriceType getOrderType(bool isMajor = true) {
		int index = 0;
		if (isMajor) {
			index = mainWindow_.UI().majorPriceTypeComboBox->currentIndex();
		}
		else {
			index = mainWindow_.UI().majorPriceTypeComboBox->currentIndex();
		}
		return (0 == index) ? type::OrderPriceType::LimitPrice : type::OrderPriceType::FAK;
	}
	QString formatQuote(SimpleQuote const& quote, bool isInteger) {
		QString price = isInteger ? QString("%1").arg(quote.GetPrice().value()) : QString("%1").arg(quote.GetPrice().value(), 1, 'f', 2);
		price = price.leftJustified(6, ' ');
		auto simpleQuote = QString("%1| %2").arg(price).arg(quote.GetQuantity());
		return simpleQuote;
	}
private:
	JinDou& mainWindow_;
	bool isMajorSpot = false;
	bool isMinorSpot = false;
};