#include "SourceWorker.h"

#include <QUdpSocket>
#include <QThread>

#include "SourceBase.h"

#include "../../lib/WUtils.h"

// -------------------------------------------------------------------------------------------------------------------

SourceWorker::SourceWorker(QObject* pSource, const HostAddressPort& appDataSrvIP) :
	m_pSource(pSource),
	m_appDataSrvIP(appDataSrvIP),
	m_numerator(0),
	m_sentFrames(0),
	m_finishThread(false),
	m_threadIsFinished(false)
{
}

// -------------------------------------------------------------------------------------------------------------------

SourceWorker::~SourceWorker()
{
}

// -------------------------------------------------------------------------------------------------------------------

void SourceWorker::process()
{
	PS::Source* pSource = dynamic_cast<PS::Source*>(m_pSource);
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

	if (m_appDataSrvIP.isEmpty() == true)
	{
		emit finished();
		return;
	}

	int frameCount = pSource->info().frameCount;
	if (frameCount == 0)
	{
		frameCount = 1;
	}

	int currentFrameIndex = 0;

	while(m_finishThread == false)
	{
		for (int frameIndex = 0; frameIndex < frameCount; frameIndex++)
		{
			// header RupFrame
			//
			Rup::Header& header = m_simFrame.rupFrame.header;
			header.frameSize = Socket::ENTIRE_UDP_SIZE;
			header.protocolVersion = PS::SUPPORT_VERSION;
			header.flags.appData = 1;
			header.dataId = static_cast<quint32>(pSource->info().dataID);
			header.moduleType = static_cast<quint16>(pSource->info().moduleType);
			header.numerator = static_cast<quint16>(m_numerator);
			header.framesQuantity = static_cast<quint16>(frameCount);
			header.frameNumber = static_cast<quint16>(frameIndex);

			QDateTime&& time = QDateTime::currentDateTime();
			Rup::TimeStamp& timeStamp = header.timeStamp;
			timeStamp.year = static_cast<quint16>(time.date().year());
			timeStamp.month = static_cast<quint16>(time.date().month());
			timeStamp.day = static_cast<quint16>(time.date().day());

			timeStamp.hour = static_cast<quint16>(time.time().hour());
			timeStamp.minute = static_cast<quint16>(time.time().minute());
			timeStamp.second = static_cast<quint16>(time.time().second());
			timeStamp.millisecond = static_cast<quint16>(time.time().msec());

			// data RupFrame
			//
			PS::FrameData* pFrameData = pSource->frameBase().frameDataPtr(currentFrameIndex);
			if (pFrameData != nullptr)
			{
				memcpy(m_simFrame.rupFrame.data, pFrameData->data(), Rup::FRAME_DATA_SIZE);
			}

			// version and IP of simFrame
			//
			m_simFrame.simVersion = reverseUint16(PS::SIM_FRAME_VERSION);
			m_simFrame.sourceIP = reverseUint32(pSource->info().lmAddress.address32());

			// revers header
			//
			m_simFrame.rupFrame.header.reverseBytes();

			// crc64 RupFrame
			//
			m_simFrame.rupFrame.calcCRC64();

			// send udp to AppDataReceivingIP of AppDataSrv
			//
			qint64 sentBytes = pSocket->writeDatagram(reinterpret_cast<char*>(&m_simFrame), sizeof(m_simFrame), QHostAddress(m_appDataSrvIP.address().toString()), m_appDataSrvIP.port());

			if (sentBytes != sizeof(m_simFrame))
			{
				qDebug() << "Error:" << __FUNCTION__ << ", Size of frame (bytes):" << sizeof(m_simFrame) << ", but sent (bytes):" << sentBytes;
			}

			// timeout
			//
			QThread::msleep(PS::SEND_TIMEOUT);

			m_sentFrames++;

			//
			//
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
	pSocket = nullptr;

	m_threadIsFinished = true;
	emit finished();
}

// -------------------------------------------------------------------------------------------------------------------

