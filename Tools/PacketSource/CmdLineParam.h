#ifndef CMDLINEPARAM_H
#define CMDLINEPARAM_H

#include <QObject>

#include "../../ServiceLib/CommandLineParser.h"
#include "../../lib/HostAddressPort.h"

// ==============================================================================================

class CmdLineParam
{

public:

	explicit CmdLineParam();
	CmdLineParam(int& argc, char** argv);
	virtual ~CmdLineParam();

public:

	int argCout() { return m_cmdLineParser.argCount(); }

	QString cfgEquipmentID() const { return m_cfgEquipmentID; }
	HostAddressPort cfgServIP() const { return m_cfgSrvIP; }
	QString adsEquipmentID() const { return m_adsEquipmentID; }
	HostAddressPort ualTesterIP() const { return m_ualTesterIP; }
	const QStringList& sourcesForRunList() const { return m_sourcesForRunList; }

	void getParams(int& argc, char** argv);
	bool parse();

protected:

	// main keys
	//
	static const char* const SETTING_CFG_EQUIPMENT_ID;
	static const char* const SETTING_CFG_SERVICE_IP;
	static const char* const SETTING_ADS_EQUIPMENT_ID;
	static const char* const SETTING_UAL_TESTER_IP;

	// optional keys
	//
	static const char* const SETTING_SOURCES_FOR_RUN;
	static const char* const SETTING_OPTION_FILENAME;

	static const char* const REQUIRED_OPTIONS;

private:

	CommandLineParser m_cmdLineParser;

	// main keys
	//
	QString m_cfgEquipmentID;
	QString m_cfgIP;
	QString m_adsEquipmentID;
	QString m_utIP;

	HostAddressPort m_cfgSrvIP;
	HostAddressPort m_ualTesterIP;

	// optional keys
	//
	QStringList m_sourcesForRunList;
	QString m_optionFileName;

	void updateStartSourcesParam(QString& cmdLine);
};

// ==============================================================================================

#endif // CMDLINEPARAM_H
