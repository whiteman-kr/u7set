#include "SimTestUDPThread.h"
#include "PacketsMessages.h"

#include <QBasicTimer>
#include <QDebug>
#include <QNetworkDatagram>
#include <QSettings>
#include <QVariant>


SimTestUDPWorker::SimTestUDPWorker(QObject* parent) :
	QObject(parent)
{
	createSocket();
	createTimer();
}

void SimTestUDPWorker::getStat()
{
	QByteArray data = createRequestState(SGW_COMMAND_GET_STATE);

	int result = m_socket->writeDatagram(data.data(), data.size(), QHostAddress{m_settings.ip}, m_settings.portRemote);
	if (result == -1 && m_showServerState)
	{
		emit resultReady("getStat error in que");
	}
	else
	{
		if (result != data.size() && m_showServerState)
		{
			emit resultReady("getStat error size don't match");
		}
	}
}

void SimTestUDPWorker::onSimControlMode(const QString& mode)
{
	int dataType = -1;

	if (mode == "Start")
	{
		dataType = SGW_COMMAND_START;
	}
	else if (mode == "Stop")
	{
		dataType = SGW_COMMAND_STOP;
	}
	else if (mode == "Pause")
	{
		dataType = SGW_COMMAND_PAUSE;
	}
	else if (mode == "Resume")
	{
		dataType = SGW_COMMAND_RESUME;
	}
	else
	{
		emit resultReady(QString("Unknown mode: %1 ").arg(mode));
		return;
	}

	QByteArray data = createRequestState(dataType);

	int result = m_socket->writeDatagram(data.data(), data.size(), QHostAddress{m_settings.ip}, m_settings.portRemote);
	if (result == -1)
	{
		emit resultReady(QString("simControlMode : %1 error in que").arg(dataType));
	}
	else
	{
		if (result != data.size())
		{
			emit resultReady(QString("simControlMode : %1 error size don't match").arg(dataType));
		}
		else
		{
			emit resultReady(QString("simControlMode : %1 sent successfully").arg(dataType));
		}
	}
}

void SimTestUDPWorker::read(const QString& signalID)
{
	QByteArray data = createRequestRead(signalID);
	int result = m_socket->writeDatagram(data.data(), data.size(), QHostAddress{m_settings.ip}, m_settings.portRemote);

	m_pendingSignalID = signalID;

	if (result == -1)
	{
		emit resultReady("read error");
	}
	else
	{
		if (result != data.size())
		{
			emit resultReady("read error size don't match");
		}
	}
}

void SimTestUDPWorker::write(const QString& signalID, const QString& value)
{
	QByteArray data = createRequestWrite(signalID, value);
	int result = m_socket->writeDatagram(data.data(), data.size(), QHostAddress{m_settings.ip}, m_settings.portRemote);
	if (result == -1)
	{
		emit resultReady("write error in que");
	}
	else
	{
		if (result != data.size())
		{
			emit resultReady("write error size don't match");
		}
	}
}

void SimTestUDPWorker::createSocket()
{
	m_settings = AppSettings::load();
	if (m_socket)
	{
		m_socket->disconnect(this);
		m_socket->close();
		m_socket->deleteLater();
	}

	m_socket = new QUdpSocket(this);
	m_socket->bind(m_settings.portLocal);
	connect(m_socket, &QUdpSocket::readyRead, this, &SimTestUDPWorker::onReadyRead);
}

void SimTestUDPWorker::createTimer()
{
	m_timer.start(2000, this);
}

void SimTestUDPWorker::setShowServerState(bool enable)
{
	m_showServerState = enable;
}

