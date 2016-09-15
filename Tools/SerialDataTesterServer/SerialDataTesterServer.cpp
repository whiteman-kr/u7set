#include "SerialDataTesterServer.h"
#include "ui_SerialDataTesterServer.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QSettings>
#include <string>
#include <math.h>
#include <QDebug>

QByteArray bitsToBytes(QBitArray bits) {
	QByteArray bytes;
	bytes.resize(bits.count()/8);
	bytes.fill(0);
	// Convert from QBitArray to QByteArray
	for(int b=0; b<bits.count(); ++b)
		bytes[b/8] = ( bytes.at(b/8) | ((bits[b]?1:0)<<(b%8)));
	return bytes;
}

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
	ui->countOfPackets->setRange(0, 1000000);

	QSettings applicationSettings;

	ui->pathToXml->setText(applicationSettings.value("pathToSignals").toString());
	ui->countOfPackets->setValue(applicationSettings.value("amountOfPackets").toInt());
	ui->timeInterval->setValue(applicationSettings.value("timeInterval").toInt());
	ui->portsList->setCurrentText(applicationSettings.value("currentPort").toString());
	ui->baudList->setCurrentText(applicationSettings.value("currentBaud").toString());

	ui->countOfPackets->setEnabled(false);

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

	ui->signature->setText("424D4C47");
	ui->version->setText("1");

	connect(ui->openFileButton, &QPushButton::clicked, this, &SerialDataTesterServer::setFile);
	connect(m_timerForPackets, &QTimer::timeout, this, &SerialDataTesterServer::sendPacket);
	connect(ui->startServerButton, &QPushButton::clicked, this, &SerialDataTesterServer::startServer);
	connect(ui->stopServerButton, &QPushButton::clicked, this, &SerialDataTesterServer::stopServer);
	connect(ui->usePackets, &QCheckBox::clicked, this, &SerialDataTesterServer::usePacketAmount);
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
		applicationSettngs.setValue("currentPort", ui->portsList->currentText());
		applicationSettngs.setValue("currentBaud", ui->baudList->currentText());
	}
}

