#include "SerialDataTesterServer.h"
#include "ui_SerialDataTesterServer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QSettings>

SerialDataTesterServer::SerialDataTesterServer(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SerialDataTesterServer)
{
	ui->setupUi(this);
	for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
	{
		ui->portsList->addItem(port.portName());
	}

	ui->baudList->addItem(QString::number(QSerialPort::Baud115200));
	ui->baudList->addItem(QString::number(QSerialPort::Baud57600));
	ui->baudList->addItem(QString::number(QSerialPort::Baud38400));
	ui->baudList->addItem(QString::number(QSerialPort::Baud19200));

	ui->timeInterval->setRange(0, 10000);
	ui->countOfPackets->setRange(0, 10000);

	QSettings applicationSettings;

	ui->pathToXml->setText(applicationSettings.value("pathToSignals").toString());
	ui->countOfPackets->setValue(applicationSettings.value("amountOfPackets").toInt());
	ui->timeInterval->setValue(applicationSettings.value("timeInterval").toInt());

	m_timerForPackets = new QTimer(this);

	ui->stopServerButton->setEnabled(false);

	qsrand(qrand());

	connect(ui->openFileButton, &QPushButton::clicked, this, &SerialDataTesterServer::setFile);
	connect(m_timerForPackets, &QTimer::timeout, this, &SerialDataTesterServer::sendPacket);
	connect(ui->startServerButton, &QPushButton::clicked, this, &SerialDataTesterServer::startServer);
	connect(ui->stopServerButton, &QPushButton::clicked, this, &SerialDataTesterServer::stopServer);
}

SerialDataTesterServer::~SerialDataTesterServer()
{
	delete ui;
}

void SerialDataTesterServer::setFile()
{
	ui->pathToXml->setText(QFileDialog::getOpenFileName(this, tr("Open Signals.xml"), "", tr("XML-file (*.xml)")));
	QSettings applicationSettings;
	applicationSettings.setValue("pathToSignals", ui->pathToXml->text());
}

void SerialDataTesterServer::startServer()
{
	m_serialPort = new QSerialPort(this);
	m_serialPort->setPortName(ui->portsList->currentText());
	m_serialPort->setBaudRate(ui->baudList->currentText().toInt());
	m_serialPort->setDataBits(QSerialPort::Data8);
	m_serialPort->setParity(QSerialPort::NoParity);
	m_serialPort->setStopBits(QSerialPort::OneStop);
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

	if (m_serialPort->open(QIODevice::WriteOnly) == false)
	{
		QMessageBox::critical(this, tr("Critical error"), m_serialPort->errorString());
	}
	else if (ui->pathToXml->text().isEmpty())
	{
		QMessageBox::critical(this, tr("Critical error"), tr("Error: no path to xml-file"));
	}
	else
	{
		ui->portStatus->setText("Open");
		ui->baud->setText(QString::number(m_serialPort->baudRate()));
		ui->port->setText(m_serialPort->portName());

		ui->pathToXml->setEnabled(false);
		ui->portsList->setEnabled(false);
		ui->baudList->setEnabled(false);
		ui->countOfPackets->setEnabled(false);
		ui->startServerButton->setEnabled(false);
		ui->stopServerButton->setEnabled(true);
		ui->timeInterval->setEnabled(false);
		ui->openFileButton->setEnabled(false);

		parseFile();

		m_numberOfPacket = 0;

		m_timerForPackets->start(ui->timeInterval->text().toInt());

		QSettings applicationSettngs;
		applicationSettngs.setValue("amountOfPackets", ui->countOfPackets->text().toInt());
		applicationSettngs.setValue("timeInterval", ui->timeInterval->text().toInt());
	}
}

