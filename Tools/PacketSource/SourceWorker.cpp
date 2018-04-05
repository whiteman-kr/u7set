#include "SourceWorker.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QUdpSocket>

#include "SourceBase.h"

#include "../../lib/WUtils.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SourceWorker::SourceWorker(QObject* pSource) :
	m_pSource(pSource),
	m_finishThread(false),
	m_numerator(0)
{
}

// -------------------------------------------------------------------------------------------------------------------

SourceWorker::~SourceWorker()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceWorker::process()
{
	SourceItem* pSource = (SourceItem*) m_pSource;
	if (pSource == nullptr)
	{
		return;
	}

	QUdpSocket* pSocket = new QUdpSocket();
	if (pSocket == nullptr)
	{
		return;
	}

	pSocket->bind(QHostAddress::LocalHost, PS_PORT);

	while(m_finishThread == false)
	{
		for (int fn = 0; fn < pSource->frameCount(); fn++)
		{
			// header
			//
			Rup::Header& header = m_psFrame.rupFrame.header;
			header.frameSize = ENTIRE_UDP_SIZE;
			header.protocolVersion = 5; //
			header.flags.appData = 1;
			header.dataId = 0;
			header.moduleType = pSource->moduleType();
			header.numerator = m_numerator;
			header.framesQuantity = pSource->frameCount();
			header.frameNumber = fn;

			QDateTime&& time = QDateTime::currentDateTime();
			Rup::TimeStamp& timeStamp = header.timeStamp;
			timeStamp.year = time.date().year();
			timeStamp.month = time.date().month();
			timeStamp.day = time.date().day();

			timeStamp.hour = time.time().hour();
			timeStamp.minute = time.time().minute();
			timeStamp.second = time.time().second();
			timeStamp.millisecond = time.time().msec();

			// data
			//
			m_psFrame.rupFrame.data;

			// crc64
			//
			m_psFrame.rupFrame.calcCRC64();

			// version and IP
			//
			m_psFrame.version = reverseUint16(PS_FRAME_VERSION);
			m_psFrame.destIP = reverseUint32(QHostAddress(pSource->ip()).toIPv4Address());

			// send udp
			//
			pSocket->writeDatagram(reinterpret_cast<char*>(&m_psFrame), sizeof(m_psFrame), QHostAddress(theOptions.source().serverIP()), theOptions.source().serverPort());
			//pSocket->writeDatagram(reinterpret_cast<char*>(&m_psFrame.rupFrame), sizeof(m_psFrame.rupFrame), QHostAddress(theOptions.source().serverIP()), theOptions.source().serverPort());

			QThread::msleep(PS_SEND_FRAME_TIMEOUT);
		}

		m_numerator++;
	}

	pSocket->close();
	delete pSocket;

	emit finished();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
