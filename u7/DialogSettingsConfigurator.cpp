#include "DialogSettingsConfigurator.h"
#include "AppSettings.h"
#include "ui_DialogSettingsConfigurator.h"


DialogSettingsConfigurator::DialogSettingsConfigurator(QWidget* parent) :
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

			if (port == theAppSettings.configuratorSerialPort())
			{
				serialPortFound = true;
				ui->serialPortCombo->setCurrentText(port);
			}
		}

		if (serialPortFound == false)
		{
			ui->serialPortCombo->setCurrentIndex(0);
			theAppSettings.configuratorSerialPort() = ports[0].systemLocation();
		}
	}

	ui->showDebugInfo->setChecked(theAppSettings.configuratorShowDebugInfo());
	ui->verifyData->setChecked(theAppSettings.configuratorVerify());

	return;
}

DialogSettingsConfigurator::~DialogSettingsConfigurator()
{
	delete ui;
}

void DialogSettingsConfigurator::on_DialogSettingsConfigurator_accepted()
{
	theAppSettings.setConfiguratorSerialPort(ui->serialPortCombo->currentText());
	theAppSettings.setConfiguratorShowDebugInfo((ui->showDebugInfo->checkState() == Qt::Checked));
	theAppSettings.setConfiguratorVerify((ui->verifyData->checkState() == Qt::Checked));

	theAppSettings.save();
}
