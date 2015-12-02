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

	m_file->addAction(m_reloadCfg);
	m_file->addAction(m_changeSignalSettingsFile);
	m_file->addSeparator();
	m_file->addAction(m_exit);

	// Menu "Settings"
	//

	m_settings = new QMenu(tr("&Settings"));
	m_setPort = new QMenu(tr("Set Port"));

	for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
	{
		m_setPort->addAction(port.portName());
	}

	for (QAction* port : m_setPort->actions())
	{
		port->setCheckable(true);
	}

	m_setBaud = new QMenu(tr("Set Baud"));

	m_setBaud->addAction(QString::number(QSerialPort::Baud115200));
	m_setBaud->addAction(QString::number(QSerialPort::Baud57600));
	m_setBaud->addAction(QString::number(QSerialPort::Baud38400));
	m_setBaud->addAction(QString::number(QSerialPort::Baud19200));

	m_settings->addMenu(m_setPort);
	m_settings->addMenu(m_setBaud);

	ui->menuBar->addMenu(m_file);
	ui->menuBar->addMenu(m_settings);

	ui->signalsTable->setColumnCount(6);

	ui->signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(caption, new QTableWidgetItem(tr("Caption")));
	ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));
	ui->signalsTable->setHorizontalHeaderItem(value, new QTableWidgetItem(tr("Value")));

	// Try to load application settings. If there is no port
	// in settings, call application settings dialog and reconfigure
	// settings
	//

	m_applicationSettingsDialog = new SettingsDialog();

	QSettings applicationSettings;
	if (applicationSettings.value("port").isNull())
	{
		m_applicationSettingsDialog->exec();
	}

	delete m_applicationSettingsDialog;
	m_applicationSettingsDialog = nullptr;

	// Read stored application settings
	//

	QString portName = applicationSettings.value("port").toString();
	int portBaud = applicationSettings.value("baud").toInt();

	ui->portNameLabel->setText(portName);
	ui->baudRateLabel->setText(QString::number(portBaud));

	for (QAction* port : m_setPort->actions())
	{
		if (portName == port->text())
		{
			port->setChecked(true);
		}
	}

	for (QAction* baud : m_setBaud->actions())
	{
		baud->setCheckable(true);
		if (QString::number(portBaud) == baud->text())
		{
			baud->setChecked(true);
		}
	}

	m_portReceiver = new PortReceiver();

	m_pathToSignalsXml = applicationSettings.value("pathToSignals").toString();

	// Receiver timer
	//

	receiveTimeout = new QTimer();

	// Read xml file
	//

	parseFile();

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
	connect(m_portReceiver, &PortReceiver::portError, this, &SerialDataTester::portError);
	connect(this, &SerialDataTester::portChanged, m_portReceiver, &PortReceiver::setNewPort);
	connect(m_portReceiver, &PortReceiver::dataFromPort, this, &SerialDataTester::dataReceived);
	connect(receiveTimeout, &QTimer::timeout, this, &SerialDataTester::signalTimeout);

	// Start port receiver in another
	// thread
	//

	m_portReceiver->openPort();

	m_PortThread = new QThread(this);
	m_portReceiver->moveToThread(m_PortThread);

	m_PortThread->start();
}

SerialDataTester::~SerialDataTester()
{
	delete ui;
	m_PortThread->terminate();
	delete m_PortThread;
}

void SerialDataTester::parseFile()
{
	// Try to open signals xml to read signals
	//

	QFile slgnalsXmlFile(m_pathToSignalsXml);
	bool errorLoadingXml = false;

	if (slgnalsXmlFile.exists() == false)
	{
		QMessageBox::critical(this, tr("Critical error"), "File not found: " + m_pathToSignalsXml);
		ui->statusBar->showMessage("Error loading " + m_pathToSignalsXml);
		errorLoadingXml = true;
	}

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
		ui->statusBar->showMessage("Error loading " + m_pathToSignalsXml);
		m_signalsFromXml.clear();
	}
	else
	{
		ui->statusBar->showMessage(m_pathToSignalsXml + " has been succsessfully loaded");
	}

	slgnalsXmlFile.close();

	// Test data from xml-file
	// DEBUG ONLY
	//

	int numberOfSignalFromVector = 0;

	for (SignalData& signalData : m_signalsFromXml)
	{
		ui->signalsTable->setRowCount(numberOfSignalFromVector + 1);
		ui->signalsTable->setItem(numberOfSignalFromVector, strId, new QTableWidgetItem(signalData.strId));
		ui->signalsTable->setItem(numberOfSignalFromVector, caption, new QTableWidgetItem(signalData.caption));
		ui->signalsTable->setItem(numberOfSignalFromVector, offset, new QTableWidgetItem(QString::number(signalData.offset)));
		ui->signalsTable->setItem(numberOfSignalFromVector, bit, new QTableWidgetItem(QString::number(signalData.bit)));
		ui->signalsTable->setItem(numberOfSignalFromVector, type, new QTableWidgetItem(signalData.type));
		ui->signalsTable->setItem(numberOfSignalFromVector, value, new QTableWidgetItem(QString::number(0)));

		numberOfSignalFromVector++;
	}
}

