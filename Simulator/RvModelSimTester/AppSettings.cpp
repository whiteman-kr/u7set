#include "AppSettings.h"
#include <QSettings>

AppSettings AppSettings::load() {
	QSettings settings( QSettings::UserScope);
	AppSettings result;

	settings.beginGroup("Network");
	result.ip = settings.value("IP", "127.0.0.1").toString();
	result.portLocal = settings.value("PortLocal", "9998").toInt();
	result.portRemote = settings.value("PortRemote", "9999").toInt();
	settings.endGroup();

	return result;
}

void AppSettings::save()
{
	QSettings settings(QSettings::UserScope);

	settings.beginGroup("Network");
	settings.setValue("IP", ip);
	settings.setValue("PortLocal", portLocal);
	settings.setValue("PortRemote", portRemote);
	settings.endGroup();
}
