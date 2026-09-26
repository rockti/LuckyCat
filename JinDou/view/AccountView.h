#pragma once
#include "presenter/AccountPresenter.h"
#include "JinDou.h"
#include "ViewUtils.h"

class AccountView : public ui::IAccountView {
	inline static const std::vector<TableSchema> TableSchemas = {
		{0,"InvestorID"},
		{1,"PreBalance"},
		{2,"FrozenMargin"},
		{3,"Margin"},
		{4,"Fee"},
		{5,"Available"},
		{6,"CloseProfit"},
		{7,"PositionProfit"},
	};
public:
	AccountView(JinDou& mainWindow) :mainWindow_(mainWindow) {
		table_ = mainWindow_.UI().accountTableWidget;
		table_->setRowCount(0);
	}
	virtual void SetPresenter(ui::AccountPresenter* presenter) override {
		presenter_ = presenter;
	}
	virtual void UpdateAccount(InvestorAccount const& account) override {
		auto row = findRow(account);
		if (row >= 0) {
			updateRow(row, account);
		}else {
			table_->insertRow(0);
			updateRow(0, account);
		}
	}

	virtual void OnPasswordChange(TradePlatform platform, SubSystem subSystem, Error error) override {
		changeZTPPasswordError_ = error;
	}

	void UpdatePositionProfit(TradePlatform platform, double positionProfit) {
		for (int row = 0; row < table_->rowCount(); row++) {
			auto* profitItem = table_->item(row, 7);
			auto storePlatform = (TradePlatform)profitItem->data(Qt::UserRole).toInt();
			if (platform == storePlatform) {
				QString text;
				text.sprintf("%.2f", positionProfit);			
				profitItem->setText(text);
				return;
			}
		}
	}
	Error LastPasswordChangeError() const { return changeZTPPasswordError_; }
private:
	void updateRow(int row, InvestorAccount const& account) {
 		auto investorItem = itemFrom(account.InvestorID);
		investorItem->setData(Qt::UserRole, account.InvestorID.value());
		int column = 0;
		table_->setItem(row, column++, investorItem);
		table_->setItem(row, column++, itemFrom(account.PreBalance));
		table_->setItem(row, column++, itemFrom(account.FrozenMargin));
		table_->setItem(row, column++, itemFrom(account.Margin));
		table_->setItem(row, column++, itemFrom(account.Fee));
		table_->setItem(row, column++, itemFrom(account.Available));
		table_->setItem(row, column++, itemFrom(account.CloseProfit));
		auto profitItem = itemFrom(account.PositionProfit);
		profitItem->setData(Qt::UserRole, int(account.Platform));
		table_->setItem(row, column++, profitItem);
	}
	int findRow(InvestorAccount const& account) {
		auto rowc = table_->rowCount();
		for (int row = 0; row < table_->rowCount(); row++) {
			auto accountID = table_->item(row, 0)->data(Qt::UserRole).toString();
			QByteArray array = accountID.toLocal8Bit();
			char* buffer = array.data();
			if ( account.InvestorID == buffer) {
				return row;
			}
		}
		return -1;
	}

	JinDou& mainWindow_;
	QTableWidget* table_;
	ui::AccountPresenter* presenter_;
	Error changeZTPPasswordError_;
};