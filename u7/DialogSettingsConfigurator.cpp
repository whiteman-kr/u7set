#include "DialogSettingsConfigurator.h"
#include "ui_DialogSettingsConfigurator.h"
#include "Settings.h"
#include <QSerialPortInfo>

DialogSettingsConfigurator::DialogSettingsConfigurator(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSettingsConfigurator)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
	setSizeGripEnabled(true);

	// Enumerate all com ports
	//
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

	for (const QSerialPortInfo& pi : ports)
	{
		qDebug() << "Port";
		qDebug() << pi.description();
		qDebug() << pi.manufacturer();
		qDebug() << pi.portName();
		qDebug() << pi.serialNumber();
		qDebug() << pi.systemLocation();
	}

	// ComPort
	//
	if (ports.size() != 0)
	{

		bool serialPortFound = false;

		for (const QSerialPortInfo& pi : ports)
		{
			QString port = pi.systemLocation();
			ui->serialPortCombo->addItem(port);

			if (port == theSettings.m_configuratorSerialPort)
			{
				serialPortFound = true;
				ui->serialPortCombo->setCurrentText(port);
			}
		}

		if (serialPortFound == false)
		{
			ui->serialPortCombo->setCurrentIndex(0);
			theSettings.m_configuratorSerialPort = ports[0].systemLocation();
		}
	}

	ui->showDebugInfo->setChecked(theSettings.m_configuratorShowDebugInfo);
	ui->verifyData->setChecked(theSettings.m_configuratorVerify);


}

DialogSettingsConfigurator::~DialogSettingsConfigurator()
{
	delete ui;
}

void DialogSettingsConfigurator::on_DialogSettingsConfigurator_accepted()
{
	theSettings.m_configuratorSerialPort = ui->serialPortCombo->currentText();
	theSettings.m_configuratorShowDebugInfo = (ui->showDebugInfo->checkState() == Qt::Checked);
	theSettings.m_configuratorVerify = (ui->verifyData->checkState() == Qt::Checked);

	theSettings.writeSystemScope();

}
