#ifndef SETTINGS_H
#define SETTINGS_H

#include "../lib/HostAddressPort.h"

class TuningPageSettings
{
public:
	int m_columnCount = 0;
	std::vector<int> m_columnsIndexes;
	std::vector<int> m_columnsWidth;
};

class Settings
{
public:
	Settings();

	void StoreUser();
	void RestoreUser();

	QString instanceStrId();

	HostAddressPort configuratorAddress1();
	HostAddressPort configuratorAddress2();

public:

	int m_requestInterval = 100;

	//

	QPoint m_mainWindowPos;
	QByteArray m_mainWindowGeometry;
	QByteArray m_mainWindowState;		// Toolbars/dock's

	QByteArray m_mainWindowSplitterState;


	// Tuning pages settings
	//
	std::vector<TuningPageSettings> m_tuningPageSettings;

private:

	QString m_instanceStrId;

	QString m_configuratorIpAddress1;
	int m_configuratorPort1;

	QString m_configuratorIpAddress2;
	int m_configuratorPort2;


	QMutex m;

};

extern Settings theSettings;

#endif // SETTINGS_H
