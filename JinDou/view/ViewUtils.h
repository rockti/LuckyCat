#pragma once
#include "presenter/OrderPresenter.h"
#include "JinDou.h"

struct TableSchema {
	int Column;
	QString Name;
};

struct QTableWidgetItemProxy {

	QTableWidgetItemProxy(QTableWidgetItem* item) : item_(item) {
		item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
	}

	operator QTableWidgetItem* () {
		return item_;
	}
	QTableWidgetItem* operator->() {
		return item_;
	}
	QTableWidgetItemProxy& LeftAlign() {
		item_->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		return *this;
	}

	QTableWidgetItemProxy& CenterAlign() {
		item_->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		return *this;
	}

	QTableWidgetItemProxy& RightAlign() {
		item_->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		return *this;
	}

	QTableWidgetItem* item_;
};

inline QTableWidgetItemProxy itemFrom(int v) {
	return new QTableWidgetItem(QString::number(v));
}

inline QTableWidgetItemProxy itemFrom(uint32_t v) {
	return new QTableWidgetItem(QString::number(v));
}

inline QTableWidgetItemProxy itemFrom(QString const& v) {
	return new QTableWidgetItem(v);
}

inline QTableWidgetItemProxy itemFrom(std::string const& v) {
	return new QTableWidgetItem(v.c_str());
}

inline QTableWidgetItemProxy itemFrom(char const* v) {
	return new QTableWidgetItem(QString::fromLocal8Bit(v));
}

template<typename Tag>
inline QTableWidgetItemProxy itemFrom(DoubleType<Tag> v) {
	QString text;
	text.sprintf("%.2f", v.value());
	return new QTableWidgetItem(text);
}

template<typename Base, typename Tag>
inline QTableWidgetItemProxy itemFrom(NumberType<Base,Tag> v) {
	return new QTableWidgetItem(QString::number(v.value()));
}

template<uint8_t size, typename Tag>
inline QTableWidgetItemProxy itemFrom(StrongCharArray<size,Tag> const& v) {
	return new QTableWidgetItem(v.value());
}

inline QTableWidgetItemProxy itemFrom(double v) {
	QString text;
	text.sprintf("%.2f", v);
	return new QTableWidgetItem(text);
}