void SimTestUDPWorker::onReadyRead()
{
	while (m_socket->hasPendingDatagrams())
	{
		QNetworkDatagram datagram = m_socket->receiveDatagram();
		QByteArray data = datagram.data();

		if (data.size() < sizeof(SimulatorBridgePacket) + sizeof(int16_t))
		{
			if (m_showServerState)
			{
				emit resultReady(QString("Datagram size is too small (%1): expected at least %2 bytes")
									 .arg(data.size())
									 .arg(sizeof(SimulatorBridgePacket)));
			}

			return;
		}

		const char* ptr = data.constData();

		const SimulatorBridgePacket* packet = reinterpret_cast<const SimulatorBridgePacket*>(ptr);
		ptr += sizeof(SimulatorBridgePacket);

		/*
		int16_t marker = *reinterpret_cast<const int16_t*>(ptr);
		ptr += sizeof(int16_t);

		int16_t pv = *reinterpret_cast<const int16_t*>(ptr);
		ptr += sizeof(int16_t);

		ptr += sizeof(int16_t); // reserved

		int16_t pSize = *reinterpret_cast<const int16_t*>(ptr);
		ptr += sizeof(int16_t);

		int16_t packetType = *reinterpret_cast<const int16_t*>(ptr);
		ptr += sizeof(int16_t);*/

		// chack size
		//
		bool sizeOk = (data.size() == packet->size);
		if (!sizeOk)
		{
			if (m_showServerState)
			{
				emit resultReady(QString("Datagram size mismatch: expected %1, got %2").arg(packet->size).arg(data.size()));
			}
			return;
		}

		// chack crc
		//
		int16_t receivedCrc = *reinterpret_cast<const int16_t*>(data.constData() + data.size() - sizeof(int16_t));
		int16_t calculatedCrc = calcCrc16(data.constData(), data.size() - sizeof(int16_t));
		bool crcOk = (receivedCrc == calculatedCrc);

		if (!crcOk)
		{
			if (m_showServerState)
			{
				emit resultReady(QString("Datagram CRC mismatch: expected %1, calculated %2").arg(receivedCrc).arg(calculatedCrc));
			}
			return;
		}

		switch (packet->packetType)
		{
		case SGW_COMMAND_GET_STATE:
		case SGW_COMMAND_START:
		case SGW_COMMAND_STOP:
		case SGW_COMMAND_PAUSE:
		case SGW_COMMAND_RESUME:
			{
				int expectedReplySize =
					sizeof(SimulatorBridgePacket) + sizeof(ErrorCode) + sizeof(SimulatorStateCode) + sizeof(int16_t) /*crc*/;
				if (packet->size != expectedReplySize)
				{
					emit resultReady(
						QString("SGW_COMMAND_GET_STATE: packet size is wrong (%1), expected: %2").arg(packet->size).arg(expectedReplySize));
					break;
				}

				ErrorCode errorCode = *reinterpret_cast<const ErrorCode*>(ptr);
				ptr += sizeof(ErrorCode);

				SimulatorStateCode state = *reinterpret_cast<const SimulatorStateCode*>(ptr);
				ptr += sizeof(SimulatorStateCode);

				emit resultReady(QString("Received: ErrorCode=%1, SimulatorStateCode=%2").arg(errorCodeToString(errorCode)).arg(state));

				emit simStateReady(errorCode, state);


				break;
			}
		case SGW_SIGNAL_READ:
			{
				// Check counter
				//
				int16_t counter = *reinterpret_cast<const int16_t*>(ptr);
				if (counter == 0)
				{
					emit resultReady(QString("SGW_SIGNAL_READ: wrong number of signals (%1), expected: 1").arg(counter));
					break;
				}
				ptr += sizeof(int16_t);

				// Check packet size
				int expectedReplySize =
					sizeof(SimulatorBridgePacket) + sizeof(int16_t) + sizeof(SignalState) * counter + sizeof(int16_t) /*crc*/;
				if (packet->size != expectedReplySize)
				{
					emit resultReady(
						QString("SGW_SIGNAL_READ: packet size is wrong (%1), expected: %2").arg(packet->size).arg(expectedReplySize));
					break;
				}

				// Process signal state
				//

				QString resultLog; // Grouped log for all signals in this reply
				const SignalState* state = reinterpret_cast<const SignalState*>(ptr);
				ptr += sizeof(SignalState);
				for (int i = 0; i < counter; ++i)
				{
					QString signalIdStr = m_pendingSignalID;
					if (counter > 1)
					{
						// If multiple, extract correct signalID from string
						QStringList ids = m_pendingSignalID.split('\n', Qt::SkipEmptyParts);
						if (i < ids.size())
							signalIdStr = ids[i];
					}
					else
					{
						signalIdStr = m_pendingSignalID;
					}
					state++; // chack state

					resultLog += QString("Read signal %1:\n").arg(signalIdStr);
					resultLog += QString("Value=%1, Hash=%2, Time=%3, Flags={valid: %4, stateAvailable: %5, simulated: %6, blocked: %7, "
										 "mismatch: %8, aboveHighLimit: %9, belowLowLimit: %10}\n")
									 .arg(state->value.fValue)
									 .arg(state->hash)
									 .arg(state->time)
									 .arg(state->flags.bits.valid)
									 .arg(state->flags.bits.stateAvailable)
									 .arg(state->flags.bits.simulated)
									 .arg(state->flags.bits.blocked)
									 .arg(state->flags.bits.mismatch)
									 .arg(state->flags.bits.aboveHighLimit)
									 .arg(state->flags.bits.belowLowLimit);

					emit resultReady(resultLog);
				}

				break;
			}
		case SGW_SIGNAL_WRITE:
			{
				// Check counter
				//
				int16_t counter = *reinterpret_cast<const int16_t*>(ptr);
				if (counter == 0)
				{
					emit resultReady(QString("SGW_SIGNAL_WRITE: wrong number of signals (%1), expected: 1").arg(counter));
					break;
				}
				ptr += sizeof(int16_t);

				// Check packet size
				int expectedReplySize =
					sizeof(SimulatorBridgePacket) + sizeof(int16_t) + sizeof(ErrorCode) * counter + sizeof(int16_t) /*crc*/;
				if (packet->size != expectedReplySize)
				{
					emit resultReady(
						QString("SGW_SIGNAL_WRITE: packet size is wrong (%1), expected: %2").arg(packet->size).arg(expectedReplySize));
					break;
				}


				// Get ErrorCode
				//
				const ErrorCode* errorCode = reinterpret_cast<const ErrorCode*>(ptr);
				ptr += sizeof(ErrorCode);
				for (int i = 0; i < counter; ++i)
				{
					emit resultReady(QString("Write result: ErrorCode = %1.").arg(errorCodeToString(*errorCode)));
					errorCode++; // chack error
				}
				break;
			}
		}
		int16_t crc = *reinterpret_cast<const int16_t*>(data.constData() + data.size() - sizeof(int16_t));

		// Test datagram
		emit resultReady(QString("Datagram arrived: marker=%1, version=%2, size=%3, type=%4, crc=%5")
							 .arg(packet->marker)
							 .arg(packet->packetVersion)
							 .arg(packet->size)
							 .arg(packet->packetType)
							 .arg(crc));
	}
}


