#pragma once
#include <QLabel.h>
#include <QEvent.h>

class TTipWidget :public QLabel {
	Q_OBJECT

public:
	TTipWidget(QWidget* parent = nullptr);
	virtual ~TTipWidget();
	static TTipWidget& Instance();
	void SetMesseage(const QString& strMessage, const QPoint* pPoint = nullptr);

private:

	void OnTimer();
	void enterEvent(QEvent*);
	void leaveEvent(QEvent*);

private:
	QWidget* mpParent;
	QTimer* mpTimer;
	bool     mbEnter;
	int      mnTransparent;
};