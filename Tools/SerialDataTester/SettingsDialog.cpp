#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);

	for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
	{
		ui->comboBox->addItem(port.portName());
	}

	ui->comboBox_2->addItem("115200");
	ui->comboBox_2->addItem("57600");
	ui->comboBox_2->addItem("38400");
	ui->comboBox_2->addItem("19200");

	connect (ui->pushButton, &QPushButton::clicked, this, &SettingsDialog::browseSignalsXmlFile);
	connect (ui->pushButton_3, &QPushButton::clicked, this, &SettingsDialog::settingsConfirmed);
	connect (ui->pushButton_2, &QPushButton::clicked, this, &SettingsDialog::settingsCanceled);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::settingsConfirmed()
{	
	QFile settingsFile("settings.conf");
	QTextStream writeDataInFile(&settingsFile);

	if (settingsFile.open(QIODevice::WriteOnly|QIODevice::Text))
	{
		writeDataInFile << ui->lineEdit->text() << "\n";
		writeDataInFile << ui->comboBox->currentText() << "\n";
		writeDataInFile << ui->comboBox_2->currentText();
		settingsFile.close();
		this->close();
		emit sendSettingsCreated();
	}
	else
	{
		QMessageBox::critical(this, tr("Critical error"), tr("Can not write data to settings.conf"));
	}
}

void SettingsDialog::settingsCanceled()
{
	this->close();
}

void SettingsDialog::browseSignalsXmlFile()
{
	m_pathToSignalsXml = QFileDialog::getOpenFileName(this, tr("Open Signals.xml"), "", tr("XML-file (*.xml)"));
	ui->lineEdit->setText(m_pathToSignalsXml);
}