QByteArray SimTestUDPWorker::createRequestState(int dataType)
{
	QByteArray result;

	// int size = sizeof(int16_t) * 5 + sizeof(int16_t);
	int size = sizeof(SimulatorBridgePacket) + sizeof(int16_t) /*crc*/;

	result.resize(size);
	result.fill(0);

	SimulatorBridgePacket* packet = reinterpret_cast<SimulatorBridgePacket*>(result.data());
	packet->marker = SGW_MARKER;
	packet->packetVersion = SGW_VERSION;
	packet->size = size;
	packet->packetType = dataType;

	/*

	int16_t* marker = reinterpret_cast<int16_t*>(data);
	*marker = SGW_MARKER;
	data += sizeof(int16_t);

	int16_t* pv = reinterpret_cast<int16_t*>(data);
	*pv = SGW_VERSION;
	data += sizeof(int16_t);

	data += sizeof(int16_t); // reserved

	int16_t* pSize = reinterpret_cast<int16_t*>(data);
	*pSize = size;
	data += sizeof(int16_t);

	int16_t* packetType = reinterpret_cast<int16_t*>(data);
	*packetType = dataType;
	data += sizeof(int16_t);*/


	int16_t* crc = reinterpret_cast<int16_t*>(result.data() + size - sizeof(int16_t));
	*crc = calcCrc16(result.data(), size - sizeof(int16_t));

	return result;
}

