#include "SimTestUDPThread.h"
#include "PacketsMessages.h"

#include <QBasicTimer>
#include <QDebug>
#include <QNetworkDatagram>
#include <QSettings>
#include <QVariant>

#include "../../UtilsLib/Crc.h"


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
		emit resultReady(tr("getStat error in queue"));
	}
	else
	{
		if (result != data.size() && m_showServerState)
		{
			emit resultReady(tr("getStat error size don't match"));
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
		emit resultReady(QString(tr("Unknown mode: %1 ")).arg(mode));
		return;
	}

	QByteArray data = createRequestState(dataType);

	int result = m_socket->writeDatagram(data.data(), data.size(), QHostAddress{m_settings.ip}, m_settings.portRemote);
	if (result == -1)
	{
		emit resultReady(QString(tr("simControlMode : %1 error in queue")).arg(dataType));
	}
	else
	{
		if (result != data.size())
		{
			emit resultReady(QString(tr("simControlMode : %1 error size don't match")).arg(dataType));
		}
		else
		{
			emit resultReady(QString(tr("simControlMode : %1 sent successfully")).arg(dataType));
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
		emit resultReady(tr("read error"));
	}
	else
	{
		if (result != data.size())
		{
			emit resultReady(tr("read error size don't match"));
		}
	}
}

void SimTestUDPWorker::write(const QString& signalID, const QString& value)
{
	QByteArray data = createRequestWrite(signalID, value);
	m_pendingSignalID = signalID;
	int result = m_socket->writeDatagram(data.data(), data.size(), QHostAddress{m_settings.ip}, m_settings.portRemote);
	if (result == -1)
	{
		emit resultReady(tr("write error in queue"));
	}
	else
	{
		if (result != data.size())
		{
			emit resultReady(tr("write error size don't match"));
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
				emit resultReady(QString(tr("Datagram size is too small (%1): expected at least %2 bytes"))
									 .arg(data.size())
									 .arg(sizeof(SimulatorBridgePacket)));
			}

			return;
		}

		const char* ptr = data.constData();

		const SimulatorBridgePacket* packet = reinterpret_cast<const SimulatorBridgePacket*>(ptr);
		ptr += sizeof(SimulatorBridgePacket);

		// check size
		//
		bool sizeOk = (data.size() == packet->size);
		if (sizeOk == false)
		{
			if (m_showServerState == true)
			{
				emit resultReady(QString(tr("Datagram size mismatch: expected %1, got %2")).arg(packet->size).arg(data.size()));
			}
			return;
		}

		// check crc
		//
		int16_t receivedCrc = *reinterpret_cast<const int16_t*>(data.constData() + data.size() - sizeof(int16_t));
		int16_t calculatedCrc = calcCrc16(data.constData(), static_cast<int>(data.size() - sizeof(int16_t)));
		bool crcOk = (receivedCrc == calculatedCrc);

		if (crcOk == false)
		{
			if (m_showServerState == true)
			{
				emit resultReady(QString(tr("Datagram CRC mismatch: expected %1, calculated %2")).arg(receivedCrc).arg(calculatedCrc));
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
						QString(tr("SGW_COMMAND_GET_STATE: packet size is wrong (%1), expected: %2")).arg(packet->size).arg(expectedReplySize));
					break;
				}

				ErrorCode errorCode = *reinterpret_cast<const ErrorCode*>(ptr);
				ptr += sizeof(ErrorCode);

				SimulatorStateCode state = *reinterpret_cast<const SimulatorStateCode*>(ptr);
				ptr += sizeof(SimulatorStateCode);

				emit resultReady(QString(tr("Received: ErrorCode=%1, SimulatorStateCode=%2")).arg(errorCodeToString(errorCode)).arg(state));

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
					emit resultReady(QString(tr("SGW_SIGNAL_READ: wrong number of signals (%1), expected: 1")).arg(counter));
					break;
				}
				ptr += sizeof(int16_t);

				// Check packet size
				int expectedReplySize =
					sizeof(SimulatorBridgePacket) + sizeof(int16_t) + sizeof(SignalState) * counter + sizeof(int16_t) /*crc*/;
				if (packet->size != expectedReplySize)
				{
					emit resultReady(
						QString(tr("SGW_SIGNAL_READ: packet size is wrong (%1), expected: %2")).arg(packet->size).arg(expectedReplySize));
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
						QStringList ids = m_pendingSignalID.split('|', Qt::SkipEmptyParts);
						if (i < ids.size())
						{
							signalIdStr = ids[i];
						}
					}

					
					if (signalIdStr.contains("|") == true)
					{
						continue;
					}

					double resiveValue = 0;
					switch (m_valueType)
					{
					case SignalType::AnalogFloat:
						resiveValue = state->value.fValue;
						break;
					case SignalType::AnalogInt32:
						resiveValue = state->value.iValue;
						break;
					case SignalType::Discrete:
						resiveValue = state->value.bValue;
						break;
					default:
						Q_ASSERT(false);
					}

					resultLog += QString(tr("Read signal %1:\n")).arg(signalIdStr);
					resultLog += QString(tr("Value=%1, Hash=%2, Time=%3, Flags={valid: %4, stateAvailable: %5, simulated: %6, blocked: %7, "
										 "mismatch: %8, aboveHighLimit: %9, belowLowLimit: %10}\n"))
									 .arg(resiveValue)
									 .arg(state->hash)
									 .arg(state->time)
									 .arg(state->flags.bits.valid)
									 .arg(state->flags.bits.stateAvailable)
									 .arg(state->flags.bits.simulated)
									 .arg(state->flags.bits.blocked)
									 .arg(state->flags.bits.mismatch)
									 .arg(state->flags.bits.aboveHighLimit)
									 .arg(state->flags.bits.belowLowLimit);
					state++; // check state
				}
				emit resultReady(resultLog);

				break;
			}
		case SGW_SIGNAL_WRITE:
			{
				// Check counter
				//
				int16_t counter = *reinterpret_cast<const int16_t*>(ptr);
				if (counter == 0)
				{
					emit resultReady(QString(tr("SGW_SIGNAL_WRITE: wrong number of signals (%1), expected: 1")).arg(counter));
					break;
				}
				ptr += sizeof(int16_t);

				// Check packet size
				int expectedReplySize =
					sizeof(SimulatorBridgePacket) + sizeof(int16_t) + sizeof(ErrorCode) * counter + sizeof(int16_t) /*crc*/;
				if (packet->size != expectedReplySize)
				{
					emit resultReady(
						QString(tr("SGW_SIGNAL_WRITE: packet size is wrong (%1), expected: %2")).arg(packet->size).arg(expectedReplySize));
					break;
				}


				// Get ErrorCode
				//
				const ErrorCode* errorCode = reinterpret_cast<const ErrorCode*>(ptr);
				ptr += sizeof(ErrorCode);
				for (int i = 0; i < counter; ++i)
				{
					if (*errorCode == Success)
					{
						emit resultReady(QString(tr("Write result: ErrorCode = %1.")).arg(errorCodeToString(*errorCode)));
					}
					else
					{
						QString signalIdStr = m_pendingSignalID;
						if (counter > 1)
						{
							// If multiple, extract correct signalID from string
							QStringList ids = m_pendingSignalID.split('|', Qt::SkipEmptyParts);
							if (i < ids.size())
							{
								signalIdStr = ids[i];
							}

							emit resultReady(
								QString(tr("Signal %1 write error: ErrorCode = %2.")).arg(signalIdStr).arg(errorCodeToString(*errorCode)));
						}

						errorCode++; // check error
					}
				}
			}
			int16_t crc = *reinterpret_cast<const int16_t*>(data.constData() + data.size() - sizeof(int16_t));

			// Test datagram
			emit resultReady(QString(tr("Datagram arrived: marker=%1, version=%2, size=%3, type=%4, crc=%5"))
								 .arg(packet->marker)
								 .arg(packet->packetVersion)
								 .arg(packet->size)
								 .arg(packet->packetType)
								 .arg(crc));
		}
	}
}

