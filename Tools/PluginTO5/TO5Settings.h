#pragma once

struct PluginTO5Settings
{
	QString equipmentId;
	QString ipAddress1;
	QString port1;
	QString ipAddress2;
	QString port2;

	static PluginTO5Settings restore();
	void store();
};