#include "SerialDataTesterServer.h"
#include "ui_SerialDataTesterServer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QSettings>
#include <string>
#include <math.h>
#include <QDebug>

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
	ui->portsList->setCurrentText(applicationSettings.value("currentPort").toString());
	ui->baudList->setCurrentText(applicationSettings.value("currentBaud").toString());

	m_timerForPackets = new QTimer(this);

	ui->stopServerButton->setEnabled(false);

	qsrand(qrand());

	const quint64 polynom = 0xd800000000000000ULL;	// CRC-64-ISO
	quint64 tempValue;
	for (int i=0; i<256; i++)
	{
		tempValue=i;
		for (int j = 8; j>0; j--)
		{
			if (tempValue & 1)
				tempValue = (tempValue >> 1) ^ polynom;
			else
				tempValue >>= 1;
		}
		crc_table[i] = tempValue;
	}

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
	if (ui->timeInterval->text().toInt() <= 10)
		QMessageBox::warning(this, tr("Warning"), tr("Time interval is lower or equal 10: possible loss of data"));

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
		applicationSettngs.setValue("currentPort", ui->portsList->currentText());
		applicationSettngs.setValue("currentBaud", ui->baudList->currentText());
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

			if(xmlReader.name() == "port")
			{
				if (attributes.hasAttribute("PortInfoStrID")
					&& attributes.hasAttribute("ID")
					&& attributes.hasAttribute("DataID")
					&& attributes.hasAttribute("Speed")
					&& attributes.hasAttribute("Bits")
					&& attributes.hasAttribute("StopBits")
					&& attributes.hasAttribute("ParityControl")
					&& attributes.hasAttribute("DataSize"))
				{
					m_dataSize = attributes.value("DataSize").toInt()/8;
				}
			}

			if(xmlReader.name() == "Signal")
			{

				if(attributes.hasAttribute("SignalStrID")
				   && attributes.hasAttribute("ExtStrID")
				   && attributes.hasAttribute("Name")
				   && attributes.hasAttribute("Type")
				   && attributes.hasAttribute("Unit")
				   && attributes.hasAttribute("DataSize")
				   && attributes.hasAttribute("DataFormat")
				   && attributes.hasAttribute("ByteOrder")
				   && attributes.hasAttribute("Offset")
				   && attributes.hasAttribute("BitNo"))
				{
					currentSignal.strId  = attributes.value("SignalStrID").toString();
					currentSignal.exStrId = attributes.value("ExtStrID").toString();
					currentSignal.name =  attributes.value("Name").toString();
					currentSignal.type = attributes.value("Type").toString();
					currentSignal.unit = attributes.value("Unit").toString();
					currentSignal.dataSize = attributes.value("DataSize").toInt();
					currentSignal.dataFormat = attributes.value("DataFormat").toString();
					currentSignal.byteOrder = attributes.value("ByteOrder").toString();
					currentSignal.offset = attributes.value("Offset").toInt();
					currentSignal.bit = attributes.value("BitNo").toInt();

					if (currentSignal.dataFormat == "discrete")
					{
						currentSignal.dataSize = 8;
					}
					else
					{
						currentSignal.dataSize/=8;
					}

					m_signalsFromXml.push_back(currentSignal);
					m_amountOfSignals++;
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Error reading attributes"));
					errorLoadingXml = true;
				}
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
	m_data.fill(0, m_dataSize*8);
	m_packet.clear();

	for (SignalData signal : m_signalsFromXml)
	{
		// Get type of the signal
		//

		quint64 value;

		if (signal.type == "analog")
		{
			quint64 maxDataSize = pow(2, signal.dataSize*8);

			value = qrand()%maxDataSize;

			if (signal.dataSize == 2 && signal.dataFormat == "SignedInt")
			{
				value += qrand()%maxDataSize;
			}

			if (signal.dataSize == 4)
			{
				value*=qrand()%10000*qrand()%100;
			}

			if (signal.dataFormat == "Float")
			{
				int pos = 31; // declare variables
				float f = value * 0.1;
				int bit = 0;

				int *b = reinterpret_cast<int*>(&f);
				for (int binaryNumber = 31; binaryNumber >=0; binaryNumber--)
				{
				bit = ((*b >> binaryNumber)&1);
				if (bit == 0)
					m_data.setBit(signal.offset + signal.bit + pos, 0);
				else
					m_data.setBit(signal.offset + signal.bit + pos, 1);
				pos--;
				}
			}
			else
			{

				for (int pos = 0; value>0; pos++)
				{
					if (value%2 == 0)
					{
						m_data.setBit(signal.offset + signal.bit + pos, 0);
					}
					else
					{
						m_data.setBit(signal.offset + signal.bit + pos, 1);
					}
					value/=2;
				}
			}
		}
		else
		{
			m_data.setBit(signal.offset + signal.bit, qrand()%2);
		}
	}

	QByteArray recorderedValues;
	recorderedValues.resize(m_dataSize);

	recorderedValues.fill(0);

	for(int b=0; b<m_data.count();++b) {
		recorderedValues[b/8] = (recorderedValues.at(b/8) | ((m_data[b]?1:0)<<((b%8))));
	}

	m_dataAmount = m_dataSize;


	QDataStream packetDataStream(&m_packet, QIODevice::WriteOnly);

	quint32 signature;

	if (m_numberOfPacket%45 == 0 && m_numberOfPacket != 0)
	{
		signature = 0x00000000;
	}
	else
	{
		signature = m_signature;
	}

	packetDataStream << signature;
	ui->signature->setText(QString::number(signature, 16));
	//}

	QString stringData;
	for (int currentBitPos = 0; currentBitPos < m_data.size(); currentBitPos++)
	{
		switch(m_data.at(currentBitPos))
		{
			case 0: stringData.append("0"); break;
			case 1: stringData.append("1"); break;
		}
		if ((currentBitPos+1) % 8 == 0)
		{
			stringData.append(" ");
		}
	}

	packetDataStream << m_version << m_Id << m_numerator << m_dataAmount << m_dataUniqueId << recorderedValues;

	ui->version->setText(QString::number(m_version));
	ui->transmissionId->setText(QString::number(m_Id));
	ui->numerator->setText(QString::number(m_numerator));
	ui->dataUniqueId->setText(QString::number(m_dataUniqueId));
	ui->dataAmount->setText(QString::number(m_dataAmount));
	ui->data->setText(stringData);

	qDebug() << m_data;

	ui->crc->setText("Calculated crc");

	// Calculate CRC-64
	//

	std::string dataToCalculateCrc = QString::fromLocal8Bit(recorderedValues).toStdString();

	char *crcData = &dataToCalculateCrc[0];

	quint64 crc = 0;
	for (int i=0; i<dataToCalculateCrc.size(); i++)
	{
		crc = crc_table[(crc ^ (crcData[i])) & 0xFF] ^ (crc >> 8);
	}
	crc = ~crc;

	if (m_numberOfPacket%30 == 0 && m_numberOfPacket != 0)
	{
		crc = 0;
		ui->crc->setText("Error crc");
	}

	packetDataStream << crc;

	m_serialPort->write(m_packet);
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
