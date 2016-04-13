#include "MonitorConfigThread.h"

MonitorConfigController::MonitorConfigController(HostAddressPort address1, HostAddressPort address2)
{
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
			QMessageBox::critical(nullptr,
								  qApp->applicationName(),
								  QString("Cannot create or attach to shared memory to determine software instance no. Error: %1").arg(instanceNoMemory.errorString()));
			return;
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

	qDebug() << "MonitorInstanceNo: " << instanceNo;

	// --
	//
	m_cfgLoader = new CfgLoader("Monitor", instanceNo, address1,  address2);
	m_cfgLoaderThread = new CfgLoaderThread(m_cfgLoader);

	connect(m_cfgLoader, &CfgLoader::signal_configurationReady, this, &MonitorConfigController::slot_configurationReady);

	m_cfgLoaderThread->start();

	return;
}

MonitorConfigController::~MonitorConfigController()
{
	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;
}

void MonitorConfigController::slot_configurationReady(const QByteArray /*configurationXmlData*/, const BuildFileInfoArray /*buildFileInfoArray*/)
{
	qDebug() << "MonitorConfigThread::slot_configurationReady";
}

