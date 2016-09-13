#include "PortReceiver.h"
#include "SerialDataParser.h"
#include <QSettings>
#include <QDir>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

PortReceiver::PortReceiver(QObject *parent) : QThread(parent)
{

	QSettings applicationSettings;
	QString portName = applicationSettings.value("port").toString();
	quint32 baud = applicationSettings.value("baud").toInt();

	m_serialPort = new QSerialPort(this);
	m_serialPort->setPortName(portName);
	m_serialPort->setBaudRate(baud);
	m_serialPort->setDataBits(QSerialPort::Data8);
	m_serialPort->setParity(QSerialPort::NoParity);
	m_serialPort->setStopBits(QSerialPort::TwoStop);
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

	connect(m_serialPort, &QSerialPort::readyRead, this, &PortReceiver::dataReceived);
}

PortReceiver::~PortReceiver()
{
}

void PortReceiver::setPort(const QString &port)
{
	m_serialPort->setPortName(port);
	emit portClosed();
}

void PortReceiver::setBaud(const int& baud)
{
	m_serialPort->setBaudRate(baud);
	emit portClosed();
}

void PortReceiver::setDataBits(const QSerialPort::DataBits &dataBits)
{
	m_serialPort->setDataBits(dataBits);
	emit portClosed();
}

void PortReceiver::setStopBits(const QSerialPort::StopBits& stopBits)
{
	m_serialPort->setStopBits(stopBits);
	emit portClosed();
}

void PortReceiver::openPort()
{
	m_serialPort->open(QIODevice::ReadOnly);

	if (m_serialPort->isOpen() == false)
	{
		emit portError("Serial Port: " + m_serialPort->errorString());
		emit portClosed();
	}
}

void PortReceiver::closePort()
{
	if (m_serialPort->isOpen())
	{
		m_serialPort->close();
	}

	emit portClosed();
}

void PortReceiver::dataReceived()
{
	QByteArray receivedData = m_serialPort->read(SerialParserBufferSize);

	emit dataFromPort(receivedData);
}
