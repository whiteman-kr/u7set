#ifndef BUILDOPTION_H
#define BUILDOPTION_H

#include "../../lib/HostAddressPort.h"

// ==============================================================================================

#define BUILD_REG_KEY "Options/Build/"

// ----------------------------------------------------------------------------------------------

class BuildOption : public QObject
{
	Q_OBJECT

public:

	BuildOption(QObject *parent = nullptr);
	BuildOption(const BuildOption& from, QObject *parent = nullptr);
	virtual ~BuildOption() override;

public:

	void clear();

	//
	//
	QString cfgSrvEquipmentID() const { return m_cfgSrvEquipmentID; }
	void setСfgSrvEquipmentID(const QString& equipmentID) { m_cfgSrvEquipmentID = equipmentID; }

	HostAddressPort cfgSrvIP() const { return m_cfgSrvIP; }
	void setCfgSrvIP(const HostAddressPort& ip) { m_cfgSrvIP = ip; }

	QString appDataSrvEquipmentID() const { return m_appDataSrvEquipmentID; }
	void setAppDataSrvEquipmentID(const QString& equipmentID) { m_appDataSrvEquipmentID = equipmentID; }

	HostAddressPort ualTesterIP() const { return m_ualTesterIP; }
	void setUalTesterIP(const HostAddressPort& ip) { m_ualTesterIP = ip; }
	
	//
	//
	QString signalsStatePath() const { return m_signalsStatePath; }
	void setSignalsStatePath(const QString& path) { m_signalsStatePath = path; }

	QStringList sourcesForRunList() const { return m_sourcesForRunList; }
	void setSourcesForRunList(const QStringList& list) { m_sourcesForRunList = list; }
	void appendSourcesForRunToList(const QString& sourceEquipmentID) { m_sourcesForRunList.append(sourceEquipmentID); }

	//
	//
	void load();
	void save();

	//
	//
	BuildOption& operator=(const BuildOption& from);

private:

	QString m_cfgSrvEquipmentID;
	HostAddressPort m_cfgSrvIP;
	QString m_appDataSrvEquipmentID;
	HostAddressPort m_ualTesterIP;

	QString m_signalsStatePath;
	QStringList m_sourcesForRunList;
};

// ==============================================================================================

#endif // BUILDOPTION_H
