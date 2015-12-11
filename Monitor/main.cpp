#include "MonitorMainWindow.h"
#include "Settings.h"
#include <QSharedMemory>
#include <QMessageBox>
#include "../VFrame30/VFrame30Library.h"
#include "MonitorConfigThread.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// --
	//
	a.setApplicationName("Monitor 3.0");
	a.setOrganizationName("Radiy");
	a.setOrganizationDomain("radiy.com");

	// Communication instance no
	//
	QSharedMemory instanceNoMemory("MonitorInstanceNo");
	bool ok = instanceNoMemory.create(16);
	if (ok == false)
	{
		if (instanceNoMemory.error() == QSharedMemory::SharedMemoryError::AlreadyExists)
		{
			ok = instanceNoMemory.attach();
		}

		if (ok == false)
		{
			QMessageBox::critical(nullptr, a.applicationName(),
								  QString("Cannot create or attach to shared memory to determine software instance no. Error: %1").arg(instanceNoMemory.errorString()));
			return 1;
		}
	}
	else
	{
		// Shared memory created, initialize it
		//
		instanceNoMemory.lock();
		quint32* sharedData = static_cast<quint32*>(instanceNoMemory.data());
		*sharedData = 0;
		instanceNoMemory.unlock();
	}

	assert(instanceNoMemory.isAttached() == true);

	instanceNoMemory.lock();
	quint32* sharedData = static_cast<quint32*>(instanceNoMemory.data());

	int instanceNo = static_cast<int>(*sharedData);
	instanceNo++;
	*sharedData = static_cast<quint32>(instanceNo);

	instanceNoMemory.unlock();

	qDebug() << "instanceNo: " << instanceNo;

	MonitorConfigThread congigThread(theSettings.configuratorIpAddress1(), theSettings.configuratorPort1(),
									 theSettings.configuratorIpAddress2(), theSettings.configuratorPort2(),
									 "Monitor", instanceNo);

	congigThread.start();

	// --
	//
	VFrame30::VFrame30Library::Init();
	//Hardware::Init();

	// Read settings
	//
	theSettings.load();

	// --
	//
	MonitorMainWindow w;
	w.show();


	// Communication
	//
	HostAddressPort addr1;
	HostAddressPort addr2;

	addr1.setAddress(theSettings.configuratorIpAddress1());
	addr1.setPort(theSettings.configuratorPort1());

	addr2.setAddress(theSettings.configuratorIpAddress2());
	addr2.setPort(theSettings.configuratorPort2());

	CfgLoader* cfgLoader = new CfgLoader(theSettings.instanceStrId(), instanceNo, addr1, addr2);
	Tcp::Thread* m_cfgLoaderThread = new Tcp::Thread(cfgLoader);
	m_cfgLoaderThread->start();

	// --
	//
	int result = a.exec();

	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;

	// Shutting down
	//
	VFrame30::VFrame30Library::Shutdown();
	//Hardware::Shutdwon();
	//google::protobuf::ShutdownProtobufLibrary();

	return result;
}
