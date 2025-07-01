#pragma once

#include <QString>

struct AppSettings
{
	static AppSettings load();
	void save();


	QString ip = nullptr;
	int portRemote = 0;
	int portLocal = 0;
};
