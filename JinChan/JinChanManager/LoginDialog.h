#pragma once

#include <QDialog.h>
#include "ui_LoginDialog.h"

class LoginDialog : public QDialog {
	Q_OBJECT

public:
	LoginDialog(QWidget* parent = Q_NULLPTR);
	Ui::LoginDialog ui;
	QString Password;
	enum LoginStatus {Inited, StartLogin, Logining, Cancelled, Logined, LoginFailed	};
	LoginStatus Status = Inited;
private slots:
	void on_login_button_clicked();
	void on_cancel_button_clicked();

};