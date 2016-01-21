#include "serialdatatester.h"
#include "SettingsDialog.h"
#include "PortReceiver.h"
#include "ui_serialdatatester.h"
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QSerialPortInfo>
#include <QThread>
#include <QDebug>
#include <string>

SerialDataTester::SerialDataTester(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SerialDataTester)
{
	ui->setupUi(this);

	// Menu "File"
	//

	m_file = new QMenu(tr("&File"));
	m_reloadCfg = new QAction(tr("Reload singals xml file"), this);
	m_changeSignalSettingsFile = new QAction(tr("Change signals file"), this);
	m_exit = new QAction(tr("Exit"), this);
	m_startReceiving = new QAction(tr("Start"), this);
	m_stopReceiving = new QAction(tr("Stop"), this);

	m_file->addAction(m_reloadCfg);
	m_file->addAction(m_changeSignalSettingsFile);
	m_file->addSeparator();
	m_file->addAction(m_startReceiving);
	m_file->addAction(m_stopReceiving);
	m_file->addSeparator();
	m_file->addAction(m_exit);

	// Menu "Settings"
	//

	m_settings = new QMenu(tr("&Settings"));
	m_setPort = new QMenu(tr("Set Port"));

	// Detect all ports
	//

	for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
	{
		m_setPort->addAction(port.portName());
	}

	for (QAction* port : m_setPort->actions())
	{
		port->setCheckable(true);
	}

	// Create Baud menu for port
	//

	m_setBaud = new QMenu(tr("Set Baud"));

	m_setBaud->addAction(QString::number(QSerialPort::Baud115200));
	m_setBaud->addAction(QString::number(QSerialPort::Baud57600));
	m_setBaud->addAction(QString::number(QSerialPort::Baud38400));
	m_setBaud->addAction(QString::number(QSerialPort::Baud19200));

	for (QAction* baud : m_setBaud->actions())
	{
		baud->setCheckable(true);
	}

	// Create Data Bits menu
	//

	m_setDataBits = new QMenu(tr("Set Data Bits"));

	m_setDataBits->addAction(QString::number(QSerialPort::Data8));
	m_setDataBits->addAction(QString::number(QSerialPort::Data7));
	m_setDataBits->addAction(QString::number(QSerialPort::Data6));
	m_setDataBits->addAction(QString::number(QSerialPort::Data5));

	for (QAction* dataBits : m_setDataBits->actions())
	{
		dataBits->setCheckable(true);
	}

	// Create Stop Bits menu
	//

	m_setStopBits = new QMenu(tr("Set Stop Bits"));

	m_setStopBits->addAction("One stop");
	m_setStopBits->addAction("One and half stop");
	m_setStopBits->addAction("Two stop");

	for (QAction* stopBits : m_setStopBits->actions())
	{
		stopBits->setCheckable(true);
	}

	// Create menu entry for erasing packet data and
	// for load old settings
	//

	m_erasePacketData = new QAction(tr("Erase packet info"), this);
	m_loadDefaultSettings = new QAction(tr("Load saved port settings"), this);

	m_settings->addMenu(m_setPort);
	m_settings->addMenu(m_setBaud);
	m_settings->addMenu(m_setDataBits);
	m_settings->addMenu(m_setStopBits);
	m_settings->addSeparator();
	m_settings->addAction(m_erasePacketData);
	m_settings->addAction(m_loadDefaultSettings);

	ui->menuBar->addMenu(m_file);
	ui->menuBar->addMenu(m_settings);

	// Draw the table headers
	//

	ui->signalsTable->setColumnCount(6);

	ui->signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(name, new QTableWidgetItem(tr("Name")));
	ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));
	ui->signalsTable->setHorizontalHeaderItem(value, new QTableWidgetItem(tr("Value")));

	// Detect old settings. In case, that there was no old settings,
	// open settings dialog to create them
	//

	m_applicationSettingsDialog = new SettingsDialog(this);

	QSettings applicationSettings;
	if (applicationSettings.value("port").isNull())
	{
		if (QSerialPortInfo::availablePorts().count() == 0)
		{
			QMessageBox::critical(this, tr("Serial Data Tester"), tr("No ports detected! Program will not work until ports would be detected"));
		}
		else
		{
			QMessageBox::warning(this, tr("First time start"), tr("Look like you started programm for the first time. Select default settings before use it"));

			m_applicationSettingsDialog->exec();

			applicationSettings.setValue("bits", QSerialPort::Data8);
			applicationSettings.setValue("stopBits", QSerialPort::TwoStop);
		}
	}

	delete m_applicationSettingsDialog;

	// Create port receiver object
	//

	m_portReceiver = new PortReceiver();

	// Receiver timer, it will count 5 second after last received packet.
	// When five seconds will pass - all packet data will be deleted from table.
	//

	receiveTimeout = new QTimer();

	// Calculate crc-64 table
	//

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
		m_crc_table[i] = tempValue;
	}

	connect(m_reloadCfg, &QAction::triggered, this, &SerialDataTester::reloadConfig);
	connect(m_changeSignalSettingsFile, &QAction::triggered, this, &SerialDataTester::selectNewSignalsFile);
	connect(m_exit, &QAction::triggered, this, &SerialDataTester::applicationExit);
	connect(m_setPort, &QMenu::triggered, this, &SerialDataTester::setPort);
	connect(m_setBaud, &QMenu::triggered, this, &SerialDataTester::setBaud);
	connect(m_setDataBits, &QMenu::triggered, this, &SerialDataTester::setBits);
	connect(m_setStopBits, &QMenu::triggered, this, &SerialDataTester::setStopBits);
	connect(m_portReceiver, &PortReceiver::portError, this, &SerialDataTester::portError);
	connect(this, &SerialDataTester::portChanged, m_portReceiver, &PortReceiver::setPort);
	connect(this, &SerialDataTester::baudChanged, m_portReceiver, &PortReceiver::setBaud);
	connect(this, &SerialDataTester::bitsChanged, m_portReceiver, &PortReceiver::setDataBits);
	connect(this, &SerialDataTester::stopBitsChanged, m_portReceiver, &PortReceiver::setStopBits);
	connect(m_portReceiver, &PortReceiver::dataFromPort, this, &SerialDataTester::dataReceived);
	connect(receiveTimeout, &QTimer::timeout, this, &SerialDataTester::signalTimeout);
	connect(m_erasePacketData, &QAction::triggered, this, &SerialDataTester::erasePacketData);
	connect(m_loadDefaultSettings, &QAction::triggered, this, &SerialDataTester::loadLastUsedSettings);
	connect(m_startReceiving, &QAction::triggered, this, &SerialDataTester::startReceiver);
	connect(m_stopReceiving, &QAction::triggered, this, &SerialDataTester::stopReceiver);

	// If xml-file exists, we will parse it
	//

	if (QFile(applicationSettings.value("pathToSignals").toString()).exists())
	{
		m_pathToSignalsXml = applicationSettings.value("pathToSignals").toString();
		parseFile();
	}
	else
	{
		QMessageBox::warning(this, tr("Program start"), tr("xml-file is corrupted or not exist! Program will load last used port with last used settings"));
		loadLastUsedSettings();
	}

	// Place portReceiver in another
	// thread
	//

	//m_portReceiver->openPort();

	m_PortThread = new QThread(this);
	m_portReceiver->moveToThread(m_PortThread);
	m_PortThread->start();

	// Clean old packet data
	//

	erasePacketData();
}

