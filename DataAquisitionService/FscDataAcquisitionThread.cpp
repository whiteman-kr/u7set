#include "FscDataAcquisitionThread.h"


FscDataAcquisitionSocket::FscDataAcquisitionSocket(HostAddressPort fscDataAcquisitionAddressPort)
{/*
		udpSocket->bind(QHostAddress::LocalHost, 7755);
	connect(udpSocket, SIGNAL(readyRead()),
			this, SLOT(readPendingDatagrams()));*/
}


FscDataAcquisitionSocket::~FscDataAcquisitionSocket()
{

}


FscDataAcquisitionThread::FscDataAcquisitionThread(HostAddressPort fscDataAcquisitionAddressPort, QObject *parent) :
	QObject(parent)
{
	FscDataAcquisitionSocket* dataAcquisitionSocket = new FscDataAcquisitionSocket(fscDataAcquisitionAddressPort);

	connect(&m_thread, &QThread::finished, dataAcquisitionSocket, &QObject::deleteLater);

	dataAcquisitionSocket->moveToThread(&m_thread);

	m_thread.start();
}


FscDataAcquisitionThread::~FscDataAcquisitionThread()
{
	m_thread.quit();
	m_thread.wait();
}
