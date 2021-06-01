#pragma once

#include "../OnlineLib/SocketIO.h"
#include "MonitorSignalSnapshot.h"
#include "../lib/Ui/DialogSignalSearch.h"

class MonitorAppSettings
{
public:
	MonitorAppSettings();
	~MonitorAppSettings() = default;

public:
	struct Data
	{
		QString equipmentId = "SYSTEM_RACKID_WS00_MONITOR";
		QString windowCaption = "Monitor";

		QString cfgSrvIpAddress1 = "127.0.0.1";
		int cfgSrvPort1 = PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST;

		QString cfgSrvIpAddress2 = "127.0.0.1";
		int cfgSrvPort2 = PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST;

		int requestTimeIntervalMs = 20;	// 20 ms

		bool showLogo = true;
		bool showItemsLabels = false;
		bool singleInstance = false;
	};

	// Public methods
	//
public:
	static MonitorAppSettings& instance();

	void save() const;
	void restore();

	bool saveToFile(QString fileName) const;
	bool loadFromFile(QString fileName);

private:
	void save(QSettings& settings) const;
	void load(const QSettings& settings);

	// Properties
	//
public:
	MonitorAppSettings::Data get() const;
	void set(const MonitorAppSettings::Data& src);

	//--
	//
	QString equipmentId() const;
	QString windowCaption() const;

	HostAddressPort configuratorAddress1() const;
	HostAddressPort configuratorAddress2() const;

	QString configuratorIpAddress1() const;
	int configuratorPort1() const;

	QString configuratorIpAddress2() const;
	int configuratorPort2() const;

	int requestTimeInterval() const;

	bool showLogo() const;
	bool showItemsLabels() const;
	bool singleInstance() const;

private:
	mutable QMutex m_mutex;
	Data m_data;
};


