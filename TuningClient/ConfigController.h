#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

#include <vector>
#include <set>
#include <QThread>
#include <QSharedMemory>
#include "../lib/CfgServerLoader.h"
#include "../lib/SocketIO.h"
#include "../VFrame30/Schema.h"

#include "Settings.h"


class ConfigController : public QObject
{
	Q_OBJECT

public:
	ConfigController() = delete;

	ConfigController(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, QWidget* parent);
	virtual ~ConfigController();

	// Methods
	//
public:

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr);

	Tcp::ConnectionState getConnectionState() const;
	QString getStateToolTip();

	int schemaCount() const;
	QString schemaCaptionById(const QString& schemaId) const;
	QString schemaCaptionByIndex(int schemaIndex) const;
	QString schemaIdByIndex(int schemaIndex) const;

	// signals
	//
signals:
	void tcpClientConfigurationArrived(HostAddressPort address, bool autoApply, LmStatusFlagMode lmStatusFlagMode);

	void signalsArrived(QByteArray data);
	void filtersArrived(QByteArray data);

	void configurationArrived();

	// slots
	//
public slots:
	void start();

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);

private:

	bool xmlReadBuildInfoNode(const QDomNode& node, ConfigSettings* outSetting);
	bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting);

	//bool xmlReadSettingsNode(const QDomNode& settingsNode, ConfigSettings* outSetting);

	bool xmlReadSettingsSection(const QByteArray& cfgFiledata, ConfigSettings* outSetting);

	// Public properties
public:
	// Data section
	//
private:
	QSharedMemory m_appInstanceSharedMemory;
	int m_appInstanceNo = -1;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	QWidget* m_parent = nullptr;

	std::map<QString, QString> m_filesMD5Map;

	HostAddressPort m_address1;
	HostAddressPort m_address2;

	mutable QMutex m_mutex;
	VFrame30::SchemaDetailsSet m_schemaDetailsSet;
};


#endif // CONFIGCONTROLLER_H
