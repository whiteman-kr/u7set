#include "serialdatatester.h"
#include "ui_serialdatatester.h"
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QDebug>

SerialDataTester::SerialDataTester(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SerialDataTester)
{
	ui->setupUi(this);

	m_file = new QMenu(tr("&File"));
	m_reloadCfg = new QAction(tr("Reload singals xml file"), this);

	m_file->addAction(m_reloadCfg);

	ui->menuBar->addMenu(m_file);

	ui->tableWidget->setColumnCount(5);

	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui->tableWidget->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->tableWidget->setHorizontalHeaderItem(caption, new QTableWidgetItem(tr("Caption")));
	ui->tableWidget->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->tableWidget->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->tableWidget->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));

	// Try to load application settings
	//

	m_applicationSettingsDialog = new SettingsDialog();

	QFile settingsFile("settings.conf");

	if (settingsFile.open(QIODevice::ReadOnly) == false)
	{
		m_applicationSettingsDialog->exec();
	}
	else
	{
		// If settings.conf exist, start parsing file.
		// Othervise, wait until SettingsDialog window
		// send signal, that file where created

		parseFile();
	}

	connect(m_reloadCfg, &QAction::triggered, this, &SerialDataTester::reloadConfig);
	connect(m_applicationSettingsDialog, &SettingsDialog::sendSettingsCreated, this, &SerialDataTester::parseFile);
}

SerialDataTester::~SerialDataTester()
{
	delete ui;
}

void SerialDataTester::parseFile()
{	
	QFile settingsFile("settings.conf");
	QTextStream readSettings(&settingsFile);
	QString pathToSignalsXml;

	// Open application settings file to read
	// path to signals xml file
	//

	if (settingsFile.open(QIODevice::ReadOnly) == false)
	{
		QMessageBox::critical(this, tr("Critical error"), tr("Can not open settings.conf"));
		pathToSignalsXml = "Error";
	}
	else
	{
		pathToSignalsXml = readSettings.readLine();
	}

	// Try to open signals xml to read signals
	//

	QFile* slgnalsXmlFile = new QFile(pathToSignalsXml);
	bool errorLoadingXml = false;

	if (slgnalsXmlFile->exists() == false)
	{
		QMessageBox::critical(this, tr("Critical error"), tr("File not found: config.xml"));
		ui->statusBar->showMessage(tr("Error loading config.xml"));
		errorLoadingXml = true;
	}

	if (slgnalsXmlFile->open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		QMessageBox::critical(this, tr("Critical error"), tr("Error opening config.xml"));
		ui->statusBar->showMessage(tr("Error loading config.xml"));
		errorLoadingXml = true;
	}

	// Ok, now start processing XML-file and write all signals
	// from file to vector
	//

	QXmlStreamReader xmlReader(slgnalsXmlFile);
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
		ui->statusBar->showMessage( tr("Error loading config.xml"));
		signalsFromXml.clear();
	}
	else
	{
		ui->statusBar->showMessage( tr("config.xml has been succsessfully loaded"));
	}

	// Test data from xml-file
	// DEBUG ONLY
	//

	int numberOfSignalFromVector = 0;

	for (SignalData& signalData : signalsFromXml)
	{
		ui->tableWidget->setRowCount(numberOfSignalFromVector + 1);
		ui->tableWidget->setItem(numberOfSignalFromVector, strId, new QTableWidgetItem(signalData.strId));
		ui->tableWidget->setItem(numberOfSignalFromVector, caption, new QTableWidgetItem(signalData.caption));
		ui->tableWidget->setItem(numberOfSignalFromVector, offset, new QTableWidgetItem(QString::number(signalData.offset)));
		ui->tableWidget->setItem(numberOfSignalFromVector, bit, new QTableWidgetItem(QString::number(signalData.bit)));
		ui->tableWidget->setItem(numberOfSignalFromVector, type, new QTableWidgetItem(signalData.type));

		numberOfSignalFromVector++;
	}
}

void SerialDataTester::reloadConfig()
{
	ui->tableWidget->clear();

	ui->tableWidget->setHorizontalHeaderItem(strId, new QTableWidgetItem(tr("StrID")));
	ui->tableWidget->setHorizontalHeaderItem(caption, new QTableWidgetItem(tr("Caption")));
	ui->tableWidget->setHorizontalHeaderItem(offset, new QTableWidgetItem(tr("Offset")));
	ui->tableWidget->setHorizontalHeaderItem(bit, new QTableWidgetItem(tr("Bit")));
	ui->tableWidget->setHorizontalHeaderItem(type, new QTableWidgetItem(tr("Type")));

	signalsFromXml.clear();
	parseFile();
}
