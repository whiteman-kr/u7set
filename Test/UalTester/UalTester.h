#ifndef UALTESTER_H
#define UALTESTER_H

// ==============================================================================================

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

#include "../../lib/CfgServerLoader.h"
#include "../../lib/ServiceSettings.h"

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

	UalTester(int& argc, char** argv);
	~UalTester();

private:

	CmdLineParam m_cmdLineParam;

	SignalBase m_signalBase;
	TuningSourceBase m_tuningSourceBase;

	SoftwareInfo m_softwareInfo;

	// loader to receive signals from CfgSrv
	//
	CfgLoaderThread* m_cfgLoaderThread = nullptr;
	void runCfgLoaderThread();
	void stopCfgLoaderThread();

	TestClientSettings m_cfgSettings;
	bool readConfiguration(const QByteArray& cfgFileData);
	bool readAppSignals(const QByteArray& cfgFileData);

	// test file
	//
	TestFile m_testfile;
	bool parseTestFile(const QString& testFileName);

	// run SignalStateSocket and TuningSocket
	//
	bool runSockets();
	QTimer m_waitSocketsConnectionTimer;

	//
	//
	SignalStateSocket* m_pSignalStateSocket = nullptr;
	SimpleThread* m_pSignalStateSocketThread = nullptr;
	bool runSignalStateThread();
	void stopSignalStateThread();
	bool signalStateSocketIsConnected();

	TuningSocket* m_pTuningSocket = nullptr;
	SimpleThread* m_pTuningSocketThread = nullptr;
	bool runTuningThread();
	void stopTuningThread();
	bool tuningSocketIsConnected();

	// run commands from test file
	//
	int runTestFile();

public:

	bool start();
	void stop();

signals:

	void configurationLoaded();
	void signal_socketsReady();

private slots:

	void slot_loadConfiguration(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);
	void slot_parseTestFile();

	void slot_waitSocketsReady();
	void slot_runTestFile();

public slots:

};

#endif // UALTESTER_H
