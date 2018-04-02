#include "SourceWorker.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QUdpSocket>

#include "SourceBase.h"

#include "../../lib/WUtils.h"

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

	pSocket->bind(QHostAddress::LocalHost, PS::UDP_PORT);

	while(m_finishThread == false)
	{
		for (int frameNumber = 0; frameNumber < pSource->frameCount(); frameNumber++)
		{
			// header RupFrame
			//
			Rup::Header& header = m_psFrame.rupFrame.header;
			header.frameSize = Socket::ENTIRE_UDP_SIZE;
			header.protocolVersion = PS::SUPPORT_VERSION;
			header.flags.appData = 1;
			header.dataId = pSource->dataID();
			header.moduleType = pSource->moduleType();
			header.numerator = m_numerator;
			header.framesQuantity = pSource->frameCount();
			header.frameNumber = frameNumber;

			QDateTime&& time = QDateTime::currentDateTime();
			Rup::TimeStamp& timeStamp = header.timeStamp;
			timeStamp.year = time.date().year();
			timeStamp.month = time.date().month();
			timeStamp.day = time.date().day();

			timeStamp.hour = time.time().hour();
			timeStamp.minute = time.time().minute();
			timeStamp.second = time.time().second();
			timeStamp.millisecond = time.time().msec();

			// data RupFrame
			//
			m_psFrame.rupFrame.data;

			// crc64 RupFrame
			//
			m_psFrame.rupFrame.calcCRC64();

			// version and IP of psFrame
			//
			m_psFrame.version = reverseUint16(PS::FrameVersion);
			m_psFrame.lmIP = reverseUint32(pSource->lmAddress().address32());

			// send udp
			//
			pSocket->writeDatagram(reinterpret_cast<char*>(&m_psFrame), sizeof(m_psFrame), pSource->serverAddress().address(), pSource->serverAddress().port());

			// timeout
			//
			QThread::msleep(PS::SendFrameTimeout);
		}

		m_numerator++;
	}

	pSocket->close();
	delete pSocket;

	emit finished();
}

// -------------------------------------------------------------------------------------------------------------------

