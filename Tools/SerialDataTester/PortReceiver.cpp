#include "PortReceiver.h"
#include <QSettings>
#include <QDir>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

PortReceiver::PortReceiver(QObject *parent) : QObject(parent)
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
	m_serialPort->close();
	m_serialPort->setPortName(port);
	openPort();
}

void PortReceiver::setBaud(const int& baud)
{
	m_serialPort->close();
	m_serialPort->setBaudRate(baud);
	openPort();
}

void PortReceiver::setDataBits(const QSerialPort::DataBits &dataBits)
{
	m_serialPort->close();
	m_serialPort->setDataBits(dataBits);
	openPort();
}

void PortReceiver::setStopBits(const QSerialPort::StopBits& stopBits)
{
	m_serialPort->close();
	m_serialPort->setStopBits(stopBits);
	openPort();
}

void PortReceiver::openPort()
{
	m_serialPort->open(QIODevice::ReadOnly);

	if (m_serialPort->isOpen() == false)
	{
		emit portError("Serial Port: " + m_serialPort->errorString());
	}
}

void PortReceiver::closePort()
{
	m_serialPort->close();
}

void PortReceiver::dataReceived()
{
	QByteArray receivedData;
	receivedData = m_serialPort->readAll();

	emit dataFromPort(receivedData);
}
