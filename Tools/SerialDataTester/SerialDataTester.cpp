#include "SerialDataTester.h"
#include "ui_SerialDataTester.h"


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

	// Detect all ports to fill ports submenu
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

	// Fill settings menu
	//

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
	ui->signalsTable->setEditTriggers(QAbstractItemView::EditTriggers(0));

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

	m_PortThread = new QThread();
	m_portReceiver = new PortReceiver(m_PortThread);
	m_portReceiver->moveToThread(m_PortThread);
	m_PortThread->start();
	// Receiver timer, it will count 5 second after last received packet.
	// When five seconds will pass - all packet data will be deleted from table.
	//

	receiveTimeout = new QTimer();

	// Create parcer class. Parser will parse incoming packet data and check
	// CRC sum. Data will be processed in SerialDataTester class.
	//

	m_ParserThread = new QThread();
	m_parser = new SerialDataParser(m_ParserThread);
	m_parser->moveToThread(m_ParserThread);
	m_ParserThread->start();

	// Main configuration signals
	//

	connect(m_reloadCfg, &QAction::triggered, this, &SerialDataTester::reloadConfig);									// Reload configuration button
	connect(m_changeSignalSettingsFile, &QAction::triggered, this, &SerialDataTester::selectNewSignalsFile);			// Change signals settings file button
	connect(m_exit, &QAction::triggered, this, &SerialDataTester::applicationExit);										// Application exit button
	connect(m_setPort, &QMenu::triggered, this, &SerialDataTester::setPort);											// Port config submenu
	connect(m_setBaud, &QMenu::triggered, this, &SerialDataTester::setBaud);											// Baud config submenu
	connect(m_setDataBits, &QMenu::triggered, this, &SerialDataTester::setBits);										// DataBits config submenu
	connect(m_setStopBits, &QMenu::triggered, this, &SerialDataTester::setStopBits);									// StopBits config submenu
	connect(receiveTimeout, &QTimer::timeout, this, &SerialDataTester::signalTimeout);									// Timer timeout signal (for packets data reset)
	connect(m_erasePacketData, &QAction::triggered, this, &SerialDataTester::erasePacketData);							// Reset amount of received packets
	connect(m_loadDefaultSettings, &QAction::triggered, this, &SerialDataTester::loadLastUsedSettings);					// Load last used settings
	connect(m_startReceiving, &QAction::triggered, this, &SerialDataTester::startReceiver);								// Open port and start receiving
	connect(m_stopReceiving, &QAction::triggered, this, &SerialDataTester::stopReceiver);								// Close port and stop receiving

	// Port receiver control signals
	//

	connect(m_portReceiver, &PortReceiver::portError, this, &SerialDataTester::portError);								// Port error signal from portReceiver class
	connect(this, &SerialDataTester::portChanged, m_portReceiver, &PortReceiver::setPort);								// Port changed signal from SerialDataTester class
	connect(this, &SerialDataTester::baudChanged, m_portReceiver, &PortReceiver::setBaud);								// Baud changed signal from SerialDataTester class
	connect(this, &SerialDataTester::bitsChanged, m_portReceiver, &PortReceiver::setDataBits);							// Data bits changed signal from SerialDataTester class
	connect(this, &SerialDataTester::stopBitsChanged, m_portReceiver, &PortReceiver::setStopBits);						// Stop bits changed signal from SerialDataTester class

	// Packet parser control signals
	//

	connect(m_portReceiver, &PortReceiver::dataFromPort, m_parser, &SerialDataParser::parse);							// Send received data from Port Received to Packet parser
	connect(m_parser, &SerialDataParser::packetProcessed, this, &SerialDataTester::dataReceived);						// Packet parser signal for packet to be processed
	connect(m_parser, &SerialDataParser::crcError, this, &SerialDataTester::crcError);									// Packet parser signal for CRC calculation error

	// If xml-file exists in application stored setting, we will parse it
	//

	if (QFile(applicationSettings.value("pathToSignals").toString()).exists())
	{
		m_pathToSignalsXml = applicationSettings.value("pathToSignals").toString();										// Get path to file
		parseFile();
	}
	else
	{
		// In case of non-existing file, program will load last used settings
		//

		QMessageBox::warning(this, tr("Program start"), tr("xml-file is corrupted or not exist! Program will load last used port with last used settings"));
		loadLastUsedSettings();
	}

	// Clean old packet data
	//

	erasePacketData();
}