SerialDataTester::~SerialDataTester()
{
	m_portReceiver->closePort();
	m_PortThread->terminate();

	delete m_portReceiver;
	delete ui;
	delete m_PortThread;
}

void SerialDataTester::parseFile()
{
	int calculatedDataSize=0;

	// Try to open signals xml to read signals
	//

	QFile slgnalsXmlFile(m_pathToSignalsXml);
	bool errorLoadingXml = false;

	/*if (slgnalsXmlFile.exists() == false)
	{
		QMessageBox::critical(this, tr("Critical error"), "File not found: " + m_pathToSignalsXml);
		ui->statusBar->showMessage("Error loading " + m_pathToSignalsXml);
		errorLoadingXml = true;
	}*/

	if (slgnalsXmlFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		QMessageBox::critical(this, tr("Critical error"), "Error opening " + m_pathToSignalsXml);
		ui->statusBar->showMessage("Error loading " + m_pathToSignalsXml);
		errorLoadingXml = true;
	}

	// Ok, now start processing XML-file and write all signals
	// from file to vector
	//

	QXmlStreamReader xmlReader(&slgnalsXmlFile);
	SignalData currentSignal;

	while(xmlReader.atEnd() == false && xmlReader.error() == false && errorLoadingXml == false)
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
					bool portExists = false;
					for (QAction* port : m_setPort->actions())
					{
						port->setChecked(false);
						if (port->text() == attributes.value("PortInfoStrID").toString())
						{
							portExists = true;
							port->setChecked(true);
							m_portReceiver->setPort(attributes.value("PortInfoStrID").toString());
							m_portReceiver->setBaud(attributes.value("Speed").toInt());
							switch (attributes.value("Bits").toInt())
							{
								case 5: m_portReceiver->setDataBits(QSerialPort::Data5); break;
								case 6: m_portReceiver->setDataBits(QSerialPort::Data6); break;
								case 7: m_portReceiver->setDataBits(QSerialPort::Data7); break;
								case 8: m_portReceiver->setDataBits(QSerialPort::Data8); break;
								default: QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Data bits value in xml-file of port " + port->text() + " is wrong. port will not work")));
							}

							QString stopBitsMenuEntryName;

							switch (attributes.value("StopBits").toInt())
							{
								case 1:
								{
									m_portReceiver->setStopBits(QSerialPort::OneStop);
									stopBitsMenuEntryName = "One stop";
									break;
								}
								case 2:
								{
									m_portReceiver->setStopBits(QSerialPort::TwoStop);
									stopBitsMenuEntryName = "Two stop";
									break;
								}
								case 3:
								{
									m_portReceiver->setStopBits(QSerialPort::OneAndHalfStop);
									stopBitsMenuEntryName = "One and half stop";
									break;
								}
								default: QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Stop bits value in xml-file of port " + port->text() + " is wrong. port will not work")));
							}
							ui->portName->setText(attributes.value("PortInfoStrID").toString());
							ui->baudRate->setText(attributes.value("Speed").toString());
							ui->bits->setText(attributes.value("Bits").toString());
							QSettings applicationSettings;

							applicationSettings.setValue("port", attributes.value("PortInfoStrID").toString());
							applicationSettings.setValue("baud", attributes.value("Speed").toInt());
							applicationSettings.setValue("bits", attributes.value("Bits").toInt());
							applicationSettings.setValue("stopBits", attributes.value("StopBits").toInt());

							for (QAction* baudFromMenu : m_setBaud->actions())
							{
								if (attributes.value("Speed").toString() == baudFromMenu->text())
								{
									baudFromMenu->setChecked(true);
								}
							}

							for (QAction* bitsFromMenu : m_setDataBits->actions())
							{
								if(bitsFromMenu->text() == attributes.value("Bits").toString())
								{
									bitsFromMenu->setChecked(true);
								}
							}

							for (QAction* stopBitsFromMenu : m_setStopBits->actions())
							{
								if(stopBitsFromMenu->text() == stopBitsMenuEntryName)
								{
									stopBitsFromMenu->setChecked(true);
								}
							}

							ui->stopBits->setText(stopBitsMenuEntryName);
						}
					}

					m_dataSize = attributes.value("DataSize").toInt();

					if (!portExists)
					{
						QMessageBox::warning(this, tr("Critical error"), tr("XML file has port which was not detected or not exist on this machine. Programm will load last used settings"));
						loadLastUsedSettings();
					}
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Error reading attributes"));
					errorLoadingXml = true;
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

					calculatedDataSize += currentSignal.dataSize;

					m_signalsFromXml.push_back(currentSignal);
				}
				else
				{
					QMessageBox::critical(this, tr("Serial Data Tester"), tr("Error reading attributes"));
					errorLoadingXml = true;
				}
			}
		}
	}

	if (xmlReader.error())
	{
		QMessageBox::critical(this, tr("Serial Data Tester"), xmlReader.errorString());
		errorLoadingXml = true;
	}

	if (errorLoadingXml)
	{
		ui->statusBar->showMessage("Error loading " + m_pathToSignalsXml);
		m_signalsFromXml.clear();
	}
	else
	{
		ui->statusBar->showMessage(m_pathToSignalsXml + " has been succsessfully loaded");
	}

	slgnalsXmlFile.close();

	// Show all signals
	//

	int numberOfSignalFromVector = 0;

	for (SignalData& signalData : m_signalsFromXml)
	{
		ui->signalsTable->setRowCount(numberOfSignalFromVector + 1);
		ui->signalsTable->setItem(numberOfSignalFromVector, strId, new QTableWidgetItem(signalData.strId));
		ui->signalsTable->setItem(numberOfSignalFromVector, name, new QTableWidgetItem(signalData.name));
		ui->signalsTable->setItem(numberOfSignalFromVector, offset, new QTableWidgetItem(QString::number(signalData.offset)));
		ui->signalsTable->setItem(numberOfSignalFromVector, bit, new QTableWidgetItem(QString::number(signalData.bit)));
		ui->signalsTable->setItem(numberOfSignalFromVector, type, new QTableWidgetItem(signalData.type));
		ui->signalsTable->setItem(numberOfSignalFromVector, value, new QTableWidgetItem(QString::number(0)));

		numberOfSignalFromVector++;
	}

	// In case errors in xml-file show message
	//

	if (m_dataSize != calculatedDataSize)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Packet size is not equal to summ of signals size: possible loss of data. Please, check the packet data amount, and sizes of each signal data."));
		m_dataSize = 0;
		ui->signalsTable->clear();
		ui->signalsTable->setRowCount(0);

		ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
		ui->signalsTable->setHorizontalHeaderItem(name, new QTableWidgetItem(tr("Name")));
		ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
		ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
		ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));
		ui->signalsTable->setHorizontalHeaderItem(value, new QTableWidgetItem(tr("Value")));
	}
}

