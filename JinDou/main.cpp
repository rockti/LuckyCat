#include "JinDou.h"
#include <QtWidgets/QApplication>
#include "config/Config.h"
#include "runner/Framework.h"
#include "utils/Log.h"
#include "utils/Encryption.h"
#include <QTemporaryFile.h>
#include <QMessageBox.h>
#include <sstream>
#include "utils/Platform.h"
#include "utils/License.h"

int loadConfig(bool unsafe);
class CommonHelper {
public:
	static void setStyle(const QString& style) {
		QFile qss(style);
		qss.open(QFile::ReadOnly);
		qApp->setStyleSheet(qss.readAll());
		qss.close();
	}
};

const char* Version = "2.1.8";

int main(int argc, char *argv[]) {
#ifdef UNITTEST
	int testmain(int argc, char* argv[]);
	return testmain(argc,argv);
#else

	Log::Init("JinDou", "jindou.log");
	Log::Info(fmt::format("--------JinDou Start Version {} ----------", Version));

	std::string mac;
	GetMacByGetAdaptersInfo(mac);

	QCoreApplication::setOrganizationName(QString("LianDaoTech"));
	QCoreApplication::setApplicationName(QString("JinDou"));
// 	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc, argv);


	CommonHelper::setStyle("style.white.qss");
	bool unsafe = false;
	if (argc > 1) {
		std::string argv2 = argv[1];
		if (argv2 == "-unsafe") {
			unsafe = true;
		}
	}
	unsafe = true;

	int result = loadConfig(unsafe);
	if (result != 0) return result;
	JinDou w;
	w.showMinimized();
//	w.show();
	auto rtn = a.exec();

	Framework::Instance().Stop();
	Framework::Instance().Join();
	return rtn;
#endif

}

int loadConfig(bool unsafe) {

	if (!unsafe) {
		SecureFile sf;
		auto result = sf.Decrypt(_T("jindou.sol"));
		if (!result) {
			QMessageBox::critical(nullptr,
				QString::fromLocal8Bit("解密配置失败"),
				QString::fromLocal8Bit("解密配置失败,请检查文件是否正确"));
			return -1;
		}
		auto configContent = result.value();
		if (configContent.length() < 5) {
			QMessageBox::critical(nullptr,
				QString("%1-%2").arg(QString::fromLocal8Bit("解密配置失败")).arg(configContent.c_str()), 
				QString::fromLocal8Bit("解密配置失败,请检查文件是否正确"));
			return -1;
		}
		ConfigLoader::Load(result.value());
	} else {
		ConfigLoader::Load("jindou.toml");
	}
	auto& c = ConfigLoader::Get();
	if (c.TradeSystemConfigs.empty()) {
		QMessageBox::critical(nullptr,
			QString::fromLocal8Bit("读取配置失败"),
			QString::fromLocal8Bit("读取配置失败,请检查文件是否正确"));
		return -1;
	}
	if (c.License.empty()) {
		QMessageBox::critical(nullptr,
			QString::fromLocal8Bit("读取配置失败"),
			QString::fromLocal8Bit("读取配置失败1,请检查文件是否正确"));
		return -1;
	}
	
	if (!unsafe && !License::Instance().Init(c.License)) {
		QMessageBox::critical(nullptr,
			QString::fromLocal8Bit("读取配置失败"),
			QString::fromLocal8Bit("读取配置失败2,请检查文件是否正确"));
		return -1;
	}
	return 0;
}