SerialDataTester::~SerialDataTester()
{
	// Class destructor. Close ports, terminate threads, and free RAM
	//

	m_portReceiver->closePort();

	m_PortThread->quit();

	m_ParserThread->quit();

	delete m_portReceiver;
	delete ui;
	delete m_PortThread;
	delete m_parser;
	delete m_ParserThread;

	delete m_file;
	delete m_settings;
	delete m_reloadCfg;
	delete m_changeSignalSettingsFile;
	delete m_exit;
	delete m_startReceiving;
	delete m_stopReceiving;

	delete m_settings;
	delete m_setPort;
	delete m_setBaud;
	delete m_setDataBits;
	delete m_setStopBits;
}

void SerialDataTester::parseFile()
{
	// Here we will parse selected xml-file
	//

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
	// from file to vector. Each signal from xml-file will be
	// converted to SignalData structure, and then pushed to
	// signals vector
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
			// If xml reader found element beginning - we will process it
			//

			QXmlStreamAttributes attributes = xmlReader.attributes();

			if(xmlReader.name() == "PortInfo")
			{
				// In case port information element found - we will read info about it
				//

				// Check all attributes
				//

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

					// This cycle will search suggested port from file
					//

					for (QAction* port : m_setPort->actions())
					{
						port->setChecked(false);

						if (port->text() == attributes.value("StrID").toString())
						{
							// We found suggested port
							//

							portExists = true;
							port->setChecked(true);	// Set this port selected in GUI config menus (Settings menu)

							// Send port info to Port receiver
							//

							m_portReceiver->setPort(attributes.value("StrID").toString());
							m_portReceiver->setBaud(attributes.value("Speed").toInt());

							// Remember packet data size
							//

							m_dataSize = attributes.value("DataSize").toInt();

							// Send port data bits to Port Receiver
							//

							switch (attributes.value("Bits").toInt())
							{
							case 5: m_portReceiver->setDataBits(QSerialPort::Data5); break;
							case 6: m_portReceiver->setDataBits(QSerialPort::Data6); break;
							case 7: m_portReceiver->setDataBits(QSerialPort::Data7); break;
							case 8: m_portReceiver->setDataBits(QSerialPort::Data8); break;
							default: QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Data bits value in xml-file of port " + port->text() + " is wrong. Default value has been set.")));
							}

							// Send port stop bits to Port Receiver
							//

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

							// This part will show port info to GUI info panels (Port info panel)
							//

							ui->portName->setText(attributes.value("StrID").toString());
							ui->baudRate->setText(attributes.value("Speed").toString());
							ui->bits->setText(attributes.value("Bits").toString());

							// Write current settings to program settings files
							//

							QSettings applicationSettings;

							applicationSettings.setValue("port", attributes.value("StrID").toString());
							applicationSettings.setValue("baud", attributes.value("Speed").toInt());
							applicationSettings.setValue("bits", attributes.value("Bits").toInt());
							applicationSettings.setValue("stopBits", attributes.value("StopBits").toInt());

							// Last thing to do: configurate GUI menus according to port settings
							//

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

					if (portExists == false)
					{
						// If port was not found - load last used port
						//

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
				// In case signal element found - we will read it, and place to vector
				//

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

					// Check signal size. It must be 1, 8, 16 or 32 bits
					//

					if (currentSignal.dataSize != 1 && currentSignal.dataSize != 16 && currentSignal.dataSize != 32 && currentSignal.dataSize != 8)
					{
						QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Error reading size attribute on " + currentSignal.strId + ": wrong size")));
						errorLoadingXml = true;
					}

					// According to IEEE 754 standard for floating point transaction, float signals must be
					// 32-bits size
					//

					if (currentSignal.dataFormat == "Float" && currentSignal.dataSize != 32)
					{
						QMessageBox::critical(this, tr("Serial Data Tester"), tr(qPrintable("Error reading size attribute on " + currentSignal.strId + ": only 32 bits need to work with float value")));
						errorLoadingXml = true;
					}

					// If signal is analog - divide it is size by 8, to find his size in bytes
					//

					if (currentSignal.type == "analog")
					{
						currentSignal.dataSize/=8;
					}

					// Write new signal to vector

					m_signalsFromXml.push_back(currentSignal);

					strIds.push_back(new QTableWidgetItem(currentSignal.strId));
					names.push_back(new QTableWidgetItem(currentSignal.name));
					offsets.push_back(new QTableWidgetItem(QString::number(currentSignal.offset)));
					bits.push_back(new QTableWidgetItem(QString::number(currentSignal.bit)));
					types.push_back(new QTableWidgetItem(currentSignal.type));
					values.push_back(new QTableWidgetItem(QString::number(0)));
				}
				else
				{
					QMessageBox::critical(this, tr("Serial Data Tester"), tr("Error reading attributes"));
					errorLoadingXml = true;
				}
			}
		}
	}


	// In case errors in xml-file show message
	//

	if (xmlReader.error())
	{
		QMessageBox::critical(this, tr("Serial Data Tester"), xmlReader.errorString());
		errorLoadingXml = true;
	}

	if (errorLoadingXml)
	{
		for (int currentSignal = 0; currentSignal < m_signalsFromXml.size(); currentSignal++)
		{
			delete strIds.at(currentSignal);
			delete names.at(currentSignal);
			delete offsets.at(currentSignal);
			delete bits.at(currentSignal);
			delete types.at(currentSignal);
			delete values.at(currentSignal);
		}

		strIds.clear();
		names.clear();
		offsets.clear();
		bits.clear();
		types.clear();
		values.clear();

		ui->statusBar->showMessage("Error loading " + m_pathToSignalsXml);
		m_signalsFromXml.clear();
	}
	else
	{
		ui->statusBar->showMessage(m_pathToSignalsXml + " has been succsessfully loaded");
	}

	slgnalsXmlFile.close();

	// Show all signals to program table.
	//

	int numberOfSignalFromVector = 0;

	for (SignalData& signalData : m_signalsFromXml)
	{
		ui->signalsTable->setRowCount(numberOfSignalFromVector + 1);

		ui->signalsTable->setItem(numberOfSignalFromVector, strId, strIds.at(numberOfSignalFromVector));
		ui->signalsTable->setItem(numberOfSignalFromVector, name, names.at(numberOfSignalFromVector));
		ui->signalsTable->setItem(numberOfSignalFromVector, offset, offsets.at(numberOfSignalFromVector));
		ui->signalsTable->setItem(numberOfSignalFromVector, bit, bits.at(numberOfSignalFromVector));
		ui->signalsTable->setItem(numberOfSignalFromVector, type, types.at(numberOfSignalFromVector));
		ui->signalsTable->setItem(numberOfSignalFromVector, value, values.at(numberOfSignalFromVector));

		numberOfSignalFromVector++;
	}
}

