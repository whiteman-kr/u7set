#pragma once

// ========================7======================================================================

// Algorithm:
//
//   runCfgLoaderThread
//        readConfiguration
//        readAppSignals
//
//   parseTestFile
//
//   runSignalStateThread - connect to AppDataSrv
//   runTuningThread - connect to TuningSrv
//   Waiting connection to AppDataSrv and TuningSrv
//
//   runTestFile

// ==============================================================================================

#include <QObject>

#include "../../OnlineLib/CfgServerLoader.h"
#include "../../lib/SoftwareSettings.h"

#include "CmdLineParam.h"
#include "SignalBase.h"
#include "TuningSourceBase.h"
#include "TestFile.h"
#include "SignalStateSocket.h"
#include "TuningSocket.h"

class UalTester : public QObject
{
	Q_OBJECT

public:

	UalTester(int& argc, char** argv, std::shared_ptr<CircularLogger> logger);
	~UalTester();

public:
	bool start();
	void stop();

signals:
	void configurationLoaded();
	void signal_socketsReady();

private slots:
	void slot_loadConfiguration(const QByteArray configurationXmlData,
								const BuildFileInfoArray buildFileInfoArray,
								SessionParams sessionParams,
								std::shared_ptr<const SoftwareSettings> curSettingsProfile);
	void slot_parseTestFile();

	void slot_waitSocketsReady();
	void slot_runTestFile();

private:
	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	bool readAppSignals(const QByteArray& cfgFileData);

	bool parseTestFile(const QString& testFileName);
	bool runSockets();

	bool runSignalStateThread();
	void stopSignalStateThread();
	bool signalStateSocketIsConnected();

	bool runTuningThread();
	void stopTuningThread();
	bool tuningSocketIsConnected();

	int runTestFile();

private:
	std::shared_ptr<CircularLogger> m_log;
	CmdLineParam m_cmdLineParam;
	SignalBase m_signalBase;
	TuningSourceBase m_tuningSourceBase;
	SoftwareInfo m_softwareInfo;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;

	TestClientSettings m_cfgSettings;

	TestFile m_testfile;

	QTimer m_waitSocketsConnectionTimer;

	SignalStateSocket* m_pSignalStateSocket = nullptr;
	SimpleThread* m_pSignalStateSocketThread = nullptr;

	TuningSocket* m_pTuningSocket = nullptr;
	SimpleThread* m_pTuningSocketThread = nullptr;
};

