#ifndef CMDLINEPARAM_H
#define CMDLINEPARAM_H

#include <QObject>

#include "../../lib/CommandLineParser.h"
#include "../../lib/HostAddressPort.h"

// ==============================================================================================

class CmdLineParam
{

public:

	explicit	CmdLineParam();
				CmdLineParam(int& argc, char** argv);
	virtual		~CmdLineParam();

protected:

	// main keys
	//
	static const char* const SETTING_BUILD_DIR;
	static const char* const SETTING_APP_DATA_SERVICE_IP;
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
	QString m_buildDir;
	QString m_adsIP;
	QString m_utIP;

	// optional keys
	//
	QStringList m_sourcesForRunList;
	QString m_optionFileName;

	//
	//
	HostAddressPort m_appDataSrvIP;
	HostAddressPort m_ualTesterIP;

	void updateStartSourcesParam(QString& cmdLine);

public:

	int argCout() { return m_cmdLineParser.argCount(); }

	QString buildDir() const { return m_buildDir; }
	HostAddressPort appDataSrvIP() const { return m_appDataSrvIP; }
	HostAddressPort ualTesterIP() const { return m_ualTesterIP; }
	const QStringList& sourcesForRunList() const { return m_sourcesForRunList; }

	void getParams(int& argc, char** argv);
	bool parse();

signals:

public slots:

};

// ==============================================================================================

#endif // CMDLINEPARAM_H
