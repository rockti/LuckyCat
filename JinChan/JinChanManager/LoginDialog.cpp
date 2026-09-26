#include "LoginDialog.h"
#include "Utils.h"
#include "Config.h"

#include <QMessageBox.h>
#include "ZTPClient.h"
#include "JinChanManager.h"

using namespace ztp;

LoginDialog::LoginDialog(QWidget* parent) :QDialog(parent) {
	ui.setupUi(this);
	setFixedSize(this->width(), this->height());

	ui.Broker2LineEdit->hide();
	ui.Label_7->hide();
	ui.Label_6->hide();
	ui.Port2LineEdit->hide();

	Config const& config = Configs::Get();
	ui.ServerLineEdit->setText(config.Server);
	ui.BrokerLineEdit->setText(config.Broker1.ID);
	ui.PortLineEdit->setText(QString::number(config.Broker1.Port));
	ui.Broker2LineEdit->setText(config.Broker2.ID);
	ui.Port2LineEdit->setText(QString::number(config.Broker2.Port));
	ui.OperatorLineEdit->setText(config.User);

	ui.PasswordLineEdit->setFocus();

	connect(ui.loginButton, SIGNAL(clicked()), this, SLOT(on_login_button_clicked()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(on_cancel_button_clicked()));
}

void warning(QString const& msg) {
	QMessageBox* msgBox = new QMessageBox(ft("提醒"), msg,
		QMessageBox::Warning,							///--这里是在消息框显示的图标
		QMessageBox::Ok | QMessageBox::Default,		///---这里是显示消息框上的按钮情况
		QMessageBox::Cancel | QMessageBox::Escape,	///---这里与 键盘上的 escape 键结合。当用户按下该键，消息框将执行cancel按钮事件
		0);
	msgBox->show();

}

void LoginDialog::on_login_button_clicked() {
	Config config;
	config.Server = ui.ServerLineEdit->text();
	config.Broker1.ID = ui.BrokerLineEdit->text();
	config.Broker2.ID = ui.Broker2LineEdit->text();
	config.Broker1.Port = ui.PortLineEdit->text().toInt();
	config.Broker2.Port = ui.Port2LineEdit->text().toInt();
	config.User = ui.OperatorLineEdit->text();
	QString password = ui.PasswordLineEdit->text();

	if (config.Server.isEmpty()) {
		warning(ft("服务地址不能为空"));
		return;
	}
	if (config.Broker1.ID.isEmpty()) {
		warning(ft("席位号不能为空"));
		return;
	}

	if (config.Broker1.Port < 1024) {
		warning(ft("端口号不正确"));
		return;
	}

	if (config.User.isEmpty()) {
		warning(ft("用户不能为空"));
		return;
	}

	if (password.isEmpty()) {
		warning(ft("密码不能为空"));
		return;
	}
	Password = ui.PasswordLineEdit->text();
	Configs::Update(config);
	Configs::SaveToFile();

	QDialog::accept();
	Status = StartLogin;
}

void LoginDialog::on_cancel_button_clicked() {
	QDialog::reject();
	Status = Cancelled;
}