void SerialDataTester::reloadConfig()
{
	ui->signalsTable->clear();

	ui->signalsTable->setRowCount(0);

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(name, new QTableWidgetItem(tr("Name")));
	ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));
	ui->signalsTable->setHorizontalHeaderItem(value, new QTableWidgetItem(tr("Value")));

	m_signalsFromXml.clear();
	parseFile();
}

void SerialDataTester::selectNewSignalsFile()
{
	QString newPathToSignals = QFileDialog::getOpenFileName(this, tr("Open Signals.xml"), "", tr("XML-file (*.xml)"));
	if (newPathToSignals.isEmpty() == false)
	{
		m_pathToSignalsXml = newPathToSignals;
		QSettings applicationSettings;
		applicationSettings.setValue("pathToSignals", m_pathToSignalsXml);
		reloadConfig();
	}
}

void SerialDataTester::applicationExit()
{
	this->close();
}

void SerialDataTester::setPort(QAction* newPort)
{
	this->stopReceiver();
	for (QAction* port : m_setPort->actions())
	{
		if(port->text() != newPort->text())
		{
			port->setChecked(false);
		}
	}

	emit portChanged(newPort->text());

	QSettings applicationSettings;

	applicationSettings.setValue("port", newPort->text());
	ui->portName->setText(newPort->text());
}