void SerialDataTesterServer::parseFile()
{
	m_signalsFromXml.clear();
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

			if(xmlReader.name() == "PortInfo")
			{
				if (attributes.hasAttribute("StrID")
				        && attributes.hasAttribute("ID")
				        && attributes.hasAttribute("DataID")
				        && attributes.hasAttribute("Speed")
				        && attributes.hasAttribute("Bits")
				        && attributes.hasAttribute("StopBits")
				        && attributes.hasAttribute("ParityControl")
				        && attributes.hasAttribute("DataSize"))
				{
					m_dataSize = attributes.value("DataSize").toInt();
				}
			}

			if(xmlReader.name() == "Signal")
			{

				if(attributes.hasAttribute("StrID")
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
						currentSignal.dataSize = 1;
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
	// Just stopping server: close serial port and stop timer. After that unlock all UI
	//

	m_serialPort->close();
	m_timerForPackets->stop();

	ui->portStatus->setText("Closed");
	ui->baud->setText("No data");
	ui->port->setText("No data");
	ui->pathToXml->setEnabled(true);
	ui->portsList->setEnabled(true);
	ui->baudList->setEnabled(true);
	ui->timeInterval->setEnabled(true);
	ui->countOfPackets->setEnabled(ui->usePackets->isChecked());
	ui->startServerButton->setEnabled(true);
	ui->openFileButton->setEnabled(true);
	ui->stopServerButton->setEnabled(false);
}

void SerialDataTesterServer::sendPacket()
{
	// First of all - clear port
	//

	m_serialPort->clear();

	// Create signature and write down data to structure
	//

	Signature sign;

	sign.uint32 = 0x424D4C47;

	// Create header, and write down data to header
	//

	HeaderUnion head;

	head.hdr.version = 4;
	head.hdr.id = 1;
	head.hdr.num = 1;
	head.hdr.amount = m_dataSize;

	// Now, start data processing. First of all we need create data array,
	// where we can store

	QByteArray dataToSend;
	dataToSend.clear();

	// Now, create bit array, where we will record our values
	//

	QBitArray generatedData;
	generatedData.resize(m_dataSize*8);
	generatedData.fill(0);

	for (SignalData signal : m_signalsFromXml)
	{
		// Get type of the signal
		//

		quint64 value;

		if (signal.type == "Analog")
		{
			quint64 maxDataSize = pow(2, signal.dataSize*8);

			value = qrand()%maxDataSize;

			if (signal.dataSize == 16 && signal.dataFormat == "SignedInt")
			{
				value += qrand()%maxDataSize;
			}

			if (signal.dataSize == 32)
			{
				value*=qrand()%10000*qrand()%100;
			}

			if (signal.dataFormat == "Float")
			{
				int pos = 31;
				float f = qrand()%maxDataSize * 0.1;
				int bit = 0;

				int *b = reinterpret_cast<int*>(&f);
				for (int binaryNumber = 31; binaryNumber >=0; binaryNumber--)
				{
					bit = ((*b >> binaryNumber)&1);
					if (bit == 0)
						generatedData.setBit(signal.offset*8 + signal.bit + pos, 0);
					else
						generatedData.setBit(signal.offset*8 + signal.bit + pos, 1);
					pos--;
				}
			}
			else
			{
				for (int pos = 0; value>0; pos++)
				{
					if (value%2 == 0)
					{
						generatedData.setBit(signal.offset*8 + signal.bit + pos, 0);
					}
					else
					{
						generatedData.setBit(signal.offset*8 + signal.bit + pos, 1);
					}
					value/=2;
				}
			}
		}
		else
		{
			generatedData.setBit(signal.offset*8 + signal.bit, qrand()%2);
		}
	}

	QString visualizeTransferringData;

	for (int currentBit=0; currentBit<generatedData.size(); currentBit++)
	{
		visualizeTransferringData.append(generatedData.at(currentBit) == 1 ? "1" : "0");
		if ((currentBit+1)%8 == 0)
			visualizeTransferringData.append(" ");
	}

	dataToSend.append(bitsToBytes(generatedData));

	ui->data->setText(visualizeTransferringData);

	QByteArray bytes;
	bytes.clear();

	bytes.reserve(12);

	int amountOfRandomTrashBytes = qrand()%1001;

	for (int i=0; i<amountOfRandomTrashBytes; i++)
	{
		bytes.append((char)qrand()%256);
	}

	// Write down signature to packet (4 bytes);
	//

	bytes.append(sign.bytes, 4);

	// Write down header bytes (8 bytes);
	//

	bytes.append(head.bytes, 8);

	// Write down packet data
	//

	DataUniqueId dataID;

	dataID.bytes[0] = '1';
	dataID.bytes[1] = '1';
	dataID.bytes[2] = '1';
	dataID.bytes[3] = '1';

	bytes.append(dataID.bytes, 4);
	bytes.append(dataToSend, dataToSend.length());

	QByteArray dataForCrc;

	dataForCrc.append(head.bytes, 8);
	dataForCrc.append(dataID.bytes, 4);
	dataForCrc.append(dataToSend, dataToSend.length());

	CrcRepresentation crc;
	crc.uint64 = Crc::crc64(dataForCrc, dataForCrc.length());

	bytes.append(crc.bytes, 8);

	// Write packet to port
	//

	amountOfRandomTrashBytes = qrand()%1001;

	for (int i=0; i<amountOfRandomTrashBytes; i++)
	{
		bytes.append((char)qrand()%256);
	}
	qDebug() << "Sent: " <<  m_serialPort->write(bytes, bytes.size()) << " bytes";

	// Send data
	//

	ui->transmissionId->setText(QString::number(head.hdr.id));
	ui->numerator->setText(QString::number(head.hdr.num));
	ui->dataAmount->setText(QString::number(head.hdr.amount));
	ui->dataUniqueId->setText(QString::number(dataID.uint32));
	ui->crc->setText(QString::number(crc.uint64));
	m_numberOfPacket++;
	ui->packetNumber->setText(QString::number(m_numberOfPacket));

	if (m_numberOfPacket == ui->countOfPackets->text().toInt() && ui->usePackets->isChecked())
	{
		stopServer();
	}
}

void SerialDataTesterServer::usePacketAmount()
{
	ui->countOfPackets->setEnabled(ui->usePackets->isChecked());
}
