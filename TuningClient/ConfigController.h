#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H

#include "Stable.h"
#include <vector>
#include <set>
#include <QThread>
#include <QSharedMemory>
#include "../lib/CfgServerLoader.h"
#include "../lib/SocketIO.h"

#include "Settings.h"


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

	bool requestObjectFilters();
	bool requestSchemasDetails();
	bool requestTuningSignals();

	bool getFileBlocked(const QString& pathFileName, QByteArray* fileData, QString* errorStr);
	bool getFileBlockedById(const QString& id, QByteArray* fileData, QString* errorStr);

	Tcp::ConnectionState getConnectionState() const;
	QString getStateToolTip();

	// signals
	//
signals:
	void configurationArrived();
	void signalsArrived();
	void serversArrived(HostAddressPort address1, HostAddressPort address2);

	// slots
	//
public slots:
	void start();

private slots:
	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);


private:

	bool xmlReadSoftwareNode(const QDomNode& softwareNode, ConfigSettings* outSetting);
	bool xmlReadSettingsNode(const QDomNode& settingsNode, ConfigSettings* outSetting);
	bool xmlReadSchemasNode(const QDomNode& schemasNode, const BuildFileInfoArray &buildFileInfoArray, ConfigSettings* outSetting);

	void addEventMessage(const QString& text);

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

	QStringList m_eventLog;
};


#endif // CONFIGCONTROLLER_H
