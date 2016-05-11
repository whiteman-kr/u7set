#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QList>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
	{
		ui->comPortSelectionBox->addItem(port.portName());
	}

	/*for (quint32 baudRate : QSerialPortInfo::standardBaudRates())
	{
		ui->portBaudBox->addItem(QString::number(baudRate));
	}*/

	ui->portBaudBox->addItem(QString::number(QSerialPort::Baud115200));
	ui->portBaudBox->addItem(QString::number(QSerialPort::Baud57600));
	ui->portBaudBox->addItem(QString::number(QSerialPort::Baud38400));
	ui->portBaudBox->addItem(QString::number(QSerialPort::Baud19200));

	connect(ui->browseButton, &QPushButton::clicked, this, &SettingsDialog::browseSignalsXmlFile);
	connect(ui->okButton, &QPushButton::clicked, this, &SettingsDialog::settingsConfirmed);
	connect(ui->cancelButton, &QPushButton::clicked, this, &SettingsDialog::settingsCanceled);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::settingsConfirmed()
{	
	if (ui->pathToSignalsXmlFile->text().isEmpty())
	{
		QMessageBox::warning(this, tr("Serial Data Tester"), tr("XML-file path is not set!"));
	}
	{
		QSettings settings;

		settings.setValue("pathToSignals", ui->pathToSignalsXmlFile->text());
		settings.setValue("port", ui->comPortSelectionBox->currentText());
		settings.setValue("baud", ui->portBaudBox->currentText());

		this->close();
	}
}

void SettingsDialog::settingsCanceled()
{
	this->close();
}

void SettingsDialog::browseSignalsXmlFile()
{
	m_pathToSignalsXml = QFileDialog::getOpenFileName(this, tr("Open Signals.xml"), "", tr("XML-file (*.xml)"));
	ui->pathToSignalsXmlFile->setText(m_pathToSignalsXml);
}
