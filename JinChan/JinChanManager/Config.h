#pragma once

#include <QString.h>
#include <QSettings.h>
#include <QTextCodec.h>
struct Broker {
	QString ID;
	int32_t Port;
};

struct Config {
	QString Server;
	Broker Broker1;
	Broker Broker2;
	QString User;

};


class Configs {
	static inline Config s_configs;
	static inline const char* config_file_name = "Manager.conf"; 
public:
	static inline void LoadFromFile() {
		QSettings config(config_file_name, QSettings::IniFormat);
		config.setIniCodec(QTextCodec::codecForName("System"));

		auto s = config.status();
		s_configs.Server = config.value("Manager/Server", "127.0.0.1").toString();
		s_configs.Broker1.ID = config.value("Manager/Broker1", "").toString();
		s_configs.Broker1.Port = config.value("Manager/Port1", "8888").toInt();
		s_configs.Broker2.ID = config.value("Manager/Broker2", "").toString();
		s_configs.Broker2.Port = config.value("Manager/Port2", "6666").toInt();
		s_configs.User = config.value("Manager/User", "").toString();
	}
	static inline void SaveToFile() {
		QSettings config(config_file_name, QSettings::IniFormat);
		config.setIniCodec(QTextCodec::codecForName("System"));
	
		config.setValue("Manager/Server", s_configs.Server);
		config.setValue("Manager/Broker1", s_configs.Broker1.ID);
		config.setValue("Manager/Port1", s_configs.Broker1.Port);
		config.setValue("Manager/Broker2", s_configs.Broker2.ID);
		config.setValue("Manager/Port2", s_configs.Broker2.Port);
		config.setValue("Manager/User", s_configs.User);

		config.sync();
	}

	static inline Config const& Get() {
		return s_configs;
	}

	static inline void Update(Config const& config) {
		s_configs = config;
	}

};