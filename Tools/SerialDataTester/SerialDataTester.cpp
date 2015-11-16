#include "serialdatatester.h"
#include "ui_serialdatatester.h"
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QSerialPortInfo>

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

	ui->signalsTable->setColumnCount(5);

	ui->signalsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(caption, new QTableWidgetItem(tr("Caption")));
	ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));

	// Try to load application settings
	//

	m_applicationSettingsDialog = new SettingsDialog();

	QSettings applicationSettings;
	if (applicationSettings.value("port").isNull())
	{
		m_applicationSettingsDialog->exec();
	}

	delete m_applicationSettingsDialog;
	m_applicationSettingsDialog = nullptr;

	// Read application settings from application settings
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

	m_portReceiver = new PortReceiver(this);

	m_pathToSignalsXml = applicationSettings.value("pathToSignals").toString();

	// Create port connection
	//

	// Now read
	//

	connect(m_reloadCfg, &QAction::triggered, this, &SerialDataTester::reloadConfig);
	connect(m_changeSignalSettingsFile, &QAction::triggered, this, &SerialDataTester::selectNewSignalsFile);
	connect(m_exit, &QAction::triggered, this, &SerialDataTester::applicationExit);
	connect(m_setPort, &QMenu::triggered, this, &SerialDataTester::setPort);
	connect(m_setBaud, &QMenu::triggered, this, &SerialDataTester::setBaud);
	connect(m_portReceiver, &PortReceiver::portError, this, &SerialDataTester::portError);
	connect(this, &SerialDataTester::portChanged, m_portReceiver, &PortReceiver::setNewPort);

	parseFile();
	m_portReceiver->openPort();
}

SerialDataTester::~SerialDataTester()
{
	delete ui;
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

				signalsFromXml.push_back(currentSignal);
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
		signalsFromXml.clear();
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

	for (SignalData& signalData : signalsFromXml)
	{
		ui->signalsTable->setRowCount(numberOfSignalFromVector + 1);
		ui->signalsTable->setItem(numberOfSignalFromVector, strId, new QTableWidgetItem(signalData.strId));
		ui->signalsTable->setItem(numberOfSignalFromVector, caption, new QTableWidgetItem(signalData.caption));
		ui->signalsTable->setItem(numberOfSignalFromVector, offset, new QTableWidgetItem(QString::number(signalData.offset)));
		ui->signalsTable->setItem(numberOfSignalFromVector, bit, new QTableWidgetItem(QString::number(signalData.bit)));
		ui->signalsTable->setItem(numberOfSignalFromVector, type, new QTableWidgetItem(signalData.type));

		numberOfSignalFromVector++;
	}
}

void SerialDataTester::reloadConfig()
{
	ui->signalsTable->clear();

	ui->signalsTable->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->signalsTable->setHorizontalHeaderItem(caption, new QTableWidgetItem(tr("Caption")));
	ui->signalsTable->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->signalsTable->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->signalsTable->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));

	signalsFromXml.clear();
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
