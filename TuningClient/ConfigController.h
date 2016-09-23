#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

#include "Stable.h"
#include <vector>
#include <set>
#include <QThread>
#include <QSharedMemory>
#include "../lib/CfgServerLoader.h"
#include "../lib/SocketIO.h"

class ConfigConnection
{
	ConfigConnection() {}

public:
	ConfigConnection(QString EquipmentId, QString ipAddress, int port);

	QString equipmentId() const;
	QString ip() const;
	int port() const;

	HostAddressPort address() const;

protected:
	QString m_equipmentId;
	QString m_ip;
	int m_port;

	friend struct ConfigSettings;
};

struct ConfigSettings
{
	ConfigConnection tuns1;				// Tuning Service connection params
	ConfigConnection tuns2;				// Tuning Service connection params

	bool updateFilters = false;
	bool updateSchemas = false;
	bool updateSignals = false;

	QString errorMessage;				// Parsing error message, empty if no errors
};

class ConfigController : public QObject
{
	Q_OBJECT

public:
	ConfigController() = delete;

	ConfigController(QWidget* parent, HostAddressPort address1, HostAddressPort address2);
	virtual ~ConfigController();

	// Methods
	//
public:
	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFile(const QString& pathFileName, QByteArray* fileData);

	bool getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr);
	bool getFileById(const QString& id, QByteArray* fileData);

	Tcp::ConnectionState getConnectionState() const;

	bool getObjectFilters();
	bool getSchemasDetails();
	bool getTuningSignals();
	// signals
	//
signals:
	void configurationArrived(ConfigSettings settings);

	// slots
	//
public slots:
	void start();

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:
	bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting);
	bool xmlReadSettingsNode(const QDomNode& settingsNode, ConfigSettings* outSetting);

	// Public properties
public:
	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	int m_appInstanceNo = -1;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	mutable QMutex m_mutex;

	QWidget* m_parent = nullptr;

	QString m_md5Filters;
	QString m_md5Schemas;
	QString m_md5Signals;
};


#endif // CONFIGCONTROLLER_H
