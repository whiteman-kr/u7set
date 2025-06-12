#pragma once

#include <QString> 

struct AppSettings{

	static AppSettings load();
	void save();
	
	
	QString ip;
	int portRemote;
	int portLocal;

	static inline const QString settingsFile = "settings.ini";
};

