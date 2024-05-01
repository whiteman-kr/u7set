#ifndef PACKETSOURCECORE_H
#define PACKETSOURCECORE_H

// ==============================================================================================
//
//Options:
//    -h                           Print this help.
//    -cfgid=EQUIPMENT_ID_CFG      EquipmentID of software "Configuration Service".
//    -cfgip=IPv4                  IP-address of software "Configuration Service".
//    -adsid=EQUIPMENT_ID_ADS      EquipmentID of software "Application Data Service".
//    -utip=IPv4                   IP-addres for listening commands from UalTester.
//    -rsid=SOURCE_EQUIPMENT_ID    EquipmentID of source for run.
//    -b=OptionsFileName.txt       Options of command line in the file name.

//Options: "-cfgid", "-cfgip", "-adsid" and "-utip" - must be filled!
//For example: -cfgid=EQUIPMENT_ID_CFG -cfgip=127.0.0.1 -adsid=EQUIPMENT_ID_ADS -utip=127.0.0.1
//
// ==============================================================================================

#include "CmdLineParam.h"
#include "BuildOption.h"
#include "UalTesterServer.h"
#include "SourceBase.h"
#include "SignalBase.h"
#include "History.h"

// ==============================================================================================

class PacketSourceCore : public QObject
{
	Q_OBJECT

public:

	explicit PacketSourceCore(QObject *parent = nullptr);
	PacketSourceCore(const CmdLineParam& cmdLine, QObject *parent = nullptr);
	virtual ~PacketSourceCore() override;

public:

	void clear();
	void clearAllBases();

	//
	//
	BuildOption& buildOption() { return m_buildOption; }
	void setBuildOption(const BuildOption& buildOption) { m_buildOption = buildOption; }

	// bases
	//
	SignalBase& signalBase() { return m_signalBase; }
	SourceBase& sourceBase() { return m_sourceBase; }
	SignalHistory& history() { return m_signalHistory; }

private:

	// options of app
	//
	BuildOption m_buildOption;
	bool buildOptionIsValid();

	// bases
	//
	SignalBase m_signalBase;
	SourceBase m_sourceBase;
	SignalHistory m_signalHistory;

	// UalTester listen connection
	//
	UalTesterServer* m_ualTesterSever = nullptr;
	UalTesterServerThread* m_ualTesterServerThread = nullptr;

signals:

	void signalsLoadedInSources();

	// signals for UalTesterServer
	//
	void ualTesterSocketConnect(bool isConnect);

public slots:

	// slot of bases
	//
	void loadSignalsInSources();
	void saveSourceState();

	// slot of UalTesterServer
	//
	bool runUalTesterServerThread();
	void stopUalTesterServerThread();
	void signalStateChanged(Hash hash, double prevState, double state);
	void exitApplication();
};

// ==============================================================================================

#endif // PACKETSOURCECORE_H