void SerialDataTester::setBaud(QAction* newBaud)
{
	this->stopReceiver();
	for (QAction* baud : m_setBaud->actions())
	{
		if(baud->text() != newBaud->text())
		{
			baud->setChecked(false);
		}
	}

	emit baudChanged(newBaud->text().toInt());

	QSettings applicationSettings;

	applicationSettings.setValue("baud", newBaud->text());
	ui->baudRate->setText(newBaud->text());
}

void SerialDataTester::setBits(QAction *newBits)
{
	this->stopReceiver();
	for (QAction* bits : m_setDataBits->actions())
	{
		if(bits->text() != newBits->text())
		{
			bits->setChecked(false);
		}
	}

	QSettings applicationSettings;

	switch (newBits->data().toInt())
	{
		case 5:
		{
			emit bitsChanged(QSerialPort::Data5);
			applicationSettings.setValue("bits", QSerialPort::Data5);
			break;
		}
		case 6:
		{
			emit bitsChanged(QSerialPort::Data6);
			applicationSettings.setValue("bits", QSerialPort::Data6);
			break;
		}
		case 7:
		{
			emit bitsChanged(QSerialPort::Data7);
			applicationSettings.setValue("bits", QSerialPort::Data7);
			break;
		}
		case 8:
		{
			emit bitsChanged(QSerialPort::Data8);
			applicationSettings.setValue("bits", QSerialPort::Data8);
			break;
		}
	}
	ui->bits->setText(newBits->text());
}