void SerialDataTester::reloadConfig()
{
	// This function just reloading configuration from file
	//

	ui->signalsTable->clear();

	ui->signalsTable->setRowCount(0);

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(name, new QTableWidgetItem(tr("Name")));
	ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));
	ui->signalsTable->setHorizontalHeaderItem(value, new QTableWidgetItem(tr("Value")));

	for (int currentSignal = 0; currentSignal < m_signalsFromXml.size(); currentSignal++)
	{
		delete strIds.at(currentSignal);
		delete names.at(currentSignal);
		delete offsets.at(currentSignal);
		delete bits.at(currentSignal);
		delete types.at(currentSignal);
		delete values.at(currentSignal);
	}

	strIds.clear();
	names.clear();
	offsets.clear();
	bits.clear();
	types.clear();
	values.clear();

	m_signalsFromXml.clear();

	parseFile();
}

void SerialDataTester::selectNewSignalsFile()
{
	// This part of code will call file selection window to select
	// xml-file.
	//

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
	// This function calling when we are changing current port
	//

	// Stop receiving before do something
	//

	this->stopReceiver();

	// Deselect old port
	//

	for (QAction* port : m_setPort->actions())
	{
		if(port->text() != newPort->text())
		{
			port->setChecked(false);
		}
	}

	// Actually, change port in Port Receiver
	//

	emit portChanged(newPort->text());

	// Write new port to application settings
	//

	QSettings applicationSettings;

	applicationSettings.setValue("port", newPort->text());

	// At last, write new port to program info panel (Port info)
	//

	ui->portName->setText(newPort->text());
}