QByteArray SimTestUDPWorker::createRequestRead(const QString& signalID)
{
	QByteArray result;
	bool isMultypleSignals = signalID.contains('\n');
	int signalCount = 1;

	if (isMultypleSignals)
	{
		signalCount = signalID.count('\n') + 1;
	}

	int size = sizeof(SimulatorBridgePacket) + sizeof(int16_t) * signalCount + sizeof(Hash) * signalCount + sizeof(int16_t) /*crc*/;

	result.resize(size);
	result.fill(0);

	char* data = result.data();

	SimulatorBridgePacket* packet = reinterpret_cast<SimulatorBridgePacket*>(data);
	packet->marker = SGW_MARKER;
	packet->packetVersion = SGW_VERSION;
	packet->size = size;
	packet->packetType = SGW_SIGNAL_READ;
	data += sizeof(SimulatorBridgePacket);

	/*
	char* data = result.data();

	int16_t* marker = reinterpret_cast<int16_t*>(data);
	*marker = SGW_MARKER;
	data += sizeof(int16_t);

	int16_t* pv = reinterpret_cast<int16_t*>(data);
	*pv = SGW_VERSION;
	data += sizeof(int16_t);

	data += sizeof(int16_t); // reserved

	int16_t* pSize = reinterpret_cast<int16_t*>(data);
	*pSize = size;
	data += sizeof(int16_t);

	int16_t* packetType = reinterpret_cast<int16_t*>(data);
	*packetType = SGW_SIGNAL_READ;
	data += sizeof(int16_t);*/

	int16_t* signalIDCount = reinterpret_cast<int16_t*>(data);
	*signalIDCount = signalCount;
	data += sizeof(int16_t);

	// Calculate hash

	if (isMultypleSignals)
	{
		QStringList signalIDs = signalID.split('\n', Qt::SkipEmptyParts);

		for (const QString& id : signalIDs)
		{
			Hash* signalIDHash = reinterpret_cast<Hash*>(data);
			*signalIDHash = calcHash(id);
			data += sizeof(Hash);
		}
	}
	else
	{
		Hash* signalIDHash = reinterpret_cast<Hash*>(data);
		*signalIDHash = calcHash(signalID);
		data += sizeof(Hash);
	}


	int16_t* crc = reinterpret_cast<int16_t*>(result.data() + size - sizeof(int16_t));
	*crc = calcCrc16(result.data(), size - sizeof(int16_t));


	return result;
}

