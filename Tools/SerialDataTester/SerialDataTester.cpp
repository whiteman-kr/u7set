#include "serialdatatester.h"
#include "ui_serialdatatester.h"
#include <QMessageBox>
#include <QXmlStreamReader>

SerialDataTester::SerialDataTester(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SerialDataTester)
{
	ui->setupUi(this);

	/*m_file = new QMenu(tr("&File"));
	m_reloadCfg = new QAction(tr("Load config file"), this);*/

	m_settingsMenu = new QMenu(tr("Settings"));
	m_setComPort = new QAction(tr("&Change port"), this);

	m_setPortBaud = new QMenu(tr("&Change Baud"), this);
	m_baud115200 = new QAction(QIcon(":/resources/images/check.png"), tr("115200bps"), this);
	m_baud57600 = new QAction(tr("57600bps"), this);
	m_baud38400 = new QAction(tr("38400bps"), this);
	m_baud19200 = new QAction(tr("19200bps"), this);

	//m_file->addAction(m_reloadCfg);

	m_settingsMenu->addAction(m_setComPort);
	m_settingsMenu->addMenu(m_setPortBaud);

	m_setPortBaud->addAction(m_baud115200);
	m_setPortBaud->addAction(m_baud57600);
	m_setPortBaud->addAction(m_baud38400);
	m_setPortBaud->addAction(m_baud19200);

	//ui->menuBar->addMenu(m_file);
	ui->menuBar->addMenu(m_settingsMenu);

	ui->tableWidget->setColumnCount(5);

	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	parseFile();

	//connect(m_reloadCfg, &QAction::triggered, this, &SerialDataTester::reloadConfig);

}

SerialDataTester::~SerialDataTester()
{
	delete ui;
}

void SerialDataTester::parseFile()
{	
	ui->tableWidget->setHorizontalHeaderItem(tableColumns.strId, new QTableWidgetItem("StrID"));
	ui->tableWidget->setHorizontalHeaderItem(tableColumns.caption, new QTableWidgetItem("Caption"));
	ui->tableWidget->setHorizontalHeaderItem(tableColumns.offset, new QTableWidgetItem("Offset"));
	ui->tableWidget->setHorizontalHeaderItem(tableColumns.bit, new QTableWidgetItem("Bit"));
	ui->tableWidget->setHorizontalHeaderItem(tableColumns.type, new QTableWidgetItem("Type"));

	// Try to open configuration file to read signals
	//

	QFile* file = new QFile(":/resources/config.xml");

	if (file->exists() == false)
	{
		QMessageBox::critical(this, tr("Critical error"), tr("File not found: config.xml"));
		ui->statusBar->showMessage("Error loading config.xml");
	}

	if (file->open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		QMessageBox::critical(this, tr("Critical error"), tr("Error opening config.xml"));
		ui->statusBar->showMessage("Error loading config.xml");
	}

	// Ok, now start processing XML-file and write all signals
	// from file to vector
	//

	QXmlStreamReader xmlReader(file);
	SignalData currentSignal;
	bool errorLoadingXml = false;

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
				if(attributes.hasAttribute("STRID"))
				{
					currentSignal.strId  = attributes.value("STRID").toString();
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
		ui->statusBar->showMessage("Error loading config.xml");
		signalsFromXml.clear();
	}
	else
	{
		ui->statusBar->showMessage("config.xml has been succsessfully loaded");
	}

	// Test data from xml-file
	// DEBUG ONLY
	//

	/*numberOfSignalFromVector = 0;

	for (SignalData signalData : signalsFromXml)
	{
		ui->tableWidget->setRowCount(numberOfSignalFromVector + 1);
		ui->tableWidget->setItem(numberOfSignalFromVector, tableColumns.strId, new QTableWidgetItem(signalData.strId));
		ui->tableWidget->setItem(numberOfSignalFromVector, tableColumns.caption, new QTableWidgetItem(signalData.caption));
		ui->tableWidget->setItem(numberOfSignalFromVector, tableColumns.offset, new QTableWidgetItem(QString::number(signalData.offset)));
		ui->tableWidget->setItem(numberOfSignalFromVector, tableColumns.bit, new QTableWidgetItem(QString::number(signalData.bit)));
		ui->tableWidget->setItem(numberOfSignalFromVector, tableColumns.type, new QTableWidgetItem(signalData.type));

		numberOfSignalFromVector++;
	}*/
}

/*void SerialDataTester::reloadConfig()
{
	ui->tableWidget->clear();
	signalsFromXml.clear();
	parseFile();
}*/
