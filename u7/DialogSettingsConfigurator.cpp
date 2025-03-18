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

	AppSettings appSettings;
	appSettings.load();

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

			if (port == appSettings.configuratorSerialPort())
			{
				serialPortFound = true;
				ui->serialPortCombo->setCurrentText(port);
			}
		}

		if (serialPortFound == false)
		{
			ui->serialPortCombo->setCurrentIndex(0);
			appSettings.configuratorSerialPort() = ports[0].systemLocation();
		}
	}

	ui->showDebugInfo->setChecked(appSettings.configuratorShowDebugInfo());
	ui->verifyData->setChecked(appSettings.configuratorVerify());

	return;
}

DialogSettingsConfigurator::~DialogSettingsConfigurator()
{
	delete ui;
}

void DialogSettingsConfigurator::on_DialogSettingsConfigurator_accepted()
{
	AppSettings appSettings;
	appSettings.load();

	appSettings.setConfiguratorSerialPort(ui->serialPortCombo->currentText());
	appSettings.setConfiguratorShowDebugInfo((ui->showDebugInfo->checkState() == Qt::Checked));
	appSettings.setConfiguratorVerify((ui->verifyData->checkState() == Qt::Checked));

	appSettings.save();
}