void SerialDataTester::reloadConfig()
{
	ui->signalsTable->clear();

	ui->signalsTable->setRowCount(0);

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(caption, new QTableWidgetItem(tr("Caption")));
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
	ui->portNameLabel->setText(newPort->text());
}

void SerialDataTester::setBaud(QAction* newBaud)
{
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
	ui->baudRateLabel->setText(newBaud->text());
}

void SerialDataTester::portError(QString error)
{
	QMessageBox::critical(this, tr("Critical error"), error);
	ui->portNameLabel->setText("Error");
}

void SerialDataTester::dataReceived(QByteArray data)
{
	receiveTimeout->stop();
	receiveTimeout->start(5000);
	QDataStream packet(&data, QIODevice::ReadOnly);

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

	quint8 m_offset = 0x00;
	quint8 m_bit = 0x00;
	qint16 m_value = 0x0000;

	for (int currentSignal = 0; currentSignal < amount / 4; currentSignal++)
	{
		packet >> m_offset;
		packet >> m_bit;
		packet >> m_value;
	}

	QString dataForCrc = QString::number(m_offset);
	dataForCrc.append(QString::number(m_bit));
	dataForCrc.append(QString::number(m_value));

	char *dataForCrcPtr = dataForCrc.toLocal8Bit().data();

	quint64 crc = 0;
	for (int i=0; i<dataForCrc.size(); i++)
	{
		crc = m_crc_table[(crc ^ (dataForCrcPtr[i])) & 0xFF] ^ (crc >> 8);
	}
	crc = ~crc;

	quint64 packetCrc;
	packet >> packetCrc;

	if (crc != packetCrc)
	{
		ui->crc->setText("ERROR");
		ui->corruptedPackets->setText(QString::number(ui->corruptedPackets->text().toInt() + 1));
		ui->statusBar->showMessage("Corrupted packet (CRC error)");
	}
	else
	{
		ui->crc->setText("OK");
		if (signature != m_signature)
		{
			ui->corruptedPackets->setText(QString::number(ui->corruptedPackets->text().toInt() + 1));
			ui->statusBar->showMessage("Unknown signature");
		}
		else
		{
			for (int currentSignal = 0; currentSignal < amount / 4; currentSignal++)
			{
				bool signalExists = false;
				SignalData receivedSignal;
				int rowNumber = 0;
				int itemNumber = 0;

				for (SignalData signalFromXml : m_signalsFromXml)
				{
					if (signalFromXml.offset == m_offset && signalFromXml.bit == m_bit)
					{
						signalExists = true;
						receivedSignal = signalFromXml;
						rowNumber = itemNumber;
					}
					itemNumber++;
				}

				if (signalExists)
				{
					ui->signalsTable->setItem(rowNumber, value, new QTableWidgetItem(QString::number(m_value)));
					ui->processedPackets->setText(QString::number(ui->processedPackets->text().toInt() + 1));
				}
				else
				{
					ui->corruptedPackets->setText(QString::number(ui->corruptedPackets->text().toInt() + 1));
				}
			}
		}
	}
	ui->totalPackets->setText(QString::number(ui->totalPackets->text().toInt() + 1));
	//ui->caption->setText(dataFromPacket);
}

void SerialDataTester::signalTimeout()
{
	for (int currentRow = 0; currentRow < ui->signalsTable->rowCount(); currentRow++)
	{
		ui->signalsTable->setItem(currentRow, value, new QTableWidgetItem(QString::number(0)));
	}
	ui->statusBar->showMessage("Signal timeout!");
}
