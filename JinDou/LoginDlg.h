#pragma once

#include <QDialog>
#include <QElapsedTimer.h>
#include "ui_Login.h"

class LoginView;
class LoginDlg : public QDialog, public Ui::LoginDlg
{
	Q_OBJECT
public:
	LoginDlg(QWidget* parent = 0);
	void SetView(LoginView* view) { loginView_ = view; }

protected:
	void timerEvent(QTimerEvent* event);
	void keyPressEvent(QKeyEvent* event);	//¹ýÂËesc°´Å¥²Ù×÷
	
private slots:
	virtual void accept();
	void clear();
	void showPlatform();
	void showConfig();

private:
	LoginView* loginView_;
public:
	QElapsedTimer elapsedTimer_;
};
