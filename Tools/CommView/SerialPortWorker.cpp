#include "SerialPortWorker.h"

#include <QApplication>
#include <QDebug>
#include <QThread>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SerialPortWorker::SerialPortWorker(SerialPortOption *option) :
	m_option(option),
	m_port(nullptr),
    m_finishThread(false),
	m_disconnectSerialPort(false),
    m_timeout(0)
{
	m_port = new QSerialPort(this);
}

// -------------------------------------------------------------------------------------------------------------------

SerialPortWorker::~SerialPortWorker()
{
	if (m_port != nullptr)
	{
		if (m_port->isOpen() == true)
		{
			m_port->close();
		}

		delete m_port;
		m_port = nullptr;
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortWorker::openSerialPort()
{
	closeSerialPort();

	if (m_port == nullptr)
	{
		return false;
	}

	if (m_option == nullptr)
	{
		return false;
	}

	if (m_option->portName().isEmpty() == true)
	{
//		m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Port name is empty").arg(__FUNCTION__).arg(m_portName);
//		qDebug("%s", qPrintable(m_lastError));
//		emit error(m_lastError);
		return false;
	}

	if (m_option->baudRate() == 0)
	{
//		m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Unknown BaudRate").arg(__FUNCTION__).arg(m_portName);
//		qDebug("%s", qPrintable(m_lastError));
//		emit error(m_lastError);
		return false;
	}

	if (m_option->type() < 0 || m_option->type() >= RS_TYPE_COUNT)
	{
//		m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: Unknown type of RS").arg(__FUNCTION__).arg(m_portName);
//		qDebug("%s", qPrintable(m_lastError));
//		emit error(m_lastError);
		return false;
	}

	m_port->setPortName(m_option->portName());
	m_port->setBaudRate(m_option->baudRate());

	m_port->setDataBits(QSerialPort::Data8);
	m_port->setParity(QSerialPort::NoParity);
	m_port->setStopBits(QSerialPort::OneStop);
	m_port->setFlowControl(QSerialPort::NoFlowControl);

	if (m_port->open(QIODevice::ReadOnly) == false)
	{
//		m_lastError = tr("Calibrator error! Function: %1, Serial port: %2, Error description: %3 (%4)").arg(__FUNCTION__).arg(m_portName).arg(m_port.errorString()).arg(m_port.error());
//		qDebug("%s", qPrintable(m_lastError));
//		emit error(m_lastError);
		return false;
	}

	m_option->setConnected(true);

	connect(m_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));

	m_disconnectSerialPort = false;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortWorker::closeSerialPort()
{
	if (m_port != nullptr && m_port->isOpen() == true)
	{
		disconnect(m_port, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(serialPortError(QSerialPort::SerialPortError)));
		m_port->close();
	}

	if (m_option != nullptr)
	{
		m_option->setConnected(false);
		m_option->setReceivedBytes(0);
		m_option->setSkippedBytes(0);
        m_option->setPacketCount(0);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortWorker::serialPortError(QSerialPort::SerialPortError error)
{
	if (error == QSerialPort::ResourceError)	// on serial port disconnected
	{
		m_disconnectSerialPort = true;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SerialPortWorker::process()
{
	QByteArray requestData;

	while(m_finishThread == false)
	{
		if (m_port == nullptr || m_option == nullptr)
		{
			QThread::msleep(REQUEST_SERIAL_PORT_TIMEOUT);
			continue;
		}

		// try open port
		//
		if (m_port->isOpen() == false)
		{
			if (openSerialPort() == false)
			{
				QThread::msleep(REQUEST_SERIAL_PORT_TIMEOUT);
				continue;
			}
		}

		// close port if we have command about disconnected
		//
		if (m_disconnectSerialPort == true)
		{
			closeSerialPort();
			QThread::msleep(REQUEST_SERIAL_PORT_TIMEOUT);
			continue;
		}

		//
		//
		qApp->processEvents();

        if (m_timeout == TIMEOUT_COUNT && m_option->isNoReply() == false)
        {
            m_timeout = 0;
            m_option->setNoReply(true);

			m_option->setReceivedBytes(0);
            m_option->setSkippedBytes(0);
            m_option->setPacketCount(0);
        }

		m_option->setQueueBytes(static_cast<int>(m_port->bytesAvailable()));

		if (m_port->bytesAvailable() == 0)
		{
            m_timeout ++;
			QThread::msleep(REQUEST_SERIAL_PORT_TIMEOUT);
			continue;
		}

		// we have some data
		//
        m_timeout = 0;
        m_option->setNoReply(false);

		// read data
		//
		requestData += m_port->read(1);
		m_option->incReceivedBytes(1);

		// if amount of received data == size of packet
		//
		if (requestData.count() == m_option->dataSize())
		{
			SerialPortDataHeader* pHeader = reinterpret_cast<SerialPortDataHeader*>(requestData.data());	// take header of packet
			if (pHeader->Signature == SERIAL_PORT_DATA_SIGN)												// if header is correct
			{
				m_option->setData(requestData);																// update data in packet
				requestData.clear();																		// clear received data

				m_option->incPacketCount(1);																// inc packet count
			}
			else
			{
				requestData.remove(0, 1);																	//
				m_option->incSkippedBytes(1);																// inc skipped bytes
			}
		}

		// for test
		//
		if (m_option->testResult().isRunning() == true)
		{
			if (m_option->receivedBytes() >= (m_option->dataSize() * theOptions.testOption().maxPacketCount()))
			{
				m_option->saveTestResult();
			}
		}
	}

	emit finished();
}

// -------------------------------------------------------------------------------------------------------------------

bool SerialPortWorker::runTest()
{
	if (m_port == nullptr || m_option == nullptr)
	{
		return false;
	}

	if (m_port->isOpen() == false)
	{
		return false;
	}

	if (m_option->isNoReply() == true)
	{
		return false;
	}

	m_option->runTest();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
