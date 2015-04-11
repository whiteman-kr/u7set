#include "FscDataSource.h"
#include "../include/FscDataFormat.h"


FscDataSourceWorker::FscDataSourceWorker(HostAddressPort hostAddressPort, QHostAddress sourceAddress, quint32 interval, quint32 partCount, QObject *parent) :
	QObject(parent),
	m_socket(this),
	m_timer(this),
	m_hostAddressPort(hostAddressPort),
	m_sourceAddress(sourceAddress),
	m_interval(interval),
	m_partCount(partCount)
{
	if (m_interval < 10)
	{
		m_interval = 10;
	}
}


void FscDataSourceWorker::init()
{
	connect(&m_timer, &QTimer::timeout, this, &FscDataSourceWorker::onTimer);

	m_timer.setInterval(m_interval);
	m_timer.start();

	bool res = m_socket.bind(m_sourceAddress);

	if (res == false)
	{
		qDebug() << "Socket binding error to: " << m_sourceAddress.toString();
	}
}


void FscDataSourceWorker::onTimer()
{
	m_packetNo++;

	FSC_PACKET packet;

	memset(&packet, 0, sizeof(FSC_PACKET));

	packet.header.packetSize = sizeof(FSC_PACKET);
	packet.header.protocolVersion = 1;
	packet.header.flags.registration = 1;
	packet.header.moduleFactoryNo = 55;
	packet.header.moduleType = 1;
	packet.header.subblockID = m_hostAddressPort.address32() & 0xFFFF;
	packet.header.partCount = m_partCount;
	packet.header.packetNo = m_packetNo;

	QDateTime now = QDateTime::currentDateTime();

	FSC_DATE_TIME_STAMP& timeStamp = packet.header.timeStamp;

	timeStamp.day = now.date().day();
	timeStamp.month = now.date().month();
	timeStamp.year = now.date().year();

	timeStamp.millisecond = now.time().msec();
	timeStamp.second = now.time().second();
	timeStamp.minute = now.time().minute();
	timeStamp.hour = now.time().hour();

	for(quint32 i = 0; i < m_partCount; i++)
	{
		packet.header.partNo = i;

		m_socket.writeDatagram(reinterpret_cast<const char*>(&packet), sizeof(packet), m_hostAddressPort.address(), m_hostAddressPort.port());
	}
}


FscDataSource::FscDataSource(HostAddressPort hostAddressPort, QHostAddress sourceAddress, quint32 interval, quint32 partCount, QObject *parent) :
	QObject(parent)
{
	FscDataSourceWorker* worker = new FscDataSourceWorker(hostAddressPort, sourceAddress, interval, partCount);

	connect(&m_thread, &QThread::started, worker, &FscDataSourceWorker::init);
	connect(&m_thread, &QThread::finished, worker, &QObject::deleteLater);

	worker->moveToThread(&m_thread);

	m_thread.start();
}


FscDataSource::~FscDataSource()
{
	m_thread.quit();
	m_thread.wait();
}
