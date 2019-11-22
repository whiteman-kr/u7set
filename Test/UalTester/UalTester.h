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
#include "TuningSignalBase.h"
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

	static UalTester* m_pUalTester;
	static void exitApp(int sig);

	CmdLineParam m_cmdLineParam;

	SignalBase m_signalBase;
	TuningBase m_tuningBase;

	SoftwareInfo m_softwareInfo;

	CfgLoaderThread* m_cfgLoaderThread = nullptr;
	bool runCfgLoaderThread();
	void stopCfgLoaderThread();

	void printToReportFile(const QStringList& msgList);

	TestClientSettings m_cfgSettings;
	bool readConfiguration(const QByteArray& cfgFileData);
	bool readAppSignals(const QByteArray& cfgFileData);

	TestFile m_testfile;
	bool parseTestFile(const QString& testFileName);

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

	bool runSockets();
	QTimer m_waitSocketsConnectionTimer;
	void runWaitSocketsConnectionTimer();

	void runTestFile();

public:

	bool start();
	void stop();

signals:

	void signal_configurationParsed();
	void signal_socketsReady();

private slots:

	void slot_configurationReceived(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray);
	void slot_parseTestFile();

	void slot_waitSocketsConnection();
	void slot_socketsReady();

public slots:

};

#endif // UALTESTER_H
