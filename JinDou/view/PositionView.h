#pragma once
#include "presenter/PositionPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"
#include "utils/String.h"

class PositionView : public ui::IPositionView {
	inline static const std::vector<TableSchema> TableSchemas = {
		{0,"InstCode"},
		{1,"Direction"},
		{2,"TotalQty"},
		{3,"YdQty"},
		{4,"TdQty"},
		{5,"AvailableQty"},
		{6,"AvgPrice"},
		{7,"UsedMargin"},
		{8,"Profit"},
	};
public:
	PositionView(JinDou& mainWindow) :mainWindow_(mainWindow) {
		table_ = mainWindow_.UI().positionTableWidget;
		table_->setRowCount(0);
	}
	virtual void AddPosition(InvestorPosition const& position) override {
		table_->insertRow(0);
		updateRow(0, position);
	}
	virtual void UpdatePosition(InvestorPosition const& position) override {
		int row = findRow(position);
		if (row < 0) {
			row = 0;
			table_->insertRow(0);
		}
		updateRow(row, position);
	}
	virtual void DeletePosition(type::InstrumentCode instCode, type::Direction direction) override {
		std::string positionKey;
		positionKey += instCode.value();
		positionKey += IsBuy(direction) ? ":B" : ":S";
		int removedRow = -1;
		for (int row =0; row < table_->rowCount(); ++row) {
			auto key = table_->item(row, 0)->data(Qt::UserRole).toString().toStdString();
			if (key == positionKey) {
				removedRow = row;
				break;
			}
		}
		if (removedRow >= 0) {
			table_->removeRow(removedRow);
		}
	}
	//交易连接断线重连后，持仓需要Reload
	virtual void Reload(ui::Positions const& positions) override {
		table_->setRowCount(0);
		for (auto& position : positions) {
			AddPosition(position);
		}
	}

	std::tuple<type::OffsetFlag, PositionResult, int> GetPosition(QTableWidgetItem* item) {
		auto* firstItem = table_->item(item->row(), 0);
		auto key = firstItem->data(Qt::UserRole).toString().toStdString();
		auto values = split(key,":");
		auto instCode = type::InstrumentCode(values[0].c_str());
		auto instID = ServiceSinglton::Services().Instrument->toID(instCode);
		type::Direction dir = values[1] == "B" ? type::Direction::Buy : type::Direction::Sell;
		auto result = ServiceSinglton::Services().Position->Get(instID, dir);
		if (result) {
			if (result->Exchange == type::Exchange::SHFE) {
				if (item->column() == YDPOSITION_COLUMN) {
					return std::make_tuple(type::OffsetFlag::Close, result, result->YdPosition);
				}
				else {
					return std::make_tuple(type::OffsetFlag::CloseToday, result, result->Position);
				}
			} else {
				return std::make_tuple(type::OffsetFlag::Close, result, result->TotalVolume());
			}
		}
		return { type::OffsetFlag::Close, result, 0};
	}

	virtual void UpdatePositionProfit(TradePlatform platform, double positionProfit) override {
		mainWindow_.UpdatePositionProfit(platform, positionProfit);
	}

	void SyncPosition() {
		presenter_->Sync();
	}
private:
	const int YDPOSITION_COLUMN = 3;
	std::string toPositionKeyString(InvestorPosition const& position) {
		std::string result;
		result += position.InstrumentCode.value();
		result += IsBuy(position.Direction) ? ":B" : ":S";
		return result;
	}
	void updateRow(int row, InvestorPosition const& position) {
		auto instCode = itemFrom(position.InstrumentCode);
		instCode->setData(Qt::UserRole, toPositionKeyString(position).c_str());
		int column = 0;
		table_->setItem(row, column++, instCode);	
		table_->setItem(row, column++, itemFrom(NameService::Name(position.Direction)));
		table_->setItem(row, column++, itemFrom(position.Position + position.YdPosition));
		table_->setItem(row, column++, itemFrom(position.YdPosition));
		table_->setItem(row, column++, itemFrom(position.Position));
		int c = position.Position + position.YdPosition - position.FrozenPosition - position.FrozenYDPosition;
		table_->setItem(row, column++, itemFrom(c));	//总量-冻结=可平
		table_->setItem(row, column++, itemFrom(position.TotalPositionCost()));
		table_->setItem(row, column++, itemFrom(position.UsedMargin));
		table_->setItem(row, column++, itemFrom(position.PositionProfit));
	}
	int findRow(InvestorPosition const& position) {
		auto key = toPositionKeyString(position);
		for (int row = 0; row < table_->rowCount(); ++row) {
			int count = table_->rowCount();
			auto* item = table_->item(row, 0);
			auto posKey = item->data(Qt::UserRole).toString().toStdString();
			if (key == posKey) {
				return row;
			}
		}
		return -1;
	}

	JinDou& mainWindow_;
	QTableWidget* table_;
};