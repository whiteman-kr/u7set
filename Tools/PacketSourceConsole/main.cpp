#include <QCoreApplication>

#include "../../lib/MemLeaksDetection.h"
#include "../../Proto/network.pb.h"

#include "../PacketSource/ConfigSocket.h"
#include "../PacketSource/PacketSourceCore.h"

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

int main(int argc, char *argv[])
{
	initMemoryLeaksDetection();

	// check version of RUP packets
	//
	#if RUP_VERSION != PS_SUPPORT_VERSION
		#error Current version of Rup packets is unknown
	#endif

	QCoreApplication a(argc, argv);

	a.setApplicationName("PacketSourceConsole");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	setlocale(LC_ALL,"Russian");

	// check keys of cmd line
	//
	CmdLineParam cmdLine(argc, argv);
	if (cmdLine.parse() == false)
	{
		return -1; // if exit code == -1, then problems with cmd line
	}

	// init SoftwareInfo
	//
	SoftwareInfo si;
	si.init(E::SoftwareType::TestClient, cmdLine.cfgEquipmentID(), 1, 0);

	// PacketSourceCore
	//
	PacketSourceCore ps(cmdLine);

	// create config socket thread
	//
	ConfigSocket* pConfigSocket = nullptr;
	pConfigSocket = new ConfigSocket(si, cmdLine.cfgServIP(), &ps);
	if (pConfigSocket == nullptr)
	{
		assert(pConfigSocket);
		return -1;
	}

	// start
	//
	pConfigSocket->start();

	int result = a.exec();

	delete pConfigSocket;
	ps.clear();

	google::protobuf::ShutdownProtobufLibrary();

	dumpMemoryLeaks();

	return result;
}