void SerialDataTester::setStopBits(QAction *newStopBits)
{
	this->stopReceiver();
	for (QAction* stopBits : m_setStopBits->actions())
	{
		if(stopBits->text() != newStopBits->text())
		{
			stopBits->setChecked(false);
		}
	}

	QSettings applicationSettings;

	if (newStopBits->text() == "One stop")
	{
		emit stopBitsChanged(QSerialPort::OneStop);
		applicationSettings.setValue("stopBits", QSerialPort::OneStop);
	}
	if (newStopBits->text() == "One and half stop")
	{
		emit stopBitsChanged(QSerialPort::OneAndHalfStop);
		applicationSettings.setValue("stopBits", QSerialPort::OneAndHalfStop);
	}
	if (newStopBits->text() == "Two stop")
	{
		emit stopBitsChanged(QSerialPort::TwoStop);
		applicationSettings.setValue("stopBits", QSerialPort::TwoStop);
	}
	ui->stopBits->setText(newStopBits->text());
}

void SerialDataTester::portError(QString error)
{
	QMessageBox::critical(this, tr("Critical error"), error);
	ui->portName->setText("Error");
}

void SerialDataTester::dataReceived(QByteArray data)
{
	// Reset timer
	//

	receiveTimeout->stop();
	receiveTimeout->start(5000);

	QDataStream packet(&data, QIODevice::ReadOnly);

	// Read packet "head" and check signature with crc
	//

	quint32 signature;
	packet >> signature;
	ui->signature->setText(QString::number(signature, 16));

	quint16 version;
	packet >> version;

	ui->version->setText(QString::number(version));

	quint16 transmissionId;
	packet >> transmissionId;

	ui->transmissionId->setText(QString::number(transmissionId));

	quint16 numerator;
	packet >> numerator;

	ui->numerator->setText(QString::number(numerator));

	quint16 amount;
	packet >> amount;

	quint16 dataUniqueId;
	packet >> dataUniqueId;

	QByteArray receivedSignalWithCrc;
	QByteArray receivedValues;
	packet >> receivedSignalWithCrc;

	// According to data amount read data from packet without
	// crc
	//

	receivedValues = receivedSignalWithCrc.mid(0, amount);
	std::string dataToCalculateCrc = QString::fromLocal8Bit(receivedValues).toStdString();

	// Calculate received data crc
	//

	char *dataForCrcPtr = &dataToCalculateCrc[0];

	quint64 crc = 0;
	for (int i=0; i<dataToCalculateCrc.size(); i++)
	{
		crc = m_crc_table[(crc ^ (dataForCrcPtr[i])) & 0xFF] ^ (crc >> 8);
	}
	crc = ~crc;

	quint64 packetCrc;
	packet >> packetCrc;

	bool packageCorrupted = false;

	// Check calculated and received crc's
	//

	if (crc != packetCrc)
	{
		ui->crc->setText("ERROR");
		packageCorrupted = true;
		ui->statusBar->showMessage("Corrupted packet (CRC error): Calculated: " + QString::number(crc) + " Requested: " + QString::number(packetCrc));
	}
	else
	{
		// Check signature
		//

		ui->crc->setText("OK");
		if (signature != m_signature)
		{
			packageCorrupted = true;
			ui->statusBar->showMessage("Unknown signature");
		}
		else
		{

			// If signature and crc ok - start processing received packet
			// First of all, transform byteArray to bitArray. We need it to read signal
			// values.
			//

			int rowNumber = 0;

			QString allReceivedBitsArray;
			QBitArray dataArray;
			dataArray.resize(m_dataSize*8);
			dataArray.fill(0);
			for(int currentByte=0; currentByte<receivedValues.count(); ++currentByte)
			{
				for(int currentBit=0; currentBit<8; ++currentBit)
				{
					dataArray.setBit(currentByte*8+currentBit, receivedValues.at(currentByte)&(1<<(currentBit)));
					allReceivedBitsArray.append(QString::number(dataArray.at(currentByte*8+currentBit)));
				}
				allReceivedBitsArray.append(" ");
			}

			qDebug() << dataArray;

			ui->statusBar->showMessage("Processed data: " + allReceivedBitsArray);

			for (SignalData signal : m_signalsFromXml)
			{

				// Read data for every signal, and transform it to
				// selected value (signedInt, unsignedInt, float)
				//

				QBitArray signalValueBits;

				signalValueBits.resize(signal.dataSize*8);
				QString valueString;
				for (int pos = signal.offset + signal.bit; pos < signal.offset + signal.bit + signal.dataSize*8; pos ++)
				{
					switch (dataArray.at(pos))
					{
						case 1: valueString.append("1"); break;
						case 0: valueString.append("0"); break;
					}
				}

				std::reverse(valueString.begin(), valueString.end());
				qDebug() << valueString;
				int result = valueString.toInt(false, 2);
				QString resultString;
				if (signal.dataFormat == "UnsignedInt")
				{
					resultString = QString::number(result);
				}
				if (signal.dataFormat == "SignedInt")
				{
					resultString = QString::number(result - pow(2, signal.dataSize*8)/2);
				}
				if (signal.dataFormat == "Float")
				{
					resultString = QString::number(result * 0.1);
				}
				ui->signalsTable->setItem(rowNumber, value, new QTableWidgetItem(resultString));
				rowNumber++;
			}
		}
	}

	if (packageCorrupted)
	{
		ui->corruptedPackets->setText(QString::number(ui->corruptedPackets->text().toInt()+1));
	}
	else
	{
		ui->processedPackets->setText(QString::number(ui->processedPackets->text().toInt() + 1));
	}

	ui->totalPackets->setText(QString::number(ui->totalPackets->text().toInt() + 1));
}