QByteArray SimTestUDPWorker::createRequestWrite(const QString& signalID, const QString& value)
{
	QByteArray result;

	bool isMultypleSignals = signalID.contains('\n');
	int signalCount = 1;

	if (isMultypleSignals)
	{
		signalCount = signalID.count('\n') + 1;
	}

	int size = sizeof(SimulatorBridgePacket) + sizeof(int16_t) + sizeof(Hash) * signalCount + sizeof(SignalValue) * signalCount +
			   sizeof(int16_t) /*crc*/;

	result.resize(size);
	result.fill(0);

	char* data = result.data();

	SimulatorBridgePacket* packet = reinterpret_cast<SimulatorBridgePacket*>(data);
	packet->marker = SGW_MARKER;
	packet->packetVersion = SGW_VERSION;
	packet->size = size;
	packet->packetType = SGW_SIGNAL_WRITE;
	data += sizeof(SimulatorBridgePacket);

	/*	int16_t* marker = reinterpret_cast<int16_t*>(data);
		*marker = SGW_MARKER;
		data += sizeof(int16_t);

		int16_t* pv = reinterpret_cast<int16_t*>(data);
		*pv = SGW_VERSION;
		data += sizeof(int16_t);

		data += sizeof(int16_t); // reserved


		int16_t* pSize = reinterpret_cast<int16_t*>(data);
		*pSize = size;
		data += sizeof(int16_t);


		int16_t* packetType = reinterpret_cast<int16_t*>(data);
		*packetType = SGW_SIGNAL_WRITE;
		data += sizeof(int16_t);*/

	int16_t* signalIDCount = reinterpret_cast<int16_t*>(data);
	*signalIDCount = signalCount;
	data += sizeof(int16_t);


	if (isMultypleSignals)
	{
		QStringList signalIDList = signalID.split('\n', Qt::SkipEmptyParts);
		for (const QString& id : signalIDList)
		{
			Hash* signalIDHash = reinterpret_cast<Hash*>(data);
			*signalIDHash = calcHash(id);
			data += sizeof(Hash);
		}
	}
	else
	{
		Hash* signalIDHash = reinterpret_cast<Hash*>(data);
		*signalIDHash = calcHash(signalID);
		data += sizeof(Hash);
	}

	// SignalValue v;
	if (isMultypleSignals)
	{
		QStringList valueList = value.split('\n', Qt::SkipEmptyParts);
		for (const QString& valueNumber : valueList)
		{
			SignalValue* valueData = reinterpret_cast<SignalValue*>(data);
			if (m_valueType == SignalType::AnalogInt32)
			{
				valueData->iValue = valueNumber.toInt();
			}
			else if (m_valueType == SignalType::Discrete)
			{
				valueData->bValue = valueNumber.contains("true");
			}
			else if (m_valueType == SignalType::AnalogFloat)
			{
				valueData->fValue = valueNumber.toFloat();
			}

			data += sizeof(SignalValue);
		}
	}
	else
	{
		SignalValue* valueData = reinterpret_cast<SignalValue*>(data);
		if (m_valueType == SignalType::AnalogInt32)
		{
			valueData->iValue = value.toInt();
		}
		else if (m_valueType == SignalType::Discrete)
		{
			valueData->bValue = value.contains("true");
		}
		else if (m_valueType == SignalType::AnalogFloat)
		{
			valueData->fValue = value.toFloat();
		}
		//*valueData = v;
		data += sizeof(SignalValue);
	}


	int16_t* crc = reinterpret_cast<int16_t*>(result.data() + size - sizeof(int16_t));
	*crc = calcCrc16(result.data(), size - sizeof(int16_t));

	return result;
}

void SimTestUDPWorker::timerEvent(QTimerEvent* event)
{
	getStat();
}

void SimTestUDPWorker::setValueType(SignalType type)
{
	m_valueType = type;
}

SimTestUDPController::SimTestUDPController()
{
	SimTestUDPWorker* worker = new SimTestUDPWorker;
	worker->moveToThread(&workerThread);
	connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

	connect(this, &SimTestUDPController::operateGetStat, worker, &SimTestUDPWorker::getStat);
	connect(this, &SimTestUDPController::operateRead, worker, &SimTestUDPWorker::read);
	connect(this, &SimTestUDPController::operateWrite, worker, &SimTestUDPWorker::write);
	connect(this, &SimTestUDPController::simControlMode, worker, &SimTestUDPWorker::onSimControlMode);

	connect(this, &SimTestUDPController::reloadSettings, worker, &SimTestUDPWorker::createSocket);

	connect(worker, &SimTestUDPWorker::resultReady, this, &SimTestUDPController::resultReady);

	connect(this, &SimTestUDPController::showServerStateChanged, worker, &SimTestUDPWorker::setShowServerState);

	connect(worker, &SimTestUDPWorker::simStateReady, this, &SimTestUDPController::simStateReady);

	connect(this, &SimTestUDPController::setValueType, worker, &SimTestUDPWorker::setValueType);

	workerThread.start();
}

SimTestUDPController::~SimTestUDPController()
{
	workerThread.quit();
	workerThread.wait();
}

void SimTestUDPController::setShowServerState(bool enable)
{
	emit showServerStateChanged(enable);
}