void SerialDataTester::setBaud(QAction* newBaud)
{
	// This function calling when we are changing current baud
	//

	// Stop receiving before do something
	//

	this->stopReceiver();

	// Deselect old baud
	//

	for (QAction* baud : m_setBaud->actions())
	{
		if(baud->text() != newBaud->text())
		{
			baud->setChecked(false);
		}
	}

	// Actually, change baud in Port Receiver
	//

	emit baudChanged(newBaud->text().toInt());

	// Write new baud to application settings
	//

	QSettings applicationSettings;

	applicationSettings.setValue("baud", newBaud->text());

	// At last, write new baud to program info panel (Port info)
	//

	ui->baudRate->setText(newBaud->text());
}

void SerialDataTester::setBits(QAction *newBits)
{
	// This function calling when we are changing current Data Bits
	//

	// Stop receiving before do something
	//

	this->stopReceiver();

	// Deselect old Data Bits value
	//

	for (QAction* bits : m_setDataBits->actions())
	{
		if(bits->text() != newBits->text())
		{
			bits->setChecked(false);
		}
	}

	// Actually, change Data Bits in settings
	//

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

	// At last, write new Data Bits value to program info panel (Port info)
	//

	ui->bits->setText(newBits->text());
}

void SerialDataTester::setStopBits(QAction *newStopBits)
{
	// This function calling when we are changing current Stop Bits
	//

	// Stop receiving before do something
	//

	this->stopReceiver();

	// Deselect old Stop Bits value
	//

	for (QAction* stopBits : m_setStopBits->actions())
	{
		if(stopBits->text() != newStopBits->text())
		{
			stopBits->setChecked(false);
		}
	}

	// Actually, change Stop Bits in settings
	//

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

	// At last, write new Stop Bits value to program info panel (Port info)
	//

	ui->stopBits->setText(newStopBits->text());
}

void SerialDataTester::portError(QString error)
{
	// This function shows up all port errors
	//

	QMessageBox::critical(this, tr("Critical error"), error);
	ui->portName->setText("Error");

	for (QAction* port : m_setPort->actions())
	{
		port->setChecked(false);
	}
}