void SerialDataTester::signalTimeout()
{
	for (int currentRow = 0; currentRow < ui->signalsTable->rowCount(); currentRow++)
	{
		ui->signalsTable->setItem(currentRow, value, new QTableWidgetItem(QString::number(0)));
	}
	ui->statusBar->showMessage("Signal timeout!");
}

void SerialDataTester::erasePacketData()
{
	ui->corruptedPackets->setText("0");
	ui->totalPackets->setText("0");
	ui->processedPackets->setText("0");
	ui->signature->setText("No data");
	ui->version->setText("No data");
	ui->transmissionId->setText("No data");
	ui->numerator->setText("No data");
	ui->crc->setText("No data");
	ui->statusBar->showMessage("Packet data has been removed");
}

void SerialDataTester::loadLastUsedSettings()
{
	QSettings applicationSettings;

	QString portName = applicationSettings.value("port").toString();
	if (portName.isNull())
	{
		portName = "Error";
		QMessageBox::warning(this, tr("Loading old settings"), tr("Last used port is not avaiable! Please, select port manually, or check port status."));
	}
	int baud = applicationSettings.value("baud").toInt();
	int bits = applicationSettings.value("bits").toInt();
	int stopBits = applicationSettings.value("stopBits").toInt();

	if (portName != "Error")
	{
		m_portReceiver->setPort(portName);
	}

	m_portReceiver->setBaud(baud);
	switch (bits)
	{
		case 5: m_portReceiver->setDataBits(QSerialPort::Data5); break;
		case 6: m_portReceiver->setDataBits(QSerialPort::Data6); break;
		case 7: m_portReceiver->setDataBits(QSerialPort::Data7); break;
		case 8: m_portReceiver->setDataBits(QSerialPort::Data8); break;
	}

	QString stopBitsMenuEntryName;

	switch (stopBits)
	{
		case 1:
		{
			m_portReceiver->setStopBits(QSerialPort::OneStop);
			stopBitsMenuEntryName = "One stop";
			break;
		}
		case 2:
		{
			m_portReceiver->setStopBits(QSerialPort::TwoStop);
			stopBitsMenuEntryName = "Two stop";
			break;
		}
		case 3:
		{
			m_portReceiver->setStopBits(QSerialPort::OneAndHalfStop);
			stopBitsMenuEntryName = "One and half stop";
			break;
		}
	}

	for (QAction* portFromMenu : m_setPort->actions())
	{
		if (portName == portFromMenu->text())
		{
			portFromMenu->setChecked(true);
		}
	}

	for (QAction* baudFromMenu : m_setBaud->actions())
	{
		if (QString::number(baud) == baudFromMenu->text())
		{
			baudFromMenu->setChecked(true);
		}
	}

	for (QAction* bitsFromMenu : m_setDataBits->actions())
	{
		if(bitsFromMenu->text() == QString::number(bits))
		{
			bitsFromMenu->setChecked(true);
		}
	}

	for (QAction* stopBitsFromMenu : m_setStopBits->actions())
	{
		if(stopBitsFromMenu->text() == stopBitsMenuEntryName)
		{
			stopBitsFromMenu->setChecked(true);
		}
	}

	ui->portName->setText(portName);
	ui->baudRate->setText(QString::number(baud));
	ui->bits->setText(QString::number(bits));
	ui->stopBits->setText(stopBitsMenuEntryName);
	QMessageBox::warning(this, tr("Warning"), tr("Loaded last used port"));
}

