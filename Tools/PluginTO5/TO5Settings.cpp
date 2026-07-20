
#include "TO5Settings.h"

void PluginTO5Settings::store()
{
	QSettings().setValue("TestSuite/testSuiteId", equipmentId);
	QSettings().setValue("TestSuite/ipAddress1", ipAddress1);
	QSettings().setValue("TestSuite/port1", port1);
	QSettings().setValue("TestSuite/ipAddress2", ipAddress2);
	QSettings().setValue("TestSuite/port2", port2);
}

PluginTO5Settings PluginTO5Settings::restore()
{
	PluginTO5Settings settings;

	settings.equipmentId = QSettings().value("TestSuite/testSuiteId", "SYSTEMID_WS00_TESTSUITE").toString();
	settings.ipAddress1 = QSettings().value("TestSuite/ipAddress1", "127.0.0.1").toString();
	settings.port1 = QSettings().value("TestSuite/port1", "13312").toString();
	settings.ipAddress2 = QSettings().value("TestSuite/ipAddress2", "127.0.0.1").toString();
	settings.port2 = QSettings().value("TestSuite/port2", "13312").toString();

	return settings;
}
