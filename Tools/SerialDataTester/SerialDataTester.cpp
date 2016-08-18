#include "SerialDataTester.h"
#include "SettingsDialog.h"
#include "PortReceiver.h"
#include "ui_SerialDataTester.h"
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

	m_parser = new SerialDataParser();

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
	connect(m_portReceiver, &PortReceiver::dataFromPort, m_parser, &SerialDataParser::parse);
	connect(m_parser, &SerialDataParser::packetProcessed, this, &SerialDataTester::dataReceived);
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

	m_ParserThread = new QThread(this);
	m_parser->moveToThread(m_ParserThread);
	m_ParserThread->start();

	// Clean old packet data
	//

	erasePacketData();
}

SerialDataTester::~SerialDataTester()
{
	m_portReceiver->closePort();
	m_PortThread->terminate();
	m_ParserThread->terminate();

	delete m_portReceiver;
	delete ui;
	delete m_PortThread;
	delete m_parser;
	delete m_ParserThread;
}

void SerialDataTester::parseFile()
{
	// Try to open signals xml to read signals
	//

	QFile slgnalsXmlFile(m_pathToSignalsXml);
	bool errorLoadingXml = false;

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
					bool portExists = false;
					for (QAction* port : m_setPort->actions())
					{
						port->setChecked(false);
						if (port->text() == attributes.value("StrID").toString())
						{
							portExists = true;
							port->setChecked(true);
							m_portReceiver->setPort(attributes.value("StrID").toString());
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
							ui->portName->setText(attributes.value("StrID").toString());
							ui->baudRate->setText(attributes.value("Speed").toString());
							ui->bits->setText(attributes.value("Bits").toString());
							QSettings applicationSettings;

							applicationSettings.setValue("port", attributes.value("StrID").toString());
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

					if (!portExists)
					{
						QMessageBox::warning(this, tr("Critical error"), tr("XML file has port which was not detected or not exist on this machine. Programm will load last used settings"));
						loadLastUsedSettings();
					}
					m_dataSize = attributes.value("DataSize").toInt();
				}
				else
				{
					QMessageBox::critical(this, tr("Critical error"), tr("Error reading attributes"));
					errorLoadingXml = true;
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
					currentSignal.strId  = attributes.value("StrID").toString();
					currentSignal.exStrId = attributes.value("ExtStrID").toString();
					currentSignal.name =  attributes.value("Name").toString();
					currentSignal.type = attributes.value("Type").toString();
					currentSignal.unit = attributes.value("Unit").toString();
					currentSignal.dataSize = attributes.value("DataSize").toInt();
					currentSignal.dataFormat = attributes.value("DataFormat").toString();
					currentSignal.byteOrder = attributes.value("ByteOrder").toString();
					currentSignal.offset = attributes.value("Offset").toInt();
					currentSignal.bit = attributes.value("BitNo").toInt();

					if (currentSignal.dataSize != 1 && currentSignal.dataSize != 16 && currentSignal.dataSize != 32 && currentSignal.dataSize != 8)
					{
						QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Error reading size attribute on " + currentSignal.strId + ": wrong size")));
						errorLoadingXml = true;
					}

					if (currentSignal.dataFormat == "Float" && currentSignal.dataSize != 32)
					{
						QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Error reading size attribute on " + currentSignal.strId + ": only 32 bits need to work with float value")));
						errorLoadingXml = true;
					}

					if (currentSignal.type == "analog")
					{
						currentSignal.dataSize/=8;
					}

					//					calculatedDataSize += currentSignal.dataSize;

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

	for (QAction* port : m_setPort->actions())
	{
		port->setChecked(false);
	}
}

void SerialDataTester::dataReceived(QByteArray receivedValues)
{
	// Reset timer
	//

	receiveTimeout->stop();
	receiveTimeout->start(5000);

	bool packageCorrupted = false;

	if (receivedValues.size() != 7)
	{
		qDebug() << "Wrong packet size!";
		packageCorrupted = true;
		ui->statusBar->showMessage("Data received: Packet error!");
	}
	else
	{/*
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
		{*/

		// If signature and crc ok - start processing received packet
		// First of all, transform byteArray to bitArray. We need it to read signal
		// values.
		//

		int rowNumber = 0;

		QBitArray dataArray;
		dataArray.resize(m_dataSize*8);
		dataArray.fill(0);

		QString dataVisualisation;

		dataArray = bytesToBits(receivedValues);

		for (int currentBit = 0; currentBit < dataArray.size(); currentBit++)
		{
			dataVisualisation.append(dataArray.at(currentBit) == 1 ? "1" : "0");

			if ((currentBit+1) % 8 == 0)
				dataVisualisation += " ";
		}

		ui->statusBar->showMessage("Data received: " + dataVisualisation);

		qDebug() << "QBitArray data: " << dataArray;

		/*for (int currentBit = 0; currentBit < m_dataSize*8; currentBit++)
			{
				(dataArray.at(currentBit) == 1) ? allReceivedBitsArray.append("1") : allReceivedBitsArray.append("0");
			}

			ui->statusBar->showMessage("Processed data: " + allReceivedBitsArray);*/

		for (SignalData signal : m_signalsFromXml)
		{

			// Read data for every signal, and transform it to
			// selected value (signedInt, unsignedInt, float)
			//

			QBitArray signalValueBits;

			signalValueBits.resize(signal.dataSize*8);
			QString valueString;
			for (int pos = signal.offset*8 + signal.bit; pos < signal.offset*8 + signal.bit + signal.dataSize; pos ++)
			{
				switch (dataArray.at(pos))
				{
				case 1: valueString.append("1"); break;
				case 0: valueString.append("0"); break;
				}
			}

			std::reverse(valueString.begin(), valueString.end());
			bool falseValue = false;
			qint64 result = valueString.toInt(&falseValue, 2);
			qDebug() << "Result: " << result;
			QString resultString;
			if (signal.dataFormat == "UnsignedInt")
			{
				resultString = QString::number(result);
				qDebug() << "ResultString: " << resultString;
			}
			if (signal.dataFormat == "SignedInt")
			{
				if (valueString[0] == '0')
				{
					resultString = QString::number(result);
				}
				else
				{
					result = result - pow (2, (signal.dataSize*8)-1);
					resultString = QString::number(result * -1);
				}
			}
			if (signal.dataFormat == "Float")
			{
				float exponentCalculation = 0;

				for (int bitPos = 8; bitPos>0; bitPos--)
				{
					(valueString[bitPos] == '1') ? exponentCalculation += pow(2, 8-bitPos) : exponentCalculation += 0;
				}

				if (exponentCalculation != 0)
					exponentCalculation = pow(2, exponentCalculation - 127);
				else
					exponentCalculation = pow(2, -126);

				float mantissa;

				(exponentCalculation == 0) ? mantissa = 0 : mantissa = 1;

				for (int bitPos = 9; bitPos < 32; bitPos++)
				{
					(valueString[bitPos] == '1') ? mantissa += pow(0.5, bitPos-8) : mantissa += 0;
				}

				int sign;

				(valueString[0] == '1') ? sign = -1 : sign = 1;

				resultString = QString::number( sign * exponentCalculation * mantissa );
				//resultString = QString::number(result * 0.1);
			}
			ui->signalsTable->setItem(rowNumber, value, new QTableWidgetItem(resultString));
			rowNumber++;
			//}
			//}
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
	/*for (int currentRow = 0; currentRow < ui->signalsTable->rowCount(); currentRow++)
	{
		ui->signalsTable->setItem(currentRow, value, new QTableWidgetItem(QString::number(0)));
	}
	ui->statusBar->showMessage("Signal timeout!");*/
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

QBitArray SerialDataTester::bytesToBits(QByteArray bytes)
{
	QBitArray bits(bytes.count()*8);
	// Convert from QByteArray to QBitArray
	for(int i=0; i<bytes.count(); ++i)
		for(int b=0; b<8; ++b)
			bits.setBit(i*8+b, bytes.at(i)&(1<<b));
	return bits;
}

QByteArray SerialDataTester::bitsToBytes(QBitArray bits)
{
	QByteArray bytes;
	bytes.resize(bits.count()/8);
	// Convert from QBitArray to QByteArray
	for(int b=0; b<bits.count(); ++b)
		bytes[b/8] = ( bytes.at(b/8) | ((bits[b]?1:0)<<(b%8)));
	return bytes;
}
