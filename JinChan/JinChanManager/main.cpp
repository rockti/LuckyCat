#include "JinChanManager.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include "Logger.h"
#include "Config.h"
#include "Widget/framelesswindow.h"
#include "Utils.h"


int main(int argc, char *argv[])
{

	Log::Init("Manager", "Manager.log");
	Configs::LoadFromFile();
	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon("JinChan.ico"));
  	JinChanManager w;
  	w.show();


// 	FramelessWindow framelessWindow;
// 	//framelessWindow.setWindowState(Qt::WindowFullScreen);
// 	//framelessWindow.setWindowTitle("test title");
// //	framelessWindow.setWindowIcon(a.style()->standardIcon(QStyle::SP_DesktopIcon));
// 	framelessWindow.setWindowIcon(QIcon("JinChan.ico"));
// 
// 	// create our mainwindow instance
// 	JinChanManager* mainWindow = new JinChanManager;
// 
// 	// add the mainwindow to our custom frameless window
// 	framelessWindow.setContent(mainWindow);
// 	framelessWindow.show();

//	blackTheme();
	whiteTheme();
	return a.exec();
}
