#ifndef MONITORCONFIGTHREAD_H
#define MONITORCONFIGTHREAD_H

#include <QThread>
#include "../include/CfgServerLoader.h"
#include "../include/SocketIO.h"


class ConfigConnection
{
	ConfigConnection() {}

public:
	ConfigConnection(QString EquipmentId, QString ipAddress, int port);

protected:
	QString m_equipmentId;
	QString m_ip;
	int m_port;

	friend struct ConfigSettings;
};

struct ConfigSettings
{
	QString startSchemaId;				// Start Schema ID
	ConfigConnection das1;				// Data Aquisition Service connection params
	ConfigConnection das2;				// Data Aquisition Service connection params

	QString errorMessage;				// Parsing error message, empty if no errors
};

class MonitorConfigController : public QObject
{
	Q_OBJECT

public:
	MonitorConfigController() = delete;

	MonitorConfigController(HostAddressPort address1, HostAddressPort address2);
	virtual ~MonitorConfigController();

public:

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	bool xmlReadSoftwareNode(QDomNode& softwareNode, ConfigSettings* outSetting);
	bool xmlReadSettingsNode(QDomNode& settingsNode, ConfigSettings* outSetting);

	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	int m_appInstanceNo = -1;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;
};

#endif // MONITORCONFIGTHREAD_H
