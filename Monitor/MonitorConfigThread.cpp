#include "MonitorConfigThread.h"

MonitorConfigThread::MonitorConfigThread(QString ip1, int port1, QString ip2, int port2, QString instanceStrId, int instanceNo)
{
	HostAddressPort addr1;
	HostAddressPort addr2;

	addr1.setAddress(ip1);
	addr1.setPort(port1);

	addr2.setAddress(ip2);
	addr2.setPort(port2);

	m_cfgLoader = new CfgLoader(instanceStrId, instanceNo, addr1, addr2);
	m_cfgLoaderThread = new Tcp::Thread(m_cfgLoader);
	m_cfgLoaderThread->start();

	return;
}

MonitorConfigThread::~MonitorConfigThread()
{
	requestInterruption();
	for (int i = 0; i < 200 && isFinished() == false; i++)		// Wait for about 6 sec to complete thread
	{
		QThread::msleep(30);
	}
	if (isRunning() == true)
	{
		qDebug() << "MonitorConfigThread IS NOT FINISHED";
	}

	m_cfgLoaderThread->quit();
	delete m_cfgLoaderThread;
}


void MonitorConfigThread::run()
{
	qDebug() << "MonitorConfigThread::run() START";

	while (isInterruptionRequested() == false)
	{
		QThread::msleep(0);
	}

	qDebug() << "MonitorConfigThread::run() EXIT";
	return;
}

void MonitorConfigThread::reconnect(QString /*ip1*/, int /*port1*/, QString /*ip2*/, int /*port2*/, QString /*instanceStrId*/, int /*instanceNo*/)
{
	assert(false);		// TO DO
	//assert(m_cfgLoader);
	//m_cfgLoader->setServers();
}

void MonitorConfigThread::slot_configurationReady(const QByteArray configurationXmlData, const BuildFileInfoArray buildFileInfoArray)
{

}