void SerialDataTesterServer::parseFile()
{
	QFile slgnalsXmlFile(ui->pathToXml->text());
	bool errorLoadingXml = false;

	if (slgnalsXmlFile.exists() == false)
	{
		QMessageBox::critical(this, tr("Critical error"), "File not found: " + ui->pathToXml->text());
		errorLoadingXml = true;
	}

	if (slgnalsXmlFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		QMessageBox::critical(this, tr("Critical error"), "Error opening " + ui->pathToXml->text());
		errorLoadingXml = true;
	}

	// Ok, now start processing XML-file and write all signals
	// from file to vector
	//

	QXmlStreamReader xmlReader(&slgnalsXmlFile);
	SignalData currentSignal;

	while(xmlReader.atEnd() == false && xmlReader.error() == false)
	{
		QXmlStreamReader::TokenType token = xmlReader.readNext();

		if(token == QXmlStreamReader::StartDocument)
		{
			continue;
		}

		if(token == QXmlStreamReader::StartElement)
		{
			QXmlStreamAttributes attributes = xmlReader.attributes();

			if(xmlReader.name() == "signal")
			{
				if(attributes.hasAttribute("strId"))
				{
					currentSignal.strId  = attributes.value("strId").toString();
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Can not read STRID"));
					errorLoadingXml = true;
				}

				if (attributes.hasAttribute("caption"))
				{
					currentSignal.caption =  attributes.value("caption").toString();
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Can not read caption"));
					errorLoadingXml = true;
				}

				if (attributes.hasAttribute("offset"))
				{
					currentSignal.offset = attributes.value("offset").toInt();
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Can not read offset"));
					errorLoadingXml = true;
				}

				if (attributes.hasAttribute("bit"))
				{
					currentSignal.bit = attributes.value("bit").toInt();
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Can not read signal bit"));
					errorLoadingXml = true;
				}

				if (attributes.hasAttribute("type"))
				{
					currentSignal.type = attributes.value("type").toString();
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Can not read signal type"));
					errorLoadingXml = true;
				}

				m_signalsFromXml.push_back(currentSignal);
				m_amountOfSignals++;
			}
		}
	}

	if (xmlReader.error())
	{
		QMessageBox::critical(this, tr("Critical error"), xmlReader.errorString());
		errorLoadingXml = true;
	}

	if (errorLoadingXml)
	{
		m_signalsFromXml.clear();
	}

	slgnalsXmlFile.close();
}

void SerialDataTesterServer::stopServer()
{
	m_serialPort->close();
	ui->portStatus->setText("Closed");
	ui->baud->setText("No data");
	ui->port->setText("No data");
	m_timerForPackets->stop();
	ui->pathToXml->setEnabled(true);
	ui->portsList->setEnabled(true);
	ui->baudList->setEnabled(true);
	ui->timeInterval->setEnabled(true);
	ui->countOfPackets->setEnabled(true);
	ui->startServerButton->setEnabled(true);
	ui->openFileButton->setEnabled(true);
	ui->stopServerButton->setEnabled(false);
}

void SerialDataTesterServer::sendPacket()
{
	int signalNumber = qrand()%m_amountOfSignals;

	m_dataOffset = m_signalsFromXml[signalNumber].offset;
	m_dataBits = m_signalsFromXml[signalNumber].bit;
	if (m_signalsFromXml[signalNumber].type == "Analog")
		m_dataValue = 500 - qrand()%1001;
	else
		m_dataValue = qrand()%2;

	m_bytes.clear();

	QDataStream packetDataStream(&m_bytes, QIODevice::WriteOnly);

	packetDataStream << m_signature << m_version << m_Id << m_numerator << m_dataAmount << m_dataUniqueId << m_dataOffset << m_dataBits << m_dataValue;

	ui->signature->setText(QString::number(m_signature, 16));
	ui->version->setText(QString::number(m_version));
	ui->transmissionId->setText(QString::number(m_Id));
	ui->numerator->setText(QString::number(m_numerator));
	ui->dataUniqueId->setText(QString::number(m_dataUniqueId));

	m_serialPort->write(m_bytes);
	bool result = m_serialPort->flush();

	if (result == false)
	{
		QMessageBox::critical(this, tr("Critical error"), m_serialPort->errorString());
	}

	m_numberOfPacket++;
	ui->packetNumber->setText(QString::number(m_numberOfPacket));

	if (m_numberOfPacket == ui->countOfPackets->text().toInt())
	{
		stopServer();
	}
}