void SerialDataTester::dataReceived(QString version, QString trId, QString numerator, QByteArray dataId, QByteArray receivedValues)
{
	// Here we are receiving raw processed data without CRC and header headers from packet
	//

	// Reset timer
	//

	receiveTimeout->stop();
	receiveTimeout->start(5000);

	// Show up received header headers
	//

	ui->version->setText(version);
	ui->signature->setText("424D4C47");
	ui->transmissionId->setText(trId);
	ui->numerator->setText(numerator);
	ui->dataUniqueId->setText(dataId);
	ui->crc->setText("OK");

	// This value will show us: corrupted package, or not
	//

	bool packageCorrupted = false;

	if (receivedValues.size() != m_dataSize)
	{
		// Here we are check received data size
		//

		qDebug() << "Wrong packet size!";
		packageCorrupted = true;
		ui->statusBar->showMessage("Data received: Packet error!");
	}
	else
	{
		// Here we will show all received values in table, but before we need
		// pull them out from raw data and process them from binary to float, or int value
		//

		// Number of current table row
		//

		int rowNumber = 0;

		// In this part, we show all received bytes in status line
		//

		QBitArray dataArray;
		dataArray.resize(m_dataSize*8);
		dataArray.fill(0);

		QString dataVisualisation;

		// We are receiving already transformed data from bits to bytes.
		// To show received bits, we need transform them back to bits
		//

		dataArray = bytesToBits(receivedValues);

		/*for (int currentBit = 0; currentBit < dataArray.size(); currentBit++) // Show every received bit
		{
			dataVisualisation.append(dataArray.at(currentBit) == 1 ? "1" : "0"); // Writing all received bits to string value

			if ((currentBit+1) % 8 == 0)
				dataVisualisation += " ";
		}

		ui->statusBar->showMessage("Data received: " + dataVisualisation);*/

		// Ok, now we will use our signals vector to find every signal
		// in received data array
		//

		for (SignalData signal : m_signalsFromXml)
		{

			// Read data for every signal, and transform it to
			// selected value (signedInt, unsignedInt, float)
			//

			// As you can see, we read signal position (signal byte and offset) from vector,
			// and by using this information, searching signal value in received data
			//

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

			// Transform selected binary value to
			// our result (only for unsigned int)
			//

			bool falseValue = false;
			qint64 result = valueString.toInt(&falseValue, 2);

			QString resultString;

			if (signal.dataFormat == "UnsignedInt")
			{
				resultString = QString::number(result);
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
				// Processing received values according to IEEE 754 standard for floating point transaction
				//

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
			}

			// After all processing actions, we have result, that wrote in resultString
			//

			//ui->signalsTable->setItem(rowNumber, value, new QTableWidgetItem(resultString));
			values.at(rowNumber)->setText(resultString);
			//ui->signalsTable->setItem(rowNumber, value, values.at(rowNumber));
			rowNumber++;
		}

	}

	// If our package corrupted - show message
	//

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

void SerialDataTester::crcError(QString version, QString trId, QString numerator, QByteArray dataId)
{
	// This function will be called when crc error has been happen.
	//

	// Reset timer
	//

	receiveTimeout->stop();
	receiveTimeout->start(5000);

	// Write packet info to GUI information panel
	//

	ui->version->setText(version);
	ui->signature->setText("424D4C47");
	ui->transmissionId->setText(trId);
	ui->numerator->setText(numerator);
	ui->dataUniqueId->setText(dataId);

	ui->crc->setText("ERR");
	ui->statusBar->showMessage("Crc error!");

	// Count corrupted packets
	//

	ui->corruptedPackets->setText(QString::number(ui->corruptedPackets->text().toInt()+1));
	ui->totalPackets->setText(QString::number(ui->totalPackets->text().toInt() + 1));
}

void SerialDataTester::signalTimeout()
{
	for (int currentRow = 0; currentRow < ui->signalsTable->rowCount(); currentRow++)
	{
		ui->signalsTable->setItem(currentRow, value, new QTableWidgetItem(QString::number(0)));
	}

	ui->signature->setText("No data");
	ui->version->setText("No data");
	ui->transmissionId->setText("No data");
	ui->numerator->setText("No data");
	ui->dataUniqueId->setText("No data");
	ui->crc->setText("No data");

	ui->statusBar->showMessage("Signal timeout!");
}

void SerialDataTester::erasePacketData()
{
	// This function will reset all packet data
	//

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
	// This part just loads old used configuration from program settings
	//

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

	// This function will open port and start receiver
	//

	bool errors = false;

	m_portReceiver->openPort();

	if (ui->portName->text() == "Error")
	{
		QMessageBox::critical(this, tr("Start receiver"), tr("No port specified"));
		errors = true;
	}

	if (ui->baudRate->text() == "Error")
	{
		QMessageBox::critical(this, tr("Start receiver"), tr("Wrong baud set!"));
		errors = true;
	}

	if (ui->bits->text() == "Error")
	{
		QMessageBox::critical(this, tr("Start receiver"), tr("Wrong data bits set!"));
		errors = true;
	}

	if (ui->stopBits->text() == "Error")
	{
		QMessageBox::critical(this, tr("Start receiver"), tr("Wrong stop bits set!"));
		errors = true;
	}

	if (ui->portStatus->text() == "Opened")
	{
		QMessageBox::information(this, tr("Start receiver"), tr("Port already opened!"));
	}

	if (ui->signalsTable->rowCount() == 0)
	{
		QMessageBox::critical(this, tr("Start receiver"), tr("No signals detected! Read signals first!"));
		errors = true;
	}

	if (errors == false)
	{
		m_portReceiver->openPort();
		ui->portStatus->setText("Opened");
	}
}

void SerialDataTester::stopReceiver()
{
	m_portReceiver->closePort();
	ui->portStatus->setText("Closed");
}

QBitArray SerialDataTester::bytesToBits(QByteArray bytes)
{
	QBitArray bits(bytes.count()*8);

	// Convert from QByteArray to QBitArray
	//

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
	//

	for(int b=0; b<bits.count(); ++b)
		bytes[b/8] = ( bytes.at(b/8) | ((bits[b]?1:0)<<(b%8)));
	return bytes;
}
