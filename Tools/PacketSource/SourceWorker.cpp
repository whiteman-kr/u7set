#include "SourceWorker.h"

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QUdpSocket>

#include "Options.h"
#include "SourceBase.h"

#include "../../lib/WUtils.h"

// -------------------------------------------------------------------------------------------------------------------

SourceWorker::SourceWorker(QObject* pSource) :
	m_pSource(pSource),
	m_numerator(0),
	m_sentFrames(0),
	m_finishThread(false)
{
}

// -------------------------------------------------------------------------------------------------------------------

SourceWorker::~SourceWorker()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceWorker::process()
{
	PS::Source* pSource = (PS::Source*) m_pSource;
	if (pSource == nullptr)
	{
		emit finished();
		return;
	}

	QUdpSocket* pSocket = new QUdpSocket(this);
	if (pSocket == nullptr)
	{
		emit finished();
		return;
	}

	if (pSocket->bind(QHostAddress(theOptions.path().localIP()), PS::UDP_PORT + pSource->info().index) == false)
	{
		pSocket->close();
		delete pSocket;

		emit finished();
		return;
	}

	int currentFrameIndex = 0;

	while(m_finishThread == false)
	{
		for (int frameNumber = 0; frameNumber < pSource->info().frameCount; frameNumber++)
		{
			// header RupFrame
			//
			Rup::Header& header = m_simFrame.rupFrame.header;
			header.frameSize = Socket::ENTIRE_UDP_SIZE;
			header.protocolVersion = PS::SUPPORT_VERSION;
			header.flags.appData = 1;
			header.dataId = pSource->info().dataID;
			header.moduleType = pSource->info().moduleType;
			header.numerator = m_numerator;
			header.framesQuantity = pSource->info().frameCount;
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
			PS::FrameData* pFrameData = pSource->frameBase().frameDataPtr(currentFrameIndex);
			if (pFrameData != nullptr)
			{
				memcpy(m_simFrame.rupFrame.data, pFrameData->data(), Rup::FRAME_DATA_SIZE);
			}

			// crc64 RupFrame
			//
			m_simFrame.rupFrame.calcCRC64();

			// version and IP of simFrame
			//
			m_simFrame.simVersion = reverseUint16(PS::SIM_FRAME_VERSION);
			m_simFrame.sourceIP = reverseUint32(pSource->info().lmAddress.address32());

			// revers header
			//
			m_simFrame.rupFrame.header.reverseBytes();

			// send udp
			//
			pSocket->writeDatagram(reinterpret_cast<char*>(&m_simFrame), sizeof(m_simFrame), pSource->info().serverAddress.address(), pSource->info().serverAddress.port());

			// timeout
			//
			QThread::msleep(PS::SEND_TIMEOUT);

			m_sentFrames++;

			currentFrameIndex++;
			if (currentFrameIndex >= pSource->info().frameCount)
			{
				currentFrameIndex = 0;
			}
		}

		m_numerator++;
	}

	pSocket->close();
	delete pSocket;

	emit finished();
}

// -------------------------------------------------------------------------------------------------------------------

