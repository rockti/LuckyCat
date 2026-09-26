#pragma once
#include "presenter/OrderListPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"
#include <QMessageBox>

class OrderListView : public ui::IOrderListView {
	inline static const std::vector<TableSchema> OrderTableSchemas = {
		{0,"SystemNo"},
		{1,"InstCode"},
		{2,"Direction"},
		{3,"Operation"},
		{4,"OrderPrice"},
		{5,"OrderQuantity"},
		{6,"TradeQuantity"},
		{7,"TradePrice"},
		{8,"RemainQuantity"},
		{9,"Status"},
	};
public:
	OrderListView(JinDou& mainWindow) :mainWindow_(mainWindow) {
		table_ = mainWindow_.UI().orderListTableWidget;
	}
	virtual void AddOrder(InvestorOrder const& order) override {
		table_->insertRow(0);
		updateRow(0, order);
	}
	virtual void UpdateOrder(InvestorOrder const& order) override {
		int row = findRow(order);
		if (row >= 0) {
			updateRow(row, order);
		}
	}
	virtual void DeleteOrder(InvestorOrder const& order) override {
		int row = findRow(order);
		if (row >= 0) {
			table_->removeRow(row);
		}
	}
	virtual void Reload(ui::Orders const& dealings) override {
		table_->setRowCount(0);
		for (auto& dealing : dealings) {
			AddOrder(dealing);
		}
	}

	void ViewModel(ui::OrderViewModel model) {
		presenter_->ViewModel(model);
	}

	void CancelAll(ui::OrderViewModel model) {
		auto cancelableOrders = presenter_->GetCancelableOrders(model);
		if (cancelableOrders.empty()) return;

		if (QMessageBox::No == QMessageBox::question(NULL, "Question", QStringLiteral("确定撤销全部挂单"), QMessageBox::Yes | QMessageBox::No)) {
			return;
		}

		for (auto const& order : cancelableOrders) {
			OrderCancelRequest cancel {
				order.Exchange, order.OrderSysID, order.InstrumentCode, order.InstID,
				order.OrderLocalID, type::OrderLocalID(LocalIdACC::NextNormal())
			};
			ServiceSinglton::Services().TradeSystem->Cancel(cancel);
		}
	}
private:
	void updateRow(int row, InvestorOrder const& order) {
		auto uniqueID = itemFrom(order.OrderSysID);
		uniqueID->setData(Qt::UserRole, order.UniqueID.value());
		int column = 0;
		table_->setItem(row, column++, uniqueID);
		table_->setItem(row, column++, itemFrom(order.InstrumentCode).CenterAlign());
		table_->setItem(row, column++, itemFrom(NameService::Name(order.Direction)).CenterAlign());
		table_->setItem(row, column++, itemFrom(NameService::Name(order.OffsetFlag)).CenterAlign());
		table_->setItem(row, column++, itemFrom(order.InsertTime).RightAlign());
		table_->setItem(row, column++, itemFrom(NameService::Name(order.OrderStatus)).RightAlign());
		table_->setItem(row, column++, itemFrom(order.LimitPrice).RightAlign());
		table_->setItem(row, column++, itemFrom(order.Volume).RightAlign());
		table_->setItem(row, column++, itemFrom(order.VolumeTrade).RightAlign());	
		table_->setItem(row, column++, itemFrom(order.VolumeRemain).RightAlign());
		table_->setItem(row, column++, itemFrom(order.TradePrice).RightAlign());
		table_->setItem(row, column++, itemFrom(order.CancelTime).RightAlign());
	}
	int findRow(InvestorOrder const& order) {
		for (int row = 0; row < table_->rowCount(); row++) {
			auto uniqueID = table_->item(row, 0)->data(Qt::UserRole).toString();
			if (order.UniqueID == uniqueID.toStdString().c_str()) {
				return row;
			}
		}
		return -1;
	}

	JinDou& mainWindow_;
	QTableWidget* table_;
};