void SerialDataTester::startReceiver()
{
	bool errors = false;

	m_portReceiver->openPort();

	if (ui->portName->text() == "Error")
	{
		QMessageBox::critical(this, tr("Program start"), tr("No port specified"));
		errors = true;
	}

	if (QFile(m_pathToSignalsXml).exists() == false)
	{
		QMessageBox::critical(this, tr("Program start"), tr("No xml-file selected!"));
		errors = true;
	}

	if (ui->baudRate->text() == "Error")
	{
		QMessageBox::critical(this, tr("Program start"), tr("Wrong baud set!"));
		errors = true;
	}

	if (ui->bits->text() == "Error")
	{
		QMessageBox::critical(this, tr("Program start"), tr("Wrong data bits set!"));
		errors = true;
	}

	if (ui->stopBits->text() == "Error")
	{
		QMessageBox::critical(this, tr("Program start"), tr("Wrong stop bits set!"));
		errors = true;
	}

	if (ui->portStatus->text() == "Opened")
	{
		QMessageBox::critical(this, tr("Program start"), tr("Port already opened!"));
		errors = true;
	}

	if (ui->signalsTable->rowCount() == 0)
	{
		QMessageBox::critical(this, tr("Program start"), tr("No signals detected! Read signals first!"));
		errors = true;
	}

	if (errors == false)
	{
		//m_PortThread->start();
		m_portReceiver->openPort();
		ui->portStatus->setText("Opened");
	}
	else
	{
		QMessageBox::warning(this, tr("Program start"), tr("Program won't start until all errors will be fixed"));
		ui->portStatus->setText("Closed");
	}
}

void SerialDataTester::stopReceiver()
{
	//m_PortThread->terminate();
	m_portReceiver->closePort();
	ui->portStatus->setText("Closed");
}
