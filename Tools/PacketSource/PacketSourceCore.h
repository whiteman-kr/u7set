#ifndef PACKETSOURCECORE_H
#define PACKETSOURCECORE_H

// ==============================================================================================

//Options:
//    -h                         Print this help.
//    -builddir=C:\BuildDir      Build directory
//    -adsip=IPv4                IP-addres (AppDataReceivingIP) for send packets to AppDataSrv.
//    -utip=IPv4                 IP-addres for listening commands from UalTester.
//    -rsid=SOURCE_EQUIPMENT_ID  EquipmentID of source for start.
//    -b=OptionsFileName.txt     Options of command line in the file name.
//
//Options: "-builddir", "-adsip" and "-utip" - must be filled
//
//For example: -builddir=D:\builds\mad_debug\build -adsip=192.168.11.254 -utip=127.0.0.1 -rsid=MAD_CC01_FSC10_MD00,MAD_CC01_FSC01_MD00

// ==============================================================================================

#include "CmdLineParam.h"
#include "BuildOpt.h"
#include "UalTesterServer.h"
#include "SourceBase.h"
#include "SignalBase.h"
#include "History.h"

// ==============================================================================================

class PacketSourceCore : public QObject
{
	Q_OBJECT

public:

	explicit	PacketSourceCore(QObject *parent = nullptr);
				PacketSourceCore(const CmdLineParam& cmdLine, QObject *parent = nullptr);
	virtual		~PacketSourceCore();

private:

	// options of app
	//
	BuildInfo m_buildInfo;
	bool buildInfoIsValid();

	// bases
	//
	SignalBase m_signalBase;
	SourceBase m_sourceBase;
	SignalHistory m_signalHistory;

	// UalTester listen connection
	//
	UalTesterServer* m_ualTesterSever = nullptr;
	UalTesterServerThread* m_ualTesterServerThread = nullptr;
	bool runUalTesterServerThread();
	void stopUalTesterServerThread();

	// timer for reload bases if build has been updated
	//
	QTimer* m_updateBuildFilesTimer = nullptr;
	void startUpdateBuildFilesTimer();
	void stopUpdateBuildFilesTimer();

public:

	void clear();

	//
	//
	BuildInfo& buildInfo() { return m_buildInfo; }
	void setBuildInfo(const BuildInfo& buildInfo) { m_buildInfo = buildInfo; }

	// bases
	//
	SignalBase& signalBase() { return m_signalBase; }
	SourceBase& sourceBase() { return m_sourceBase; }
	SignalHistory& history() { return m_signalHistory; }
	
	//
	//
	bool start();
	void stop();

signals:

	void signalsLoaded();

	// signals for UalTesterServer
	//
	void ualTesterSocketConnect(bool isConnect);


public slots:

	// slot of bases
	//
	bool loadSources();
	void loadSignals();
	void loadSignalsInSources();
	void reloadSource();

	// slot of timer for reload bases if build has been updated
	//
	void updateBuildFiles();

	// slot of UalTesterServer
	//
	void signalStateChanged(Hash hash, double prevState, double state);
	void exitApplication();
};

// ==============================================================================================

#endif // PACKETSOURCECORE_H
