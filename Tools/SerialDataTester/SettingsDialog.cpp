#include "settingsdialog.h"
#include "ui_settingsdialog.h"
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
	QSettings settings;

	settings.setValue("pathToSignals", ui->pathToSignalsXmlFile->text());
	settings.setValue("port", ui->comPortSelectionBox->currentText());
	settings.setValue("baud", ui->portBaudBox->currentText());

	this->close();
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
