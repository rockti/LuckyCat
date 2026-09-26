#pragma once
#include "presenter/TradeListPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"

class TradeListView : public ui::ITradeListView {
	inline static const std::vector<TableSchema> TradeTableSchemas = {
		{0,"TradeNo"},
		{1,"InstCode"},
		{2,"Direction"},
		{3,"Operation"},
		{4,"TradePrice"},
		{5,"TradeQuantity"},
	};
public:
	TradeListView(JinDou& mainWindow) :mainWindow_(mainWindow) {
		table_ = mainWindow_.UI().tradeListTableWidget;
	}
	void ViewModel(ui::TradeViewModel model) {
		if (ui::TradeViewModel::Total == model) {
			table_->setColumnHidden(6, true);
		} else {
			table_->setColumnHidden(6, false);
		}
		presenter_->ViewModel(model);
	}

	virtual void AddTrade(InvestorTrade const& trade) override {
		table_->insertRow(0);
		updateRow(0, trade);
	}
	virtual void Reload(Trades const& trade) override {
		table_->setRowCount(0);
		for (auto& trade : trade) {
			AddTrade(trade);
		}
	}

private:
	void updateRow(int row, InvestorTrade const& trade) {
		auto tradeIdItem = itemFrom(trade.TradeID);
		tradeIdItem->setData(Qt::UserRole, trade.TradeID.value());
		int column = 0;
		table_->setItem(row, column++, itemFrom(trade.InstrumentCode));
		table_->setItem(row, column++, itemFrom(NameService::Name(trade.Direction)));
		table_->setItem(row, column++, itemFrom(NameService::Name(trade.OffsetFlag)));
		table_->setItem(row, column++, itemFrom(trade.TradePrice));
		table_->setItem(row, column++, itemFrom(trade.TradeVolume));
		table_->setItem(row, column++, itemFrom(trade.UsedFee));
		table_->setItem(row, column++, itemFrom(trade.TradeTime));
	}

	JinDou& mainWindow_;
	QTableWidget* table_;
};