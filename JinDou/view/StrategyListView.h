#pragma once
#include "presenter/StrategyListPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"


class StrategyListView : public ui::IStrategyListView {
public:
	StrategyListView(JinDou& mainWindow) :mainWindow_(mainWindow) {
		table_ = mainWindow_.UI().strategyTableWidget;
	}

	virtual void OnUpdate(int id, AutoSnapshot const& snapshot) override {
		int row = findRow(id);
		if (row >= 0) {
			if (snapshot.Deleted) {
				table_->removeRow(row);
			} else {
				updateRow(row, id, snapshot);
			}
		}
	}

	void AddStrategy(AutoArgument const& arg) {
		auto id = presenter_->AddStrategy(arg);
		table_->insertRow(0);	
		updateRow(0, id, arg);
	}
	void ModifyStrategy(int id, AutoArgument const& arg) {
		presenter_->ModifyStrategy(id, arg);
	}
	void DeleteStrategy(int id) {
		presenter_->DeleteStrategy(id);
	}
	void StartStrategy(int id) {
		presenter_->StartStrategy(id);
	}
	void PauseStrategy(int id) {
		presenter_->PauseStrategy(id);
	}
	void StopStrategy(int id) {		
		presenter_->StopStrategy(id);
	}
	std::optional<AutoArgument> GetArgument(int id) const {
		return presenter_->GetArg(id);
	}
	std::optional<AutoSnapshot> GetSnapshot(int id) const {
		return presenter_->GetSnapshot(id);
	}

private:
	void updateRow(int row, int id, AutoArgument const& arg) {
		auto minorInstResult = ServiceSinglton::Services().Instrument->Get(arg.Minor);
		auto majorInstResult = ServiceSinglton::Services().Instrument->Get(arg.Major);
		auto firstItem = itemFrom(QString("%1 - %2*%3").arg(minorInstResult->Code.value()).arg(majorInstResult->Code.value()).arg(arg.VolumeMultiple.value()));
		firstItem->setData(Qt::UserRole, id);
		int column = 0;
		table_->setItem(row, column++, firstItem);
		table_->setItem(row, column++, itemFrom(NameService::Name(arg.Mode)));
		table_->setItem(row, column++, itemFrom(NameService::Name(arg.PriceMode)));
		table_->setItem(row, column++, itemFrom(arg.Quantity));
		table_->setItem(row, column++, itemFrom(arg.MajorLostTick));
		table_->setItem(row, column++, itemFrom(arg.Tolerance));
		table_->setItem(row, column++, itemFrom(QString("[%1,%2][%3,%4]").arg(arg.SellOpenSpread.value()).arg(arg.SellCloseSpread.value()).arg(arg.BuyOpenSpread.value()).arg(arg.BuyCloseSpread.value())));
		table_->setItem(row, column++, itemFrom("Paused"));
		table_->setItem(row, column++, itemFrom(0));
	}
	void updateRow(int row, int id, AutoSnapshot const& snapshot) {
		updateRow(row, id, snapshot.Argument);
		table_->setItem(row, 7, itemFrom(NameService::Name(snapshot.RunningStatus)));
		auto sellVolume = snapshot.TotalMinor.Direction == type::Direction::Sell ? snapshot.TotalMinor.Volume : snapshot.TotalMajor.Volume;
		auto buyVolume = snapshot.TotalMinor.Direction == type::Direction::Buy ? snapshot.TotalMinor.Volume : snapshot.TotalMajor.Volume;
		table_->setItem(row, 8, itemFrom(QString("S-%1 | B-%2").arg(sellVolume.value()).arg(buyVolume.value())));

		auto currentRow = mainWindow_.UI().strategyTableWidget->currentRow();
		if (currentRow >= 0) {
			auto currentId = mainWindow_.UI().strategyTableWidget->item(currentRow, 0)->data(Qt::UserRole).toInt();
			if (currentId == id) {
				bool pausedTag = snapshot.RunningStatus == RunningStatus::Paused;
				mainWindow_.UI().DeletePushButton->setEnabled(pausedTag);
				QString btnTxt = pausedTag ? QStringLiteral("Æô¶¯") : QStringLiteral("ÔÝÍ£");
				mainWindow_.UI().SwitchPushButton->setText(btnTxt);
			}
		}
	}
	int findRow(int id) {
		for (int row = 0; row < table_->rowCount(); row++) {
			auto item = table_->item(row, 0)->data(Qt::UserRole).toInt();
			if (id == item) {
				return row;
			}
		}
		return -1;
	}

	JinDou& mainWindow_;
	QTableWidget* table_;
};