QByteArray SimTestUDPWorker::createRequestState(int dataType)
{
	QByteArray result;

	int size = sizeof(SimulatorBridgePacket) + sizeof(int16_t) /*crc*/;

	result.resize(size);
	result.fill(0);

	SimulatorBridgePacket* packet = reinterpret_cast<SimulatorBridgePacket*>(result.data());
	packet->marker = SGW_MARKER;
	packet->packetVersion = SGW_VERSION;
	packet->size = size;
	packet->packetType = dataType;

	int16_t* crc = reinterpret_cast<int16_t*>(result.data() + size - sizeof(int16_t));
	*crc = calcCrc16(result.data(), size - sizeof(int16_t));

	return result;
}

QByteArray SimTestUDPWorker::createRequestRead(const QString& signalID)
{
	QByteArray result;
	bool isMultypleSignals = signalID.contains('|');
	int signalCount = 1;

	if (isMultypleSignals ==true)
	{
		signalCount = signalID.count('|') + 1;
	}

	int size = sizeof(SimulatorBridgePacket) + sizeof(int16_t) + sizeof(Hash) * signalCount + sizeof(int16_t) /*crc*/;

	result.resize(size);
	result.fill(0);

	char* data = result.data();

	SimulatorBridgePacket* packet = reinterpret_cast<SimulatorBridgePacket*>(data);
	packet->marker = SGW_MARKER;
	packet->packetVersion = SGW_VERSION;
	packet->size = size;
	packet->packetType = SGW_SIGNAL_READ;
	data += sizeof(SimulatorBridgePacket);

	int16_t* signalIDCount = reinterpret_cast<int16_t*>(data);
	*signalIDCount = signalCount;
	data += sizeof(int16_t);

	// Calculate hash
	//
	if (isMultypleSignals == true)
	{
		QStringList signalIDs = signalID.split('|', Qt::SkipEmptyParts);

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

	bool isMultypleSignals = signalID.contains('|');
	int signalCount = 1;

	if (isMultypleSignals == true)
	{
		signalCount = signalID.count('|') + 1;
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

	int16_t* signalIDCount = reinterpret_cast<int16_t*>(data);
	*signalIDCount = signalCount;
	data += sizeof(int16_t);

	if (isMultypleSignals == true)
	{
		QStringList signalIDList = signalID.split('|', Qt::SkipEmptyParts);
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
	//
	if (isMultypleSignals == true)
	{
		QStringList valueList = value.split('|', Qt::SkipEmptyParts);
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
		//
		data += sizeof(SignalValue);
	}


	int16_t* crc = reinterpret_cast<int16_t*>(result.data() + size - sizeof(int16_t));
	*crc = calcCrc16(result.data(), size - sizeof(int16_t));

	return result;
}

void SimTestUDPWorker::timerEvent(QTimerEvent* /*event*/)
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
	worker->moveToThread(&m_workerThread);
	connect(&m_workerThread, &QThread::finished, worker, &QObject::deleteLater);

	connect(this, &SimTestUDPController::operateGetStat, worker, &SimTestUDPWorker::getStat);
	connect(this, &SimTestUDPController::operateRead, worker, &SimTestUDPWorker::read);
	connect(this, &SimTestUDPController::operateWrite, worker, &SimTestUDPWorker::write);
	connect(this, &SimTestUDPController::simControlMode, worker, &SimTestUDPWorker::onSimControlMode);

	connect(this, &SimTestUDPController::reloadSettings, worker, &SimTestUDPWorker::createSocket);

	connect(worker, &SimTestUDPWorker::resultReady, this, &SimTestUDPController::resultReady);

	connect(this, &SimTestUDPController::showServerStateChanged, worker, &SimTestUDPWorker::setShowServerState);

	connect(worker, &SimTestUDPWorker::simStateReady, this, &SimTestUDPController::simStateReady);

	connect(this, &SimTestUDPController::setValueType, worker, &SimTestUDPWorker::setValueType);

	m_workerThread.start();
}

SimTestUDPController::~SimTestUDPController()
{
	m_workerThread.quit();
	m_workerThread.wait();
}

void SimTestUDPController::setShowServerState(bool enable)
{
	emit showServerStateChanged(enable);
}