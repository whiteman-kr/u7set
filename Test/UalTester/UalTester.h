#ifndef UALTESTER_H
#define UALTESTER_H

#include <QObject>

#include "../../lib/CfgServerLoader.h"
#include "../../lib/ServiceSettings.h"

#include "SignalStateSocket.h"

class UalTester : public QObject
{
	Q_OBJECT

public:

	UalTester(int& argc, char** argv);
	~UalTester();

protected:

	static const char* const SETTING_CFG_SERVICE_IP1;
	static const char* const SETTING_CFG_SERVICE_IP2;
	static const char* const SETTING_EQUIPMENT_ID;
	static const char* const SETTING_TEST_FILE_NAME;

	static const char* const SETTING_ERROR_IGNORE;
	static const char* const SETTING_TEST_ID;
	static const char* const SETTING_FROM_TEST_ID;
	static const char* const SETTING_TRACE;
	static const char* const SETTING_REPORT;
	static const char* const SETTING_PRESET_LM;

private:

	QString m_cfgServiceIP1;
	QString m_cfgServiceIP2;
	QString m_equipmentID;
	QString m_testFileName;

	QString m_errorIngnoreStr;
	QString m_testID;
	QString m_fromTestID;
	QString m_traceStr;
	QString m_reportFileName;
	QString m_presetLM;

	HostAddressPort m_cfgSocketAddress1;
	HostAddressPort m_cfgSocketAddress2;

	bool m_errorIngnore = true;
	bool m_trace = false;

	void getCmdLineParams(int& argc, char** argv);
	bool cmdLineParamsIsValid();

	SoftwareInfo m_softwareInfo;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;
	bool runCfgLoaderThread();
	void stopCfgLoaderThread();

	TestClientSettings m_cfgSettings;
	bool readConfiguration(const QByteArray& cfgFileData);
	bool readAppSignals(const QByteArray& fileData);

	SignalStateSocket* m_pSignalStateSocket = nullptr;
	SimpleThread* m_pSignalStateSocketThread = nullptr;
	bool runSignalStateThread();
	void stopSignalStateThread();


public:

	bool start();

signals:

	void signal_configurationLoaded();

private slots:

	void slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);
	void slot_runManageSignalThread();

public slots:

};

#endif // UALTESTER_H
